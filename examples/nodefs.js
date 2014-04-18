/**
 * NodeFS filesystem
 **/

var FileSystem = require('../fuse').FileSystem;
var PosixError = require('../fuse').PosixError;
var util = require('util');
var path = require('path');
var fs = require('fs');

var addon = require('../test/build/Release/addon');

var NodeFS = function(fuse, options) {
	this.fuse = fuse;
	this.options = options;

	this.generation = 1;
	this.attr_timeout = 0;
	this.entry_timeout = 0;

	FileSystem.call(this);
};

util.inherits(NodeFS, FileSystem);

(function() {
	var self = this;

	this.test_path = '/tmp';
	this.tree = { 1: this.test_path + '/' };

	this.events.on('fuse', function(msg, args) {
		console.log(msg, args[0]);
	});

	this.hash = function(str) {
		var hash = 0,
			c = '';

		for (i = 0; i < str.length; i++) {
			c = str.charCodeAt(i);
			hash = ((hash << 5) - hash) + c;
			hash = hash & hash;
		}
		return Math.abs(hash);
	};

	this.init = function(connInfo) {
		// setTimeout(function(){
		console.log('Init async');
		// }, 2000);
		console.log('init', 'Initializing NodeFS filesystem!');
	};

	this.destroy = function() {
		console.log('destroy', 'Cleaning up filesystem...');
	};

	this.lookup = function(context, parent, name, reply) {
		var fspath = path.join(self.tree[parent], name);
		var inode = self.hash(fspath);

		try {
			var stats = fs.lstatSync(fspath);
		} catch(err) {
			return reply.err(PosixError.ENOENT);
		}

		var entry = {
			inode: inode,
			attr: stats,
			generation: this.generation,
			attr_timeout: this.attr_timeout,
			entry_timeout: this.entry_timeout
		};

		reply.entry(entry);
		self.tree[inode] = fspath;
	};

	this.forget = function(context, inode, nlookup) {
		console.log('args', arguments);
	};

	this.getattr = function(context, inode, reply) {
		var fspath = self.tree[inode];

		try {
			var stats = fs.lstatSync(fspath);
		} catch(err) {
			return reply.err(PosixError.ENOENT);
		}

		stats.ino = inode;
		stats.inode = inode;

		reply.attr(stats);
	};

	this.setattr = function(context, inode, attrs, reply) {
		var fspath = self.tree[inode];
		var exception = fs.chmodSync(fspath, attrs.mode);
		
		if (exception) {
			console.log('setattr, exception', exception);
			return reply.err(PosixError.EIO);
		}

		reply.attr(fs.lstatSync(fspath));
	};

	this.mknod = function(context, parent, name, mode, rdev, reply) {
		reply.err(PosixError.ENOENT);
		// reply.entry(entry);
	};

	this.mkdir = function(context, parent, name, mode, reply) {
		var fspath = path.join(self.tree[parent], name);
		var exception = fs.mkdirSync(fspath, mode);
		var inode = self.hash(fspath);

		if (exception) {
			console.log('mkdir, exception', exception);
			return reply.err(PosixError.EIO);
		}
		
		var stats = fs.statSync(fspath),
			entry = {};
		
		self.tree[inode] = fspath;
		stats.ino = inode;
		stats.inode = inode;

		entry = {
			inode: inode,
			attr: stats,
			generation: this.generation,
			attr_timeout: this.attr_timeout,
			entry_timeout: this.entry_timeout
		};

		reply.entry(entry);
	};

	this.unlink = function(context, parent, name, reply) {
		var fspath = path.join(self.tree[parent], name);
		var exception = fs.unlinkSync(fspath);
		var inode = self.hash(fspath);

		if (exception) {
			console.log('unlink, exception', exception);
			return reply.err(PosixError.EIO);
		}

		delete self.tree[inode];

		reply.err(0);
	};

	this.rmdir = function(context, parent, name, reply) {
		var fspath = path.join(self.tree[parent], name);
		var exception = fs.rmdir(fspath);
		var inode = self.hash(fspath);

		if (exception) {
			return reply.err(PosixError.EIO);
		}

		delete self.tree[inode];

		// CHILD DIRS...
		reply.err(0);
	};

	this.link = function(context, inode, newParent, newName, reply) {
		var src_path = self.tree[inode];
		var dst_path = path.join(self.tree[newParent], newName);
		var inode = self.hash(dst_path);

		var exception = fs.linkSync(src_path, dst_path);

		if (exception) {
			console.log('link, exception', exception);
			return reply.err(PosixError.EIO);
		}

		var stats = fs.statSync(dst_path),
			entry = {};

		self.tree[inode] = dst_path;
		stats.ino = inode;
		stats.inode = inode;

		entry = {
			inode: inode,
			attr: stats,
			generation: this.generation,
			attr_timeout: this.attr_timeout,
			entry_timeout: this.entry_timeout
		};

		reply.entry(entry);
	};

	this.symlink = function(context, parent, link, name, reply) {
		var src_path = link;
		var dst_path = path.join(self.tree[parent], name);
		var inode = self.hash(dst_path);

		var exception = fs.symlinkSync(src_path, dst_path);

		if (exception) {
			console.log('symlink, exception', exception);
			return reply.err(PosixError.EIO);
		}

		var stats = fs.lstatSync(dst_path),
			entry = {};

		self.tree[inode] = dst_path;
		stats.ino = inode;
		stats.inode = inode;

		entry = {
			inode: inode,
			attr: stats,
			generation: this.generation,
			attr_timeout: this.attr_timeout,
			entry_timeout: this.entry_timeout
		};

		reply.entry(entry);
	};

	this.readlink = function(context, inode, reply) {
		var dst_path = self.tree[inode];
		var src_path = fs.readlinkSync(dst_path);

		reply.readlink(src_path);
	};

	this.rename = function(context, parent, name, newParent, newName, reply) {
		var old_path = path.join(self.tree[parent], name);
		var new_path = path.join(self.tree[newParent], newName);
		var new_inode = self.hash(new_path);

		fs.renameSync(old_path, new_path);
		reply.err(0);
		
		self.tree[new_inode] = new_path;
	};

	this.open = function(context, inode, fileInfo, reply) {
		var fspath = self.tree[inode];

		try {
			var fd = fs.openSync(fspath, 'a+');
		} catch (err) {
			var fd = fs.openSync(fspath, 'r');
		}

		if (isNaN(fd)) {
			console.log('open, exception', fd);
			return reply.err(PosixError.EIO);
		}

		fileInfo.fh = fd;
		reply.open(fileInfo);
	};

	this.read = function(context, inode, size, offset, fileInfo, reply) {
		var buf = new Buffer(size);
		fs.read(fileInfo.fh, buf, 0, size, offset, function(err, bytesRead, buf) {
			reply.buffer(buf);
		});
	};

	this.write = function(context, inode, buffer, offset, fileInfo, reply) {
		console.log('fileInfo', fileInfo);
		console.log('fspath', self.tree[inode]);

		var fspath = self.tree[inode];
		var buf = fs.readFileSync(fspath);
		var bytes = fs.writeSync(fileInfo.fh, buffer, 0, buffer.length, offset);

		reply.write(bytes);
	};

	this.flush = function(context, inode, fileInfo, reply) {
		reply.err(0);
	};

	this.release = function(context, inode, fileInfo, reply) {
		reply.err(0);
	};

	// if datasync is true then only user data is flushed, not metadata
	this.fsync = function(context, inode, datasync, fileInfo, reply) {
		reply.err(0);
	};

	this.opendir = function(context, inode, fileInfo, reply) {
		reply.open(fileInfo);
	};

	this.readdir = function(context, inode, size, offset, fileInfo, reply) {
		var fspath = self.tree[inode];
		var entries = ['..', '.'].concat(fs.readdirSync(fspath));

		if (entries.length <= offset + 1)
			return reply.buffer(new Buffer(''));

		for (var i = 0, len = entries.length; i < len; i++) {
			var stat = fs.lstatSync(path.join(fspath, entries[i]));
			var entry_path = path.join(fspath, entries[i]);
			var entry_inode = self.hash(entry_path);

			self.tree[entry_inode] = entry_path;

			stat.inode = entry_inode;
			reply.addDirEntry(entries[i], size, stat, offset + i);
		}

		reply.buffer(new Buffer(''));
	};

	this.releasedir = function(context, inode, fileInfo, reply) {
		reply.err(0);
	};

	// if datasync is true then only directory contents is flushed, not metadata
	this.fsyncdir = function(context, inode, datasync, fileInfo, reply) {
		reply.err(0);
	};

	this.statfs = function(context, inode, reply) {
		var statvfs = {
			bsize: 1024,
			/* file system block size */
			frsize: 0,
			/* fragment size */
			blocks: 0,
			/* size of fs in f_frsize units */
			bfree: 0,
			/* # free blocks */
			bavail: 0,
			/* # free blocks for unprivileged users */
			files: 5,
			/* # inodes */
			ffree: 2,
			/* # free inodes */
			favail: 2,
			/* # free inodes for unprivileged users */
			fsid: 4294967295,
			/* file system ID */
			flag: 0,
			/* mount flags */
			namemax: 1.7976931348623157e+308 /* maximum filename length */
		};

		reply.statfs(statvfs);
	};

	this.setxattr = function(context, inode, name, value, size, flags, position, reply) {
		reply.err(0);
	};

	this.getxattr = function(context, inode, name, size, position, reply) {
		if (typeof position === 'object') {
			reply = position;
		}

		reply.xattr(size);
	};

	this.listxattr = function(context, inode, size, reply) {
		reply.err(0);

		// reply.buffer(new Buffer('list,of,extended,attributes'));
		// reply.xattr(1024);
	};

	this.removexattr = function(context, inode, name, reply) {
		reply.err(0);
	};

	this.access = function(context, inode, mask, reply) {
		reply.err(0);
	};

	this.create = function(context, parent, name, mode, fileInfo, reply) {
		var fspath = path.join(self.tree[parent], name);
		var fd = fs.openSync(fspath, 'w+', mode);
		var inode = self.hash(fspath);

		if (isNaN(fd)) {
			console.log('create, exception', fd);
			return reply.err(PosixError.EIO);
		}

		var stats = fs.statSync(fspath),
			entry = {};

		self.tree[inode] = fspath;
		stats.ino = inode;
		stats.inode = inode;

		entry = {
			inode: inode,
			attr: stats,
			generation: this.generation,
			attr_timeout: this.attr_timeout,
			entry_timeout: this.entry_timeout
		};

		fileInfo.fh = fd;

		console.log('create, fspath', fspath, name, mode, fileInfo, stats);

		reply.create(entry, fileInfo);
	};

	this.getlk = function(context, inode, fileInfo, lock, reply) {
		// reply.lock(lock);
		reply.err(0);
	};

	this.setlk = function(context, inode, fileInfo, lock, sleep, reply) {
		reply.err(0);
	};

	this.bmap = function(context, inode, blocksize, index, reply) {
		reply.bmap(12344);
	};

}).call(NodeFS.prototype);

module.exports = NodeFS;