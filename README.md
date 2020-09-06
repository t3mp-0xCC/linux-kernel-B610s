# Linux Kernel 3.10.59 for SoftBank Air 3(B610s-77a)  

Overview
------------
This kernel source is for B610s-77a, porting from B618s-22d OSS kernel.  
B618s-22d source code(https://consumer.huawei.com/en/opensource/detail/?siteCode=worldwide&keywords=b618s&fileType=openSourceSoftware&pageSize=10&curPage=1)  

How to configure  
------------  
1. Copy default config file to root of kernel source  
```
cd path/to/kernel/source

cp arch/arm/configs/B610s_defconfig .config
```  
2. Configure kernel with menuconfig  
```
make menuconfig ARCH=arm
```  
3. Move config file to default configs folder
```
mv -f .config arch/arm/configs/B610s_defconfig
```  
4. Cleaning  
```
make mrproper
```

How to build  
------------  
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


