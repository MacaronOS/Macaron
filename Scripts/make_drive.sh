#!/bin/bash

if [ ! -f drive.img ]; then

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    EXT2_FORMATTER=mkfs
elif [[ "$OSTYPE" == "darwin"* ]]; then
    EXT2_FORMATTER=/usr/local/opt/e2fsprogs/sbin/mkfs.ext2
fi

qemu-img create -f raw drive.img 16M
chmod a+rw drive.img
$EXT2_FORMATTER -t ext2 -r 0 -b 1024 drive.img
mkdir -p mountpoint
fuse-ext2 drive.img mountpoint -o rw+
touch mountpoint/file.txt
bash -c 'echo "testing..." > mountpoint/file.txt'
umount mountpoint

fi