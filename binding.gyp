{
	"targets": [{
		"target_name": "fuse",
		"sources": [
			"src/bindings.cc",
			"src/file_info.cc",
			"src/filesystem.cc",
			"src/proxy.cc",
			"src/node_fuse.cc",
			"src/reply.cc",
			"src/event.cc",
		],
		"cflags": [
			"-D_FILE_OFFSET_BITS=64",
			"-DFUSE_USE_VERSION=27",
			"-Wextra -Wno-missing-field-initializers",
			"-Wno-unused-variable"
		],
		"cflags!": [ '-fno-exceptions' ],
		"cflags_cc!": [ '-fno-exceptions' ],
		"include_dirs": [
			"<!@(pkg-config fuse --cflags-only-I | sed s/-I//g)"
		],
		"link_settings": {
			"libraries": [
				"<!@(pkg-config --libs-only-l fuse)"
			]
		}
	}]
}
