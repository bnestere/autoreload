#include <acc_timer.h>

/*
 * Registers the executing current thread as reloadable.
 */
void register_reloadable();

/*
 * Register callback functions to invoke upon failure
 * Parameters:
 *   reload_point: function pointer that will be invoked after cleanup_func() function pointer
 *                 has performed the proper cleanup steps.
 *   cleanup_func: function pointer to perform cleanup before reloading program
 *   on_sigbus_notify_threads
 */
void register_autoreload(void (*reload_point)(), void (*cleanup_func)(), acc_timer_ctx_t *acc_timer);

