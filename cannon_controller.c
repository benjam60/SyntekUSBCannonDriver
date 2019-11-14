#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
void main(void) {
	int fd;
	int ret;
	fd = open("/dev/cannon", O_WRONLY | O_CREAT | O_TRUNC);
	if (fd < 0) {
		perror("Problem opening cannon");
		exit(1);
	}
	ret = write(fd, "up", 3);
	if (ret < 0) {
		perror("Problem writing to cannon");
	}
	close(fd);

}