JSR msm8x25 original kernel
===========================

Sources to build kernel for the innos D9/i6

Build Instructions
-----------------------------------------------------------------------------

    mkdir ~/d9
    cd ~/d9
    git clone --progress -v "https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6"
    git clone --progress -v "https://github.com/jsr-d9/android_vendor_jsr_kobj.git"
    git clone --progress -v "https://github.com/jsr-d9/android_kernel_msm.git"

    cd ~/d9/android_kernel_msm
    make clean
    cp -rf ../android_vendor_jsr_kobj/leds/* drivers/leds
    cp -rf ../android_vendor_jsr_kobj/ov/* drivers/media/video/msm/sensors
    export ARCH=arm
    export CROSS_COMPILE=~/d9/arm-eabi-4.6/bin/arm-eabi-
    make jsr_d9_defconfig
    make ARCH=arm -j4
