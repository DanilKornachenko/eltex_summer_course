#include "workers.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>

int key = 0;
int mem = 0;
void* memory = NULL;
int offset = 0;
int sem = 0;

struct sembuf lock = {0, -1, 0};
struct sembuf unlock[2] = {{0, 0, 0},
                            {0, 1, 0}};

static void manufacturer_init() {
  key = ftok("mem", 1);
  mem = shmget(key, MEM_SIZE, 0666);
  if (mem == -1) {
    perror("shget ");
    exit(1);
  }
  memory = shmat(mem, NULL, 0);

  sem = semget(ftok("sem", 1), 1, 0666);

}

void manufacturer_loop() {
  manufacturer_init();

  while (1) {
    int n = rand() % 100;
    if (n == 0) continue;

    int* head = (int*)memory;
    int needed = sizeof(int) + n * sizeof(int);
    if (needed + offset > MEM_SIZE) {
      int* dest_len = (int*)((char*)memory + offset);
      *dest_len = 1;
      dest_len++;
      *dest_len = 0;
    }
    else {
      int* numbers = calloc(n, sizeof(int));

      for (int i = 0; i < n; i++) {
        numbers[i] = rand() % 100;
      }

      int* dest_len = (int*)((char*)memory + offset);
      int* next_addr = (int*)((char*)memory + (offset + n));

      *dest_len = n;
      dest_len++;
      *dest_len = (int)next_addr;
      dest_len++;
      memcpy(dest_len, numbers, n * sizeof(int));

      offset += needed;
    }
    semop(sem, unlock, 0);

  }
}

static void consumer_init()
{
  key = ftok("mem", 1);
  mem = shmget(key, MEM_SIZE, 0666);
  if (mem == -1) {
    perror("shget ");
    exit(1);
  }
  memory = shmat(mem, NULL, 0);

  sem = semget(ftok("sem", 1), 1, 0666);
}

void consumer_loop()
{
  consumer_init();

  while (1)
  {

  }

  semop(sem, &lock, 0);
}
