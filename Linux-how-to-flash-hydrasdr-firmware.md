### Flash HydraSDR RFOne firmware with USB firmware mode

#### Prerequisites to build & install host tools in order to use host tools & flash firmware:

* See https://github.com/hydrasdr/rfone_host/blob/master/README.md

### Flash HydraSDR RFOne firmware

* 1. Connect HydraSDR RFOne with USB to your computer
* 2. Open a shell and type following commands:

```
cd ~
hydrasdr_spiflash -w ./rfone_fw/fw_bin/hydrasdr_rfone_fw.bin
```
* 3. Disconnect HydraSDR RFOne from USB computer
* 4. Connect HydraSDR RFOne with USB to your computer, now you can use your favorite software

### Flash HydraSDR RFOne firmware with USB DFU/Recovery mode

#### Prerequisites to flash firmware with USB DFU:

```
cd ~
git clone git://git.code.sf.net/p/dfu-util/dfu-util
cd dfu-util
./autogen.sh
./configure
make
sudo make install
```

#### Flash HydraSDR RFOne firmware with USB DFU

To flash firmware (from `rfone_fw/fw_bin/hydrasdr_rfone_fw.bin`) with DFU:
* 1. Disconnect HydraSDR RFOne from USB computer
* 2. Open HydraSDR RFOne case and set the switch `SW2` to position `1-2` (Boot USB0)
* 3. Connect HydraSDR RFOne with USB to your computer
* 4. Download & extract https://hydrasdr.com/downloads/lpcdfu-1.02_src_linux_win32_hydrasdr_fw_v1.0.0-rc2.7z
* 5. Open a shell and change directory to main directory of extracted archive lpcdfu-1.02_src_linux_win32_hydrasdr_fw_v1.0.0-rc2
* 6. Type following commands:

```
make clean
make
chmod +x dfuprog.sh
./dfuprog.sh hydrasdr_rfone_fw.bin
```
It will flash with USB DFU the firmware hydrasdr_rfone_fw_v1.0.0-rc2 included in archive (you can replace hydrasdr_rfone_fw.bin with an other firmware if you want to test other firmware).

* 7. Disconnect HydraSDR RFOne from USB computer
* 8. Set the switch `SW2` to position `2-3` (Boot SPIFI) then close the HydraSDR RFOne case
* 9. Connect HydraSDR RFOne with USB to your computer, now you can use your favorite software

Note: Tested with success on Xubuntu with dfu-util 0.8