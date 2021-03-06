#ifndef __MSG__H__
#define __MSG__H__

#include "queue.h"
#include "task.h"

#define NBQUEUE 20

struct mqueue {
    struct msg *head; /* First message */
    struct msg *tail; /* Last message */
    unsigned int size; /* Max number of messages */
    unsigned int count; /* Number of messages */
    struct list_link waiting_senders;
    struct list_link waiting_receivers;
};

struct msg {
    struct msg *next;
    int data;
};

// Crée une file de messages
int pcreate(int count);

// Détruit une file de messages
int pdelete(int id);

// Dépose un message dans une file
int psend(int id, int msg);

// Retire un message d'une file
int preceive(int id, int *msg);

// Réinitialise une file
int preset(int id);

// Renvoie l'état courant d'une file
int pcount(int id, int *count);

// Renvoie la liste dans laquelle le pid est présent
struct list_link *queue_from_msg(int pid);

/**
 * Delete and reinsert a process in a message queue.
 */
void msg_reinsert(struct task *self);
/**
 * Get the queue for a msg queue, useful to allow
 * modification (deleting the task...)
 */
struct list_link *queue_from_msg(int pid);

#endif