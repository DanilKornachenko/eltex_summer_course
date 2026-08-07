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
#include <unistd.h>

int key = 0;
int mem = 0;
void* memory = NULL;
int offset = 0;
int sem = 0;

struct sembuf lock = {0, -1, 0};
struct sembuf unlock = {0, 1, 0};

static void manufacturer_init() {
  key = ftok("mem", 1);
  mem = shmget(key, MEM_SIZE, IPC_CREAT | 0666);
  if (mem == -1) {
    perror("shget ");
    exit(1);
  }
  memory = shmat(mem, NULL, 0);

  sem = semget(ftok("sem", 1), 1, IPC_CREAT | 0666);
  semctl(sem, 0, SETVAL, 1);

}

static void delete_segments()
{
  shmdt(memory);

  shmctl(mem, IPC_RMID, NULL);

  semctl(sem, 0, IPC_RMID);
}

static int all_processed()
{
  int off = 0;
  while (1)
  {
    int* block = (int*)((char*)memory + off);
    int len = block[0];
    int next_offset = block[1];
    if (len != 0) return 0;
    if (next_offset == 0) break;
    off = next_offset;
  }
  return 1;
}

void manufacturer_loop() {
  manufacturer_init();

  while (1) {
    int n = rand() % 100;
    if (n == 0) continue;

    semop(sem, &lock, 1);

    int needed = 2 * sizeof(int) + n * sizeof(int);
    if (needed + offset > MEM_SIZE) {
      int* dest_len = (int*)((char*)memory + offset);
      *dest_len = 0;
      dest_len++;
      *dest_len = 0;
      semop(sem, &unlock, 1);
      while (!all_processed())
      {
        sleep(rand() % 5);
      }
      delete_segments();
      break;
    }
    else {
      int* numbers = calloc(n, sizeof(int));

      for (int i = 0; i < n; i++) {
        numbers[i] = rand() % 100;
      }

      int* dest_len = (int*)((char*)memory + offset);
      *dest_len = n;
      int next_offset = offset + needed;
      *((int*)((char*)memory + offset + sizeof(int))) = next_offset;
      memcpy((char*)memory + offset + 2 * sizeof(int), numbers, n * sizeof(int));

      offset += needed;
    }
    semop(sem, &unlock, 1);

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
  if (sem == -1)
  {
    perror("sem ");
    exit(1);
  }
}

void consumer_loop()
{
  consumer_init();

  int current_offset = 0;

  while (1)
  {
    semop(sem, &lock, 1);

    int* block = (int*)((char*)memory + current_offset);

    int len = block[0];
    int next_offset = block[1];

    if (len > 0) {

      int* data = malloc(len * sizeof(int));

      memcpy(data, (char*)memory + current_offset + 2 * sizeof(int), len * sizeof(int));

      int min = data[0], max = data[0];
      for (int i = 0; i < len; i++) {
        if (data[i] < min) min = data[i];
        if (data[i] > max) max = data[i];
      }
      printf("min=%d, max=%d\n", min, max);

      block[0] = 0;

      free(data);

      current_offset = next_offset;

    } else if (len == 0 && next_offset == 0) {
      semop(sem, &unlock, 1);
      break;
    } else {
      current_offset = next_offset;
    }

    semop(sem, &unlock, 1);
    sleep(rand() % 5);
  }
}
