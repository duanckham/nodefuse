#include "filesystem.h"
#include "reply.h"
#include "file_info.h"
#include "bindings.h"
#include "node_buffer.h"
#include "event.h"

namespace NodeFuse {
	static struct fuse_lowlevel_ops fuse_ops = {};

	void FileSystem::Initialize() {
		fuse_ops.init       		= FileSystem::Init;
		fuse_ops.destroy    		= FileSystem::Destroy;
		fuse_ops.lookup     		= FileSystem::Lookup;
		fuse_ops.forget     		= FileSystem::Forget;
		fuse_ops.forget_multi     	= FileSystem::ForgetMulti;
		fuse_ops.getattr    		= FileSystem::GetAttr;
		fuse_ops.setattr    		= FileSystem::SetAttr;
		fuse_ops.readlink   		= FileSystem::ReadLink;
		fuse_ops.mknod      		= FileSystem::MkNod;
		fuse_ops.mkdir      		= FileSystem::MkDir;
		fuse_ops.unlink     		= FileSystem::Unlink;
		fuse_ops.rmdir      		= FileSystem::RmDir;
		fuse_ops.symlink    		= FileSystem::SymLink;
		fuse_ops.rename     		= FileSystem::Rename;
		fuse_ops.link       		= FileSystem::Link;
		fuse_ops.open       		= FileSystem::Open;
		fuse_ops.read       		= FileSystem::Read;
		fuse_ops.write      		= FileSystem::Write;
		fuse_ops.flush      		= FileSystem::Flush;
		fuse_ops.release    		= FileSystem::Release;
		fuse_ops.fsync      		= FileSystem::FSync;
		fuse_ops.opendir    		= FileSystem::OpenDir;
		fuse_ops.readdir    		= FileSystem::ReadDir;
		fuse_ops.releasedir 		= FileSystem::ReleaseDir;
		fuse_ops.fsyncdir   		= FileSystem::FSyncDir;
		fuse_ops.statfs     		= FileSystem::StatFs;
		fuse_ops.setxattr   		= FileSystem::SetXAttr;
		fuse_ops.getxattr   		= FileSystem::GetXAttr;
		fuse_ops.listxattr  		= FileSystem::ListXAttr;
		fuse_ops.removexattr 		= FileSystem::RemoveXAttr;
		fuse_ops.access     		= FileSystem::Access;
		fuse_ops.create     		= FileSystem::Create;
		fuse_ops.getlk      		= FileSystem::GetLock;
		fuse_ops.setlk      		= FileSystem::SetLock;
		fuse_ops.bmap       		= FileSystem::BMap;
	}

	void FileSystem::Proxy(void *pUserdata, void *pArgs, const char *pName) {
		// fprintf(stderr, "--> %s\n", pName);

		Userdata *_userdata = reinterpret_cast<Userdata *>(pUserdata);

		while (_userdata->async->pending == 1) {
			usleep(100);
		}

		ThreadFunData *data = new ThreadFunData();

		data->op = pName;
		data->args = (void **)pArgs;
		_userdata->async->data = data;

		uv_async_send(_userdata->async);
	}

	void FileSystem::Init(void *userdata,
	                      struct fuse_conn_info *conn) {

		void **args = new void *[1];
		args[0] = (void *)conn;

		FileSystem::Proxy(userdata, args, "Init");
	}

	void FileSystem::Destroy(void *userdata) {
		void **args = new void *[0];
		FileSystem::Proxy(userdata, args, "Destroy");
	}

	void FileSystem::Lookup(fuse_req_t req,
	                        fuse_ino_t parent,
	                        const char *name) {

		int _len = strlen(name);
		char *_name = new char[_len + 1];
		_name[_len] = 0;
		strcpy(_name, name);

		void **args = new void *[3];

		args[0] = (void *)req;
		args[1] = (void *)parent;
		args[2] = (void *)_name;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Lookup");
	}

	void FileSystem::Forget(fuse_req_t req,
	                        fuse_ino_t ino,
	                        unsigned long nlookup) {

		// fuse_reply_none(req);
	}

	void FileSystem::ForgetMulti(fuse_req_t req,
	                             size_t count,
	                             struct fuse_forget_data *forgets) {

		for (size_t i = 0; i < count; i++)
			FileSystem::Forget(req, forgets[i].ino, forgets[i].nlookup);

		fuse_reply_none(req);
	}

	void FileSystem::GetAttr(fuse_req_t req,
	                         fuse_ino_t ino,
	                         struct fuse_file_info *fi) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "GetAttr");
	}

	void FileSystem::SetAttr(fuse_req_t req,
	                         fuse_ino_t ino,
	                         struct stat *attr,
	                         int to_set,
	                         struct fuse_file_info *fi) {

		void **args = new void *[5];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)attr;
		args[3] = (void *)to_set;
		args[4] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "SetAttr");
	}

	void FileSystem::ReadLink(fuse_req_t req,
	                          fuse_ino_t ino) {

		void **args = new void *[2];
		args[0] = (void *)req;
		args[1] = (void *)ino;

		FileSystem::Proxy(fuse_req_userdata(req), args, "ReadLink");
	}

	void FileSystem::MkNod(fuse_req_t req,
	                       fuse_ino_t parent,
	                       const char *name,
	                       mode_t mode,
	                       dev_t rdev) {

		void **args = new void *[5];
		args[0] = (void *)req;
		args[1] = (void *)parent;
		args[2] = (void *)name;
		args[3] = (void *)mode;
		args[4] = (void *)rdev;

		FileSystem::Proxy(fuse_req_userdata(req), args, "MkNod");
	}

	void FileSystem::MkDir(fuse_req_t req,
	                       fuse_ino_t parent,
	                       const char *name,
	                       mode_t mode) {

		void **args = new void *[4];
		args[0] = (void *)req;
		args[1] = (void *)parent;
		args[2] = (void *)name;
		args[3] = (void *)mode;

		FileSystem::Proxy(fuse_req_userdata(req), args, "MkDir");
	}

	void FileSystem::Unlink(fuse_req_t req,
	                        fuse_ino_t parent,
	                        const char *name) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)parent;
		args[2] = (void *)name;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Unlink");
	}

	void FileSystem::RmDir(fuse_req_t req,
	                       fuse_ino_t parent,
	                       const char *name) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)parent;
		args[2] = (void *)name;

		FileSystem::Proxy(fuse_req_userdata(req), args, "RmDir");
	}

	void FileSystem::SymLink(fuse_req_t req,
	                         const char *link,
	                         fuse_ino_t parent,
	                         const char *name) {

		void **args = new void *[4];
		args[0] = (void *)req;
		args[1] = (void *)link;
		args[2] = (void *)parent;
		args[3] = (void *)name;

		FileSystem::Proxy(fuse_req_userdata(req), args, "SymLink");
	}

	void FileSystem::Rename(fuse_req_t req,
	                        fuse_ino_t parent,
	                        const char *name,
	                        fuse_ino_t newparent,
	                        const char *newname) {

		void **args = new void *[5];
		args[0] = (void *)req;
		args[1] = (void *)parent;
		args[2] = (void *)name;
		args[3] = (void *)newparent;
		args[4] = (void *)newname;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Rename");
	}

	void FileSystem::Link(fuse_req_t req,
	                      fuse_ino_t ino,
	                      fuse_ino_t newparent,
	                      const char *newname) {

		void **args = new void *[4];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)newparent;
		args[3] = (void *)newname;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Link");
	}

	void FileSystem::Open(fuse_req_t req,
	                      fuse_ino_t ino,
	                      struct fuse_file_info *fi) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Open");
	}

	void FileSystem::Read(fuse_req_t req,
	                      fuse_ino_t ino,
	                      size_t size_,
	                      off_t off,
	                      struct fuse_file_info *fi) {


		void **args = new void *[5];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)size_;
		args[3] = (void *)off;
		args[4] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Read");
	}

	void FileSystem::Write(fuse_req_t req,
	                       fuse_ino_t ino,
	                       const char *buf,
	                       size_t size,
	                       off_t off,
	                       struct fuse_file_info *fi) {

		void **args = new void *[6];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)buf;
		args[3] = (void *)size;
		args[4] = (void *)off;
		args[5] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Write");
	}

	void FileSystem::Flush(fuse_req_t req,
	                       fuse_ino_t ino,
	                       struct fuse_file_info *fi) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Flush");
	}

	void FileSystem::Release(fuse_req_t req,
	                         fuse_ino_t ino,
	                         struct fuse_file_info *fi) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Release");
	}

	void FileSystem::FSync(fuse_req_t req,
	                       fuse_ino_t ino,
	                       int datasync_,
	                       struct fuse_file_info *fi) {

		void **args = new void *[4];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)datasync_;
		args[3] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "FSync");
	}

	void FileSystem::OpenDir(fuse_req_t req,
	                         fuse_ino_t ino,
	                         struct fuse_file_info *fi) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "OpenDir");
	}

	void FileSystem::ReadDir(fuse_req_t req,
	                         fuse_ino_t ino,
	                         size_t size_,
	                         off_t off,
	                         struct fuse_file_info *fi) {

		void **args = new void *[5];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)size_;
		args[3] = (void *)off;
		args[4] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "ReadDir");
	}

	void FileSystem::ReleaseDir(fuse_req_t req,
	                            fuse_ino_t ino,
	                            struct fuse_file_info *fi) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "ReleaseDir");
	}

	void FileSystem::FSyncDir(fuse_req_t req,
	                          fuse_ino_t ino,
	                          int datasync_,
	                          struct fuse_file_info *fi) {

		void **args = new void *[4];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)datasync_;
		args[3] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "FSyncDir");
	}

	void FileSystem::StatFs(fuse_req_t req,
	                        fuse_ino_t ino) {

		void **args = new void *[2];
		args[0] = (void *)req;
		args[1] = (void *)ino;

		FileSystem::Proxy(fuse_req_userdata(req), args, "StatFs");
	}

	void FileSystem::SetXAttr(fuse_req_t req,
	                          fuse_ino_t ino,
	                          const char *name_,
	                          const char *value_,
	                          size_t size_,
#ifdef __APPLE__
	                          int flags_,
	                          uint32_t position_) {
#else
	                          int flags_) {
#endif

#ifdef __APPLE__
		void **args = new void *[6];
#else
		void **args = new void *[5];
#endif

		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)name_;
		args[3] = (void *)value_;
		args[4] = (void *)size_;

#ifdef __APPLE__
		args[5] = (void *)flags_;
		args[6] = (void *)position_;
#else
		args[5] = (void *)flags_;
#endif

		FileSystem::Proxy(fuse_req_userdata(req), args, "SetXAttr");
	}

	void FileSystem::GetXAttr(fuse_req_t req,
	                          fuse_ino_t ino,
	                          const char *name_,
	                          size_t size_
#ifdef __APPLE__
	                          , uint32_t position_) {
#else
	                         ) {
#endif

#ifdef __APPLE__
		void **args = new void *[5];
#else
		void **args = new void *[4];
#endif

		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)name_;
		args[3] = (void *)size_;

#ifdef __APPLE__
		args[4] = (void *)position_;
#else
#endif

		FileSystem::Proxy(fuse_req_userdata(req), args, "GetXAttr");
	}

	void FileSystem::ListXAttr(fuse_req_t req,
	                           fuse_ino_t ino,
	                           size_t size_) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)size_;

		FileSystem::Proxy(fuse_req_userdata(req), args, "ListXAttr");
	}

	void FileSystem::RemoveXAttr(fuse_req_t req,
	                             fuse_ino_t ino,
	                             const char *name_) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)name_;

		FileSystem::Proxy(fuse_req_userdata(req), args, "RemoveXAttr");
	}

	void FileSystem::Access(fuse_req_t req,
	                        fuse_ino_t ino,
	                        int mask_) {

		void **args = new void *[3];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)mask_;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Access");
	}

	void FileSystem::Create(fuse_req_t req,
	                        fuse_ino_t parent,
	                        const char *name,
	                        mode_t mode,
	                        struct fuse_file_info *fi) {

		void **args = new void *[5];
		args[0] = (void *)req;
		args[1] = (void *)parent;
		args[2] = (void *)name;
		args[3] = (void *)mode;
		args[4] = (void *)fi;

		FileSystem::Proxy(fuse_req_userdata(req), args, "Create");
	}

	void FileSystem::GetLock(fuse_req_t req,
	                         fuse_ino_t ino,
	                         struct fuse_file_info *fi,
	                         struct flock *lock) {

		void **args = new void *[4];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)fi;
		args[3] = (void *)lock;

		FileSystem::Proxy(fuse_req_userdata(req), args, "GetLock");
	}

	void FileSystem::SetLock(fuse_req_t req,
	                         fuse_ino_t ino,
	                         struct fuse_file_info *fi,
	                         struct flock *lock,
	                         int sleep_) {

		void **args = new void *[5];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)fi;
		args[3] = (void *)lock;
		args[4] = (void *)sleep_;

		FileSystem::Proxy(fuse_req_userdata(req), args, "SetLock");
	}

	void FileSystem::BMap(fuse_req_t req,
	                      fuse_ino_t ino,
	                      size_t blocksize_,
	                      uint64_t idx) {

		void **args = new void *[4];
		args[0] = (void *)req;
		args[1] = (void *)ino;
		args[2] = (void *)blocksize_;
		args[3] = (void *)idx;

		FileSystem::Proxy(fuse_req_userdata(req), args, "BMap");
	}

	struct fuse_lowlevel_ops *FileSystem::GetOperations() {
		return &fuse_ops;
	}
}