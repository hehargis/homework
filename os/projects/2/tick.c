#include "tick.h"
#include "job.h"
#include "utils.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 将作业链分离成：
//
// - 可以进行调度
// - 未到调度就绪时间
// - 已经完成
//
// 三组
//
// @param 当前系统时间
// @param 系统资源状态
// @param 上一轮的可以执行调度的作业链
// @param 上一轮的未就绪作业链
// @param 上一轮已完成作业链
static void
split(const int tick,
      const struct resource *res,
      struct job *ahead,
      struct job *phead,
      struct job *fhead)
{
    struct job *j, *job;

    for (j = ahead; j != NULL && j->next != NULL;) {
        job = j->next;

        // 作业已经完成
        if (job->status == FINISHED) {
            llist_pop(j, job);
            llist_insert_after(fhead, job);
        // 作业尚未就绪
        } else if (!job_is_runnable(tick, res, job)) {
            llist_pop(j, job);
            llist_insert_after(phead, job);
        } else {
            j = j->next;
        }
    }

    for (j = phead; j != NULL && j->next != NULL;) {
        job = j->next;

        // 作业已经完成
        if (job->status == FINISHED) {
            llist_pop(j, job);
            llist_insert_after(fhead, job);
        // 作业已经就绪
        } else if (job_is_runnable(tick, res, job)) {
            llist_pop(j, job);
            llist_insert_after(ahead, job);
        } else {
            j = j->next;
        }
    }

    for (j = fhead; j != NULL && j->next != NULL;) {
        job = j->next;

        if (job->status != FINISHED) {
            llist_pop(j, job);

            // 作业已经就绪
            if (job_is_runnable(tick, res, job))
                llist_insert_after(ahead, job);
            // 作业尚未就绪
            else
                llist_insert_after(phead, job);
        } else {
            j = j->next;
        }
    }
}

int
tick(const struct resource *res, struct job **jobs, scheduler_fn scheduler)
{
    int now;                                        // 系统当前时间
    int ran;                                        // 作业运行时间
    struct job ahead, phead, fhead;                 // 系统作业链
    struct job *just_ran;

    now = 0;
    ahead.next = *jobs;
    phead.next = NULL;
    fhead.next = NULL;
    do {
        split(now, res, &ahead, &phead, &fhead);
        printf("系统时间： %d\t", now);

        if (ahead.next) {                                   // 进入调度
            // TODO mark resource as using
            ran = scheduler(now, &ahead, &just_ran);
            now = now + ran;

            // 更新刚刚运行作业的完成时间
            if (just_ran->status == FINISHED)
                just_ran->ftime = now;
        } else {                                            // 空闲状态
            printf("系统空闲中\n");
            usleep(TICK_MS);
            now = now + 1;
        }
    } while (ahead.next != NULL || phead.next != NULL);

    *jobs = fhead.next;
    return now;
}
