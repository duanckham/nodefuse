// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "bindings.h"

namespace NodeFuse {
	Persistent<FunctionTemplate> Fuse::constructor_template;

	// static Persistent<String> mountpoint_sym;
	static Persistent<String> filesystem_sym;
	static Persistent<String> options_sym;
	
	static Persistent<Object> CI;

	void Fuse::Initialize(Handle<Object> target) {
		Local<FunctionTemplate> t = FunctionTemplate::New(Fuse::New);

		t->InstanceTemplate()->SetInternalFieldCount(1);

		NODE_SET_PROTOTYPE_METHOD(t, "mount", Fuse::Mount);
		// NODE_SET_PROTOTYPE_METHOD(t, "unmount", Fuse::Unmount);

		constructor_template = Persistent<FunctionTemplate>::New(t);
		constructor_template->SetClassName(String::NewSymbol("Fuse"));

		target->Set(String::NewSymbol("fuse_version"), Integer::New(fuse_version()));
		target->Set(String::NewSymbol("Fuse"), constructor_template->GetFunction());

		// mountpoint_sym        = NODE_PSYMBOL("mountpoint");
		filesystem_sym        = NODE_PSYMBOL("filesystem");
		options_sym           = NODE_PSYMBOL("options");
	}

	Fuse::Fuse() : ObjectWrap() {}
	Fuse::~Fuse() {
		if (fargs != NULL) {
			fuse_opt_free_args(fargs);
		}

		if (session != NULL) {
			fuse_remove_signal_handlers(session);
		}

		if (channel != NULL) {
			fuse_unmount(mountpoint, channel);
			fuse_session_remove_chan(channel);
		}

		if (mountpoint != NULL) {
			free(mountpoint);
		}
	}

	Handle<Value> Fuse::New(const Arguments &args) {
		HandleScope scope;

		Fuse *fuse = new Fuse();
		Local<Object> obj = args.This();
		fuse->Wrap(obj);

		return obj;
	}

	void Fuse::AsyncWorker(void *args) {
		Baton *baton = reinterpret_cast<Baton *>(args);
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(baton->currentInstance);

		fuse->fargs = &baton->fargs;
		fuse->fsobj = baton->fsobj;

		int ret = fuse_parse_cmdline(fuse->fargs, &fuse->mountpoint, &fuse->multithreaded, &fuse->foreground);

		if (ret == -1) {
			FUSEJS_THROW_EXCEPTION("Error parsing fuse options: ", strerror(errno));
			return;
		}

		fuse->channel = fuse_mount((const char *) fuse->mountpoint, fuse->fargs);

		if (fuse->channel == NULL) {
			FUSEJS_THROW_EXCEPTION("Unable to mount filesystem: ", strerror(errno));
			return;
		}

		struct fuse_lowlevel_ops *operations = FileSystem::GetOperations();

		Userdata *userdata = new Userdata();
		userdata->fuse = fuse;
		userdata->async = &baton->s_async;

		fuse->session = fuse_lowlevel_new(fuse->fargs, operations, sizeof(*operations), userdata);

		if (fuse->session == NULL) {
			fuse_unmount(fuse->mountpoint, fuse->channel);
			fuse_opt_free_args(fuse->fargs);
			FUSEJS_THROW_EXCEPTION("Error creating fuse session: ", strerror(errno));
			return;
		}

		// ret = fuse_set_signal_handlers(fuse->session);
		if (fuse_set_signal_handlers(fuse->session) == -1) {
			fuse_session_destroy(fuse->session);
			fuse_unmount(fuse->mountpoint, fuse->channel);
			fuse_opt_free_args(fuse->fargs);
			FUSEJS_THROW_EXCEPTION("Error setting fuse signal handlers: ", strerror(errno));
			return;
		}

		fuse_session_add_chan(fuse->session, fuse->channel);
		fuse_session_loop(fuse->session);

		// Continues executing if user unmounts the fs
		fuse_remove_signal_handlers(fuse->session);

		// fuse_unmount(fuse->mountpoint, fuse->channel);
		fuse_session_remove_chan(fuse->channel);
		fuse_session_destroy(fuse->session);
		fuse_unmount(fuse->mountpoint, fuse->channel);
		fuse_opt_free_args(fuse->fargs);
	}

	void Fuse::AsyncCallback(uv_async_t *handle, int status) {
		HandleScope scope;
		
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(handle->data);

		Proxy::Call(argument->op, CI, handle->data);
	}

	Handle<Value> Fuse::Mount(const Arguments &args) {
		HandleScope scope;

		Local<Object> argsObj = args[0]->ToObject();
		Local<Value> vfilesystem = argsObj->Get(filesystem_sym);
		Local<Value> voptions = argsObj->Get(options_sym);

		Local<Function> filesystem = Local<Function>::New(Local<Function>::Cast(vfilesystem));
		Local<Array> options = Local<Array>::New(Local<Array>::Cast(voptions));
		Local<Object> currentInstance = Local<Object>::New(args.This());

		struct fuse_args fargs = FUSE_ARGS_INIT(0, NULL);

		Baton *baton = new Baton();

		baton->s_async.data = baton;
		baton->argc = options->Length();
		baton->fargs = fargs;
		baton->currentInstance = Persistent<Object>::New(args.This());

		CI = Persistent<Object>::New(args.This());

		if (baton->argc < 3) {
			options->Set(Integer::New(2), String::New("--help"));
			baton->argc++;
		}

		for (int i = 1; i < baton->argc; i++) {
			String::Utf8Value _option(options->Get(Integer::New(i))->ToString());
			const char *option = (const char *) *_option;

			if (fuse_opt_add_arg(&baton->fargs, option) == -1) {
				FUSEJS_THROW_EXCEPTION("Unable to allocate memory, fuse_opt_add_arg failed: ", strerror(errno));
				return Undefined();
			}
		}

		Local<Value> argv[2] = { currentInstance, options };
		baton->fsobj = Persistent<Object>::New(filesystem->NewInstance(2, argv));

		uv_async_init(uv_default_loop(), &baton->s_async, Fuse::AsyncCallback);
		// uv_async_send(&baton->s_async);

		uv_thread_create(&baton->worker_thread, Fuse::AsyncWorker, baton);

		//create thread
		//using the thread to do the fibo job
		//uv_thread_join(&job_ptr->worker_thread);

		return scope.Close(Undefined());
	}

	// Handle<Value> Fuse::Unmount(const Arguments& args) {
	//     HandleScope scope;

	//     Local<Object> currentInstance = args.This();
	//     Fuse *fuse = ObjectWrap::Unwrap<Fuse>(currentInstance);

	//     fuse_session_remove_chan(fuse->channel);
	//     fuse_remove_signal_handlers(fuse->session);
	//     fuse_session_destroy(fuse->session);
	//     fuse_unmount(fuse->mountpoint, fuse->channel);

	//     return currentInstance;
	// }
} //namespace NodeFuse

