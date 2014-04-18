#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_
#include "bindings.h"

namespace NodeFuse {
	class Event {
		public:
			Event();
			virtual ~Event();

			static void Emit(Fuse *fuse, const char *type, const int argc, Local<Value> argv[]);
	};
} // namespace NodeFuse

#endif // SRC_EVENT_H