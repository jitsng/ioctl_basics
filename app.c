

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#define IOCTL_TYPE 15

int main(void)
{
	int fd;
	int cmd;
	int a = getpid();

	printf ("Initial Value: %d\n", a);
	fd = open ("/dev/xyz", O_RDWR);
	
	if (fd < 0)
		perror ("Unable to open the driver\n");
	else
		printf ("File opened\n");
	
	cmd = _IO (IOCTL_TYPE, 20);	
	if (!ioctl(fd, cmd, &a))
		printf ("failed\n");

	printf("Changed value: %d\n", a);
	close (fd);

	return 0;

}
