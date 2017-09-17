#ifndef SYSTEM_CONSTROLLER_H_INCLUDED
#define SYSTEM_CONSTROLLER_H_INCLUDED
#include "common.h"


int sys_init(const struct sys_config_t* cfg);
int sys_start();
int sys_close();
int sys_destroy();



#endif // SYSTEM_CONSTROLLER_H_INCLUDED
