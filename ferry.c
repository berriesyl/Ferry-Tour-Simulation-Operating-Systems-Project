#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_CARS 12
#define NUM_MINIBUSES 10
#define NUM_TRUCKS 8
#define FERRY_CAPACITY 20
#define TOTAL_VEHICLES (NUM_CARS + NUM_MINIBUSES + NUM_TRUCKS)

typedef struct {
    int id;
    int type; // 1: car, 2: minibus, 3: truck
    int size;
    int side;
    int initial_side;
    int trip_count;
    int toll_id;
} Vehicle;

Vehicle vehicles[TOTAL_VEHICLES];

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ferry_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t toll_mutex[4];
pthread_cond_t ferry_ready = PTHREAD_COND_INITIALIZER;
pthread_cond_t vehicle_boarded = PTHREAD_COND_INITIALIZER;

int waiting[2] = {0, 0};
int ferry_side = 0;
int ferry_load = 0;
int total_completed = 0;

const char* get_type_name(int type) {
    return type == 1 ? "Car" : type == 2 ? "Minibus" : "Truck";
}

void print_skipped_vehicles(int current_load, int side) {
    int remaining_space = FERRY_CAPACITY - current_load;
    for (int i = 0; i < TOTAL_VEHICLES; i++) {
        Vehicle* v = &vehicles[i];
        if (v->side == side && v->trip_count < 2 &&
            ((side == 0 && v->trip_count == 0) || (side == 1 && v->trip_count == 1))) {
            if (v->size > remaining_space) {
                printf("    !! Vehicle %d [%s] couldn't board this trip from side %d due to insufficient space (Needs: %d, Available: %d)\n",
                       v->id, get_type_name(v->type), side, v->size, remaining_space);
            } else {
                remaining_space -= v->size;
            }
        }
    }
}

void* vehicle_thread(void* arg) {
    Vehicle* v = (Vehicle*)arg;
    while (v->trip_count < 2) {
        pthread_mutex_lock(&toll_mutex[v->toll_id]);
        printf("Vehicle %d [%s] is passing toll %d on side %d for trip %d\n",
               v->id, get_type_name(v->type), v->toll_id, v->side, v->trip_count + 1);
        sleep(rand() % 3 + 1);
        pthread_mutex_unlock(&toll_mutex[v->toll_id]);

        pthread_mutex_lock(&queue_mutex);
        waiting[v->side]++;
        pthread_mutex_unlock(&queue_mutex);

        pthread_mutex_lock(&ferry_mutex);
        while (!(v->side == ferry_side &&
                 ferry_load + v->size <= FERRY_CAPACITY &&
                 ((v->side == 0 && v->trip_count == 0) || (v->side == 1 && v->trip_count == 1)))) {
            pthread_cond_wait(&ferry_ready, &ferry_mutex);
        }

        ferry_load += v->size;
        pthread_mutex_lock(&queue_mutex);
        waiting[ferry_side]--;
        pthread_mutex_unlock(&queue_mutex);

        printf("  >> Vehicle %d [%s] boarded ferry on side %d (Load: %d)\n",
               v->id, get_type_name(v->type), ferry_side, ferry_load);

        v->side = 1 - v->side;
        v->trip_count++;

        pthread_cond_signal(&vehicle_boarded);
        pthread_mutex_unlock(&ferry_mutex);
    }

    pthread_mutex_lock(&queue_mutex);
    total_completed++;
    pthread_mutex_unlock(&queue_mutex);
    return NULL;
}

void* ferry_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&ferry_mutex);

        int boarding_possible = 1;
        while (boarding_possible) {
            boarding_possible = 0;
            for (int i = 0; i < TOTAL_VEHICLES; i++) {
                Vehicle* v = &vehicles[i];
                if (v->side == ferry_side &&
                    v->trip_count < 2 &&
                    ((ferry_side == 0 && v->trip_count == 0) || (ferry_side == 1 && v->trip_count == 1)) &&
                    ferry_load + v->size <= FERRY_CAPACITY) {
                    pthread_cond_broadcast(&ferry_ready);
                    pthread_cond_wait(&vehicle_boarded, &ferry_mutex);
                    boarding_possible = 1;
                    break;
                }
            }

            if (!boarding_possible && ferry_load == 0) {
                int remaining = 0;
                for (int i = 0; i < TOTAL_VEHICLES; i++) {
                    Vehicle* v = &vehicles[i];
                    if (v->trip_count < 2 &&
                        v->side == ferry_side &&
                        ((ferry_side == 0 && v->trip_count == 0) ||
                         (ferry_side == 1 && v->trip_count == 1))) {
                        remaining++;
                    }
                }

                if (remaining == 0) {
                    pthread_mutex_unlock(&ferry_mutex);
                    goto end;
                }
            }
        }

        print_skipped_vehicles(ferry_load, ferry_side);

        printf("\n**** The ferry is departing from side %d to side %d with total load %d. Bon voyage! ****\n\n",
               ferry_side, 1 - ferry_side, ferry_load);
        pthread_mutex_unlock(&ferry_mutex);

        sleep(5);

        pthread_mutex_lock(&ferry_mutex);
        ferry_side = 1 - ferry_side;
        ferry_load = 0;

        printf("**** The ferry has arrived at side %d and is unloading. ****\n\n", ferry_side);
        pthread_mutex_unlock(&ferry_mutex);

        sleep(3);
    }
end:
    printf("\n**** The ferry has completed its final voyage and docked at the port. Mission accomplished! ****\n");
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t vehicle_threads[TOTAL_VEHICLES];
    pthread_t ferry;

    for (int i = 0; i < 4; i++)
        pthread_mutex_init(&toll_mutex[i], NULL);

    int id = 0;
    for (int i = 0; i < NUM_CARS; i++, id++) {
        int side = rand() % 2;
        int toll = side == 0 ? rand() % 2 : 2 + rand() % 2;
        vehicles[id] = (Vehicle){.id = id, .type = 1, .size = 1, .side = side, .initial_side = side, .trip_count = 0, .toll_id = toll};
    }
    for (int i = 0; i < NUM_MINIBUSES; i++, id++) {
        int side = rand() % 2;
        int toll = side == 0 ? rand() % 2 : 2 + rand() % 2;
        vehicles[id] = (Vehicle){.id = id, .type = 2, .size = 2, .side = side, .initial_side = side, .trip_count = 0, .toll_id = toll};
    }
    for (int i = 0; i < NUM_TRUCKS; i++, id++) {
        int side = rand() % 2;
        int toll = side == 0 ? rand() % 2 : 2 + rand() % 2;
        vehicles[id] = (Vehicle){.id = id, .type = 3, .size = 3, .side = side, .initial_side = side, .trip_count = 0, .toll_id = toll};
    }

    pthread_create(&ferry, NULL, ferry_thread, NULL);

    for (int i = 0; i < TOTAL_VEHICLES; i++)
        pthread_create(&vehicle_threads[i], NULL, vehicle_thread, &vehicles[i]);

    for (int i = 0; i < TOTAL_VEHICLES; i++)
        pthread_join(vehicle_threads[i], NULL);

    pthread_mutex_lock(&ferry_mutex);
    pthread_cond_broadcast(&ferry_ready);
    pthread_mutex_unlock(&ferry_mutex);

    pthread_join(ferry, NULL);
    return 0;
}
