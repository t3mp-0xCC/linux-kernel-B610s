# Linux Kernel 3.10.59 for SoftBank Air 3(B610s-77a)  

Overview
------------
linux kernel source for B610s-77a. Porting from B618s-22d linux kernel.  
B618s-22d kernel source code(https://consumer.huawei.com/en/opensource/detail/?siteCode=worldwide&keywords=b618s&fileType=openSourceSoftware&pageSize=10&curPage=1)  

How to configure  
------------  
1. Change current directory  
```
cd path/to/kernel/source
```  
2. Configure kernel with menuconfig  
```
make ARCH=arm B610s_defconfig

make menuconfig ARCH=arm 
```
4. Overwrite defconfig
```
mv -f .config arch/arm/B610s_defconfig
```
3. Cleaning  
```
make mrproper
```

How to build  
------------  
0. Get cross compiler
I recommend linaro.org one
https://releases.linaro.org/components/toolchain/binaries/4.9-2017.01/armeb-eabi  
```
wget https://releases.linaro.org/components/toolchain/binaries/4.9-2017.01/armeb-eabi/gcc-linaro-4.9.4-2017.01-x86_64_armeb-eabi.tar.xz
tar xvf gcc-linaro-4.9.4-2017.01-x86_64_armeb-eabi.tar.xz
```
1. Set some variables  
```
export PATH=$PATH:/path/to/gcc-linaro-4.9.4-2017.01-x86_64_arm-eabi/bin

export CROSS_COMPILE=arm-eabi-
```
2. Prepare config file  
```
make ARCH=arm O=output B610s_defconfig
```  
3. Build kernel  
```
make ARCH=arm O=output -j8
```
If you finish building, you can find zImage in output/arch/arm/boot/zImage  


