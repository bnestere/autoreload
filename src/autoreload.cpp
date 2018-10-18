#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h> 

#include <cstring>

#include <vector>
#include <algorithm>
#include <mutex>

#include "autoreload.hpp"
#include <acc_timer.h>

using namespace std;

extern char *prefix;
extern int is_using_other;

static void (*reload_fptr)(void);
static void (*cleanup_fptr)(void);

static acc_timer_ctx_t *acc_timer = NULL;

vector<int> reset_tids;
mutex reset_tids_lock;

/*
 * Tests if a tid should be reloaded or not
 *
 * TODO: Probably not needed, because all threads that should be
 *       reloaded are in reset_tids..
 */
int should_reload(int test_tid) {
  if(find(reset_tids.begin(), reset_tids.end(), test_tid) != reset_tids.end()) {
    return 1;
  }
  return 0;
}

void register_reloadable() {
  int tid = syscall(__NR_gettid);

  reset_tids_lock.lock();
  reset_tids.push_back(tid);
  reset_tids_lock.unlock();
}

//void do_reload(int signo) {
//  int tid = syscall(__NR_gettid);
//  int pid = getpid();
//  printf("TID %d received SIGUSR1\n", tid);
//
//  cleanup_fptr();
//  reload_fptr();
//
//  // Assume the application finished normally, so just quit..
//  pthread_exit(0);
//}

void on_sigbus_notify_threads(int signo) {
	if(acc_timer != NULL) {
		acc_timer_begin(acc_timer, "Handle SIGBUS");
	}

  int tid = syscall(__NR_gettid);
  printf("TID %d Got SIGBUS\n", tid);
  int pid = getpid();
  char taskdir[64];
  sprintf(taskdir, "/proc/%d/task", pid);
  printf("Did sprintf %s\n", taskdir);

  /*
   * Read threads from the directory to know who should be reloaded.
   * Probably no longer needed because of register_reloadable.
   *
   * TODO: No longer needed
   */
  DIR *d;
  struct dirent *dir;
  d = opendir(taskdir);
  printf("Reading directories in %s\n", taskdir);
  int tids_to_signal[1024];
  int n_tids = 0;
  if(d) {
    while ((dir = readdir(d)) != NULL) {
      if(strcmp(dir->d_name,".") != 0 && strcmp(dir->d_name,"..") != 0) {
        int subthread_no = atoi(dir->d_name);
        tids_to_signal[n_tids] = subthread_no;
        n_tids += 1;
      }
    }
    closedir(d);
  }

  if(n_tids == 1) {
    // There is only one tid, it is the main pid, so we just want to restart ourself
    printf("Signalling self %d to reload stack with signal %d\n", tid, SIGUSR1);
    syscall(SYS_tgkill, pid, tid, SIGUSR2);
    syscall(SYS_tgkill, pid, tid, SIGUSR1);
  } else{

    for(int tidx = 0; tidx < n_tids; tidx++) {
      int tid_to_signal = tids_to_signal[tidx];
      if(!should_reload(tid_to_signal)) {
        // Not a tid on the list to signal
        continue; 
      }

      if(tid_to_signal != tid) {
        if(tid_to_signal == pid) {
          // Main pid only resets heap, not stack
          printf("Signalling main pid %d to reload heap with signal %d\n", tid_to_signal, SIGUSR2);
          // bnestere: I don't think we want to signal the main pid if it is waiting..
          syscall(SYS_tgkill, pid, tid_to_signal, SIGUSR2);
        } else {
          // We can't signal ourself until we've signaled everyone else
          printf("Signalling other thread %d to reload stack with signal %d\n", tid_to_signal, SIGUSR1);
          syscall(SYS_tgkill, pid, tid_to_signal, SIGUSR1);
        }
      }
    }

    if(tid != pid) {
      printf("Signalling self %d to reload stack with signal %d\n", tid, SIGUSR1);
      syscall(SYS_tgkill, pid, tid, SIGUSR1);
    } else {
      // Main pid only resets heap, not stack
      printf("Signalling main pid %d to reload heap with signal %d\n", tid, SIGUSR2);
      syscall(SYS_tgkill, pid, tid, SIGUSR2);
    }
  }
}

/*
 * Handler to just perform cleanup
 */
void cleanup_handler(int signo) {
  printf("cleanup_handler()::begin\n");
  cleanup_fptr();
  printf("cleanup_handler()::end\n");
}

/*
 * Use sigactions to set up signal handlers for reloading
 */
void register_autoreload(void (*_reload_fptr)(), void (*_cleanup_fptr)(), acc_timer_ctx_t *_acc_timer) {

  printf("register_autoreload()::begin\n");

  static struct sigaction sa = {};
  sa.sa_handler = on_sigbus_notify_threads;
  sa.sa_flags = SA_NODEFER;  
  sigaction(SIGBUS, &sa, NULL);;

  static struct sigaction sa2 = {};
  sa2.sa_handler = cleanup_handler;
  sa2.sa_flags = SA_NODEFER;  
  sigaction(SIGUSR1, &sa2, NULL);;

  static struct sigaction sa3 = {};
  sa3.sa_handler = cleanup_handler;
  sa3.sa_flags = SA_NODEFER;  
  sigaction(SIGUSR2, &sa3, NULL);;

  reload_fptr = _reload_fptr;
  cleanup_fptr = _cleanup_fptr;
	acc_timer = _acc_timer;

  printf("register_autoreload()::end\n");
}
