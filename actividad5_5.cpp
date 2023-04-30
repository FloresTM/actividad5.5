/*
    Actividad 5.5 Problemas de sincronización 2
    Hecho por:
    David Langarica | A01708936
    Sebastian Flores Lemus | A01709229
*/

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <cstdint>

// Definimos numero máuximo de vehiculos en el puente
const int MAX_CARS_ON_BRIDGE = 3;
// Creamos 10 hilos o vehiculos
const int NUM_CARS = 10;

// Inicializamos mutex y las variables de condicion
pthread_mutex_t bridge_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t id_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t north_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t south_cond = PTHREAD_COND_INITIALIZER;

// Creamos variables para contar vehículos en el puente y en cada direccion
int north_cars = 0;
int south_cars = 0;
int cars_on_bridge = 0;
int car_id = 1;

// Funcion para que un vehículo llegue al puente y espere si es necesario
void ArriveBridge(int direction) {
    pthread_mutex_lock(&bridge_mutex);

    if (direction == 0) {
        while (south_cars > 0 || cars_on_bridge == MAX_CARS_ON_BRIDGE) {
            pthread_cond_wait(&north_cond, &bridge_mutex);
        }
        north_cars++;
        cars_on_bridge++;
    } else {
        while (north_cars > 0 || cars_on_bridge == MAX_CARS_ON_BRIDGE) {
            pthread_cond_wait(&south_cond, &bridge_mutex);
        }
        south_cars++;
        cars_on_bridge++;
    }

    pthread_mutex_unlock(&bridge_mutex);
}

// Simulamos el cruce del puente
void CrossBridge(int direction, int id) {
    std::cout << "Vehiculo " << id << " cruzando en direccion " << (direction == 0 ? "NORTE a SUR" : "SUR a NORTE") << std::endl;
    sleep(1);
}

//  Vehiculo sale del puente y notifica a otros vehículos
void ExitBridge(int direction) {
    pthread_mutex_lock(&bridge_mutex);

    if (direction == 0) {
        north_cars--;
        cars_on_bridge--;
        if (north_cars == 0) {
            pthread_cond_broadcast(&south_cond);
        }
    } else {
        south_cars--;
        cars_on_bridge--;
        if (south_cars == 0) {
            pthread_cond_broadcast(&north_cond);
        }
    }

    pthread_mutex_unlock(&bridge_mutex);
}

// Funcion que representa el proceso completo de un vehículo cruzando el puente
void OneVehicle(int direction, int id) {
    ArriveBridge(direction);
    CrossBridge(direction, id);
    ExitBridge(direction);
}

// Cada vehiculo es un hilo
void* car_thread(void* arg) {
    int direction = *((int*) arg);
    // Asignamos id  a cada vehiculo
    pthread_mutex_lock(&id_mutex);
    int id = car_id++;

pthread_mutex_unlock(&id_mutex);
    OneVehicle(direction, id);
    pthread_exit(NULL);
}

int main() {
    pthread_t car_threads[NUM_CARS];
    int directions[NUM_CARS];

    for (int i = 0; i < NUM_CARS; i++) {
        directions[i] = i % 2;
        pthread_create(&car_threads[i], NULL, car_thread, &directions[i]);
        sleep(1);
    }

    for (int i = 0; i < NUM_CARS; i++) {
        pthread_join(car_threads[i], NULL);
    }

    return 0;
}