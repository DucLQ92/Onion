#!/bin/sh
echo -ne "\n\n==================================================================="
echo ":: PORT_LAUNCH" $0 $*
echo "--------------------------------------------------------------------"

# Launch port file
cd "`dirname "$1"`"
chmod a+x "$1"
"$1"

echo "==================================================================="

if [ -f "/tmp/MissingPortFile.tmp" ]; then
	MissingPortFile=`cat /tmp/MissingPortFile.tmp`
	echo "Game data not found: $MissingPortFile"
	echo -ne  "===================================================================\n\n"
	infoPanel --title "Lỗi chạy găme Port"  --message "Không tìm thấy tập tin Port:\n$MissingPortFile"
	rm "/tmp/MissingPortFile.tmp"
fi
