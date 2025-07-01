### Prerequisites for Windows:
* Install git from http://msysgit.github.io
* Install Python 2.7.x see https://www.python.org/downloads/windows (tested with Python 2.7.8)
    *  Default Install: `C:\Python27\`
    *  Add in environment variable `PATH` the path to Default Install: `C:\Python27\`
* Open a Command Prompt window (`cmd.exe`) and type following commands:
```
git clone https://github.com/hydrasdr/rfone_fw.git rfone_fw
cd rfone_fw
cd ./scripts
python get-pip.py
python -m pip install GitPython --upgrade
```
Note1:
* If you are using a new Python 3.x by default you can get/use get-pip.py for Python3 using the ones form `https://bootstrap.pypa.io/get-pip.py`

Note2:
* For get-pip.py if you need a proxy for internet access set following variables before to launch
python get-pip.py:
```
set http_proxy=http://proxy.myproxy.com
set https_proxy=https://proxy.myproxy.com
```

### Build HydraSDR RFOne firmware (with mingw):

MinGW (http://www.mingw.org) is required.
The firmware is set up for compilation with the GCC toolchain GNU_ARM_4_7_2013q3.

Warning other version of GCC ARM like GCC ARM 4.9 2014q4 does not work correctly and build fail or have issues, so ONLY use **GNU_ARM_4_7_2013q3** to build the firmware.

* Install GCC toolchain GNU_ARM_4_7_2013q3 from https://launchpad.net/gcc-arm-embedded/+milestone/4.7-2013-q3-update
    *  At end of Installation `Install wizard Complete` choose only `Add path to environment variable`
* Install MinGW from http://sourceforge.net/projects/mingw/files/latest/download?source=files
    * Default install: `C:\MinGW\msys\1.0`
    * During MinGW install choose `msys-base` (it includes minimal tools and make 3.81)
    * Launch msys shell from Default Install: `C:\MinGW\msys\1.0\msys.bat`

Build libopencm3 included in rfone_fw:
```
cd in root directory `rfone_fw` (which contains directories m0, m4 ...)
cd ./libopencm3
make clean
make
make install
```

Build rfone_fw:
```
cd in root directory `rfone_fw` (which contains directories m0, m4 ...)
make clean
make
```

### In case of error during build like:
```
Traceback (most recent call last):
  File "../scripts/hydrasdr_fw-version.py", line 14, in <module>
    git=Repo(search_parent_directories=True).git
TypeError: __init__() got an unexpected keyword argument 'search_parent_directories'
make[1]: *** [m0.hdr] Error 1
```
Just execute
```
python -m pip install GitPython --upgrade
```
Then relaunch the build
