### Prerequisites
Make sure that your HydraSDR RFOne is connected to a working USB port.

### Windows
1 Download the latest HydraSDR RFOne Host Tools https://github.com/hydrasdr/rfone_host/releases/

2 Right click on the archive and select "Extract all".

3 Right click on the folder where you have extracted the archive while holding the shift key and select "open command window here".

4 Type "cd x64" and hit enter if you are on a 64 bit Operating System (Or "cd x86" if you are on a 32 bit OS).

5 Type "hydrasdr_info" in the command window and hit enter.


### Linux
1 Install the latest HydraSDR RFOne Host Tools from https://github.com/hydrasdr/rfone_host

2 Run "hydrasdr_info".

### MacOS
1 Install the `hydrasdr` package from Homebrew or MacPorts.

2 Connect HydraSDR RFOne device to a USB port and run `hydrasdr_info`.

### Result from hydrasdr_info command
The output from the hydrasdr_info command should look something similar to this:
```
hydrasdr_lib_version: 1.0.0

Found HydraSDR board 1
Board ID Number: 0 (HYDRASDR)
Firmware Version: HydraSDR RFOne v1.0.0
Part ID Number: 0x6906002B 0x00000030
Serial Number: 0x0123456789ABCDEF
Supported sample rates:
        10.000000 MSPS
        2.500000 MSPS
Close board 1
```
