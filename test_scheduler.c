#include <stdio.h> /* printf */
#include "scheduler.h"
#include "priority_queue.h"
#include "task.h"
#include "heap.h"

#define GREEN_COLOR "\x1b[0;32m"
#define RED_COLOR "\x1b[0;31m"
#define DEFAULT_COLOR "\x1b[0;0m"
#define BLUE_COLOR "\x1b[0;34m"

enum EXIT_STATUS
{
	FAILED,
	SUCCESS,
	DONE
};

#define RUN_TEST(result, name)     \
	{                              \
		printf("test %s: ", name); \
		switch (result)            \
		{                          \
		case SUCCESS:              \
			printf(GREEN_COLOR);   \
			printf("Success\n");   \
			printf(DEFAULT_COLOR); \
			break;                 \
		case FAILED:               \
			printf(RED_COLOR);     \
			printf("Fail\n");      \
			printf(DEFAULT_COLOR); \
			break;                 \
		case DONE:                 \
			printf(BLUE_COLOR);    \
			printf("Done\n");      \
			printf(DEFAULT_COLOR); \
		};                         \
	}

int TestSchedulerCreate();
int TestSchedulerStop();
int TestSchedulerIsActive();
int TestSchedulerIsEmpty();
int TestSchedulerInsertTask();
int TestSchedulerSize();
int TestSchedulerRemoveTask();
int TestSchedulerCMP();
int TestSchedulerClear();
int TestSchedulerExecuteSimple();
int TestSchedulerExecuteNotSimple();
int TestSchedulerExecuteClear();
int TestSchedulerDestroy();
int EndScheduler(void *param);
int TimerEndScheduler(void *param);
int NumPlusOne(void *param);
int RemoveOneTask(void *param);
int RemoveSelfTask(void *param);
int SendToClear(void *param);

struct priority_queue
{
	heap_ty *heap;
};

struct scheduler
{
	priority_q_t *pq;
	task_t *current_task;
	volatile int is_active;
};

struct task
{
	ilrd_uid_t uid;
	op_func_t op_func;
	void *func_params;
	size_t interval;
	time_t exec_time;
	int exec_state;
};

struct test_task
{
	scheduler_t *scheduler;
	ilrd_uid_t *uid;
};

scheduler_t *scheduler = NULL;
ilrd_uid_t uid = {-1, -1, 0};
ilrd_uid_t arr_uid[4] = {{-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}, {-1, -1, 0}};

int a = 1;

int EndScheduler(void *param)
{
	SchedulerStop((scheduler_t *)param);

	return 0;
}

int TimerEndScheduler(void *param)
{
	static int counter = 0;

	if (2 == counter++)
	{
		SchedulerStop((scheduler_t *)param);
	}

	return 0;
}

int NumPlusOne(void *param)
{
	int *num = (int *)param;

	*num += 1;

	return 0;
}

int RemoveOneTask(void *param)
{
	struct test_task *temp = (struct test_task *)param;
	ilrd_uid_t *temp_uid = temp->uid;

	return SchedulerRemoveTask(temp->scheduler, *temp_uid);
	;
}

int RemoveSelfTask(void *param)
{
	struct test_task *temp = (struct test_task *)param;
	ilrd_uid_t *temp_uid = temp->uid;

	return SchedulerRemoveTask(temp->scheduler, *temp_uid);
}

int SendToClear(void *param)
{
	SchedulerClear((scheduler_t *)param);

	return 0;
}

int main(int argc, char *argv[])
{
	RUN_TEST(TestSchedulerCreate(), "SchedulerCreate");
	RUN_TEST(TestSchedulerStop(), "SchedulerStop");
	RUN_TEST(TestSchedulerIsActive(), "SchedulerIsActive");
	RUN_TEST(TestSchedulerIsEmpty(), "SchedulerIsEmpty");
	RUN_TEST(TestSchedulerInsertTask(), "SchedulerInsertTask");
	RUN_TEST(TestSchedulerSize(), "SchedulerSize");
	RUN_TEST(TestSchedulerRemoveTask(), "SchedulerRemoveTask");
	RUN_TEST(TestSchedulerCMP(), "SchedulerCMP");
	RUN_TEST(TestSchedulerClear(), "SchedulerClear");

	RUN_TEST(TestSchedulerExecuteSimple(), "SchedulerExecuteSimple");
	RUN_TEST(TestSchedulerExecuteNotSimple(), "SchedulerExecuteNotSimple");
	RUN_TEST(TestSchedulerExecuteClear(), "SchedulerExecuteClear");

	RUN_TEST(TestSchedulerDestroy(), "SchedulerDestroy");

	(void)argc;
	(void)argv;

	return 0;
}

int TestSchedulerCreate()
{
	int result = 0;

	scheduler = SchedulerCreate();
	result += (1 == scheduler->is_active);
	result += (NULL != scheduler->pq);
	/*scheduler->pq->heap->compare
	result += (NULL != scheduler->pq->heap->cmp);*/

	return (2 == result);
}

int TestSchedulerStop()
{
	SchedulerStop(scheduler);

	return (0 == scheduler->is_active);
}

int TestSchedulerIsActive()
{
	int result = 0;

	result += (0 == SchedulerIsActive(scheduler));
	scheduler->is_active = 1;
	result += (1 == SchedulerIsActive(scheduler));

	return (2 == result);
}

int TestSchedulerIsEmpty()
{
	return (1 == SchedulerIsEmpty(scheduler));
}

int TestSchedulerInsertTask()
{
	int result = 0;
	task_t *task = NULL;

	uid = SchedulerInsertTask(scheduler, EndScheduler, (void *)scheduler, 10);
	result += (-1 < uid.pid);
	result += (-1 < uid.time_stamp);
	result += (0 < uid.counter);

	task = (task_t *)PQPeek(scheduler->pq);

	result += (1 == UIDISEqual(&uid, &task->uid));
	result += (EndScheduler == task->op_func);
	result += (scheduler == (scheduler_t *)task->func_params);
	result += (10 == task->interval);
	result += ((time(NULL) + 10) == task->exec_time);
	result += (1 == task->exec_state);

	return (9 == result);
}

int TestSchedulerSize()
{
	return (1 == SchedulerSize(scheduler));
}

int TestSchedulerRemoveTask()
{
	int result = 0;

	result += (0 == SchedulerRemoveTask(scheduler, uid));
	result += (1 == SchedulerIsEmpty(scheduler));
	result += (1 == SchedulerRemoveTask(scheduler, uid));

	return (3 == result);
}

int TestSchedulerCMP()
{
	int result = 0;
	task_t *task = NULL;

	arr_uid[2] = SchedulerInsertTask(scheduler, EndScheduler, (void *)scheduler, 10);
	arr_uid[0] = SchedulerInsertTask(scheduler, NumPlusOne, (void *)&a, 2);
	arr_uid[1] = SchedulerInsertTask(scheduler, RemoveOneTask, (void *)scheduler, 2);

	task = (task_t *)PQDeQueue(scheduler->pq);
	result += (1 == UIDISEqual(arr_uid, &task->uid));
	TaskDestroy(task);
	task = (task_t *)PQDeQueue(scheduler->pq);
	result += (1 == UIDISEqual(&arr_uid[1], &task->uid));
	TaskDestroy(task);
	task = (task_t *)PQDeQueue(scheduler->pq);
	result += (1 == UIDISEqual(&arr_uid[2], &task->uid));
	TaskDestroy(task);

	return (3 == result);
}

int TestSchedulerClear()
{
	SchedulerInsertTask(scheduler, EndScheduler, (void *)scheduler, 3);
	SchedulerInsertTask(scheduler, NumPlusOne, (void *)&a, 2);
	SchedulerInsertTask(scheduler, RemoveOneTask, (void *)scheduler, 2);

	SchedulerClear(scheduler);

	return (1 == SchedulerIsEmpty(scheduler));
}

int TestSchedulerExecuteSimple()
{
	int result = 0;

	SchedulerInsertTask(scheduler, TimerEndScheduler, (void *)scheduler, 2);

	result = (0 == SchedulerExecute(scheduler));

	SchedulerClear(scheduler);
	scheduler->is_active = 1;

	return (1 == result);
}

int TestSchedulerExecuteNotSimple()
{
	int result = 0;
	struct test_task mt1 = {NULL};
	struct test_task mt2 = {NULL};

	arr_uid[3] = SchedulerInsertTask(scheduler, EndScheduler, (void *)scheduler, 5);
	arr_uid[0] = SchedulerInsertTask(scheduler, NumPlusOne, (void *)&a, 1);
	mt1.scheduler = scheduler;
	mt1.uid = &arr_uid[0];
	arr_uid[1] = SchedulerInsertTask(scheduler, RemoveOneTask, (void *)&mt1, 3);
	mt2.scheduler = scheduler;
	arr_uid[2] = SchedulerInsertTask(scheduler, RemoveSelfTask, (void *)&mt2, 4);
	mt2.uid = &arr_uid[2];

	result += (0 == SchedulerExecute(scheduler));
	result += (2 == SchedulerSize(scheduler));
	result += (3 == a);

	SchedulerClear(scheduler);
	scheduler->is_active = 1;

	return (3 == result);
}

int TestSchedulerExecuteClear()
{
	int result = 0;

	SchedulerInsertTask(scheduler, EndScheduler, (void *)scheduler, 4);
	SchedulerInsertTask(scheduler, NumPlusOne, (void *)&a, 1);
	SchedulerInsertTask(scheduler, SendToClear, (void *)scheduler, 2);

	result += (1 == SchedulerExecute(scheduler)); /* Should return 0 */
	result += (1 == SchedulerIsEmpty(scheduler));
	result += (0 == SchedulerIsActive(scheduler));

	return (2 == result);
}

int TestSchedulerDestroy()
{
	SchedulerDestroy(scheduler);

	return DONE;
}
