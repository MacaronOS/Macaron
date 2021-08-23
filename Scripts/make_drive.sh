#!/bin/bash

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    EXT2_FORMATTER=mkfs
elif [[ "$OSTYPE" == "darwin"* ]]; then
    EXT2_FORMATTER=/usr/local/opt/e2fsprogs/sbin/mkfs.ext2
fi

rm -f drive.img
qemu-img create -f raw drive.img 16M
sudo $EXT2_FORMATTER -t ext2 -r 0 -b 1024 drive.img
sudo mkdir -p mountpoint
sudo fuse-ext2 drive.img mountpoint -o rw+
sudo touch mountpoint/file.txt
sudo bash -c 'echo "testing..." > mountpoint/file.txt'
sudo umount mountpoint