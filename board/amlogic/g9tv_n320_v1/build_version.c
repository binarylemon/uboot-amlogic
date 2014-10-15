#include <common.h>
#include <version.h>

#ifdef CONFIG_UBOOT_BUILD_VERSION_INFO

#define UBOOT_VERSION_MAIN    2
#define UBOOT_VERSION_SUB1    1
#define UBOOT_VERSION_SUB2    0

static char versioninfo[256] = "N/A";
static long long version_serial = 0;
static char gitversionstr[256] = "N/A";

static int uboot_version_info_init(void) {
    static int info_is_inited = 0;
    char git_shor_version[20];
    unsigned int shortgitversion;
    int dirty_num = 0;

    if (info_is_inited > 0) {
        return 0;
    }
    info_is_inited++;

#ifdef U_BOOT_GIT_UNCOMMIT_FILE_NUM
    dirty_num = simple_strtoul(U_BOOT_GIT_UNCOMMIT_FILE_NUM, NULL, 10);
#endif

#ifdef U_BOOT_GIT_COMMIT
    if (dirty_num > 0) {
        sprintf(gitversionstr, "%s-with-%d-dirty-files", U_BOOT_GIT_COMMIT, dirty_num);
    } else {
        sprintf(gitversionstr, "%s", U_BOOT_GIT_COMMIT);
    }
#endif

    memcpy(git_shor_version, gitversionstr, 8);
    git_shor_version[8] = '\0';
    shortgitversion = simple_strtoul(git_shor_version, NULL, 16);
    version_serial = (long long) (UBOOT_VERSION_MAIN & 0xff) << 56 | (long long) (UBOOT_VERSION_SUB1 & 0xff) << 48 | (long long) (UBOOT_VERSION_SUB2 & 0xff) << 32
            | shortgitversion;

    sprintf(versioninfo, "Version:%d.%d.%d.%s", UBOOT_VERSION_MAIN, UBOOT_VERSION_SUB1, UBOOT_VERSION_SUB2, git_shor_version);

    return 0;
}

static const char *uboot_get_version_info(void) {
    uboot_version_info_init();
    return versioninfo;
}

static int64_t uboot_get_version_serail(void) {
    uboot_version_info_init();
    return version_serial;
}

static const char *uboot_get_git_version_info(void) {
    uboot_version_info_init();
    return gitversionstr;
}

static const char *uboot_get_last_chaned_time_info(void) {
#ifdef U_BOOT_LAST_CHANGED
    return U_BOOT_LAST_CHANGED;
#else
    return " Unknow ";
#endif
}

static const char *uboot_get_git_branch_info(void) {
#ifdef U_BOOT_GIT_BRANCH
    return U_BOOT_GIT_BRANCH;
#else
    return " Unknow ";
#endif
}

static const char *uboot_get_build_time_info(void) {
#ifdef U_BOOT_BUILD_TIME
    return U_BOOT_BUILD_TIME;
#else
    return " Unknow ";
#endif
}

static const char *tvservice_get_build_name_info(void) {
#ifdef U_BOOT_BUILD_NAME
    return U_BOOT_BUILD_NAME;
#else
    return " Unknow ";
#endif
}

void print_build_version_info(void) {
    uboot_version_info_init();

    printf("uboot version:%s\n", uboot_get_version_info());
    printf("uboot git version:%s\n", uboot_get_git_version_info());
    printf("uboot version serial:%llx\n", uboot_get_version_serail());
    printf("uboot git branch:%s\n", uboot_get_git_branch_info());
    printf("uboot Last Changed:%s\n", uboot_get_last_chaned_time_info());
    printf("uboot Last Build:%s\n", uboot_get_build_time_info());
    printf("uboot Builer Name:%s\n", tvservice_get_build_name_info());
    printf("\n");
}
#endif
