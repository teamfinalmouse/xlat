#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "main.h"

// OS status definitions
#define osOK 0
#define osErrorTimeout -1
#define osErrorParameter -2
#define osErrorNoMemory -3
#define osEventMessage 0x10
#define osEventTimeout 0x20

// Message queue implementation
typedef struct message_node {
    uint32_t info;
    struct message_node *next;
} message_node_t;

typedef struct {
    message_node_t *head;
    message_node_t *tail;
    pthread_mutex_t mutex;
} message_queue_t;

static message_queue_t message_queues[10]; // Support up to 10 different queues
static bool queues_initialized = false;

// Semaphore implementation
typedef struct {
    pthread_mutex_t mutex;
    bool initialized;
} semaphore_t;

static semaphore_t semaphores[10]; // Support up to 10 different semaphores
static bool semaphores_initialized = false;

static void init_queues(void) {
    if (!queues_initialized) {
        for (int i = 0; i < 10; i++) {
            message_queues[i].head = NULL;
            message_queues[i].tail = NULL;
            pthread_mutex_init(&message_queues[i].mutex, NULL);
        }
        queues_initialized = true;
    }
}

static void init_semaphores(void) {
    if (!semaphores_initialized) {
        for (int i = 0; i < 10; i++) {
            pthread_mutex_init(&semaphores[i].mutex, NULL);
            semaphores[i].initialized = true;
        }
        semaphores_initialized = true;
    }
}

SemaphoreHandle_t lvgl_mutex = &semaphores[0];

osMessageQId  msgQGfxTask;
osPoolId  gfxevt_pool;
volatile bool xlat_initialized = true;
enum xlat_mode xlat_mode;

// OS functions:

void NVIC_SystemReset(void) {
    printf("NVIC_SystemReset\n");
    exit(0);
}

void osDelay(uint32_t ms) {
    usleep(ms * 1000);
}

void *osPoolAlloc (osPoolId pool_id) {
    printf("osPoolAlloc (100 bytes)\n");
    return malloc(100);
}

osStatus osPoolFree (osPoolId pool_id, void *block) {
    printf("osPoolFree\n");
    if (block) {
        free(block);
    }
    return 0;
}

osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec) {
    if (!queues_initialized) {
        init_queues();
    }

    if (queue_id >= 10) {
        printf("osMessagePut: Invalid queue ID\n");
        return osErrorParameter;
    }

    message_node_t *node = malloc(sizeof(message_node_t));
    if (!node) {
        printf("osMessagePut: Failed to allocate message node\n");
        return osErrorNoMemory;
    }

    node->info = info;
    node->next = NULL;

    pthread_mutex_lock(&message_queues[queue_id].mutex);
    
    if (message_queues[queue_id].tail) {
        message_queues[queue_id].tail->next = node;
        message_queues[queue_id].tail = node;
    } else {
        message_queues[queue_id].head = node;
        message_queues[queue_id].tail = node;
    }

    pthread_mutex_unlock(&message_queues[queue_id].mutex);
    
    printf("osMessagePut: info=%lu\n", (unsigned long)info);
    return osOK;
}

osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec) {
    if (!queues_initialized) {
        init_queues();
    }

    osEvent event = { 0, };
    
    if (queue_id >= 10) {
        printf("osMessageGet: Invalid queue ID\n");
        event.status = osErrorParameter;
        return event;
    }

    pthread_mutex_lock(&message_queues[queue_id].mutex);
    
    if (message_queues[queue_id].head) {
        message_node_t *node = message_queues[queue_id].head;
        event.value.p = (void *)node->info;
        event.status = osEventMessage;
        
        message_queues[queue_id].head = node->next;
        if (!message_queues[queue_id].head) {
            message_queues[queue_id].tail = NULL;
        }
        
        free(node);
    } else {
        event.status = osEventTimeout;
    }
    
    pthread_mutex_unlock(&message_queues[queue_id].mutex);
    
    printf("osMessageGet: status=%d\n", event.status);
    return event;
}

uint32_t osMessageWaiting(osMessageQId queue_id) {
    if (!queues_initialized) {
        init_queues();
    }

    if (queue_id >= 10) {
        printf("osMessageWaiting: Invalid queue ID\n");
        return 0;
    }

    uint32_t count = 0;
    pthread_mutex_lock(&message_queues[queue_id].mutex);
    
    message_node_t *current = message_queues[queue_id].head;
    while (current) {
        count++;
        current = current->next;
    }
    
    pthread_mutex_unlock(&message_queues[queue_id].mutex);
    
    if (count > 0) {
        printf("osMessageWaiting: count=%lu\n", (unsigned long)count);
    }
    return count;
}

void xSemaphoreTake(SemaphoreHandle_t xSemaphore, int xTicksToWait) {
    if (!semaphores_initialized) {
        init_semaphores();
    }

    semaphore_t *sem = (semaphore_t *)xSemaphore;
    if (!sem || !sem->initialized) {
        printf("xSemaphoreTake: Invalid semaphore handle\n");
        return;
    }

    pthread_mutex_lock(&sem->mutex);
}

void xSemaphoreGive(SemaphoreHandle_t xSemaphore) {
    if (!semaphores_initialized) {
        init_semaphores();
    }

    semaphore_t *sem = (semaphore_t *)xSemaphore;
    if (!sem || !sem->initialized) {
        printf("xSemaphoreGive: Invalid semaphore handle\n");
        return;
    }

    pthread_mutex_unlock(&sem->mutex);
}

// XLAT stubs:

uint32_t xlat_latency_standard_deviation_get(enum latency_type type) {
    printf("[stub] xlat_latency_standard_deviation_get\n");
    return 0;
}

uint32_t xlat_latency_average_get(enum latency_type type) {
    printf("[stub] xlat_latency_average_get\n");
    return 0;
}

uint32_t xlat_last_latency_us_get(enum latency_type type) {
    printf("[stub] xlat_last_latency_us_get\n");
    return 0;
}

uint32_t xlat_latency_count_get(enum latency_type type) {
    printf("[stub] xlat_latency_count_get\n");
    return 0;
}

void xlat_latency_reset(void) {
    printf("[stub] xlat_latency_reset\n");
}

void xlat_auto_trigger_turn_off_action(void) {
    printf("[stub] xlat_auto_trigger_turn_off_action\n");
}

void xlat_auto_trigger_action(void) {
    printf("[stub] xlat_auto_trigger_action\n");
}

uint32_t xlat_counter_1mhz_get(void) {
    static uint32_t counter = 0;
    return counter++;
}

void xlat_print_measurement(void) {
    printf("[stub] xlat_print_measurement\n");
}


// Other stubs:

void tft_init(void) {
    printf("[stub] tft_init\n");
}

void touchpad_init(void) {
    printf("[stub] touchpad_init\n");
}

// Stubs for gfx_settings.c:
void hw_config_input_trigger_set_edge(bool rising) {
    printf("[stub] hw_config_input_trigger_set_edge: rising=%d\n", rising);
}

void hw_config_input_bias(bool enable) {
    printf("[stub] hw_config_input_bias: enable=%d\n", enable);
}

bool hw_config_input_trigger_is_rising_edge(void) {
    printf("[stub] hw_config_input_trigger_is_rising_edge\n");
    return false;
}

bool hw_config_input_bias_get(void) {
    printf("[stub] hw_config_input_bias_get\n");
    return false;
}

