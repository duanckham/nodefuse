#ifndef SRC_PROXY_H_
#define SRC_PROXY_H_
#include <string>
#include "node_fuse.h"

namespace NodeFuse {
	class Proxy {
		public:
			Proxy();
			virtual ~Proxy();

			static void Initialize();
			static void Call(const char *op, Persistent<Object> CI, void *pArgument);
			
			static void Init(Persistent<Object> CI, void *pArgument);
			static void Destroy(Persistent<Object> CI, void *pArgument);
			static void Lookup(Persistent<Object> CI, void *pArgument);
			static void Forget(Persistent<Object> CI, void *pArgument);
			static void GetAttr(Persistent<Object> CI, void *pArgument);
			static void SetAttr(Persistent<Object> CI, void *pArgument);
			static void ReadLink(Persistent<Object> CI, void *pArgument);
			static void MkNod(Persistent<Object> CI, void *pArgument);
			static void MkDir(Persistent<Object> CI, void *pArgument);
			static void Unlink(Persistent<Object> CI, void *pArgument);
			static void RmDir(Persistent<Object> CI, void *pArgument);
			static void SymLink(Persistent<Object> CI, void *pArgument);
			static void Rename(Persistent<Object> CI, void *pArgument);
			static void Link(Persistent<Object> CI, void *pArgument);
			static void Open(Persistent<Object> CI, void *pArgument);
			static void Read(Persistent<Object> CI, void *pArgument);
			static void Write(Persistent<Object> CI, void *pArgument);
			static void Flush(Persistent<Object> CI, void *pArgument);
			static void Release(Persistent<Object> CI, void *pArgument);
			static void FSync(Persistent<Object> CI, void *pArgument);
			static void OpenDir(Persistent<Object> CI, void *pArgument);
			static void ReadDir(Persistent<Object> CI, void *pArgument);
			static void ReleaseDir(Persistent<Object> CI, void *pArgument);
			static void FSyncDir(Persistent<Object> CI, void *pArgument);
			static void StatFs(Persistent<Object> CI, void *pArgument);
			static void SetXAttr(Persistent<Object> CI, void *pArgument);
			static void GetXAttr(Persistent<Object> CI, void *pArgument);
			static void ListXAttr(Persistent<Object> CI, void *pArgument);
			static void RemoveXAttr(Persistent<Object> CI, void *pArgument);
			static void Access(Persistent<Object> CI, void *pArgument);
			static void Create(Persistent<Object> CI, void *pArgument);
			static void GetLock(Persistent<Object> CI, void *pArgument);
			static void SetLock(Persistent<Object> CI, void *pArgument);
			static void BMap(Persistent<Object> CI, void *pArgument);

			struct ThreadFunData {
				void **args;
				const char *op;
			};

		private:

	};
} // namespace NodeFuse

#define TRY_CATCH_BEGIN()                                           \
	TryCatch try_catch;

#define TRY_CATCH_END()                                             \
	if (try_catch.HasCaught()) {                                    \
		FatalException(try_catch);                                  \
	}

#define FREE_ARGUMENTS()                                            \
	void **pTmpArgs = argument->args;                               \
	delete[] pTmpArgs;                                              \
	delete argument;

#endif // SRC_PROXY_H