/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Generated from the curated PocketMage SDK export list.
 * DO NOT EDIT: regenerate with tools/symbols.py.
 */

#include <stddef.h>

#include "private/elf_symbol.h"

#include <pm_app_api.h>
#include <pm_sdk_app.h>

/* Runtime symbols: unmangled prototypes so the emitted reference is
 * the literal archive symbol, whatever its real C++ type. The
 * builtin mismatch warning is expected and harmless, since only the
 * address is taken and never called through this type. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"
extern "C" {
int _Balloc();
int _Bfree();
int _Unwind_Backtrace();
int _Unwind_DeleteException();
int _Unwind_FindEnclosingFunction();
int _Unwind_Find_FDE();
int _Unwind_ForcedUnwind();
int _Unwind_GetCFA();
int _Unwind_GetDataRelBase();
int _Unwind_GetGR();
int _Unwind_GetIP();
int _Unwind_GetIPInfo();
int _Unwind_GetLanguageSpecificData();
int _Unwind_GetRegionStart();
int _Unwind_GetTextRelBase();
int _Unwind_RaiseException();
int _Unwind_Resume();
int _Unwind_Resume_or_Rethrow();
int _Unwind_SetGR();
int _Unwind_SetIP();
int _ZN10__cxxabiv111__terminateEPFvvE();
int _ZN10__cxxabiv112__unexpectedEPFvvE();
int _ZN10__cxxabiv117__class_type_infoD0Ev();
int _ZN10__cxxabiv117__class_type_infoD1Ev();
int _ZN10__cxxabiv117__class_type_infoD2Ev();
int _ZN10__cxxabiv120__si_class_type_infoD0Ev();
int _ZN10__cxxabiv120__si_class_type_infoD1Ev();
int _ZN10__cxxabiv120__si_class_type_infoD2Ev();
int _ZNK10__cxxabiv117__class_type_info10__do_catchEPKSt9type_infoPPvj();
int _ZNK10__cxxabiv117__class_type_info11__do_upcastEPKS0_PKvRNS0_15__upcast_resultE();
int _ZNK10__cxxabiv117__class_type_info11__do_upcastEPKS0_PPv();
int _ZNK10__cxxabiv117__class_type_info12__do_dyncastEiNS0_10__sub_kindEPKS0_PKvS3_S5_RNS0_16__dyncast_resultE();
int _ZNK10__cxxabiv117__class_type_info20__do_find_public_srcEiPKvPKS0_S2_();
int _ZNK10__cxxabiv120__si_class_type_info11__do_upcastEPKNS_17__class_type_infoEPKvRNS1_15__upcast_resultE();
int _ZNK10__cxxabiv120__si_class_type_info12__do_dyncastEiNS_17__class_type_info10__sub_kindEPKS1_PKvS4_S6_RNS1_16__dyncast_resultE();
int _ZNK10__cxxabiv120__si_class_type_info20__do_find_public_srcEiPKvPKNS_17__class_type_infoES2_();
int _ZNKSt11logic_error4whatEv();
int _ZNKSt13runtime_error4whatEv();
int _ZNKSt17bad_function_call4whatEv();
int _ZNKSt20bad_array_new_length4whatEv();
int _ZNKSt9bad_alloc4whatEv();
int _ZNKSt9type_info14__is_pointer_pEv();
int _ZNKSt9type_info15__is_function_pEv();
int _ZNSt11logic_errorC1EPKc();
int _ZNSt11logic_errorC2EPKc();
int _ZNSt11logic_errorD0Ev();
int _ZNSt11logic_errorD1Ev();
int _ZNSt11logic_errorD2Ev();
int _ZNSt12length_errorC1EPKc();
int _ZNSt12length_errorC2EPKc();
int _ZNSt12length_errorD0Ev();
int _ZNSt12length_errorD1Ev();
int _ZNSt12length_errorD2Ev();
int _ZNSt12out_of_rangeC1EPKc();
int _ZNSt12out_of_rangeC2EPKc();
int _ZNSt12out_of_rangeD0Ev();
int _ZNSt12out_of_rangeD1Ev();
int _ZNSt12out_of_rangeD2Ev();
int _ZNSt17bad_function_callD0Ev();
int _ZNSt17bad_function_callD1Ev();
int _ZNSt17bad_function_callD2Ev();
int _ZNSt19_Sp_make_shared_tag5_S_eqERKSt9type_info();
int _ZNSt20bad_array_new_lengthD0Ev();
int _ZNSt20bad_array_new_lengthD1Ev();
int _ZNSt20bad_array_new_lengthD2Ev();
int _ZNSt9bad_allocD0Ev();
int _ZNSt9bad_allocD1Ev();
int _ZNSt9bad_allocD2Ev();
int _ZNSt9exceptionD1Ev();
int _ZNSt9exceptionD2Ev();
int _ZNSt9type_infoD1Ev();
int _ZNSt9type_infoD2Ev();
int _ZSt10unexpectedv();
int _ZSt13get_terminatev();
int _ZSt14get_unexpectedv();
int _ZSt15get_new_handlerv();
int _ZSt17__throw_bad_allocv();
int _ZSt19__throw_logic_errorPKc();
int _ZSt20__throw_length_errorPKc();
int _ZSt20__throw_out_of_rangePKc();
int _ZSt24__throw_out_of_range_fmtPKcz();
int _ZSt25__throw_bad_function_callv();
int _ZSt28__throw_bad_array_new_lengthv();
int _ZSt9terminatev();
int _ZdaPv();
int _ZdlPv();
int _ZdlPvj();
int _Znaj();
int _ZnajRKSt9nothrow_t();
int _Znwj();
int _ZnwjRKSt9nothrow_t();
int __adddf3();
int __any_on();
int __ascii_mbtowc();
int __ascii_wctomb();
int __ashldi3();
int __assert_func();
int __atomic_compare_exchange_1();
int __atomic_compare_exchange_4();
int __atomic_exchange_4();
int __atomic_fetch_add_2();
int __atomic_fetch_add_4();
int __atomic_fetch_and_8();
int __atomic_fetch_or_8();
int __atomic_fetch_sub_4();
int __atomic_s32c1i_compare_exchange_1();
int __atomic_s32c1i_compare_exchange_4();
int __atomic_s32c1i_exchange_4();
int __atomic_s32c1i_fetch_add_2();
int __atomic_s32c1i_fetch_add_4();
int __atomic_s32c1i_fetch_sub_4();
int __b2d();
int __bswapdi2();
int __bswapsi2();
int __call_exitprocs();
int __copybits();
int __ctzdi2();
int __cxa_allocate_exception();
int __cxa_atexit();
int __cxa_begin_catch();
int __cxa_call_terminate();
int __cxa_end_catch();
int __cxa_free_exception();
int __cxa_get_globals();
int __cxa_get_globals_fast();
int __cxa_guard_abort();
int __cxa_guard_acquire();
int __cxa_guard_release();
int __cxa_init_primary_exception();
int __cxa_rethrow();
int __cxa_throw();
int __cxa_throw_bad_array_new_length();
int __cxx_eh_arena_size_get();
int __d2b();
int __deregister_frame();
int __deregister_frame_info();
int __deregister_frame_info_bases();
int __divdf3();
int __divdi3();
int __divsf3();
int __env_lock();
int __env_unlock();
int __eqdf2();
int __errno();
int __extendsfdf2();
int __ffsdi2();
int __fixdfdi();
int __fixdfsi();
int __fixunsdfsi();
int __floatdidf();
int __floatdisf();
int __floatsidf();
int __floatundidf();
int __floatundisf();
int __floatunsidf();
int __gedf2();
int __gethex();
int __gettzinfo();
int __gtdf2();
int __gxx_personality_v0();
int __hexdig_fun();
int __hexnan();
int __hi0bits();
int __i2b();
int __ieee754_rem_pio2f();
int __ieee754_sqrtf();
int __kernel_cosf();
int __kernel_rem_pio2f();
int __kernel_sinf();
int __ledf2();
int __lo0bits();
int __locale_mb_cur_max();
int __lshift();
int __lshrdi3();
int __ltdf2();
int __match();
int __mcmp();
int __mdiff();
int __moddi3();
int __muldf3();
int __multadd();
int __multiply();
int __nedf2();
int __popcountsi2();
int __pow5mult();
int __ratio();
int __register_exitproc();
int __register_frame();
int __register_frame_info();
int __register_frame_info_bases();
int __register_frame_info_table();
int __register_frame_info_table_bases();
int __register_frame_table();
int __retarget_lock_acquire();
int __retarget_lock_acquire_recursive();
int __retarget_lock_close();
int __retarget_lock_close_recursive();
int __retarget_lock_init();
int __retarget_lock_init_recursive();
int __retarget_lock_release();
int __retarget_lock_release_recursive();
int __retarget_lock_try_acquire();
int __retarget_lock_try_acquire_recursive();
int __s2b();
int __sccl();
int __sclose();
int __seofread();
int __sflags();
int __sflush_r();
int __sfp();
int __sfp_lock_acquire();
int __sfp_lock_release();
int __sfvwrite_r();
int __sinit();
int __smakebuf_r();
int __sprint_r();
int __sread();
int __srefill_r();
int __sseek();
int __ssprint_r();
int __ssrefill_r();
int __ssvfiscanf_r();
int __stack_chk_fail();
int __strtok_r();
int __subdf3();
int __submore();
int __swbuf();
int __swbuf_r();
int __swhatbuf_r();
int __swrite();
int __swsetup_r();
int __truncdfsf2();
int __tz_lock();
int __tz_unlock();
int __tzcalc_limits();
int __udivdi3();
int __ulp();
int __umoddi3();
int __unorddf2();
int __xtensa_libgcc_window_spill();
int __xtensa_nonlocal_goto();
int __xtensa_sync_caches();
int _calloc_r();
int _close_r();
int _dtoa_r();
int _exit();
int _fclose_r();
int _fcntl_r();
int _fdopen_r();
int _fflush_r();
int _fgets_r();
int _findenv_r();
int _fopen_r();
int _fputc_r();
int _fputs_r();
int _fread_r();
int _free_r();
int _fseek_r();
int _fseeko_r();
int _fstat_r();
int _ftello_r();
int _fwalk_sglue();
int _fwrite_r();
int _getenv_r();
int _getpid_r();
int _gettimeofday_r();
int _kill_r();
int _link_r();
int _localeconv_r();
int _lock_acquire();
int _lock_acquire_recursive();
int _lock_close();
int _lock_close_recursive();
int _lock_init();
int _lock_release();
int _lock_release_recursive();
int _lock_try_acquire();
int _lock_try_acquire_recursive();
int _lseek_r();
int _malloc_r();
int _mbrtowc_r();
int _open_r();
int _putc_r();
int _puts_r();
int _raise_r();
int _read_r();
int _realloc_r();
int _reclaim_reent();
int _rename_r();
int _sbrk_r();
int _sfread_r();
int _stat_r();
int _strerror_r();
int _strtod_l();
int _strtol_r();
int _strtoll_r();
int _strtoul_r();
int _strtoull_r();
int _sungetc_r();
int _svfiprintf_r();
int _svfprintf_r();
int _system_r();
int _times_r();
int _tzset_unlocked();
int _tzset_unlocked_r();
int _unlink_r();
int _user_strerror();
int _vfprintf_r();
int _vsnprintf_r();
int _write_r();
int abort();
int abs();
int atof();
int atoi();
int atol();
int bzero();
int calloc();
int ceil();
int cfree();
int clock_gettime();
int close();
int cosf();
int div();
int esp_libc_include_assert_impl();
int esp_libc_include_getentropy_impl();
int esp_libc_include_heap_impl();
int esp_libc_include_init_funcs();
int esp_libc_include_pthread_impl();
int esp_libc_include_reent_syscalls_impl();
int esp_libc_include_syscalls_impl();
int esp_libc_init();
int esp_libc_init_funcs();
int esp_libc_init_global_stdio();
int esp_libc_locks_init();
int esp_libc_time_init();
int esp_newlib_init();
int esp_newlib_locks_init();
int esp_newlib_time_init();
int esp_reent_init();
int esp_set_time_from_rtc();
int esp_setup_newlib_syscalls();
int esp_sync_timekeeping_timers();
int esp_time_impl_get_boot_time();
int esp_time_impl_get_time();
int esp_time_impl_get_time_since_boot();
int esp_time_impl_set_boot_time();
int exit();
int explicit_bzero();
int fabsf();
int fclose();
int fcntl();
int fdopen();
int feof();
int fflush();
int fgets();
int fileno();
int floorf();
int fopen();
int fprintf();
int fputc();
int fputs();
int fread();
int free();
int frexp();
int fseek();
int fstat();
int fsync();
int ftell();
int fwrite();
int getenv();
int getopt_long();
int gettimeofday();
int gmtime_r();
int isalnum();
int isalpha();
int isgraph();
int isprint();
int isspace();
int iswspace();
int iswspace_l();
int isxdigit();
int itoa();
int labs();
int localtime();
int localtime_r();
int longjmp();
int malloc();
int memchr();
int memcmp();
int memcpy();
int memmove();
int memset();
int mktime();
int nan();
int open();
int printf();
int putchar();
int puts();
int pvalloc();
int qsort();
int rand();
int read();
int realloc();
int rename();
int roundf();
int scalbnf();
int setjmp();
int settimeofday();
int setvbuf();
int sinf();
int siscanf();
int sleep();
int sniprintf();
int snprintf();
int sprintf();
int sqrtf();
int srand();
int stat();
int strcasecmp();
int strcasestr();
int strcat();
int strchr();
int strcmp();
int strcpy();
int strcspn();
int strdup();
int strerror();
int strerror_r();
int strftime();
int strlcat();
int strlcpy();
int strlen();
int strncasecmp();
int strncat();
int strncmp();
int strncpy();
int strndup();
int strnlen();
int strpbrk();
int strrchr();
int strsep();
int strspn();
int strstr();
int strtod();
int strtok();
int strtok_r();
int strtol();
int strtoll();
int strtoul();
int strtoull();
int time();
int tolower();
int toupper();
int unlink();
int usleep();
int utoa();
int valloc();
int vfprintf();
int vprintf();
int vsnprintf();
int write();
}
#pragma GCC diagnostic pop

/* Available ELF symbols table: g_customer_elfsyms */
/* C linkage: the loader core is C and references this unmangled. */
/* Explicit extern: namespace-scope const defaults to internal
 * linkage in C++, which would let the compiler discard the table. */

extern "C" {

extern const struct esp_elfsym g_customer_elfsyms[] = {
    ESP_ELFSYM_EXPORT(_Balloc),
    ESP_ELFSYM_EXPORT(_Bfree),
    ESP_ELFSYM_EXPORT(_Unwind_Backtrace),
    ESP_ELFSYM_EXPORT(_Unwind_DeleteException),
    ESP_ELFSYM_EXPORT(_Unwind_FindEnclosingFunction),
    ESP_ELFSYM_EXPORT(_Unwind_Find_FDE),
    ESP_ELFSYM_EXPORT(_Unwind_ForcedUnwind),
    ESP_ELFSYM_EXPORT(_Unwind_GetCFA),
    ESP_ELFSYM_EXPORT(_Unwind_GetDataRelBase),
    ESP_ELFSYM_EXPORT(_Unwind_GetGR),
    ESP_ELFSYM_EXPORT(_Unwind_GetIP),
    ESP_ELFSYM_EXPORT(_Unwind_GetIPInfo),
    ESP_ELFSYM_EXPORT(_Unwind_GetLanguageSpecificData),
    ESP_ELFSYM_EXPORT(_Unwind_GetRegionStart),
    ESP_ELFSYM_EXPORT(_Unwind_GetTextRelBase),
    ESP_ELFSYM_EXPORT(_Unwind_RaiseException),
    ESP_ELFSYM_EXPORT(_Unwind_Resume),
    ESP_ELFSYM_EXPORT(_Unwind_Resume_or_Rethrow),
    ESP_ELFSYM_EXPORT(_Unwind_SetGR),
    ESP_ELFSYM_EXPORT(_Unwind_SetIP),
    ESP_ELFSYM_EXPORT(_ZN10__cxxabiv111__terminateEPFvvE),
    ESP_ELFSYM_EXPORT(_ZN10__cxxabiv112__unexpectedEPFvvE),
    ESP_ELFSYM_EXPORT(_ZN10__cxxabiv117__class_type_infoD0Ev),
    ESP_ELFSYM_EXPORT(_ZN10__cxxabiv117__class_type_infoD1Ev),
    ESP_ELFSYM_EXPORT(_ZN10__cxxabiv117__class_type_infoD2Ev),
    ESP_ELFSYM_EXPORT(_ZN10__cxxabiv120__si_class_type_infoD0Ev),
    ESP_ELFSYM_EXPORT(_ZN10__cxxabiv120__si_class_type_infoD1Ev),
    ESP_ELFSYM_EXPORT(_ZN10__cxxabiv120__si_class_type_infoD2Ev),
    ESP_ELFSYM_EXPORT(_ZNK10__cxxabiv117__class_type_info10__do_catchEPKSt9type_infoPPvj),
    ESP_ELFSYM_EXPORT(_ZNK10__cxxabiv117__class_type_info11__do_upcastEPKS0_PKvRNS0_15__upcast_resultE),
    ESP_ELFSYM_EXPORT(_ZNK10__cxxabiv117__class_type_info11__do_upcastEPKS0_PPv),
    ESP_ELFSYM_EXPORT(_ZNK10__cxxabiv117__class_type_info12__do_dyncastEiNS0_10__sub_kindEPKS0_PKvS3_S5_RNS0_16__dyncast_resultE),
    ESP_ELFSYM_EXPORT(_ZNK10__cxxabiv117__class_type_info20__do_find_public_srcEiPKvPKS0_S2_),
    ESP_ELFSYM_EXPORT(_ZNK10__cxxabiv120__si_class_type_info11__do_upcastEPKNS_17__class_type_infoEPKvRNS1_15__upcast_resultE),
    ESP_ELFSYM_EXPORT(_ZNK10__cxxabiv120__si_class_type_info12__do_dyncastEiNS_17__class_type_info10__sub_kindEPKS1_PKvS4_S6_RNS1_16__dyncast_resultE),
    ESP_ELFSYM_EXPORT(_ZNK10__cxxabiv120__si_class_type_info20__do_find_public_srcEiPKvPKNS_17__class_type_infoES2_),
    ESP_ELFSYM_EXPORT(_ZNKSt11logic_error4whatEv),
    ESP_ELFSYM_EXPORT(_ZNKSt13runtime_error4whatEv),
    ESP_ELFSYM_EXPORT(_ZNKSt17bad_function_call4whatEv),
    ESP_ELFSYM_EXPORT(_ZNKSt20bad_array_new_length4whatEv),
    ESP_ELFSYM_EXPORT(_ZNKSt9bad_alloc4whatEv),
    ESP_ELFSYM_EXPORT(_ZNKSt9type_info14__is_pointer_pEv),
    ESP_ELFSYM_EXPORT(_ZNKSt9type_info15__is_function_pEv),
    ESP_ELFSYM_EXPORT(_ZNSt11logic_errorC1EPKc),
    ESP_ELFSYM_EXPORT(_ZNSt11logic_errorC2EPKc),
    ESP_ELFSYM_EXPORT(_ZNSt11logic_errorD0Ev),
    ESP_ELFSYM_EXPORT(_ZNSt11logic_errorD1Ev),
    ESP_ELFSYM_EXPORT(_ZNSt11logic_errorD2Ev),
    ESP_ELFSYM_EXPORT(_ZNSt12length_errorC1EPKc),
    ESP_ELFSYM_EXPORT(_ZNSt12length_errorC2EPKc),
    ESP_ELFSYM_EXPORT(_ZNSt12length_errorD0Ev),
    ESP_ELFSYM_EXPORT(_ZNSt12length_errorD1Ev),
    ESP_ELFSYM_EXPORT(_ZNSt12length_errorD2Ev),
    ESP_ELFSYM_EXPORT(_ZNSt12out_of_rangeC1EPKc),
    ESP_ELFSYM_EXPORT(_ZNSt12out_of_rangeC2EPKc),
    ESP_ELFSYM_EXPORT(_ZNSt12out_of_rangeD0Ev),
    ESP_ELFSYM_EXPORT(_ZNSt12out_of_rangeD1Ev),
    ESP_ELFSYM_EXPORT(_ZNSt12out_of_rangeD2Ev),
    ESP_ELFSYM_EXPORT(_ZNSt17bad_function_callD0Ev),
    ESP_ELFSYM_EXPORT(_ZNSt17bad_function_callD1Ev),
    ESP_ELFSYM_EXPORT(_ZNSt17bad_function_callD2Ev),
    ESP_ELFSYM_EXPORT(_ZNSt19_Sp_make_shared_tag5_S_eqERKSt9type_info),
    ESP_ELFSYM_EXPORT(_ZNSt20bad_array_new_lengthD0Ev),
    ESP_ELFSYM_EXPORT(_ZNSt20bad_array_new_lengthD1Ev),
    ESP_ELFSYM_EXPORT(_ZNSt20bad_array_new_lengthD2Ev),
    ESP_ELFSYM_EXPORT(_ZNSt9bad_allocD0Ev),
    ESP_ELFSYM_EXPORT(_ZNSt9bad_allocD1Ev),
    ESP_ELFSYM_EXPORT(_ZNSt9bad_allocD2Ev),
    ESP_ELFSYM_EXPORT(_ZNSt9exceptionD1Ev),
    ESP_ELFSYM_EXPORT(_ZNSt9exceptionD2Ev),
    ESP_ELFSYM_EXPORT(_ZNSt9type_infoD1Ev),
    ESP_ELFSYM_EXPORT(_ZNSt9type_infoD2Ev),
    ESP_ELFSYM_EXPORT(_ZSt10unexpectedv),
    ESP_ELFSYM_EXPORT(_ZSt13get_terminatev),
    ESP_ELFSYM_EXPORT(_ZSt14get_unexpectedv),
    ESP_ELFSYM_EXPORT(_ZSt15get_new_handlerv),
    ESP_ELFSYM_EXPORT(_ZSt17__throw_bad_allocv),
    ESP_ELFSYM_EXPORT(_ZSt19__throw_logic_errorPKc),
    ESP_ELFSYM_EXPORT(_ZSt20__throw_length_errorPKc),
    ESP_ELFSYM_EXPORT(_ZSt20__throw_out_of_rangePKc),
    ESP_ELFSYM_EXPORT(_ZSt24__throw_out_of_range_fmtPKcz),
    ESP_ELFSYM_EXPORT(_ZSt25__throw_bad_function_callv),
    ESP_ELFSYM_EXPORT(_ZSt28__throw_bad_array_new_lengthv),
    ESP_ELFSYM_EXPORT(_ZSt9terminatev),
    ESP_ELFSYM_EXPORT(_ZdaPv),
    ESP_ELFSYM_EXPORT(_ZdlPv),
    ESP_ELFSYM_EXPORT(_ZdlPvj),
    ESP_ELFSYM_EXPORT(_Znaj),
    ESP_ELFSYM_EXPORT(_ZnajRKSt9nothrow_t),
    ESP_ELFSYM_EXPORT(_Znwj),
    ESP_ELFSYM_EXPORT(_ZnwjRKSt9nothrow_t),
    ESP_ELFSYM_EXPORT(__adddf3),
    ESP_ELFSYM_EXPORT(__any_on),
    ESP_ELFSYM_EXPORT(__ascii_mbtowc),
    ESP_ELFSYM_EXPORT(__ascii_wctomb),
    ESP_ELFSYM_EXPORT(__ashldi3),
    ESP_ELFSYM_EXPORT(__assert_func),
    ESP_ELFSYM_EXPORT(__atomic_compare_exchange_1),
    ESP_ELFSYM_EXPORT(__atomic_compare_exchange_4),
    ESP_ELFSYM_EXPORT(__atomic_exchange_4),
    ESP_ELFSYM_EXPORT(__atomic_fetch_add_2),
    ESP_ELFSYM_EXPORT(__atomic_fetch_add_4),
    ESP_ELFSYM_EXPORT(__atomic_fetch_and_8),
    ESP_ELFSYM_EXPORT(__atomic_fetch_or_8),
    ESP_ELFSYM_EXPORT(__atomic_fetch_sub_4),
    ESP_ELFSYM_EXPORT(__atomic_s32c1i_compare_exchange_1),
    ESP_ELFSYM_EXPORT(__atomic_s32c1i_compare_exchange_4),
    ESP_ELFSYM_EXPORT(__atomic_s32c1i_exchange_4),
    ESP_ELFSYM_EXPORT(__atomic_s32c1i_fetch_add_2),
    ESP_ELFSYM_EXPORT(__atomic_s32c1i_fetch_add_4),
    ESP_ELFSYM_EXPORT(__atomic_s32c1i_fetch_sub_4),
    ESP_ELFSYM_EXPORT(__b2d),
    ESP_ELFSYM_EXPORT(__bswapdi2),
    ESP_ELFSYM_EXPORT(__bswapsi2),
    ESP_ELFSYM_EXPORT(__call_exitprocs),
    ESP_ELFSYM_EXPORT(__copybits),
    ESP_ELFSYM_EXPORT(__ctzdi2),
    ESP_ELFSYM_EXPORT(__cxa_allocate_exception),
    ESP_ELFSYM_EXPORT(__cxa_atexit),
    ESP_ELFSYM_EXPORT(__cxa_begin_catch),
    ESP_ELFSYM_EXPORT(__cxa_call_terminate),
    ESP_ELFSYM_EXPORT(__cxa_end_catch),
    ESP_ELFSYM_EXPORT(__cxa_free_exception),
    ESP_ELFSYM_EXPORT(__cxa_get_globals),
    ESP_ELFSYM_EXPORT(__cxa_get_globals_fast),
    ESP_ELFSYM_EXPORT(__cxa_guard_abort),
    ESP_ELFSYM_EXPORT(__cxa_guard_acquire),
    ESP_ELFSYM_EXPORT(__cxa_guard_release),
    ESP_ELFSYM_EXPORT(__cxa_init_primary_exception),
    ESP_ELFSYM_EXPORT(__cxa_rethrow),
    ESP_ELFSYM_EXPORT(__cxa_throw),
    ESP_ELFSYM_EXPORT(__cxa_throw_bad_array_new_length),
    ESP_ELFSYM_EXPORT(__cxx_eh_arena_size_get),
    ESP_ELFSYM_EXPORT(__d2b),
    ESP_ELFSYM_EXPORT(__deregister_frame),
    ESP_ELFSYM_EXPORT(__deregister_frame_info),
    ESP_ELFSYM_EXPORT(__deregister_frame_info_bases),
    ESP_ELFSYM_EXPORT(__divdf3),
    ESP_ELFSYM_EXPORT(__divdi3),
    ESP_ELFSYM_EXPORT(__divsf3),
    ESP_ELFSYM_EXPORT(__env_lock),
    ESP_ELFSYM_EXPORT(__env_unlock),
    ESP_ELFSYM_EXPORT(__eqdf2),
    ESP_ELFSYM_EXPORT(__errno),
    ESP_ELFSYM_EXPORT(__extendsfdf2),
    ESP_ELFSYM_EXPORT(__ffsdi2),
    ESP_ELFSYM_EXPORT(__fixdfdi),
    ESP_ELFSYM_EXPORT(__fixdfsi),
    ESP_ELFSYM_EXPORT(__fixunsdfsi),
    ESP_ELFSYM_EXPORT(__floatdidf),
    ESP_ELFSYM_EXPORT(__floatdisf),
    ESP_ELFSYM_EXPORT(__floatsidf),
    ESP_ELFSYM_EXPORT(__floatundidf),
    ESP_ELFSYM_EXPORT(__floatundisf),
    ESP_ELFSYM_EXPORT(__floatunsidf),
    ESP_ELFSYM_EXPORT(__gedf2),
    ESP_ELFSYM_EXPORT(__gethex),
    ESP_ELFSYM_EXPORT(__gettzinfo),
    ESP_ELFSYM_EXPORT(__gtdf2),
    ESP_ELFSYM_EXPORT(__gxx_personality_v0),
    ESP_ELFSYM_EXPORT(__hexdig_fun),
    ESP_ELFSYM_EXPORT(__hexnan),
    ESP_ELFSYM_EXPORT(__hi0bits),
    ESP_ELFSYM_EXPORT(__i2b),
    ESP_ELFSYM_EXPORT(__ieee754_rem_pio2f),
    ESP_ELFSYM_EXPORT(__ieee754_sqrtf),
    ESP_ELFSYM_EXPORT(__kernel_cosf),
    ESP_ELFSYM_EXPORT(__kernel_rem_pio2f),
    ESP_ELFSYM_EXPORT(__kernel_sinf),
    ESP_ELFSYM_EXPORT(__ledf2),
    ESP_ELFSYM_EXPORT(__lo0bits),
    ESP_ELFSYM_EXPORT(__locale_mb_cur_max),
    ESP_ELFSYM_EXPORT(__lshift),
    ESP_ELFSYM_EXPORT(__lshrdi3),
    ESP_ELFSYM_EXPORT(__ltdf2),
    ESP_ELFSYM_EXPORT(__match),
    ESP_ELFSYM_EXPORT(__mcmp),
    ESP_ELFSYM_EXPORT(__mdiff),
    ESP_ELFSYM_EXPORT(__moddi3),
    ESP_ELFSYM_EXPORT(__muldf3),
    ESP_ELFSYM_EXPORT(__multadd),
    ESP_ELFSYM_EXPORT(__multiply),
    ESP_ELFSYM_EXPORT(__nedf2),
    ESP_ELFSYM_EXPORT(__popcountsi2),
    ESP_ELFSYM_EXPORT(__pow5mult),
    ESP_ELFSYM_EXPORT(__ratio),
    ESP_ELFSYM_EXPORT(__register_exitproc),
    ESP_ELFSYM_EXPORT(__register_frame),
    ESP_ELFSYM_EXPORT(__register_frame_info),
    ESP_ELFSYM_EXPORT(__register_frame_info_bases),
    ESP_ELFSYM_EXPORT(__register_frame_info_table),
    ESP_ELFSYM_EXPORT(__register_frame_info_table_bases),
    ESP_ELFSYM_EXPORT(__register_frame_table),
    ESP_ELFSYM_EXPORT(__retarget_lock_acquire),
    ESP_ELFSYM_EXPORT(__retarget_lock_acquire_recursive),
    ESP_ELFSYM_EXPORT(__retarget_lock_close),
    ESP_ELFSYM_EXPORT(__retarget_lock_close_recursive),
    ESP_ELFSYM_EXPORT(__retarget_lock_init),
    ESP_ELFSYM_EXPORT(__retarget_lock_init_recursive),
    ESP_ELFSYM_EXPORT(__retarget_lock_release),
    ESP_ELFSYM_EXPORT(__retarget_lock_release_recursive),
    ESP_ELFSYM_EXPORT(__retarget_lock_try_acquire),
    ESP_ELFSYM_EXPORT(__retarget_lock_try_acquire_recursive),
    ESP_ELFSYM_EXPORT(__s2b),
    ESP_ELFSYM_EXPORT(__sccl),
    ESP_ELFSYM_EXPORT(__sclose),
    ESP_ELFSYM_EXPORT(__seofread),
    ESP_ELFSYM_EXPORT(__sflags),
    ESP_ELFSYM_EXPORT(__sflush_r),
    ESP_ELFSYM_EXPORT(__sfp),
    ESP_ELFSYM_EXPORT(__sfp_lock_acquire),
    ESP_ELFSYM_EXPORT(__sfp_lock_release),
    ESP_ELFSYM_EXPORT(__sfvwrite_r),
    ESP_ELFSYM_EXPORT(__sinit),
    ESP_ELFSYM_EXPORT(__smakebuf_r),
    ESP_ELFSYM_EXPORT(__sprint_r),
    ESP_ELFSYM_EXPORT(__sread),
    ESP_ELFSYM_EXPORT(__srefill_r),
    ESP_ELFSYM_EXPORT(__sseek),
    ESP_ELFSYM_EXPORT(__ssprint_r),
    ESP_ELFSYM_EXPORT(__ssrefill_r),
    ESP_ELFSYM_EXPORT(__ssvfiscanf_r),
    ESP_ELFSYM_EXPORT(__stack_chk_fail),
    ESP_ELFSYM_EXPORT(__strtok_r),
    ESP_ELFSYM_EXPORT(__subdf3),
    ESP_ELFSYM_EXPORT(__submore),
    ESP_ELFSYM_EXPORT(__swbuf),
    ESP_ELFSYM_EXPORT(__swbuf_r),
    ESP_ELFSYM_EXPORT(__swhatbuf_r),
    ESP_ELFSYM_EXPORT(__swrite),
    ESP_ELFSYM_EXPORT(__swsetup_r),
    ESP_ELFSYM_EXPORT(__truncdfsf2),
    ESP_ELFSYM_EXPORT(__tz_lock),
    ESP_ELFSYM_EXPORT(__tz_unlock),
    ESP_ELFSYM_EXPORT(__tzcalc_limits),
    ESP_ELFSYM_EXPORT(__udivdi3),
    ESP_ELFSYM_EXPORT(__ulp),
    ESP_ELFSYM_EXPORT(__umoddi3),
    ESP_ELFSYM_EXPORT(__unorddf2),
    ESP_ELFSYM_EXPORT(__xtensa_libgcc_window_spill),
    ESP_ELFSYM_EXPORT(__xtensa_nonlocal_goto),
    ESP_ELFSYM_EXPORT(__xtensa_sync_caches),
    ESP_ELFSYM_EXPORT(_calloc_r),
    ESP_ELFSYM_EXPORT(_close_r),
    ESP_ELFSYM_EXPORT(_dtoa_r),
    ESP_ELFSYM_EXPORT(_exit),
    ESP_ELFSYM_EXPORT(_fclose_r),
    ESP_ELFSYM_EXPORT(_fcntl_r),
    ESP_ELFSYM_EXPORT(_fdopen_r),
    ESP_ELFSYM_EXPORT(_fflush_r),
    ESP_ELFSYM_EXPORT(_fgets_r),
    ESP_ELFSYM_EXPORT(_findenv_r),
    ESP_ELFSYM_EXPORT(_fopen_r),
    ESP_ELFSYM_EXPORT(_fputc_r),
    ESP_ELFSYM_EXPORT(_fputs_r),
    ESP_ELFSYM_EXPORT(_fread_r),
    ESP_ELFSYM_EXPORT(_free_r),
    ESP_ELFSYM_EXPORT(_fseek_r),
    ESP_ELFSYM_EXPORT(_fseeko_r),
    ESP_ELFSYM_EXPORT(_fstat_r),
    ESP_ELFSYM_EXPORT(_ftello_r),
    ESP_ELFSYM_EXPORT(_fwalk_sglue),
    ESP_ELFSYM_EXPORT(_fwrite_r),
    ESP_ELFSYM_EXPORT(_getenv_r),
    ESP_ELFSYM_EXPORT(_getpid_r),
    ESP_ELFSYM_EXPORT(_gettimeofday_r),
    ESP_ELFSYM_EXPORT(_kill_r),
    ESP_ELFSYM_EXPORT(_link_r),
    ESP_ELFSYM_EXPORT(_localeconv_r),
    ESP_ELFSYM_EXPORT(_lock_acquire),
    ESP_ELFSYM_EXPORT(_lock_acquire_recursive),
    ESP_ELFSYM_EXPORT(_lock_close),
    ESP_ELFSYM_EXPORT(_lock_close_recursive),
    ESP_ELFSYM_EXPORT(_lock_init),
    ESP_ELFSYM_EXPORT(_lock_release),
    ESP_ELFSYM_EXPORT(_lock_release_recursive),
    ESP_ELFSYM_EXPORT(_lock_try_acquire),
    ESP_ELFSYM_EXPORT(_lock_try_acquire_recursive),
    ESP_ELFSYM_EXPORT(_lseek_r),
    ESP_ELFSYM_EXPORT(_malloc_r),
    ESP_ELFSYM_EXPORT(_mbrtowc_r),
    ESP_ELFSYM_EXPORT(_open_r),
    ESP_ELFSYM_EXPORT(_putc_r),
    ESP_ELFSYM_EXPORT(_puts_r),
    ESP_ELFSYM_EXPORT(_raise_r),
    ESP_ELFSYM_EXPORT(_read_r),
    ESP_ELFSYM_EXPORT(_realloc_r),
    ESP_ELFSYM_EXPORT(_reclaim_reent),
    ESP_ELFSYM_EXPORT(_rename_r),
    ESP_ELFSYM_EXPORT(_sbrk_r),
    ESP_ELFSYM_EXPORT(_sfread_r),
    ESP_ELFSYM_EXPORT(_stat_r),
    ESP_ELFSYM_EXPORT(_strerror_r),
    ESP_ELFSYM_EXPORT(_strtod_l),
    ESP_ELFSYM_EXPORT(_strtol_r),
    ESP_ELFSYM_EXPORT(_strtoll_r),
    ESP_ELFSYM_EXPORT(_strtoul_r),
    ESP_ELFSYM_EXPORT(_strtoull_r),
    ESP_ELFSYM_EXPORT(_sungetc_r),
    ESP_ELFSYM_EXPORT(_svfiprintf_r),
    ESP_ELFSYM_EXPORT(_svfprintf_r),
    ESP_ELFSYM_EXPORT(_system_r),
    ESP_ELFSYM_EXPORT(_times_r),
    ESP_ELFSYM_EXPORT(_tzset_unlocked),
    ESP_ELFSYM_EXPORT(_tzset_unlocked_r),
    ESP_ELFSYM_EXPORT(_unlink_r),
    ESP_ELFSYM_EXPORT(_user_strerror),
    ESP_ELFSYM_EXPORT(_vfprintf_r),
    ESP_ELFSYM_EXPORT(_vsnprintf_r),
    ESP_ELFSYM_EXPORT(_write_r),
    ESP_ELFSYM_EXPORT(abort),
    ESP_ELFSYM_EXPORT(abs),
    ESP_ELFSYM_EXPORT(atof),
    ESP_ELFSYM_EXPORT(atoi),
    ESP_ELFSYM_EXPORT(atol),
    ESP_ELFSYM_EXPORT(bzero),
    ESP_ELFSYM_EXPORT(calloc),
    ESP_ELFSYM_EXPORT(ceil),
    ESP_ELFSYM_EXPORT(cfree),
    ESP_ELFSYM_EXPORT(clock_gettime),
    ESP_ELFSYM_EXPORT(close),
    ESP_ELFSYM_EXPORT(cosf),
    ESP_ELFSYM_EXPORT(delay),
    ESP_ELFSYM_EXPORT(div),
    ESP_ELFSYM_EXPORT(esp_libc_include_assert_impl),
    ESP_ELFSYM_EXPORT(esp_libc_include_getentropy_impl),
    ESP_ELFSYM_EXPORT(esp_libc_include_heap_impl),
    ESP_ELFSYM_EXPORT(esp_libc_include_init_funcs),
    ESP_ELFSYM_EXPORT(esp_libc_include_pthread_impl),
    ESP_ELFSYM_EXPORT(esp_libc_include_reent_syscalls_impl),
    ESP_ELFSYM_EXPORT(esp_libc_include_syscalls_impl),
    ESP_ELFSYM_EXPORT(esp_libc_init),
    ESP_ELFSYM_EXPORT(esp_libc_init_funcs),
    ESP_ELFSYM_EXPORT(esp_libc_init_global_stdio),
    ESP_ELFSYM_EXPORT(esp_libc_locks_init),
    ESP_ELFSYM_EXPORT(esp_libc_time_init),
    ESP_ELFSYM_EXPORT(esp_newlib_init),
    ESP_ELFSYM_EXPORT(esp_newlib_locks_init),
    ESP_ELFSYM_EXPORT(esp_newlib_time_init),
    ESP_ELFSYM_EXPORT(esp_reent_init),
    ESP_ELFSYM_EXPORT(esp_set_time_from_rtc),
    ESP_ELFSYM_EXPORT(esp_setup_newlib_syscalls),
    ESP_ELFSYM_EXPORT(esp_sync_timekeeping_timers),
    ESP_ELFSYM_EXPORT(esp_time_impl_get_boot_time),
    ESP_ELFSYM_EXPORT(esp_time_impl_get_time),
    ESP_ELFSYM_EXPORT(esp_time_impl_get_time_since_boot),
    ESP_ELFSYM_EXPORT(esp_time_impl_set_boot_time),
    ESP_ELFSYM_EXPORT(exit),
    ESP_ELFSYM_EXPORT(explicit_bzero),
    ESP_ELFSYM_EXPORT(fabsf),
    ESP_ELFSYM_EXPORT(fclose),
    ESP_ELFSYM_EXPORT(fcntl),
    ESP_ELFSYM_EXPORT(fdopen),
    ESP_ELFSYM_EXPORT(feof),
    ESP_ELFSYM_EXPORT(fflush),
    ESP_ELFSYM_EXPORT(fgets),
    ESP_ELFSYM_EXPORT(fileno),
    ESP_ELFSYM_EXPORT(floorf),
    ESP_ELFSYM_EXPORT(fopen),
    ESP_ELFSYM_EXPORT(fprintf),
    ESP_ELFSYM_EXPORT(fputc),
    ESP_ELFSYM_EXPORT(fputs),
    ESP_ELFSYM_EXPORT(fread),
    ESP_ELFSYM_EXPORT(free),
    ESP_ELFSYM_EXPORT(frexp),
    ESP_ELFSYM_EXPORT(fseek),
    ESP_ELFSYM_EXPORT(fstat),
    ESP_ELFSYM_EXPORT(fsync),
    ESP_ELFSYM_EXPORT(ftell),
    ESP_ELFSYM_EXPORT(fwrite),
    ESP_ELFSYM_EXPORT(getenv),
    ESP_ELFSYM_EXPORT(getopt_long),
    ESP_ELFSYM_EXPORT(gettimeofday),
    ESP_ELFSYM_EXPORT(gmtime_r),
    ESP_ELFSYM_EXPORT(isalnum),
    ESP_ELFSYM_EXPORT(isalpha),
    ESP_ELFSYM_EXPORT(isgraph),
    ESP_ELFSYM_EXPORT(isprint),
    ESP_ELFSYM_EXPORT(isspace),
    ESP_ELFSYM_EXPORT(iswspace),
    ESP_ELFSYM_EXPORT(iswspace_l),
    ESP_ELFSYM_EXPORT(isxdigit),
    ESP_ELFSYM_EXPORT(itoa),
    ESP_ELFSYM_EXPORT(labs),
    ESP_ELFSYM_EXPORT(localtime),
    ESP_ELFSYM_EXPORT(localtime_r),
    ESP_ELFSYM_EXPORT(longjmp),
    ESP_ELFSYM_EXPORT(malloc),
    ESP_ELFSYM_EXPORT(memchr),
    ESP_ELFSYM_EXPORT(memcmp),
    ESP_ELFSYM_EXPORT(memcpy),
    ESP_ELFSYM_EXPORT(memmove),
    ESP_ELFSYM_EXPORT(memset),
    ESP_ELFSYM_EXPORT(mktime),
    ESP_ELFSYM_EXPORT(nan),
    ESP_ELFSYM_EXPORT(open),
    ESP_ELFSYM_EXPORT(pm_bz_begin),
    ESP_ELFSYM_EXPORT(pm_bz_end),
    ESP_ELFSYM_EXPORT(pm_bz_play_jingle),
    ESP_ELFSYM_EXPORT(pm_clock_begin),
    ESP_ELFSYM_EXPORT(pm_clock_epoch),
    ESP_ELFSYM_EXPORT(pm_clock_set_time_from_string),
    ESP_ELFSYM_EXPORT(pm_clock_timestamp),
    ESP_ELFSYM_EXPORT(pm_clock_valid),
    ESP_ELFSYM_EXPORT(pm_eink_clear),
    ESP_ELFSYM_EXPORT(pm_eink_count_lines),
    ESP_ELFSYM_EXPORT(pm_eink_draw_status_bar),
    ESP_ELFSYM_EXPORT(pm_eink_force_slow_full_update),
    ESP_ELFSYM_EXPORT(pm_eink_get_eink_text_width),
    ESP_ELFSYM_EXPORT(pm_eink_get_font_height),
    ESP_ELFSYM_EXPORT(pm_eink_get_line_spacing),
    ESP_ELFSYM_EXPORT(pm_eink_height),
    ESP_ELFSYM_EXPORT(pm_eink_max_lines),
    ESP_ELFSYM_EXPORT(pm_eink_pixel),
    ESP_ELFSYM_EXPORT(pm_eink_rect),
    ESP_ELFSYM_EXPORT(pm_eink_refresh),
    ESP_ELFSYM_EXPORT(pm_eink_reset_display),
    ESP_ELFSYM_EXPORT(pm_eink_set_fast_full_refresh),
    ESP_ELFSYM_EXPORT(pm_eink_status_bar),
    ESP_ELFSYM_EXPORT(pm_eink_width),
    ESP_ELFSYM_EXPORT(pm_font_engine_char_width),
    ESP_ELFSYM_EXPORT(pm_font_engine_font_ascent),
    ESP_ELFSYM_EXPORT(pm_font_engine_font_descent),
    ESP_ELFSYM_EXPORT(pm_font_engine_font_height_txt),
    ESP_ELFSYM_EXPORT(pm_font_height),
    ESP_ELFSYM_EXPORT(pm_i18n_app_name),
    ESP_ELFSYM_EXPORT(pm_i18n_code),
    ESP_ELFSYM_EXPORT(pm_i18n_code_at),
    ESP_ELFSYM_EXPORT(pm_i18n_day_name),
    ESP_ELFSYM_EXPORT(pm_i18n_get),
    ESP_ELFSYM_EXPORT(pm_i18n_kb_app_name),
    ESP_ELFSYM_EXPORT(pm_i18n_language),
    ESP_ELFSYM_EXPORT(pm_i18n_language_count),
    ESP_ELFSYM_EXPORT(pm_i18n_month_name),
    ESP_ELFSYM_EXPORT(pm_i18n_native_name),
    ESP_ELFSYM_EXPORT(pm_i18n_native_name_at),
    ESP_ELFSYM_EXPORT(pm_i18n_normalize_command),
    ESP_ELFSYM_EXPORT(pm_i18n_set_language),
    ESP_ELFSYM_EXPORT(pm_i18n_set_language_by_code),
    ESP_ELFSYM_EXPORT(pm_io_join_string),
    ESP_ELFSYM_EXPORT(pm_io_remove_char),
    ESP_ELFSYM_EXPORT(pm_io_split_string_count),
    ESP_ELFSYM_EXPORT(pm_io_split_string_get),
    ESP_ELFSYM_EXPORT(pm_io_string_to_int),
    ESP_ELFSYM_EXPORT(pm_kb_accept_key),
    ESP_ELFSYM_EXPORT(pm_kb_check_usbkb),
    ESP_ELFSYM_EXPORT(pm_kb_flush),
    ESP_ELFSYM_EXPORT(pm_kb_read),
    ESP_ELFSYM_EXPORT(pm_kb_state),
    ESP_ELFSYM_EXPORT(pm_kb_toggle_fn),
    ESP_ELFSYM_EXPORT(pm_kb_toggle_shift),
    ESP_ELFSYM_EXPORT(pm_layout_eink_row_pitch),
    ESP_ELFSYM_EXPORT(pm_layout_slice_that_fits),
    ESP_ELFSYM_EXPORT(pm_layout_truncate_with_ellipsis),
    ESP_ELFSYM_EXPORT(pm_layout_word_wrap_count),
    ESP_ELFSYM_EXPORT(pm_layout_word_wrap_get),
    ESP_ELFSYM_EXPORT(pm_oled_info_bar),
    ESP_ELFSYM_EXPORT(pm_oled_oled_line),
    ESP_ELFSYM_EXPORT(pm_oled_oled_scroll),
    ESP_ELFSYM_EXPORT(pm_oled_oled_word),
    ESP_ELFSYM_EXPORT(pm_oled_power_save),
    ESP_ELFSYM_EXPORT(pm_oled_send),
    ESP_ELFSYM_EXPORT(pm_oled_set_power_save),
    ESP_ELFSYM_EXPORT(pm_oled_sysmsg),
    ESP_ELFSYM_EXPORT(pm_sd_append_file),
    ESP_ELFSYM_EXPORT(pm_sd_append_to_file),
    ESP_ELFSYM_EXPORT(pm_sd_begin_io),
    ESP_ELFSYM_EXPORT(pm_sd_copy_file),
    ESP_ELFSYM_EXPORT(pm_sd_del_file),
    ESP_ELFSYM_EXPORT(pm_sd_delete_file),
    ESP_ELFSYM_EXPORT(pm_sd_delete_metadata),
    ESP_ELFSYM_EXPORT(pm_sd_end_io),
    ESP_ELFSYM_EXPORT(pm_sd_get_editing_file),
    ESP_ELFSYM_EXPORT(pm_sd_get_file_size),
    ESP_ELFSYM_EXPORT(pm_sd_get_files_list_index),
    ESP_ELFSYM_EXPORT(pm_sd_get_mode),
    ESP_ELFSYM_EXPORT(pm_sd_get_no_sd),
    ESP_ELFSYM_EXPORT(pm_sd_get_working_file),
    ESP_ELFSYM_EXPORT(pm_sd_list_dir),
    ESP_ELFSYM_EXPORT(pm_sd_load_file),
    ESP_ELFSYM_EXPORT(pm_sd_read_binary_file),
    ESP_ELFSYM_EXPORT(pm_sd_read_file),
    ESP_ELFSYM_EXPORT(pm_sd_read_file_to_string),
    ESP_ELFSYM_EXPORT(pm_sd_ren_file),
    ESP_ELFSYM_EXPORT(pm_sd_ren_metadata),
    ESP_ELFSYM_EXPORT(pm_sd_rename_file),
    ESP_ELFSYM_EXPORT(pm_sd_save_file),
    ESP_ELFSYM_EXPORT(pm_sd_write_file),
    ESP_ELFSYM_EXPORT(pm_sd_write_metadata),
    ESP_ELFSYM_EXPORT(pm_text),
    ESP_ELFSYM_EXPORT(pm_text_color),
    ESP_ELFSYM_EXPORT(pm_text_width),
    ESP_ELFSYM_EXPORT(pm_touch_get_diff),
    ESP_ELFSYM_EXPORT(pm_touch_get_dynamic_scroll),
    ESP_ELFSYM_EXPORT(pm_touch_get_last_touch),
    ESP_ELFSYM_EXPORT(pm_touch_get_last_touch_time),
    ESP_ELFSYM_EXPORT(pm_touch_get_prev_dynamic_scroll),
    ESP_ELFSYM_EXPORT(pm_touch_get_scroll_vector),
    ESP_ELFSYM_EXPORT(pm_touch_update_scroll),
    ESP_ELFSYM_EXPORT(pm_touch_update_scroll_from_touch),
    ESP_ELFSYM_EXPORT(pm_ui_begin_eink_screen),
    ESP_ELFSYM_EXPORT(pm_ui_draw_chip_text),
    ESP_ELFSYM_EXPORT(pm_ui_draw_list_item),
    ESP_ELFSYM_EXPORT(pm_ui_draw_scrollbar),
    ESP_ELFSYM_EXPORT(pm_ui_end_eink_screen),
    ESP_ELFSYM_EXPORT(pm_wifi_begin),
    ESP_ELFSYM_EXPORT(pm_wifi_clear_saved_credentials),
    ESP_ELFSYM_EXPORT(pm_wifi_connect),
    ESP_ELFSYM_EXPORT(pm_wifi_disable),
    ESP_ELFSYM_EXPORT(pm_wifi_disconnect),
    ESP_ELFSYM_EXPORT(pm_wifi_dispatch_events),
    ESP_ELFSYM_EXPORT(pm_wifi_enable),
    ESP_ELFSYM_EXPORT(pm_wifi_get_connected_ssid),
    ESP_ELFSYM_EXPORT(pm_wifi_get_ip_address),
    ESP_ELFSYM_EXPORT(pm_wifi_get_last_error),
    ESP_ELFSYM_EXPORT(pm_wifi_get_rssi),
    ESP_ELFSYM_EXPORT(pm_wifi_get_scan_result_count),
    ESP_ELFSYM_EXPORT(pm_wifi_get_state),
    ESP_ELFSYM_EXPORT(pm_wifi_get_status_message),
    ESP_ELFSYM_EXPORT(pm_wifi_has_saved_credentials),
    ESP_ELFSYM_EXPORT(pm_wifi_is_connected),
    ESP_ELFSYM_EXPORT(pm_wifi_is_scanning),
    ESP_ELFSYM_EXPORT(pm_wifi_load_saved_credentials),
    ESP_ELFSYM_EXPORT(pm_wifi_reconnect),
    ESP_ELFSYM_EXPORT(pm_wifi_scan),
    ESP_ELFSYM_EXPORT(pm_wifi_stop),
    ESP_ELFSYM_EXPORT(pocketmage_sdk_version),
    ESP_ELFSYM_EXPORT(printf),
    ESP_ELFSYM_EXPORT(putchar),
    ESP_ELFSYM_EXPORT(puts),
    ESP_ELFSYM_EXPORT(pvalloc),
    ESP_ELFSYM_EXPORT(qsort),
    ESP_ELFSYM_EXPORT(rand),
    ESP_ELFSYM_EXPORT(read),
    ESP_ELFSYM_EXPORT(realloc),
    ESP_ELFSYM_EXPORT(rename),
    ESP_ELFSYM_EXPORT(roundf),
    ESP_ELFSYM_EXPORT(scalbnf),
    ESP_ELFSYM_EXPORT(setjmp),
    ESP_ELFSYM_EXPORT(settimeofday),
    ESP_ELFSYM_EXPORT(setvbuf),
    ESP_ELFSYM_EXPORT(sinf),
    ESP_ELFSYM_EXPORT(siscanf),
    ESP_ELFSYM_EXPORT(sleep),
    ESP_ELFSYM_EXPORT(sniprintf),
    ESP_ELFSYM_EXPORT(snprintf),
    ESP_ELFSYM_EXPORT(sprintf),
    ESP_ELFSYM_EXPORT(sqrtf),
    ESP_ELFSYM_EXPORT(srand),
    ESP_ELFSYM_EXPORT(stat),
    ESP_ELFSYM_EXPORT(strcasecmp),
    ESP_ELFSYM_EXPORT(strcasestr),
    ESP_ELFSYM_EXPORT(strcat),
    ESP_ELFSYM_EXPORT(strchr),
    ESP_ELFSYM_EXPORT(strcmp),
    ESP_ELFSYM_EXPORT(strcpy),
    ESP_ELFSYM_EXPORT(strcspn),
    ESP_ELFSYM_EXPORT(strdup),
    ESP_ELFSYM_EXPORT(strerror),
    ESP_ELFSYM_EXPORT(strerror_r),
    ESP_ELFSYM_EXPORT(strftime),
    ESP_ELFSYM_EXPORT(strlcat),
    ESP_ELFSYM_EXPORT(strlcpy),
    ESP_ELFSYM_EXPORT(strlen),
    ESP_ELFSYM_EXPORT(strncasecmp),
    ESP_ELFSYM_EXPORT(strncat),
    ESP_ELFSYM_EXPORT(strncmp),
    ESP_ELFSYM_EXPORT(strncpy),
    ESP_ELFSYM_EXPORT(strndup),
    ESP_ELFSYM_EXPORT(strnlen),
    ESP_ELFSYM_EXPORT(strpbrk),
    ESP_ELFSYM_EXPORT(strrchr),
    ESP_ELFSYM_EXPORT(strsep),
    ESP_ELFSYM_EXPORT(strspn),
    ESP_ELFSYM_EXPORT(strstr),
    ESP_ELFSYM_EXPORT(strtod),
    ESP_ELFSYM_EXPORT(strtok),
    ESP_ELFSYM_EXPORT(strtok_r),
    ESP_ELFSYM_EXPORT(strtol),
    ESP_ELFSYM_EXPORT(strtoll),
    ESP_ELFSYM_EXPORT(strtoul),
    ESP_ELFSYM_EXPORT(strtoull),
    ESP_ELFSYM_EXPORT(time),
    ESP_ELFSYM_EXPORT(tolower),
    ESP_ELFSYM_EXPORT(toupper),
    ESP_ELFSYM_EXPORT(unlink),
    ESP_ELFSYM_EXPORT(usleep),
    ESP_ELFSYM_EXPORT(utoa),
    ESP_ELFSYM_EXPORT(valloc),
    ESP_ELFSYM_EXPORT(vfprintf),
    ESP_ELFSYM_EXPORT(vprintf),
    ESP_ELFSYM_EXPORT(vsnprintf),
    ESP_ELFSYM_EXPORT(write),
    ESP_ELFSYM_END
};
}
