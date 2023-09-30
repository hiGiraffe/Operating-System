#include <lib.h>

int main() {
	// int a = 0;
	// if (fork() == 0) {
	// 	if (fork() == 0 && fork() == 0 && fork() == 0 && fork() == 0 && fork() == 0) {
	// 		debugf("!@this is child %x :a:%d\n", syscall_getenvid(), a);
	// 	}
	// 	return 0;
	// }
	// debugf("!@ancestor exit\n");
	// return 0;
	for (int i = 0; i < 9; i++) {
		int who = fork();
		debugf("fork  ok1 %d who = %d \n",i,who);
		if (who == 0) {
			debugf("I'm son!\n");
			// barrier_wait();
			syscall_panic("Wrong block!");
		}
	}
	debugf("I'm finished!\n");
	return 0;
}
