fuse-ext2 drive.img mountpoint -o rw+
python3 Scripts/install_userspace.py

mkdir -p mountpoint/System/
find System/* -type f ! -name "*.*" -exec cp {} ./mountpoint/System/ \;

mkdir -p mountpoint/Resources/
cp -r Resources/* mountpoint/Resources/

umount mountpoint