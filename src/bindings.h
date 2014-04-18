// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#ifndef SRC_BINDINGS_H_
#define SRC_BINDINGS_H_

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <cstdlib>

#include "node_fuse.h"
#include "filesystem.h"
#include "proxy.h"

namespace NodeFuse {
	class Fuse : public ObjectWrap {
		public:
			static void Initialize(Handle<Object> target);
			struct ThreadFunData {
				void **args;
				const char *op;
			};

			Persistent<Object> fsobj;

			Fuse();
			virtual ~Fuse();

		protected:
			static Handle<Value> New(const Arguments &args);
			static Handle<Value> Mount(const Arguments &args);

			static void AsyncWorker(void *args);
			static void AsyncCallback(uv_async_t *handle, int status);

			// static Handle<Value> Unmount(const Arguments& args);

		private:
			int multithreaded;
			int foreground;
			char *mountpoint;
			struct fuse_args *fargs;
			struct fuse_chan *channel;
			struct fuse_session *session;
			static Persistent<FunctionTemplate> constructor_template;

			struct Userdata {
				void *fuse;
				uv_async_t *async;
			};

			struct Baton {
				uv_async_t s_async;
				uv_thread_t worker_thread;
				Persistent<Object> currentInstance;
				Persistent<Object> fsobj;
				Fuse *fuse;
				struct fuse_args fargs;
				int argc;
			};
	};
} // namespace NodeFuse

#define FUSE_SYM(name)                                              \
    static Persistent<String> name##_sym = NODE_PSYMBOL(#name);

#define BIND_OPERATION(name)                                        \
    if (op_name == #name) {                                         \
    	Proxy::name(CI, pArgument);                                 \
    }


#endif  // SRC_BINDINGS_H