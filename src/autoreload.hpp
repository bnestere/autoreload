#include <acc_timer.h>

void register_reloadable();
void register_autoreload(void (*f)(), void (*c)(), acc_timer_ctx_t *acc_timer);
void on_sigbus_notify_threads(int);

