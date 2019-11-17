#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

//add debugging code to driver -seems to be issue with fops
//maybe no open or close method for fops!
//THERES A NULL POINTER ERROR
void main(void) {
	int fd;
	int ret;
	printf("Starting");
	fd = open("/dev/cannon", O_WRONLY | O_CREAT | O_TRUNC);
	if (fd < 0) {
		perror("Problem opening cannon");
		exit(1);
	}
	printf("Opened the cannon");
	ret = write(fd, "up", 3);
	printf("Writing to the cannon");
	if (ret < 0) {
		perror("Problem writing to cannon");
	}
//	close(fd);

}
