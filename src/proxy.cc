#include "filesystem.h"
#include "reply.h"
#include "file_info.h"
#include "bindings.h"
#include "node_buffer.h"
#include "event.h"

namespace NodeFuse {

	// Symbols for FUSE operations
	FUSE_SYM(init);
	FUSE_SYM(destroy);
	FUSE_SYM(lookup);
	FUSE_SYM(forget);
	FUSE_SYM(getattr);
	FUSE_SYM(setattr);
	FUSE_SYM(readlink);
	FUSE_SYM(mknod);
	FUSE_SYM(mkdir);
	FUSE_SYM(unlink);
	FUSE_SYM(rmdir);
	FUSE_SYM(symlink);
	FUSE_SYM(rename);
	FUSE_SYM(link);
	FUSE_SYM(open);
	FUSE_SYM(read);
	FUSE_SYM(write);
	FUSE_SYM(flush);
	FUSE_SYM(release);
	FUSE_SYM(fsync);
	FUSE_SYM(opendir);
	FUSE_SYM(readdir);
	FUSE_SYM(releasedir);
	FUSE_SYM(fsyncdir);
	FUSE_SYM(statfs);
	FUSE_SYM(setxattr);
	FUSE_SYM(getxattr);
	FUSE_SYM(listxattr);
	FUSE_SYM(removexattr);
	FUSE_SYM(access);
	FUSE_SYM(create);
	FUSE_SYM(getlk);
	FUSE_SYM(setlk);
	FUSE_SYM(bmap);

	// Major version of the fuse protocol
	static Persistent<String> conn_info_proto_major_sym     = NODE_PSYMBOL("proto_major");
	// Minor version of the fuse protocol
	static Persistent<String> conn_info_proto_minor_sym     = NODE_PSYMBOL("proto_minor");
	// Is asynchronous read supported
	static Persistent<String> conn_info_async_read_sym      = NODE_PSYMBOL("async_read");
	// Maximum size of the write buffer
	static Persistent<String> conn_info_max_write_sym       = NODE_PSYMBOL("max_write");
	// Maximum readahead
	static Persistent<String> conn_info_max_readahead_sym   = NODE_PSYMBOL("max_readahead");
	// Capability flags, that the kernel supports
	static Persistent<String> conn_info_capable_sym         = NODE_PSYMBOL("capable");
	// Capability flags, that the filesystem wants to enable
	static Persistent<String> conn_info_want_sym            = NODE_PSYMBOL("want");

	void Proxy::Initialize() {
		// TODO
	}

	void Proxy::Call(const char *op, Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		std::string op_name = op;

		// fprintf(stderr, "<-- %s\n", op);

		BIND_OPERATION(Init);
		BIND_OPERATION(Destroy);
		BIND_OPERATION(Lookup);
		BIND_OPERATION(Forget);
		BIND_OPERATION(GetAttr);
		BIND_OPERATION(SetAttr);
		BIND_OPERATION(ReadLink);
		BIND_OPERATION(MkNod);
		BIND_OPERATION(MkDir);
		BIND_OPERATION(Unlink);
		BIND_OPERATION(RmDir);
		BIND_OPERATION(SymLink);
		BIND_OPERATION(Rename);
		BIND_OPERATION(Link);
		BIND_OPERATION(Open);
		BIND_OPERATION(Read);
		BIND_OPERATION(Write);
		BIND_OPERATION(Flush);
		BIND_OPERATION(Release);
		BIND_OPERATION(FSync);
		BIND_OPERATION(OpenDir);
		BIND_OPERATION(ReadDir);
		BIND_OPERATION(ReleaseDir);
		BIND_OPERATION(FSyncDir);
		BIND_OPERATION(StatFs);
		BIND_OPERATION(SetXAttr);
		BIND_OPERATION(GetXAttr);
		BIND_OPERATION(ListXAttr);
		BIND_OPERATION(RemoveXAttr);
		BIND_OPERATION(Access);
		BIND_OPERATION(Create);
		BIND_OPERATION(GetLock);
		BIND_OPERATION(SetLock);
		BIND_OPERATION(BMap);
	}

	void Proxy::Init(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);
		struct fuse_conn_info *conn = (struct fuse_conn_info *) argument->args[0];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vinit = fuse->fsobj->Get(init_sym);
		Local<Function> init = Local<Function>::Cast(vinit);

		// These properties will be read-only for now.
		// TODO set accessors for read/write properties
		Local<Object> info = Object::New();
		info->Set(conn_info_proto_major_sym, Integer::New(conn->proto_major));
		info->Set(conn_info_proto_minor_sym, Integer::New(conn->proto_minor));
		info->Set(conn_info_async_read_sym, Integer::New(conn->async_read));
		info->Set(conn_info_max_write_sym, Number::New(conn->max_write));
		info->Set(conn_info_max_readahead_sym, Number::New(conn->max_readahead));

		// TODO macro to enable certain properties given the fuse version
		// info->Set(conn_info_capable_sym, Integer::New(conn->capable));
		// info->Set(conn_info_want_sym, Integer::New(conn->want));

		const int argc = 1;
		Local<Value> argv[argc] = {info};

		TRY_CATCH_BEGIN();

		init->Call(fuse->fsobj, argc, argv);

		if (try_catch.HasCaught()) {
			FatalException(try_catch);
		}
	}

	void Proxy::Destroy(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vdestroy = fuse->fsobj->Get(destroy_sym);
		Local<Function> destroy = Local<Function>::Cast(vdestroy);

		TRY_CATCH_BEGIN();

		destroy->Call(fuse->fsobj, 0, NULL);

		if (try_catch.HasCaught()) {
			FatalException(try_catch);
		}
	}

	void Proxy::Lookup(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t parent = (fuse_ino_t) argument->args[1];
		const char *name = (const char *) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		fuse->fsobj->Get(lookup_sym);

		Local<Value> vlookup = fuse->fsobj->Get(lookup_sym);
		Local<Function> lookup = Local<Function>::Cast(vlookup);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> parentInode = Number::New(parent);
		Local<String> entryName = String::New(name);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, parentInode, entryName, replyObj};

		TRY_CATCH_BEGIN();
		lookup->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		delete (char *)argument->args[2];
		FREE_ARGUMENTS();

		Event::Emit(fuse, "lookup", argc, argv);
	}

	void Proxy::Forget(Persistent<Object> CI, void *pArgument) {

	}

	void Proxy::GetAttr(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vgetattr = fuse->fsobj->Get(getattr_sym);
		Local<Function> getattr = Local<Function>::Cast(vgetattr);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 3;
		Local<Value> argv[argc] = {context, inode, replyObj};

		TRY_CATCH_BEGIN();
		getattr->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "getattr", argc, argv);
	}

	void Proxy::SetAttr(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		struct stat *attr = (struct stat *) argument->args[2];
		int to_set = (long) argument->args[3];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[4];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vsetattr = fuse->fsobj->Get(setattr_sym);
		Local<Function> setattr = Local<Function>::Cast(vsetattr);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);

		Local<Object> attrs = GetAttrsToBeSet(to_set, attr)->ToObject();

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, inode, attrs, replyObj};

		TRY_CATCH_BEGIN();
		setattr->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "setattr", argc, argv);
	}

	void Proxy::ReadLink(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vreadlink = fuse->fsobj->Get(readlink_sym);
		Local<Function> readlink = Local<Function>::Cast(vreadlink);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 3;
		Local<Value> argv[argc] = {context, inode, replyObj};

		TRY_CATCH_BEGIN();
		readlink->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "readlink", argc, argv);
	}

	void Proxy::MkNod(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t parent = (fuse_ino_t) argument->args[1];
		const char *name = (const char *) argument->args[2];
		mode_t mode = (long) argument->args[3];
		dev_t rdev = (long) argument->args[4];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vmknod = fuse->fsobj->Get(mknod_sym);
		Local<Function> mknod = Local<Function>::Cast(vmknod);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> parentInode = Number::New(parent);

		Local<String> name_ = String::New(name);
		Local<Integer> mode_ = Integer::New(mode);
		Local<Integer> rdev_ = Integer::New(rdev);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 6;
		Local<Value> argv[argc] = {context, parentInode, name_, mode_, rdev_, replyObj};

		TRY_CATCH_BEGIN();
		mknod->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "mknod", argc, argv);
	}

	void Proxy::MkDir(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t parent = (fuse_ino_t) argument->args[1];
		const char *name = (const char *) argument->args[2];
		mode_t mode = (long) argument->args[3];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vmkdir = fuse->fsobj->Get(mkdir_sym);
		Local<Function> mkdir = Local<Function>::Cast(vmkdir);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> parentInode = Number::New(parent);

		Local<String> name_ = String::New(name);
		Local<Integer> mode_ = Integer::New(mode);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 5;
		Local<Value> argv[argc] = {context, parentInode, name_, mode_, replyObj};

		TRY_CATCH_BEGIN();
		mkdir->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "mkdir", argc, argv);
	}

	void Proxy::Unlink(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t parent = (fuse_ino_t) argument->args[1];
		const char *name = (const char *) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vunlink = fuse->fsobj->Get(unlink_sym);
		Local<Function> unlink = Local<Function>::Cast(vunlink);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> parentInode = Number::New(parent);
		Local<String> name_ = String::New(name);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, parentInode, name_, replyObj};

		TRY_CATCH_BEGIN();
		unlink->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "unlink", argc, argv);
	}

	void Proxy::RmDir(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t parent = (fuse_ino_t) argument->args[1];
		const char *name = (const char *) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vrmdir = fuse->fsobj->Get(rmdir_sym);
		Local<Function> rmdir = Local<Function>::Cast(vrmdir);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> parentInode = Number::New(parent);
		Local<String> name_ = String::New(name);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, parentInode, name_, replyObj};

		TRY_CATCH_BEGIN();
		rmdir->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "rmdir", argc, argv);
	}

	void Proxy::SymLink(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		const char *link = (const char *) argument->args[1];
		fuse_ino_t parent = (fuse_ino_t) argument->args[2];
		const char *name = (const char *) argument->args[3];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vsymlink = fuse->fsobj->Get(symlink_sym);
		Local<Function> symlink = Local<Function>::Cast(vsymlink);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> parentInode = Number::New(parent);
		Local<String> name_ = String::New(name);
		Local<String> link_ = String::New(link);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 5;
		Local<Value> argv[argc] = {context, parentInode, link_, name_, replyObj};

		TRY_CATCH_BEGIN();
		symlink->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "symlink", argc, argv);
	}

	void Proxy::Rename(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t parent = (fuse_ino_t) argument->args[1];
		const char *name = (const char *) argument->args[2];
		fuse_ino_t newparent = (fuse_ino_t) argument->args[3];
		const char *newname = (const char *) argument->args[4];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vrename = fuse->fsobj->Get(rename_sym);
		Local<Function> rename = Local<Function>::Cast(vrename);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> parentInode = Number::New(parent);
		Local<String> name_ = String::New(name);
		Local<Number> newParentInode = Number::New(newparent);
		Local<String> newName = String::New(newname);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 6;
		Local<Value> argv[argc] = {context, parentInode, name_, newParentInode, newName, replyObj};

		TRY_CATCH_BEGIN();
		rename->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "rename", argc, argv);
	}

	void Proxy::Link(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		fuse_ino_t newparent = (fuse_ino_t) argument->args[2];
		const char *newname = (const char *) argument->args[3];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vlink = fuse->fsobj->Get(link_sym);
		Local<Function> link = Local<Function>::Cast(vlink);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<Number> newParent = Number::New(newparent);
		Local<String> newName = String::New(newname);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 5;
		Local<Value> argv[argc] = {context, inode, newParent, newName, replyObj};

		TRY_CATCH_BEGIN();
		link->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "link", argc, argv);
	}

	void Proxy::Open(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vopen = fuse->fsobj->Get(open_sym);
		Local<Function> open = Local<Function>::Cast(vopen);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, inode, infoObj, replyObj};

		TRY_CATCH_BEGIN();
		open->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "open", argc, argv);
	}

	void Proxy::Read(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		size_t size_ = (long) argument->args[2];
		off_t off = (long) argument->args[3];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[4];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vread = fuse->fsobj->Get(read_sym);
		Local<Function> read = Local<Function>::Cast(vread);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<Integer> size = Integer::New(size_);
		Local<Integer> offset = Integer::New(off);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 6;
		Local<Value> argv[argc] = {context, inode, size, offset, infoObj, replyObj};

		TRY_CATCH_BEGIN();
		read->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "read", argc, argv);
	}

	void Proxy::Write(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		const char *buf = (const char *) argument->args[2];
		size_t size = (long) argument->args[3];
		off_t off = (long) argument->args[4];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[5];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vwrite = fuse->fsobj->Get(write_sym);
		Local<Function> write = Local<Function>::Cast(vwrite);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<Integer> offset = Integer::New(off);

		Buffer *buffer = Buffer::New((char *) buf, size);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 6;
		Local<Value> argv[argc] = {context, inode, Local<Object>::New(buffer->handle_), offset, infoObj, replyObj};

		TRY_CATCH_BEGIN();
		write->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "write", argc, argv);
	}

	void Proxy::Flush(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vflush = fuse->fsobj->Get(flush_sym);
		Local<Function> flush = Local<Function>::Cast(vflush);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, inode, infoObj, replyObj};

		TRY_CATCH_BEGIN();
		flush->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "flush", argc, argv);
	}

	void Proxy::Release(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vrelease = fuse->fsobj->Get(release_sym);
		Local<Function> release = Local<Function>::Cast(vrelease);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, inode, infoObj, replyObj};

		TRY_CATCH_BEGIN();
		release->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "release", argc, argv);
	}

	void Proxy::FSync(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		int datasync_ = (long) argument->args[2];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[3];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vfsync = fuse->fsobj->Get(fsync_sym);
		Local<Function> fsync = Local<Function>::Cast(vfsync);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		bool datasync = datasync_ == 0 ? false : true;

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 5;
		Local<Value> argv[argc] = {context, inode, Boolean::New(datasync)->ToObject(), infoObj, replyObj};

		TRY_CATCH_BEGIN();
		fsync->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "fsync", argc, argv);
	}

	void Proxy::OpenDir(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vopendir = fuse->fsobj->Get(opendir_sym);
		Local<Function> opendir = Local<Function>::Cast(vopendir);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, inode, infoObj, replyObj};

		TRY_CATCH_BEGIN();
		opendir->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "opendir", argc, argv);
	}

	void Proxy::ReadDir(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		size_t size_ = (long) argument->args[2];
		off_t off = (long) argument->args[3];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[4];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vreaddir = fuse->fsobj->Get(readdir_sym);
		Local<Function> readdir = Local<Function>::Cast(vreaddir);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<Integer> size = Integer::New(size_);
		Local<Integer> offset = Integer::New(off);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 6;
		Local<Value> argv[argc] = {context, inode, size, offset, infoObj, replyObj};

		TRY_CATCH_BEGIN();
		readdir->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "readdir", argc, argv);
	}

	void Proxy::ReleaseDir(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vreleasedir = fuse->fsobj->Get(releasedir_sym);
		Local<Function> releasedir = Local<Function>::Cast(vreleasedir);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, inode, infoObj, replyObj};

		TRY_CATCH_BEGIN();
		releasedir->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "releasedir", argc, argv);
	}

	void Proxy::FSyncDir(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		int datasync_ = (long) argument->args[2];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[3];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vfsyncdir = fuse->fsobj->Get(fsyncdir_sym);
		Local<Function> fsyncdir = Local<Function>::Cast(vfsyncdir);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		bool datasync = datasync_ == 0 ? false : true;

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 5;
		Local<Value> argv[argc] = {context, inode, Boolean::New(datasync)->ToObject(), infoObj, replyObj};

		TRY_CATCH_BEGIN();
		fsyncdir->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "fsyncdir", argc, argv);
	}

	void Proxy::StatFs(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vstatfs = fuse->fsobj->Get(statfs_sym);
		Local<Function> statfs = Local<Function>::Cast(vstatfs);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 3;
		Local<Value> argv[argc] = {context, inode, replyObj};

		TRY_CATCH_BEGIN();
		statfs->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "statfs", argc, argv);
	}

	void Proxy::SetXAttr(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		const char *name_ = (const char *) argument->args[2];
		const char *value_ = (const char *) argument->args[3];
		size_t size_ = (size_t) argument->args[4];
#ifdef __APPLE__
		int flags_ = (long) argument->args[5];
		uint32_t position_ = (uint32_t) argument->args[6];
#else
		int flags_ = (long) argument->args[5];
#endif

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vsetxattr = fuse->fsobj->Get(setxattr_sym);
		Local<Function> setxattr = Local<Function>::Cast(vsetxattr);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<String> name = String::New(name_);
		Local<String> value = String::New(value_);
#ifdef __APPLE__
		Local<Integer> position = Integer::New(position_);
#endif
		Local<Number> size = Number::New(size_);

		//TODO change for an object with accessors
		Local<Integer> flags = Integer::New(flags_);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

#ifdef __APPLE__
		const int argc = 8;
		Local<Value> argv[argc] = {context, inode, name, value, size, flags, position, replyObj};
#else
		const int argc = 7;
		Local<Value> argv[argc] = {context, inode, name, value, size, flags, replyObj};

#endif
		TRY_CATCH_BEGIN();
		setxattr->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "setxattr", argc, argv);
	}

	void Proxy::GetXAttr(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		const char *name_ = (const char *) argument->args[2];
		size_t size_ = (size_t) argument->args[3];
#ifdef __APPLE__
		uint32_t position_ = (uint32_t) argument->args[4];
#else
#endif

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vgetxattr = fuse->fsobj->Get(getxattr_sym);
		Local<Function> getxattr = Local<Function>::Cast(vgetxattr);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<String> name = String::New(name_);
		Local<Number> size = Number::New(size_);
#ifdef __APPLE__
		Local<Integer> position = Integer::New(position_);
#endif


		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

#ifdef __APPLE__
		const int argc = 6;
		Local<Value> argv[argc] = {context, inode, name, size, position, replyObj};
#else
		const int argc = 5;
		Local<Value> argv[argc] = {context, inode, name, size, replyObj};
#endif

		TRY_CATCH_BEGIN();
		getxattr->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "getxattr", argc, argv);
	}

	void Proxy::ListXAttr(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		size_t size_ = (size_t) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vlistxattr = fuse->fsobj->Get(listxattr_sym);
		Local<Function> listxattr = Local<Function>::Cast(vlistxattr);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<Number> size = Number::New(size_);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, inode, size, replyObj};
		
		TRY_CATCH_BEGIN();
		listxattr->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "listxattr", argc, argv);
	}

	void Proxy::RemoveXAttr(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		const char *name_ = (const char *) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vremovexattr = fuse->fsobj->Get(removexattr_sym);
		Local<Function> removexattr = Local<Function>::Cast(vremovexattr);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<String> name = String::New(name_);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, inode, name, replyObj};
		
		TRY_CATCH_BEGIN();
		removexattr->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "removexattr", argc, argv);
	}

	void Proxy::Access(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		int mask_ = (long) argument->args[2];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vaccess = fuse->fsobj->Get(access_sym);
		Local<Function> access = Local<Function>::Cast(vaccess);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<Integer> mask = Integer::New(mask_);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 4;
		Local<Value> argv[argc] = {context, inode, mask, replyObj};

		TRY_CATCH_BEGIN();
		access->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "access", argc, argv);
	}

	void Proxy::Create(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t parent = (fuse_ino_t) argument->args[1];
		const char *name = (const char *) argument->args[2];
		mode_t mode = (long) argument->args[3];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[4];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vcreate = fuse->fsobj->Get(create_sym);
		Local<Function> create = Local<Function>::Cast(vcreate);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> parentInode = Number::New(parent);
		Local<String> name_ = String::New(name);
		Local<Integer> mode_ = Integer::New(mode);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 6;
		Local<Value> argv[argc] = {context, parentInode, name_, mode_, infoObj, replyObj};

		TRY_CATCH_BEGIN();
		create->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "create", argc, argv);
	}

	void Proxy::GetLock(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[2];
		struct flock *lock = (struct flock *) argument->args[3];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vgetlk = fuse->fsobj->Get(getlk_sym);
		Local<Function> getlk = Local<Function>::Cast(vgetlk);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Local<Object> lockObj = FlockToObject(lock)->ToObject();

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 5;
		Local<Value> argv[argc] = {context, inode, infoObj, lockObj, replyObj};

		TRY_CATCH_BEGIN();
		getlk->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "getlk", argc, argv);
	}

	void Proxy::SetLock(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		struct fuse_file_info *fi = (struct fuse_file_info *) argument->args[2];
		struct flock *lock = (struct flock *) argument->args[3];
		int sleep_ = (long) argument->args[4];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vsetlk = fuse->fsobj->Get(setlk_sym);
		Local<Function> setlk = Local<Function>::Cast(vsetlk);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<Integer> sleep = Integer::New(sleep_);

		FileInfo *info = new FileInfo();
		info->fi = fi;
		Local<Object> infoObj = info->constructor_template->GetFunction()->NewInstance();
		info->Wrap(infoObj);

		Local<Object> lockObj = FlockToObject(lock)->ToObject();

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 6;
		Local<Value> argv[argc] = {context, inode, infoObj, lockObj, sleep, replyObj};

		TRY_CATCH_BEGIN();
		setlk->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "setlk", argc, argv);
	}

	void Proxy::BMap(Persistent<Object> CI, void *pArgument) {
		HandleScope scope;

		// GET ARGUMENTS
		ThreadFunData *argument = reinterpret_cast<ThreadFunData *>(pArgument);

		fuse_req_t req = (fuse_req_t) argument->args[0];
		fuse_ino_t ino = (fuse_ino_t) argument->args[1];
		size_t blocksize_ = (long) argument->args[2];
		uint64_t idx = (uint64_t) argument->args[3];

		// CREATE FUSE OBJECT
		Fuse *fuse = ObjectWrap::Unwrap<Fuse>(CI);

		Local<Value> vbmap = fuse->fsobj->Get(bmap_sym);
		Local<Function> bmap = Local<Function>::Cast(vbmap);

		Local<Object> context = RequestContextToObject(fuse_req_ctx(req))->ToObject();
		Local<Number> inode = Number::New(ino);
		Local<Integer> blocksize = Integer::New(blocksize_);
		Local<Integer> index = Integer::New(idx);

		Reply *reply = new Reply();
		reply->request = req;
		Local<Object> replyObj = reply->constructor_template->GetFunction()->NewInstance();
		reply->Wrap(replyObj);

		const int argc = 5;
		Local<Value> argv[argc] = {context, inode, blocksize, index, replyObj};

		TRY_CATCH_BEGIN();
		bmap->Call(fuse->fsobj, argc, argv);
		TRY_CATCH_END();
		FREE_ARGUMENTS();

		Event::Emit(fuse, "bmap", argc, argv);
	}
}