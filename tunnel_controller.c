#include <assert.h>
#include "tunnel_controller.h"
#include "console.h"
#include <pthread.h>

static int n_in_tunnel[2];
static unsigned long n_total[2];
//Stores the number of vehicles waiting at one each end on the tunnel
static int n_waiting[2];
//Stores the number of vehicles have passed in each direction before each side has been released
static int n_passed[2];

//initialise mutex and condion
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_exit = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t opposite_dir = PTHREAD_COND_INITIALIZER;

/**
 * @brief controls entry to the tunnel
 * @param dir the direction of travel when entering
 */
void enter_tunnel(direction_t dir) {
    //Increment indication that a vehicle is waiting
    n_waiting[dir] += 1;

    lcd_write_at(12+dir, 0, "(%s) Waiting: %2d",
        (dir == NORTH) ? "North" : "South", n_waiting[dir]);

    pthread_mutex_lock(&lock);

    //While there are cars travelling in the opposite direction or 3 cars travelling in the
    //current direction or the number of cars a car has seen pass in the opposite direction
    //then wait.
    while(n_in_tunnel[opposite(dir)] != 0 || n_in_tunnel[dir] >= 3 ||
		(n_passed[dir] >= 6 && n_waiting[opposite(dir)] > 0)) {
        pthread_cond_wait(&opposite_dir, &lock);
    }

    //Car is no longer waiting so decrement
    n_waiting[dir] -= 1;
    //There will have been no cars travelling in the opposite direction so set to 0
    n_passed[opposite(dir)] = 0;

    //A car is in the tunnel so increment
    n_in_tunnel[dir] += 1;
    lcd_write_at(4+dir, 0, "In tunnel (%s) : %2d",
            (dir == NORTH) ? "NORTH" : "SOUTH", n_in_tunnel[dir]);

    //Assert if a car is in the tunnel travelling the opposite direction
    assert(n_in_tunnel[opposite(dir)] == 0);
    //Assert if there are more than 3 cars in the tunnel
    assert(n_in_tunnel[dir] <= 3);
    //Assert if the number of car passing in one direction > 6
    assert(n_passed[dir] <= 6);

    pthread_mutex_unlock(&lock);

    //If there are cars waiting in the opposite direction then increment as they will see 1 pass
    if(n_waiting[opposite(dir)] > 0) {
        n_passed[dir] += 1;
    }
    //If no cars are waiting then they wont be able to see the car pass so dont increment
    else {
        n_passed[dir] = 0;
    }
    lcd_write_at(16+dir, 0, "(%s) Pass: %2d",
        (dir == NORTH) ? "North" : "South", n_passed[dir]);
}

/**
 * @brief controls exit from the tunnel
 * @param dir the direction of travel when leaving
 */
void exit_tunnel(direction_t dir) {
    pthread_mutex_lock(&lock);

    //Car exits the tunnel so decrement
    n_in_tunnel[dir] -= 1;
    n_total[dir] += 1;
    lcd_write_at(4+dir, 0, "In tunnel (%s) : %2d",
            (dir == NORTH) ? "NORTH" : "SOUTH", n_in_tunnel[dir]);
    lcd_write_at(6+dir, 0, "Total     (%s) : %d",
            (dir == NORTH) ? "NORTH" : "SOUTH", n_total[dir]);
    //Wake up all threads holding the lock
    pthread_cond_broadcast(&opposite_dir);
    pthread_mutex_unlock(&lock);
}

/**
 * @brief Gives the opposite direction to its argument
 * @param dir a direction, either NORTH or SOUTH
 * @returns the opposite of direction dir
 */
direction_t opposite(direction_t dir) {
    return ((dir == NORTH) ? SOUTH : NORTH);
}
