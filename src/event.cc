#include "event.h"
#include "node_fuse.h"

namespace NodeFuse {

	static Persistent<String> notice_sym = NODE_PSYMBOL("notice");

	void Event::Emit(Fuse *fuse,
	                 const char *type,
	                 const int argc,
	                 Local<Value> argv[]) {

		HandleScope scope;

		Local<Value> vnotice = fuse->fsobj->Get(notice_sym);
		Local<Function> notice = Local<Function>::Cast(vnotice);
		Local<String> msg_type = String::New(type);
		Local<Array> msg_argv = Array::New(argc);

		Local<Value> _argv[2] = { msg_type, msg_argv };

		for (int i = 0; i < argc; i++) {
			msg_argv->Set(i, argv[i]);
		}

		TryCatch try_catch;
		notice->Call(fuse->fsobj, 2, _argv);
        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }
	}
}