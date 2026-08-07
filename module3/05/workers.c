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
#include <semaphore.h>

#define SHM_NAME "/my_shm"
#define SEM_NAME "/my_sem"

static int shm_fd = -1;
static sem_t* sem = SEM_FAILED;
void* memory = NULL;
int offset = 0;

static void manufacturer_init() {
  shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open ");
    exit(1);
  }

  if (ftruncate(shm_fd, MEM_SIZE) == -1) {
    perror("ftruncate ");
    exit(1);
  }

  memory = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (memory == MAP_FAILED) {
    perror("mmap ");
    exit(1);
  }
  close(shm_fd);

  sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
  if (sem == SEM_FAILED) {
    perror("sem_open ");
    exit(1);
  }
}

static void delete_segments()
{
  if (memory != NULL && memory != MAP_FAILED) {
    munmap(memory, MEM_SIZE);
    memory = NULL;
  }
  if (shm_unlink(SHM_NAME) == -1) {
    perror("shm_unlink ");
  }
  if (sem != SEM_FAILED) {
    sem_close(sem);
    sem = SEM_FAILED;
  }
  if (sem_unlink(SEM_NAME) == -1) {
    perror("sem_unlink ");
  }
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

    sem_wait(sem);

    int needed = 2 * sizeof(int) + n * sizeof(int);
    if (needed + offset > MEM_SIZE) {
      int* dest_len = (int*)((char*)memory + offset);
      *dest_len = 0;
      dest_len++;
      *dest_len = 0;
      sem_post(sem);
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
    sem_post(sem);

  }
}

static void consumer_init()
{
  shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open (consumer) ");
    exit(1);
  }

  memory = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

  if (memory == MAP_FAILED) {
    perror("mmap (consumer) ");
    exit(1);
  }
  close(shm_fd);

  sem = sem_open(SEM_NAME, 0);
  if (sem == SEM_FAILED) {
    perror("sem_open (consumer) ");
    exit(1);
  }
}

void consumer_loop()
{
  consumer_init();

  int current_offset = 0;

  while (1)
  {
    sem_wait(sem);

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
      sem_post(sem);
      break;
    } else {
      current_offset = next_offset;
    }

    sem_post(sem);
    sleep(rand() % 5);
  }

  if (memory != NULL && memory != MAP_FAILED) {
    munmap(memory, MEM_SIZE);
    memory = NULL;
  }
  if (sem != SEM_FAILED) {
    sem_close(sem);
    sem = SEM_FAILED;
  }
}
