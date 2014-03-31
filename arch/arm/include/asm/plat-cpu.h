#ifndef __ARCH_MESON_PLAT_CPU_H__
#define __ARCH_MESON_PLAT_CPU_H__

#define MESON_CPU_TYPE_MESON1		0x10
#define MESON_CPU_TYPE_MESON2		0x20
#define MESON_CPU_TYPE_MESON3		0x30
#define MESON_CPU_TYPE_MESON6		0x60
#define MESON_CPU_TYPE_MESON6TV		0x70
#define MESON_CPU_TYPE_MESON6TVD	0x75
#define MESON_CPU_TYPE_MESON8		0x80
#define MESON_CPU_TYPE_MESON8B		0x8B

//here, right place ?
#define AML_WATCH_DOG_START() do{writel(0, P_WATCHDOG_RESET);writel((10|((1<<22)|(3<<24))), P_WATCHDOG_TC);while(1);}while(0);

#endif
