### Prerequisites to build HydraSDR RFOne firmware (Tested on Ubuntu 20.04 LTS):

Warning other version of GCC ARM like GCC ARM 4.9 2014q4 does not work correctly and build fail or have issues, so ONLY use **GNU_ARM_4_7_2013q3** to build the firmware.

* Open a shell and type following commands:

```
cd ~
sudo apt-get install gcc-multilib git dfu-util python
wget https://launchpad.net/gcc-arm-embedded/4.7/4.7-2013-q3-update/+download/gcc-arm-none-eabi-4_7-2013q3-20130916-linux.tar.bz2
tar xjf gcc-arm-none-eabi-4_7-2013q3-20130916-linux.tar.bz2
echo 'PATH=$PATH:~/gcc-arm-none-eabi-4_7-2013q3/bin' >> ~/.bashrc
git clone https://github.com/hydrasdr/rfone_fw.git rfone_fw
cd rfone_fw
cd ./scripts
sudo python get-pip.py
sudo python -m pip install GitPython --upgrade
```
Note1:
* If you are using a new Python 3.x by default you can get/use get-pip.py for Python3 using `wget https://bootstrap.pypa.io/get-pip.py`

Note2:
* For get-pip.py if you need a proxy for internet access set following variables before to launch
python get-pip.py:
```
export http_proxy=http://proxy.myproxy.com
export https_proxy=https://proxy.myproxy.com
sudo -E python get-pip.py
```

### Build HydraSDR RFOne firmware:

* Open a shell and type following commands:
```
cd ~
cd ./rfone_fw/libopencm3
make clean
make
sudo make install
```

Build rfone_fw:
```
cd ~
cd ./rfone_fw/
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
sudo python -m pip install GitPython --upgrade
```
Then relaunch the build
