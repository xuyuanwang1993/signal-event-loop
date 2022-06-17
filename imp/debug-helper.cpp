#include "debug-helper.h"
#include <execinfo.h>
#include<stdio.h>
#include<stdlib.h>
#ifdef __cplusplus
extern "C"{
#endif
void
__cyg_profile_func_enter(void *this_func,void * call_site)
{
    (void)call_site;
    auto strings=backtrace_symbols(&this_func,1);
    fprintf(stdout,"call %s begin\r\n",strings[0]);
    free(strings);
}

void
__cyg_profile_func_exit(void *this_func,void * call_site)
{
    (void)call_site;
    auto strings=backtrace_symbols(&this_func,1);
    fprintf(stdout,"call %s over\r\n",strings[0]);
    free(strings);
}
#ifdef __cplusplus
};
#endif
