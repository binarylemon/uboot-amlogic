#!/bin/sh
make distclean
make a111_m400_nand_tee_config
make -j8

cp  -v build/uboot-secureos.bin  ../../output/meson8b_m400/images/u-boot.bin
cp  -v build/uboot-secureos.bin  ../../output/meson8b_m400/images/u-boot-comp.bin
cp  -v build/ddr_init.bin  ../../output/meson8b_m400/images/

