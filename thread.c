#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

void *thread_function(void *data)
{
	//pid_t pid;
	//pthread_t tid;
	//pid = getpid();
	//tid = pthread_self();

	//char* thread_name = (char*)data;
	
	int i = 0;
	while (i < 5)
	{
		printf("%s, count %d\n", (char*)data, i);
		i++;
		sleep(1);
	}
}

int main()
{
	pthread_t p_thread;
	char p1[] = "thread1";
	char p2[] = "main";
	int status; //무엇?

	thread_function((void*)p2);

	pthread_create(&p_thread, NULL, thread_function, (void *)p1);

	if (p_thread < 0)
	{
		perror("nono~\n");
		exit(0);
	}

	

	pthread_join(p_thread, (void**)&status);
	printf("bye~\n");

	return 0;
}
