/*
 * file.cpp
 */
#include "file.hpp"
#include "string.h"

CFile::CFile() {
	fd = NULL;
	state = FS_CLOSED;
}

CFile::CFile(const char *fileName, char *flag) {
	state = FS_CLOSED;

	if ((fd = fopen(fileName, flag)) != NULL) {
		state = FS_OPENED;

		stat(fileName, &sstat);
	}
}

CFile::~CFile() {
	close();
}

void CFile::open(const char *fileName, char *flag) {
	close();

	if ((fd = fopen(fileName, flag)) != NULL) {
		state = FS_OPENED;

		stat(fileName, &sstat);
	}
}

int CFile::read(void *buff, int len) {
	if (state != FS_OPENED) {
		return -1;
	}

	if (buff == NULL || fd == NULL) {
		return -2;
	}

	return fread(buff, 1, len, fd);
}

int CFile::write(const void *buff, int len) {
	if (state != FS_OPENED) {
		return -1;
	}

	if (buff == NULL || len <= 0 || fd == NULL) {
		return -2;
	}

	return fwrite(buff, 1, len, fd);
}

int CFile::seek(int offset, int whence) {
	if (state != FS_OPENED) {
		return -1;
	}

	return fseek(fd, offset, whence);
}

int CFile::flush() {
	if (state != FS_OPENED) {
		return -1;
	}

	return fflush(fd);
}

int CFile::close() {
	if (state == FS_OPENED && fd != NULL) {
		state = FS_CLOSED;
		int ret = fclose(fd);
		fd = NULL;

		return ret;
	}

	return -1;
}

int CFile::size() {
	if (state != FS_OPENED) {
		return -1;
	}

	return sstat.st_size;
}

time_t CFile::mtime() {
	time_t time = 0;

	if (state != FS_OPENED) {
		return time;
	}

	time = sstat.st_mtime;

	return time;
}

