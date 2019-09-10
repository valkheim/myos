#ifndef _KERNEL_SECURITY_H
#define _KERNEL_SECURITY_H

void *__stack_chk_guard = 0;

void __stack_chk_fail(void);

#endif
