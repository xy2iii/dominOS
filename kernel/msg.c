#include "msg.h"
#include "task.h"
#include "mem.h"

#define __MQUEUE_UNUSED 0

static struct mqueue *mqueues[NBQUEUE] = { __MQUEUE_UNUSED };

#define GET_MQUEUE_PTR(id) (mqueues[id])
#define MQUEUE_USED(id) (GET_MQUEUE_PTR(id) != MQUEUE_UNUSED)
#define MQUEUE_UNUSED(id) (GET_MQUEUE_PTR(id) == __MQUEUE_UNUSED)
#define MQUEUE_EMPTY(id) (GET_MQUEUE_PTR(id)->count == 0)
#define MQUEUE_FULL(id) (GET_MQUEUE_PTR(id)->count == GET_MQUEUE_PTR(id)->size)

static int first_available_queue(void)
{
	int mqueue_id;
	for (mqueue_id = 0; mqueue_id < NBQUEUE; mqueue_id++) {
		if (MQUEUE_UNUSED(mqueue_id))
			return mqueue_id;
	}
	return mqueue_id;
}

static void alloc_mqueue(int mqueue_id, int count)
{
	struct mqueue * mqueue_ptr = GET_MQUEUE_PTR(mqueue_id);
	mqueue_ptr = (struct mqueue *)mem_alloc(sizeof(struct mqueue));
	mqueue_ptr->head = NULL;
	mqueue_ptr->size = count;
	mqueue_ptr->count = 0;
	INIT_LIST_HEAD(&mqueue_ptr->waiting_senders);
	INIT_LIST_HEAD(&mqueue_ptr->waiting_receivers);
}

static void free_mqueue(int mqueue_id)
{
	struct mqueue * mqueue_ptr = GET_MQUEUE_PTR(mqueue_id);
	struct msg *msg_ptr = mqueue_ptr->head;
	struct msg *next = msg_ptr;
	while(next != NULL){
		next = msg_ptr->next;
		mem_free(msg_ptr, sizeof(struct msg *));
		msg_ptr = next;
	}
	mem_free(mqueue_ptr, sizeof(struct mqueue *));
	GET_MQUEUE_PTR(mqueue_id) = __MQUEUE_UNUSED;
}

int pcreate(int count)
{
	int mqueue_id;
	if (count <= 0)
		return -2;
	if ((mqueue_id = first_available_queue()) == -1)
		return -1;
	alloc_mqueue(mqueue_id, count);
	return mqueue_id;
}

static void __add_msg(int id, int msg)
{
	struct mqueue * mqueue_ptr = GET_MQUEUE_PTR(id);
	struct msg * msg_ptr = mem_alloc(sizeof(struct msg));
	msg_ptr->data = msg;
	msg_ptr->next = mqueue_ptr->head;
	mqueue_ptr->head = msg_ptr;
	mqueue_ptr->count++;
}

static int __pop_msg(int id)
{
	struct mqueue * mqueue_ptr = GET_MQUEUE_PTR(id);
	mqueue_ptr->count--;
	return mqueue_ptr->head->data;
}

int psend(int id, int msg)
{
	if (MQUEUE_UNUSED(id))
		return -1;
	struct task * last = queue_out(&GET_MQUEUE_PTR(id)->waiting_receivers, struct task, tasks);

	// On réveille un processus en attente sur la lecture
	if (MQUEUE_EMPTY(id) && last != NULL) {
		__add_msg(id, msg);

		add_ready_task(last);
		schedule();

		return 0;
	}

	while (MQUEUE_FULL(id)) {
		// DEBUG Il faudra ajouter la task dans waiting_senders
		schedule(); // DEBUG Il faudra passer la task en BLOCKED
	}

	// Test pdelete
	if (MQUEUE_UNUSED(id))
		return -1;

	__add_msg(id, msg);
	return 0;
}

int preceive(int id, int * message)
{
	if (MQUEUE_UNUSED(id))
		return -1;
	struct task * last = queue_out(&GET_MQUEUE_PTR(id)->waiting_senders, struct task, tasks);

	// On réveille un processus en attente sur l'écriture
	if (MQUEUE_FULL(id) && last != NULL) {
		int msg = __pop_msg(id);
		if (message == NULL)
			*message = msg;

		add_ready_task(last);
		schedule();

		return 0;
	}
	
	while (MQUEUE_EMPTY(id)) {
		// DEBUG Il faudra ajouter la task dans waiting_receivers
		schedule(); // DEBUG Il faudra passer la task en BLOCKED
	}

	// Test pdelete
	if (MQUEUE_UNUSED(id))
		return -1;

	int msg = __pop_msg(id);
	if (message == NULL)
		*message = msg;
	return 0;
}

int pdelete(int id)
{
	if (MQUEUE_UNUSED(id))
		return -1;

	// Il faut débloquer les processus en attente avec une valeur négative
	struct task * last = queue_out(&GET_MQUEUE_PTR(id)->waiting_senders, struct task, tasks);
	while(last != NULL){
		add_ready_task(last);
		last = queue_out(&GET_MQUEUE_PTR(id)->waiting_senders, struct task, tasks);
	}
	last = queue_out(&GET_MQUEUE_PTR(id)->waiting_receivers, struct task, tasks);
	while(last != NULL){
		add_ready_task(last);
		last = queue_out(&GET_MQUEUE_PTR(id)->waiting_receivers, struct task, tasks);
	}

    // Liberer les ressources
	free_mqueue(id);

	return 0;
}

int pcount(int id, int *count){
	if (MQUEUE_UNUSED(id))
		return -1;

	if(count == 0)
		return 0;
	
	struct task *p;
	if(GET_MQUEUE_PTR(id)->count == 0){
		int count_wr = 0;
		queue_for_each(p, &GET_MQUEUE_PTR(id)->waiting_receivers, struct task, tasks) {
			count_wr++;
		}
		return -1 * count_wr;
	}else{
		int count_ws = 0;
		queue_for_each(p, &GET_MQUEUE_PTR(id)->waiting_senders, struct task, tasks) {
			count_ws++;
		}
		return count_ws + GET_MQUEUE_PTR(id)->count;
	}
}

int preset(int id){
	if (MQUEUE_UNUSED(id))
		return -1;
	// TODO 
	// vérifier que les messages en attente sur la file avant le reset sont bien supprimés (pareil pour les lectures)
	// Sinon, il faut créer un compteur reset, au début des fonctions on fixe rst = reset
	// Et si on constate rst < reset après le retour du blocage, on return -1

	int count = GET_MQUEUE_PTR(id)->count;
	pdelete(id);
	alloc_mqueue(id, count);

	return 0;
}