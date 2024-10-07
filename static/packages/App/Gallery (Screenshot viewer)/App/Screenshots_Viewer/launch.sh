#!/bin/sh
echo ":: Gallery - Screenshots Viewer ::"

infoPanel -d /mnt/SDCARD/Screenshots --show-theme-controls
ec=$?

# cancel or success from infoPanel
if [ $ec -eq 255 ] || [ $ec -eq 0 ]; then
    exit 0
elif [ $ec -eq 1 ]; then
    infoPanel -t Thư viện -m "Không có ảnh chụp màn hình nào"
else
    # something went wrong
    infoPanel -t Thư viện -m "Có lỗi xảy ra - mã: $ec"
fi
