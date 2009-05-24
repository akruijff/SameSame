
#ifndef AK_CONFIGURE_H
#define AK_CONFIGURE_H

// #define WITHOUT_LOGIC
// #define WITH_DISK_STORAGE
// #define HAVE_MMAP
// #define DEBUG

#ifndef TEMP_STORAGE_DIR
#define TEMP_STORAGE_DIR	"/tmp/"
#endif
#ifndef PATH_INIT
#define PATH_INIT		256
#endif
#ifndef STATIC_CACHE_CAPACITY
#define STATIC_CACHE_CAPACITY	8192
#endif

#endif // AK_CONFIGURE_H
