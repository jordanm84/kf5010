#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#include "console.h"
#include "tunnel_controller.h"

#define N_VEHICLES 10

static void *vehicle_thr(void *arg);
unsigned int scale_random(double r, int l, int u);

int main (void) {
    int rc;
    unsigned long i;
    pthread_t thread[N_VEHICLES];
    
    console_init();
    for (i = 0; i < N_VEHICLES; i += 1) {
        rc = pthread_create(&thread[i], NULL, vehicle_thr, (void *)i);
        assert(rc == 0);
    }
    for (i = 0; i < N_VEHICLES; i += 1) {
        rc = pthread_join(thread[i], NULL);
        assert(rc == 0);
    }
    
    while (true) {
        /* skip */
    }
    exit(0);
}

static void *vehicle_thr(void *arg) {
    double r;
    unsigned long id = (unsigned long)arg;
    unsigned long i;
    struct drand48_data rand_state;
    int rc;
    direction_t dir;
    unsigned int delay;

    /* initialise random seed */
    rc = srand48_r((long)id * 10 + time(NULL), &rand_state); 
    assert(rc == 0);

    while (true) {
        /* pick a direction of travel at random */
        rc = drand48_r(&rand_state, &r);
        assert(rc == 0);
        if (scale_random(r, 0, 1) == 0) {
            dir = NORTH;
        } else {
            dir = SOUTH;
        }

        /* approach tunnel */
        rc = drand48_r(&rand_state, &r);
        assert(rc == 0);
        usleep(scale_random(r, 1000000, 3000000));

        /* enter tunnel */
        enter_tunnel(dir);

        /* travel through tunnel */
        rc = drand48_r(&rand_state, &r);
        assert(rc == 0);
        usleep(scale_random(r, 500000, 1500000));

        /* exit tunnel */
        exit_tunnel(dir);       
    }
}

unsigned int scale_random(double r, int l, int u) {
    assert((0 <= r) && (r < 1) && (0 <= l) && (l < u));
    return (unsigned int)(lround(l + ((u - l) * r)));
}
