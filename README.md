<p align="center">
<img src="https://github.com/vpachkov/MacaronOS/blob/master/Assets/MacaronOS_logo.png" width="500" alt="Macaron OS">
</p>


## Features
### Kernel
- **Higher Half Kernel**
- **Keyboard**, **mouse**, **ATA**, **PCI** and **BGA** drivers
- **VFS** with **DevFS** and **EXT2** filesystem support
- **Paging**, **physical** and **virtual** **memory managers**
- **Userspace processes** and **threads**

### Userspace
- **Libc** - At this moment contains Posix syscalls and malloc
- **Libsystem** - Custom MacaronOS syscalls, logging and EventLoop
- **Libipc** - Low level one to many connections sending bytes to each other
- **Tools/protocol_generator.py** - Genrates a complex serializable messages what works on top of Libipc
- **System/WindowServer** - Compositing window server for applications
- **Libgraphics** - Android-like library for graphcis primitives
- **Libui** - Android-like GUI library
- **Macaronlib** - A library that's shared beetween userspace and kernel. Declares the system's ABI and contains STL-like templates 

## Build instructions

### Get fuse-ext2 tools
#### MacOS
Install from here [fuse-ext2](https://github.com/alperakcan/fuse-ext2)
#### Linux
```bash
apt install fuseext2
```

### Get Ext2 formatter
#### MacOS
```bash
brew install e2fsprogs
```
#### Linux
```bash
apt install e2fsprogs
```

### Get [Macabuilder](https://github.com/MacaronOS/Macabuilder) 
Build system developed to simplify Macaron OS building process

### Get a crosscompiler:
#### MacOS
```bash
brew install i686-elf-gcc
```
#### Linux
```bash
./i686-elf-tools.sh
```

After evertythin's set up, just launch [Macabuilder](https://github.com/MacaronOS/Macabuilder) from the root project directory.
</br>
It will find MacaronOS.maca file, build and run MacaronOS.