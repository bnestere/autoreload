#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "autoreload.h"

void funcy1(void *ptr) {
  int pval = *(int *) ptr;
  printf("Inside funcy1 pval=%d\n", pval);
  sleep(20);
  printf("Finished funcy1\n");
}

int main(int argc, char *argv[]) {
  register_autoreload();

  int nt = 2;
  pthread_t threads[nt];
  for(int i = 0; i < nt; i++) {
    int *arg = malloc(sizeof(int));
    *arg = i;
    if(pthread_create(&threads[i], NULL, funcy1, arg)) {
      perror("Failed to alloc thread");
      exit(EXIT_FAILURE);
    }
  }

  //sleep(1);
  //save_stack(0);
  printf("Waiting for threads\n");
  for(int i = 0; i < nt; i++) {
    int jret = pthread_join(threads[i], NULL);
    if(jret != 0) {
      perror("Failed to join thread");
      if(jret == EINVAL) {
        perror("EINVAL");
      } else if(jret == ESRCH) {
        perror("ESRCH");
      } else if(jret == EDEADLK) {
        perror("EDEADLK");
      } else {
        perror("Unknown");
      }
      exit(EXIT_FAILURE);
    }
  }

  printf("Finished main\n");
  return 0;
}
