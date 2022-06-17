#ifndef DEBUGHELPER_H
#define DEBUGHELPER_H
//QMAKE_LFLAGS += -rdynamic
#ifdef __cplusplus
extern "C"{
#endif
 void __attribute__((no_instrument_function))
__cyg_profile_func_enter(void *this_func,void * call_site) ;

 void __attribute__((__no_instrument_function__))
__cyg_profile_func_exit(void *this_func,void * call_site);
#ifdef __cplusplus
};
#endif
#endif // DEBUGHELPER_H
