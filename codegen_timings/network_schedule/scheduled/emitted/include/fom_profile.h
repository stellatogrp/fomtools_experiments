#ifndef FOM_PROFILE_H
#define FOM_PROFILE_H

/* Portable, allocation-free region profiler. Active only when the
   translation unit is compiled with -DFOM_PROFILE; otherwise every
   macro is a no-op, so it never affects the shipped solver. */

#include <time.h>

static inline double fom_now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1.0e9 + (double)ts.tv_nsec;
}

#ifdef FOM_PROFILE
void fom_prof_add(const char *name, double ns);
void fom_prof_report(void);
void fom_prof_reset(void);
#define FOM_PROF_BEGIN(name) double fom__t_##name = fom_now_ns()
#define FOM_PROF_END(name) fom_prof_add(#name, fom_now_ns() - fom__t_##name)
#else
#define FOM_PROF_BEGIN(name) ((void)0)
#define FOM_PROF_END(name) ((void)0)
static inline void fom_prof_report(void) {}
static inline void fom_prof_reset(void) {}
#endif

#endif /* FOM_PROFILE_H */
