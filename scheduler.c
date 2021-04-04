#include "scheduler.h
#include "task.h"
#include "priority_queue.h"

#include <stdlib.h> /* malloc(), free(), size_t */
#include <string.h> /*memset() */
#include <assert.h> /* assert()*/
#include <unistd.h> /*sleep() */

enum ActiveOrNot
{
    NOT_ACTIVE,
    ACTIVE
};

enum SuccessOfFailure
{
    SUCCESS,
    FAILURE
};

enum Initilize
{
    INIT
};

struct scheduler
{
    priority_q_t *pq;
    task_t *current_task;
    int is_active;
};

static int CmpIMP(const void *data1, const void *data2, void *param);
static int MatchToEraseIMP(void *data, void *param);
static void RemoveAllTasksIMP(scheduler_t *scheduler);
static int RemoveFromPQIMP(scheduler_t *scheduler, task_t *task_to_remove, ilrd_uid_t uid);

scheduler_t *SchedulerCreate()
{
    scheduler_t *new_scheduler = (scheduler_t *)malloc(sizeof(*new_scheduler));

    if (NULL == new_scheduler)
    {
        return NULL;
    }

    new_scheduler->pq = PQCreate(CmpIMP);

    if (NULL == new_scheduler->pq)
    {
        memset(new_scheduler, INIT, sizeof(*new_scheduler));
        free(new_scheduler);
        new_scheduler = NULL;

        return NULL;
    }
    new_scheduler->current_task = NULL;
    new_scheduler->is_active = ACTIVE;
    return new_scheduler;
}

void SchedulerDestroy(scheduler_t *scheduler)
{
    assert(NULL != scheduler);
    assert(NULL != scheduler->pq);

    RemoveAllTasksIMP(scheduler);
    PQDestroy(scheduler->pq);

    memset(scheduler, INIT, sizeof(scheduler_t));
    scheduler->current_task = NULL;
    free(scheduler);
    scheduler = NULL;
}
int SchedulerExecute(scheduler_t *scheduler)
{
    int res = SUCCESS;
    time_t time_to_sleep = INIT;
    time_t new_time = INIT;
    task_t *current_task = NULL;

    assert(NULL != scheduler);
    assert(NULL != scheduler->pq);

    if (PQueueIsEmpty(scheduler->pq) || ACTIVE != SchedulerIsActive(scheduler))
    {
        return FAILURE;
    }

    while (ACTIVE == SchedulerIsActive(scheduler) && !SchedulerIsEmpty(scheduler) && SUCCESS == res)
    {
        current_task = (task_t *)PQDeQueue(scheduler->pq);
        scheduler->current_task = current_task;

        time_to_sleep = TaskGetExecutionTime(current_task) - time(NULL);

        while (INIT < time_to_sleep)
        {
            time_to_sleep = sleep(time_to_sleep);
        }

        if (SUCCESS != TaskExecute(current_task) && NULL != scheduler->current_task)
        {
            SchedulerRemoveTask(scheduler, TaskGetUid(scheduler->current_task));
        }

        if (NULL != scheduler->current_task)
        {
            new_time = TaskGetInterval(current_task) + time(NULL);
            TaskSetExecutionTime(current_task, new_time);
            res = PQEnQueue(scheduler->pq, current_task);
            if (FAILURE == res)
            {
                TaskDestroy(current_task);
            }
        }
    }

    scheduler->current_task = NULL;

    return ACTIVE == SchedulerIsActive(scheduler);
}

void SchedulerClear(scheduler_t *scheduler)
{
    assert(NULL != scheduler);
    assert(NULL != scheduler->pq);

    RemoveAllTasksIMP(scheduler);
}

ilrd_uid_t SchedulerInsertTask(scheduler_t *scheduler,
                               int (*op_func)(void *func_params),
                               void *func_params,
                               size_t interval)
{
    task_t *new_task = NULL;
    time_t time_now = time(NULL);
    int res = SUCCESS;

    assert(NULL != scheduler);
    assert(NULL != scheduler->pq);
    assert(NULL != op_func);
    assert(NOT_ACTIVE != interval);

    new_task = TaskCreate(op_func, func_params, interval);
    if (NULL == new_task)
    {
        return BadUIDCreate();
    }
    TaskSetExecutionTime(new_task, time_now + interval);
    res = PQEnQueue(scheduler->pq, new_task);

    if (FAILURE == res)
    {
        TaskDestroy(new_task);
        return BadUIDCreate();
    }

    return TaskGetUid(new_task);
}

int SchedulerRemoveTask(scheduler_t *scheduler, ilrd_uid_t uid)
{

    task_t *task_to_remove = NULL;
    ilrd_uid_t uid_schedular = BadUIDCreate();
    int res = SUCCESS;

    assert(NULL != scheduler);
    assert(NULL != scheduler->pq);

    if (NULL != scheduler->current_task)
    {
        uid_schedular = TaskGetUid(scheduler->current_task);
        if (UIDISEqual(&uid_schedular, &uid))
        {
            TaskDestroy(scheduler->current_task);
            scheduler->current_task = NULL;
        }
        else
        {
            res = RemoveFromPQIMP(scheduler, task_to_remove, uid);
        }
    }
    else
    {
        res = RemoveFromPQIMP(scheduler, task_to_remove, uid);
    }

    return res;
}

size_t SchedulerSize(scheduler_t *scheduler)
{
    assert(NULL != scheduler);
    assert(NULL != scheduler->pq);

    return PQSize(scheduler->pq);
}

int SchedulerIsEmpty(scheduler_t *scheduler)
{
    assert(NULL != scheduler);
    assert(NULL != scheduler->pq);

    return PQueueIsEmpty(scheduler->pq);
}

void SchedulerStop(scheduler_t *scheduler)
{
    assert(NULL != scheduler);

    scheduler->is_active = NOT_ACTIVE;
}

int SchedulerIsActive(scheduler_t *scheduler)
{
    assert(NULL != scheduler);
    assert(NULL != scheduler->pq);

    return ACTIVE == scheduler->is_active;
}

static int MatchToEraseIMP(void *data, void *param)
{
    ilrd_uid_t *uid1 = NULL;
    ilrd_uid_t uid2;

    assert(NULL != data);
    assert(NULL != param);

    uid1 = (ilrd_uid_t *)param;
    uid2 = TaskGetUid((task_t *)data);

    return UIDISEqual(uid1, &uid2);
}

static int CmpIMP(const void *data1, const void *data2, void *param)
{
    task_t *task1 = NULL;
    task_t *task2 = NULL;

    assert(NULL != data1);
    assert(NULL != data2);

    task1 = (task_t *)data1;
    task2 = (task_t *)data2;

    (void)param;
    return (int)(TaskGetExecutionTime(task2) - TaskGetExecutionTime(task1));
}

static void RemoveAllTasksIMP(scheduler_t *scheduler)
{
    task_t *task_to_remove = NULL;

    assert(NULL != scheduler);
    assert(NULL != scheduler->pq);

    if (NULL != scheduler->current_task)
    {
        free(scheduler->current_task);
        scheduler->current_task = NULL;
    }

    while (!PQueueIsEmpty(scheduler->pq))
    {
        task_to_remove = (task_t *)PQDeQueue(scheduler->pq);
        TaskDestroy(task_to_remove);
    }
}

static int RemoveFromPQIMP(scheduler_t *scheduler, task_t *task_to_remove, ilrd_uid_t uid)
{
    task_to_remove = (task_t *)PQErase(scheduler->pq, MatchToEraseIMP, (const void *)&uid);

    if (NULL == task_to_remove)
    {
        return FAILURE;
    }
    TaskDestroy(task_to_remove);
    return SUCCESS;
}
