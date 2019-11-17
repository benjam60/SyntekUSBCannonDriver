#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void main(void) {
	int fd;
	int ret;
	char buffer[10];
	int isACommand;	
	while(1) {
		printf("Type one of the following commands:\n");
		printf("up | down | left | right | fire\n");
		gets(buffer);
		isACommand = (strstr(buffer, "up") != NULL) ||
		       	     (strstr(buffer, "down") != NULL) ||
			     (strstr(buffer, "left") != NULL) ||
			     (strstr(buffer, "right") != NULL) ||
			     (strstr(buffer, "fire") != NULL);
		if (isACommand) {
			fd = open("/dev/cannon", O_WRONLY | O_CREAT | O_TRUNC);
			if (fd < 0) {
				perror("Problem opening cannon");
				exit(1);
			}
			write(fd, buffer, strlen(buffer) + 1);
			write(fd, "stop", 5);
			close(fd);
		}
		else { printf("Not a valid command"); }
	
	}

}
