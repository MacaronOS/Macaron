import os

from shutil import copyfile

iconname = 'icon.bmp'
binaryname = 'bin'
build_folder = 'MacaBuild'


def copy_applicatons(apps_path, mountpoint):
    for application in os.listdir(apps_path):
        real_path = os.path.join(apps_path, application)
        if os.path.isdir(real_path):
            bin_path = os.path.join(os.path.join(real_path, build_folder), application)
            icon_path = os.path.join(real_path, iconname)

            app_folder_on_drive = os.path.join(mountpoint, application)
            os.makedirs(app_folder_on_drive, exist_ok=True)

            bin_path_on_drive = os.path.join(app_folder_on_drive, binaryname)
            icon_path_on_drive = os.path.join(app_folder_on_drive, iconname)

            copyfile(bin_path, bin_path_on_drive)
            copyfile(icon_path, icon_path_on_drive)


def copy_binaries(bin_path, mountpoint):
    os.makedirs(mountpoint, exist_ok=True)

    for bin in os.listdir(bin_path):
        real_path = os.path.join(bin_path, bin)
        if os.path.isdir(real_path):
            bin_path = os.path.join(os.path.join(real_path, build_folder), bin)
            bin_path_on_drive = os.path.join(mountpoint, bin)
            copyfile(bin_path, bin_path_on_drive)


if __name__ == '__main__':
    copy_applicatons('Applications', 'mountpoint/Applications')
    copy_binaries('Binaries', 'mountpoint/bin')
