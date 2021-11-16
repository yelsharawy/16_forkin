#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include <time.h>

#define NUM_CHILDREN 2

void child(int i, int pipe_to_parent) {
	// makes sure each child is seeded differently:
	srand(rand()+i);
	int wait_for_micro = rand() % (3 * 1000 * 1000 + 1) + 2 * 1000 * 1000;
	float wait_for_sec = wait_for_micro/(1000.0 * 1000);
	printf("[Child %d]\tPID %d, will wait for %.2f seconds\n",
			i, getpid(), wait_for_sec);
	usleep(wait_for_micro);
	printf("[Child %d]\tComplete!\n", i);
	
	// sends seconds waited to parent
	write(pipe_to_parent, &wait_for_sec, sizeof(float));
	close(pipe_to_parent);
	exit(0);
}

// only to be used if `val` is known to be in `arr`
// undefined behavior otherwise
int indexof(int *arr, int val) {
	int i;
	for (i = 0; arr[i] != val; i++);
	return i;
}

int main() {
	// by my design, this seed will affect all children
	// so setting it to a constant value will result in the same wait times
	srand(time(NULL));
	printf("[Parent]\tStarting children\n");
	int read_pipes[NUM_CHILDREN];
	int childpids[NUM_CHILDREN];
	int i;
	for (i = 0; i < NUM_CHILDREN; i++) {
		int pipe_fds[NUM_CHILDREN];
		pipe(pipe_fds);
		
		int childpid;
		if (!(childpid = fork())) {
			// close all read fds
			// (doesn't seem to make a difference, but we don't want
			// this child to have any access to other children's pipes)
			int j;
			for (j = 0; j < i; j++) close(read_pipes[j]);
			
			close(pipe_fds[0]); // child closes own read fd
			child(i, pipe_fds[1]); // gives write fd to child
			// `child` exits, will not continue
		}
		close(pipe_fds[1]); // parent closes write fd
		read_pipes[i] = pipe_fds[0]; // parent keeps read fd
		childpids[i] = childpid;
		printf("[Parent]\tChild %d started with PID %d\n", i, childpid);
	}
	// i decided to wait for both
	for (i = 0; i < NUM_CHILDREN; i++) {
		int wstatus;
		int childpid = wait(&wstatus);
		int childnum = indexof(childpids, childpid);
		printf("[Parent]\tChild %d (PID %d) finished with exit status %d\n",
				childnum, childpid, WEXITSTATUS(wstatus));
		if (!WEXITSTATUS(wstatus)) {
			float f;
			read(read_pipes[childnum], &f, sizeof(float));
			close(read_pipes[childnum]);
			printf("[Parent]\tChild %d waited for %.2f seconds\n", childnum, f);
		}
	}
	printf("[Parent]\tAll finished!\n");
}
