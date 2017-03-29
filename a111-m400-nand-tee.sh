#!/bin/sh
make distclean
make a111_m400_nand_tee_config
make -j8
cp  -v build/uboot-secureos.bin  ../buildroot/output/images/u-boot.bin
cp  -v build/u-boot-comp.bin  ../buildroot/output/images/
cp  -v build/ddr_init.bin  ../buildroot/output/images/
