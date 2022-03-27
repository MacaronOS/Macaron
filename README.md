<p align="center">
<img src="https://github.com/vpachkov/MacaronOS/blob/master/Assets/MacaronOS_logo.png" width="500" alt="Macaron OS">
</p>

## Progress
<img src="https://github.com/vpachkov/MacaronOS/blob/master/Assets/MacaronOS-august-2021.gif" width="400" alt="Macaron OS">

## Features
### Kernel
- **Higher Half Kernel**
- **Keyboard**, **mouse**, **ATA**, **PCI** and **BGA** drivers
- **VFS** with **Dev** and **Ext2** FileSystem support
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
The building process is described [here](https://apple.stackexchange.com/questions/226981/how-do-i-install-fuse-ext2-to-use-with-osxfuse). Just follow the top answer.
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

### Get qemu
#### MacOS
```bash
brew install qemu
```
But the best option for mac is to build it yourself. More info [here](https://www.reddit.com/r/osdev/comments/j1a6hn/bga_driver_double_buffering_issue/).
#### Linux
```bash
apt install qemu-system-i386 qemu-utils
```

### Get a crosscompiler:
#### MacOS
```bash
brew install i686-elf-gcc
```
#### Linux
```bash
./Scripts/i686-elf-tools.sh
```

### Get NASM
#### MacOS
```bash
brew install nasm
```
#### Linux
```bash
apt install nasm
```

### Get [Macabuilder](https://github.com/MacaronOS/Macabuilder) 
Macabuilder is a build system developed to simplify Macaron OS building process.
You can find build instructions [here](https://github.com/MacaronOS/Macabuilder#how-to-build-this-project).

### Run Macaron OS
After everything's set up, just launch Macabuilder executable from the root project directory. It will find MacaronOS.maca file, build and run MacaronOS.