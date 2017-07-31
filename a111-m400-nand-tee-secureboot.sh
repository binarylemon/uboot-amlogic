#!/bin/sh
make distclean
make a111_m400_nand_tee_config
make -j8

# burn to nand/emmc
cp  -v build/uboot-secureos.bin.aml.encrypt  ../../output/meson8b_m400/images/u-boot.bin.aml.encrypt

# no encrypt
cp  -v build/ddr_init.bin  ../../output/meson8b_m400/images/
cp  -v build/uboot-secureos.bin  ../../output/meson8b_m400/images/u-boot-comp.bin

# encrypt
cp  -v build/u-boot-usb.bin.aml.encrypt.usb.start  ../../output/meson8b_m400/images/
cp  -v build/u-boot-usb.bin.aml.encrypt  ../../output/meson8b_m400/images/

# burn to efuse
cp  -v build/u-boot.bin.aml.efuse  ../../output/meson8b_m400/images/u-boot.bin.aml.efuse

