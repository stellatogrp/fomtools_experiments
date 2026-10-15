#include "fom_profile.h"

#ifdef FOM_PROFILE
#include <stdio.h>
#include <string.h>

#define FOM_PROF_MAX 64
static char fom_prof_names[FOM_PROF_MAX][48];
static double fom_prof_totals[FOM_PROF_MAX];
static long fom_prof_counts[FOM_PROF_MAX];
static int fom_prof_n = 0;

void fom_prof_reset(void) { fom_prof_n = 0; }

void fom_prof_add(const char *name, double ns) {
    int i;
    for (i = 0; i < fom_prof_n; ++i) {
        if (strcmp(fom_prof_names[i], name) == 0) {
            fom_prof_totals[i] += ns;
            fom_prof_counts[i] += 1;
            return;
        }
    }
    if (fom_prof_n < FOM_PROF_MAX) {
        strncpy(fom_prof_names[fom_prof_n], name, 47);
        fom_prof_names[fom_prof_n][47] = 0;
        fom_prof_totals[fom_prof_n] = ns;
        fom_prof_counts[fom_prof_n] = 1;
        fom_prof_n += 1;
    }
}

void fom_prof_report(void) {
    int i, j;
    double grand = 0.0;
    for (i = 0; i < fom_prof_n; ++i) grand += fom_prof_totals[i];
    for (i = 0; i < fom_prof_n; ++i) {
        for (j = i + 1; j < fom_prof_n; ++j) {
            if (fom_prof_totals[j] > fom_prof_totals[i]) {
                double td = fom_prof_totals[i];
                long cd = fom_prof_counts[i];
                char nd[48];
                fom_prof_totals[i] = fom_prof_totals[j]; fom_prof_totals[j] = td;
                fom_prof_counts[i] = fom_prof_counts[j]; fom_prof_counts[j] = cd;
                memcpy(nd, fom_prof_names[i], 48);
                memcpy(fom_prof_names[i], fom_prof_names[j], 48);
                memcpy(fom_prof_names[j], nd, 48);
            }
        }
    }
    for (i = 0; i < fom_prof_n; ++i) {
        double pct = (grand > 0.0) ? (100.0 * fom_prof_totals[i] / grand) : 0.0;
        printf("PROF region=%s total_us=%.4f calls=%ld pct=%.2f\n",
               fom_prof_names[i], fom_prof_totals[i] / 1000.0,
               fom_prof_counts[i], pct);
    }
}
#endif /* FOM_PROFILE */
