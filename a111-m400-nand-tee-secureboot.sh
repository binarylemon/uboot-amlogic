#!/bin/sh
make distclean
make a111_m400_nand_tee_config
make -j8

cp  -v build/uboot-secureos.bin.aml.encrypt  ../../output/meson8b_m400/images/u-boot.bin.aml.encrypt
cp  -v build/ddr_init.bin  ../../output/meson8b_m400/images/
cp  -v build/uboot-secureos.bin  ../../output/meson8b_m400/images/u-boot-comp.bin
