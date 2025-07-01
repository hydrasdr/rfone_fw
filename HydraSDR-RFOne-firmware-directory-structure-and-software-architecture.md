HydraSDR RFOne firmware directory structure and software architecture

* `rfone_fw/m0` directory contains code for M0 core and it manages (file `m0.c`):
  * Boot initialization (function `main`) manages:
    * USB init
    * M0 core IRQ (USB0 & M4 Core)
  * R828D Tuner
    * Manage R828D depending on USB command see `usb_req.c` function
  * Communication with M4 core
    * It is done through M4 core IRQ function `m4core_isr` and just acknowledge M4 IRQ
  * Settings depending on Sample Rate and containing R828D BW conf are stored in file `rfone_fw/common/hydrasdr_rfone_conf.c`
  * Main loop `main` function just check shared "data offset" to transfer the data over USB
  * USB command/answer are managed through `rfone_fw/m0/usb_req.c` function
    * Main USB manager is `usb_vendor_request()` which dispatch all commands supported and defined in `usb_req_init()`

* `rfone_fw/m0s` directory contains code for M0s core (file `m0s.c`):
  * M0s is free for custom firmware (it is disabled and shall be enabled, see main in `rfone_fw/m4/m4.c`)

* `rfone_fw/m4` directory contains code for M4F core and it manages (file `m4.c`):
  * Boot initialization (function `main`) manages:
    * Pin setup
    * System clock init
    * M4 core IRQ (DMA & M0 Core)
    * ADC init
    * Boot M0 core
  * Analog to digital conversion (ADC)
    * Managed through `dma_isr` function, it mainly manages:
      * Some error/performances statistics
      * Update data offset (corresponding to ADC data read from DMA) shared with M0 core
  * Communication with M0 core
      * It is done through M0 core IRQ function `m0core_isr` and manages:
      * Commands from M0 in order to Start/Stop ADC and change Sample Rate
  * Settings for Init, ADC, SI5351C are stored in file `rfone_fw/common/hydrasdr_rfone_conf.c`
  * Main loop `main` function do nothing except entering in low power mode(Wait For Event)

* `rfone_fw/fw_bin` directory contains:
 *  m0, m0s and m4 built together in final firmware `hydrasdr_rfone_fw.bin`
 * `usbprog_hydrasdr` directory contains tools to flash the firmware through USB firmware mode (only for Windows)
 * `dfuprog_hydrasdr` directory contains tools to flash the firmware through USB DFU/Recovery mode (only for Windows)

* `common` directory contains code for M0 or M4 core and specific to HydraSDR RFOne hardware:
  * It contains mainly drivers:
    * CORE mainly dedicated to LPC4370
    * R828D Tuner
    * SI5351C
    * USB
    * SPI Flash(W25Q80BV/S25FL032P)

* `libopencm3` directory contains mainly code for M0, M0s or M4 peripherals/boot

* `scripts` contains Python scripts mainly to retrieve FW version from git release

In this architecture the main manager is the M0 core mainly because it manage USB answer/reply in order
 to be as low power as possible and to let M4F core free for future features.
