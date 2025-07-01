@echo off
echo Downloading algo...
dfu-util -d 1fc9:000c -t 2048 -R -D iram_dfu_util_spiflash.bin.hdr
echo Programming ../hydrasdr_rfone_fw.bin
lpcdfu -d 3 -e -D ../hydrasdr_rfone_fw.bin -U
Pause
