### Flash HydraSDR RFOne firmware with USB firmware mode

To flash firmware (from `rfone_fw/fw_bin/hydrasdr_rfone_fw.bin`) with USB firmware:
* 1. Connect HydraSDR RFOne with USB to your computer
* 2. Launch `rfone_fw/fw_bin/usbprog_hydrasdr/hydrasdr_spiflash.bat`
* 3. Disconnect HydraSDR RFOne from USB computer
* 4. Connect HydraSDR RFOne with USB to your computer, now you can use your favorite software

### Flash HydraSDR RFOne firmware with USB DFU/Recovery mode

To flash firmware (from `rfone_fw/fw_bin/hydrasdr_rfone_fw.bin`) with DFU:
* 1. Disconnect HydraSDR RFOne from USB computer
* 2. Open HydraSDR RFOne case and set the switch `SW2` to position `1-2` (Boot USB0)
* 3. Connect HydraSDR RFOne with USB to your computer
* 4. Download & Install USB DFU drivers from https://www.nxp.com/downloads/en/software/lpc_driver_setup.exe
* 5. Launch `rfone_fw/fw_bin/dfuprog_hydrasdr/dfuprog_hydrasdr.bat`
  * If an error happens disconnect the USB and reconnect it on an other USB port and retry this step
  * Avoid USB 3.0 port and try a USB 2.0 Hub if you have problems.
* 6. Disconnect HydraSDR RFOne from USB computer
* 7. Set switch `SW2` to position `2-3` (Boot SPIFI) then close the HydraSDR RFOne case
* 8. Connect HydraSDR RFOne with USB to your computer, now you can use your favorite software
