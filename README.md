## FuseJS - Low level bindings for Fuse

Fusejs are a set of NodeJS bindings for [Fuse](http://fuse.sourceforge.net/) low level API. 
It allows you to write filesystems, in userspace, using Javascript and NodeJS. 
Even though most of the Fuse functions are already binded, this module has not 
been used in production. Use it at your own risk. 

## How it works
Fuse low level API is inherently asynchronous, therefore, there is no need for libuv thread pool. FuseJS workflow looks like:

```                          
          node example/hello.js /tmp/hello_fs -ofsname=hellofs -orw -d
                                         ↕ 
                                Google V8 / FuseJS
Flow starts here!                        ↕
	ls -lah /tmp/hello_fs             libfuse
         	  ↕                          ↕
       		glibc                      glibc
Userspace     ↕                          ↕
---------------------------------------------         
Kernel        ↕                          ↕	
			  ↕                          ↕
             VFS ↔ ↔ ↔ ↔ ↔ ↔ ↔ ↔ ↔ ↔ ↔  FUSE
             
             							...
                                        Ext4
                                        NFS
                                        ZFS
```

## Installation

On Ubuntu:

```
# include the libfuse-dev
sudo apt-get install libfuse-dev

# compiler the fusejs
node-gyp configure build
```



## License
(The MIT License)

Copyright 2012 Camilo Aguilar. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
