#ifndef SRC_FILESYSTEM_H_
#define SRC_FILESYSTEM_H_
#include <unistd.h>
#include "node_fuse.h"

namespace NodeFuse {
	class FileSystem {
		public:
			FileSystem();
			virtual ~FileSystem();

			static struct fuse_lowlevel_ops *GetOperations();
			static void Initialize();

			static void Init(void *userdata,
			                 struct fuse_conn_info *conn);
			static void Destroy(void *userdata);
			static void Lookup(fuse_req_t req,
			                   fuse_ino_t parent,
			                   const char *name);
			static void Forget(fuse_req_t req,
			                   fuse_ino_t ino,
			                   unsigned long nlookup);
			static void ForgetMulti(fuse_req_t req,
			                        size_t count,
			                        struct fuse_forget_data *forgets);
			static void GetAttr(fuse_req_t req,
			                    fuse_ino_t ino,
			                    struct fuse_file_info *fi);
			static void SetAttr(fuse_req_t req,
			                    fuse_ino_t ino,
			                    struct stat *attr,
			                    int to_set,
			                    struct fuse_file_info *fi);
			static void ReadLink(fuse_req_t req, fuse_ino_t ino);
			static void MkNod(fuse_req_t req,
			                  fuse_ino_t parent,
			                  const char *name,
			                  mode_t mode,
			                  dev_t rdev);
			static void MkDir(fuse_req_t req,
			                  fuse_ino_t parent,
			                  const char *name,
			                  mode_t mode);
			static void Unlink(fuse_req_t req,
			                   fuse_ino_t parent,
			                   const char *name);
			static void RmDir(fuse_req_t req,
			                  fuse_ino_t parent,
			                  const char *name);
			static void SymLink(fuse_req_t req,
			                    const char *link,
			                    fuse_ino_t parent,
			                    const char *name);
			static void Rename(fuse_req_t req,
			                   fuse_ino_t parent,
			                   const char *name,
			                   fuse_ino_t newparent,
			                   const char *newname);
			static void Link(fuse_req_t req,
			                 fuse_ino_t ino,
			                 fuse_ino_t newparent,
			                 const char *newname);
			static void Open(fuse_req_t req,
			                 fuse_ino_t ino,
			                 struct fuse_file_info *fi);
			static void Read(fuse_req_t req,
			                 fuse_ino_t ino,
			                 size_t size,
			                 off_t off,
			                 struct fuse_file_info *fi);
			static void Write(fuse_req_t req,
			                  fuse_ino_t ino,
			                  const char *buf,
			                  size_t size,
			                  off_t off,
			                  struct fuse_file_info *fi);
			static void Flush(fuse_req_t req,
			                  fuse_ino_t ino,
			                  struct fuse_file_info *fi);
			static void Release(fuse_req_t req,
			                    fuse_ino_t ino,
			                    struct fuse_file_info *fi);
			static void FSync(fuse_req_t req,
			                  fuse_ino_t ino,
			                  int datasync,
			                  struct fuse_file_info *fi);
			static void OpenDir(fuse_req_t req,
			                    fuse_ino_t ino,
			                    struct fuse_file_info *fi);
			static void ReadDir(fuse_req_t req,
			                    fuse_ino_t ino,
			                    size_t size,
			                    off_t off,
			                    struct fuse_file_info *fi);
			static void ReleaseDir(fuse_req_t req,
			                       fuse_ino_t ino,
			                       struct fuse_file_info *fi);
			static void FSyncDir(fuse_req_t req,
			                     fuse_ino_t ino,
			                     int datasync,
			                     struct fuse_file_info *fi);
			static void StatFs(fuse_req_t req, fuse_ino_t ino);
			static void SetXAttr(fuse_req_t req,
			                     fuse_ino_t ino,
			                     const char *name,
			                     const char *value,
			                     size_t size,
#ifdef __APPLE__
			                     int flags,
			                     uint32_t position);
#else
			                     int flags);

#endif
			static void GetXAttr(fuse_req_t req,
			                     fuse_ino_t ino,
			                     const char *name,
#ifdef __APPLE__
			                     size_t size,
			                     uint32_t position);
#else
			                     size_t size);
#endif
			static void ListXAttr(fuse_req_t req,
			                      fuse_ino_t ino,
			                      size_t size);
			static void RemoveXAttr(fuse_req_t req,
			                        fuse_ino_t ino,
			                        const char *name);
			static void Access(fuse_req_t req,
			                   fuse_ino_t ino,
			                   int mask);
			static void Create(fuse_req_t req,
			                   fuse_ino_t parent,
			                   const char *name,
			                   mode_t mode,
			                   struct fuse_file_info *fi);
			static void GetLock(fuse_req_t req,
			                    fuse_ino_t ino,
			                    struct fuse_file_info *fi,
			                    struct flock *lock);
			static void SetLock(fuse_req_t req,
			                    fuse_ino_t ino,
			                    struct fuse_file_info *fi,
			                    struct flock *lock,
			                    int sleep);
			static void BMap(fuse_req_t req,
			                 fuse_ino_t ino,
			                 size_t blocksize,
			                 uint64_t idx);

			static void Proxy(void *pUserdata,
			                  void *pArgs,
			                  const char *pName);

			struct ThreadFunData {
				void **args;
				const char *op;
			};

		private:
			struct Userdata {
				void *fuse;
				uv_async_t *async;
			};
	};
} // namespace NodeFuse

#endif // SRC_FILESYSTEM_H