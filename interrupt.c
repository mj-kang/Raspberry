#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <poll.h>


typedef struct {
	int gpio;
	void (*handler)(void);
}gpio_interrupt_t;

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

//void *thread_function(void *data)
//{
//	int i = 0;
//	while (i < 5)
//	{
//		printf("%s, count %d\n", (char*)data, i);
//		i++;
//		sleep(1);
//	}
//}

int gpio_export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}
int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

int gpio_set_dir(unsigned int gpio, unsigned int out_flag) 
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}

	if (out_flag) 
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	return 0;
}

int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}

int gpio_get_value(unsigned int gpio, unsigned int* value)
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}

	read(fd, &ch, 1);

	if (ch != '0') {
		*value = 1;
	}
	else {
		*value = 0;
	}

	close(fd);
	return 0;
}

int gpio_set_edge(unsigned int gpio, char* edge)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}

	write(fd, edge, strlen(edge) + 1);
	close(fd);
	return 0;
}

int gpio_fd_open(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}
int gpio_fd_close(int fd)
{
	return close(fd);
}

void gpio_handler(void)
{
	printf("HI!\n");
}

//void* CreateGPIOInterruptHandler(gpio_interrupt_t* config)
void* CreateGPIOInterruptHandler(void* cfg)
{
	struct pollfd fdset[2];
	int nfds = 2;
	int gpio_fd, timeout, rc;
	char* buf[MAX_BUF];
	unsigned int gpio;
	int len;
	gpio_interrupt_t* config = (gpio_interrupt_t*)cfg;

	gpio_export(config->gpio);
	gpio_set_dir(config->gpio, 0); //input
	gpio_set_edge(config->gpio, "rising"); //rising, falling, level?
	gpio_fd = gpio_fd_open(config->gpio);

	timeout = POLL_TIMEOUT; //3sec


	while (1) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO; //표준입력 0
		fdset[0].events = POLLIN; 

		fdset[1].fd = gpio_fd;
		fdset[1].events = POLLPRI; 

		rc = poll(fdset, nfds, timeout); 

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return NULL;
		}

		if (rc == 0) {
			printf(".");
		}

		if (fdset[1].revents & POLLPRI) {
			lseek(fdset[1].fd, 0, SEEK_SET); //파일 시작
			len = read(fdset[1].fd, buf, MAX_BUF);

			// printf("\npoll() GPIO %d interrupt occurred\n", gpio);
			//gpio->handler();
			config->handler(); //???
		}

		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int)buf[0]);
		}

		fflush(stdout);
	}

	gpio_fd_close(gpio_fd);
}

int main()
{
	pthread_t p_thread;
	char p1[] = "thread1";
	char p2[] = "main";
	int status; 
	gpio_interrupt_t config;
	config.gpio = 18;
	config.handler = gpio_handler;
	pthread_create(&p_thread, NULL, CreateGPIOInterruptHandler, &config);
	//p_thread에 저 함수를 config매개변수로 가져와?
	//pthread_join(p_thread, (void**)&status);




	//thread_function((void*)p2);

	//pthread_create(&p_thread, NULL, thread_function, (void *)p1);

	if (p_thread < 0)
	{
		perror("nono~\n");
		exit(0);
	}

	

	pthread_join(p_thread, (void**)&status); //status에 종료한것 넣은것임
	printf("bye~\n");

	return 0;
}
