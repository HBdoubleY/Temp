#ifndef FS_H
#define FS_H

#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>

inline int yaffs_unlink(const char *pathname) {
	return unlink(pathname);
}

/*
 inline int mkdir(const char *pathname, mode_t mode)
 {
 return  1;//yaffs_mkdir(pathname, mode);
 }
 */

inline int rmdir(const char *pathname) {
	return 1; //yaffs_rmdir(pathname);
}

typedef dirent direct;

inline direct *yaffs_readdir(DIR *dp) {
	return readdir(dp);
}

#endif  //FS_H

