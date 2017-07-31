#!/bin/sh
make distclean
make a111_m400_nand_config
make -j8
cp  -v build/u-boot.bin  ../../output/meson8b_m400/images/u-boot.bin
cp  -v build/u-boot-comp.bin  ../../output/meson8b_m400/images/
cp  -v build/ddr_init.bin  ../../output/meson8b_m400/images/
cp  -v build/u-boot.bin.aml.encrypt  ../../output/meson8b_m400/images/
cp  -v build/u-boot-usb.bin.aml.encrypt  ../../output/meson8b_m400/images/
cp  -v build/u-boot-usb.bin.aml.encrypt.usb.start  ../../output/meson8b_m400/images/

