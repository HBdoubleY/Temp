#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <sys/stat.h>

class CFile {
public:
	enum {
		FS_OPENED, FS_CLOSED,
	};

public:
	CFile();
	CFile(const char *fileName, char *flag = (char*) "rb");
	virtual ~CFile();

	int getState() {
		return state;
	}

	void open(const char *fileName, char *flag = (char*) "rb");
	FILE *getFd() {
		return fd;
	}
	int read(void *buff, int len);
	int write(const void *buff, int len);
	int seek(int offset, int whence);
	int flush();
	int close();
	int size();

	time_t mtime();

private:
	FILE *fd;
	struct stat sstat;
	int state;

};

#endif /* FILE_H_ */
