// Copyright 2012, Camilo Aguilar. Cloudescape, LLC.
#include "node_fuse.h"
#include "file_info.h"
#include "reply.h"
#include "bindings.h"

namespace NodeFuse {

	// stat struct symbols
	FUSE_SYM(uid);
	FUSE_SYM(gid);
	FUSE_SYM(pid);
	FUSE_SYM(dev);
	FUSE_SYM(mode);
	FUSE_SYM(nlink);
	FUSE_SYM(rdev);
	FUSE_SYM(size);
	FUSE_SYM(blksize);
	FUSE_SYM(blocks);
	FUSE_SYM(atime);
	FUSE_SYM(mtime);
	FUSE_SYM(ctime);

	// statvfs struct symbols
	FUSE_SYM(bsize);
	FUSE_SYM(frsize);
	FUSE_SYM(bfree);
	FUSE_SYM(bavail);
	FUSE_SYM(files);
	FUSE_SYM(ffree);
	FUSE_SYM(favail);
	FUSE_SYM(fsid);
	FUSE_SYM(flag);
	FUSE_SYM(namemax);

	// entry symbols
	FUSE_SYM(inode);
	FUSE_SYM(generation);
	FUSE_SYM(attr);
	FUSE_SYM(attr_timeout);
	FUSE_SYM(entry_timeout);

	// lock symbols
	FUSE_SYM(type);
	FUSE_SYM(whence);
	FUSE_SYM(start);
	FUSE_SYM(len);

	void InitializeFuse(Handle<Object> target) {
		HandleScope scope;

		Fuse::Initialize(target);
		FileSystem::Initialize();
		Proxy::Initialize();
		Reply::Initialize();
		FileInfo::Initialize();

		target->Set(String::NewSymbol("version"),
		            String::New(NODE_FUSE_VERSION));

		target->Set(String::NewSymbol("fuse_version"),
		            Integer::New(fuse_version()));
	}

	int ObjectToFuseEntryParam(Handle<Value> value, struct fuse_entry_param *entry) {
		HandleScope scope;
		int ret = -1;

		memset(entry, 0, sizeof(entry));

		Local<Object> obj = value->ToObject();
		entry->ino = obj->Get(inode_sym)->IntegerValue();
		entry->generation = obj->Get(generation_sym)->IntegerValue();
		entry->attr_timeout = obj->Get(attr_timeout_sym)->NumberValue();
		entry->entry_timeout = obj->Get(entry_timeout_sym)->NumberValue();

		// struct stat statbuf;
		ret = ObjectToStat(obj->Get(attr_sym), &entry->attr);

		return ret;
	}

	int ObjectToStat(Handle<Value> value, struct stat *statbuf) {
		HandleScope scope;

		memset(statbuf, 0, sizeof(statbuf));

		Local<Object> obj = value->ToObject();

		statbuf->st_dev = obj->Get(dev_sym)->IntegerValue();
		statbuf->st_ino = obj->Get(inode_sym)->IntegerValue();
		statbuf->st_mode = obj->Get(mode_sym)->IntegerValue();
		statbuf->st_nlink = obj->Get(nlink_sym)->IntegerValue();
		statbuf->st_uid = obj->Get(uid_sym)->IntegerValue();
		statbuf->st_gid = obj->Get(gid_sym)->IntegerValue();
		statbuf->st_rdev = obj->Get(rdev_sym)->IntegerValue();
		statbuf->st_size = obj->Get(size_sym)->NumberValue();
		statbuf->st_blksize = obj->Get(blksize_sym)->IntegerValue();
		statbuf->st_blocks = obj->Get(blocks_sym)->IntegerValue();
		statbuf->st_atime = NODE_V8_UNIXTIME(obj->Get(atime_sym));
		statbuf->st_mtime = NODE_V8_UNIXTIME(obj->Get(mtime_sym));
		statbuf->st_ctime = NODE_V8_UNIXTIME(obj->Get(ctime_sym));

		return 0;
	}

	int ObjectToStatVfs(Handle<Value> value, struct statvfs *statbuf) {
		HandleScope scope;

		memset(statbuf, 0, sizeof(statbuf));

		Local<Object> obj = value->ToObject();

		statbuf->f_bsize = obj->Get(bsize_sym)->NumberValue();
		statbuf->f_frsize = obj->Get(blocks_sym)->NumberValue();

		statbuf->f_blocks = obj->Get(blocks_sym)->IntegerValue();
		statbuf->f_bfree = obj->Get(bfree_sym)->IntegerValue();
		statbuf->f_bavail = obj->Get(bavail_sym)->IntegerValue();
		statbuf->f_files = obj->Get(files_sym)->IntegerValue();
		statbuf->f_ffree = obj->Get(ffree_sym)->IntegerValue();
		statbuf->f_favail = obj->Get(favail_sym)->NumberValue();

		statbuf->f_fsid = obj->Get(fsid_sym)->NumberValue();
		statbuf->f_flag = obj->Get(flag_sym)->NumberValue();
		statbuf->f_namemax = obj->Get(namemax_sym)->NumberValue();

		return 0;
	}

	int ObjectToFlock(Handle<Value> value, struct flock *lock) {
		HandleScope scope;

		memset(lock, 0, sizeof(lock));

		Local<Object> obj = value->ToObject();

		lock->l_type = obj->Get(type_sym)->IntegerValue();
		lock->l_whence = obj->Get(whence_sym)->IntegerValue();
		lock->l_start = obj->Get(start_sym)->IntegerValue();
		lock->l_len = obj->Get(len_sym)->IntegerValue();
		lock->l_pid = obj->Get(pid_sym)->IntegerValue();

		return 0;
	}

	Handle<Value> GetAttrsToBeSet(int to_set, struct stat *stat) {
		HandleScope scope;
		Local<Object> attrs = Object::New();

		if (to_set & FUSE_SET_ATTR_MODE) {
			attrs->Set(mode_sym, Integer::New(stat->st_mode));
		}

		if (to_set & FUSE_SET_ATTR_UID) {
			attrs->Set(uid_sym, Integer::New(stat->st_uid));
		}

		if (to_set & FUSE_SET_ATTR_GID) {
			attrs->Set(gid_sym, Integer::New(stat->st_gid));
		}

		if (to_set & FUSE_SET_ATTR_SIZE) {
			attrs->Set(size_sym, Number::New(stat->st_size));
		}

		if (to_set & FUSE_SET_ATTR_ATIME) {
			attrs->Set(atime_sym, NODE_UNIXTIME_V8(stat->st_atime));
		}

		if (to_set & FUSE_SET_ATTR_MTIME) {
			attrs->Set(mtime_sym, NODE_UNIXTIME_V8(stat->st_mtime));
		}

		return scope.Close(attrs);
	}

	Handle<Value> RequestContextToObject(const struct fuse_ctx *ctx) {
		HandleScope scope;
		Local<Object> context = Object::New();

		context->Set(uid_sym, Integer::New(ctx->uid));
		context->Set(gid_sym, Integer::New(ctx->gid));
		context->Set(pid_sym, Integer::New(ctx->pid));

		return scope.Close(context);
	}

	Handle<Value> FlockToObject(const struct flock *lock) {
		HandleScope scope;
		Local<Object> rv = Object::New();

		//Specifies the type of the lock; one of F_RDLCK, F_WRLCK, or F_UNLCK.
		rv->Set(type_sym, Integer::New(lock->l_type)); //TODO convert to object with accessors
		//This corresponds to the whence argument to fseek or lseek, and specifies what the offset is relative to. Its value can be one of SEEK_SET, SEEK_CUR, or SEEK_END.
		rv->Set(whence_sym, Integer::New(lock->l_whence)); //TODO convert to object with accessors
		rv->Set(start_sym, Integer::New(lock->l_start));
		rv->Set(len_sym, Integer::New(lock->l_len));
		rv->Set(pid_sym, Integer::New(lock->l_pid));

		return scope.Close(rv);
	}

	NODE_MODULE(fuse, InitializeFuse)
} //namespace NodeFuse

