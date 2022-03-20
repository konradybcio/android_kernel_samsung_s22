/*
 * On-time Migration Feature for Exynos Mobile Scheduler (EMS)
 *
 * Copyright (C) 2018 Samsung Electronics Co., Ltd
 * LEE DAEYEONG <daeyeong.lee@samsung.com>
 */

#include "../sched.h"
#include "ems.h"

#include <trace/events/ems.h>
#include <trace/events/ems_debug.h>

/****************************************************************/
/*			On-time migration			*/
/****************************************************************/
static int ontime_enabled[CGROUP_COUNT];

static int support_ontime(struct task_struct *p)
{
	int group_idx = cpuctl_task_group_idx(p);

	return ontime_enabled[group_idx];
}

static struct list_head *dom_list;

static LIST_HEAD(default_dom_list);

static struct ontime_dom *get_dom(int cpu, struct list_head *list)
{
	struct ontime_dom *dom;

	list_for_each_entry(dom, list, node)
		if (cpumask_test_cpu(cpu, &dom->cpus))
			return dom;

	return NULL;
}

static unsigned long get_upper_boundary(int cpu, struct task_struct *p)
{
	struct ontime_dom *dom = get_dom(cpu, dom_list);

	if (!dom)
		return ULONG_MAX;

	return dom->upper_boundary;
}

static inline unsigned long get_lower_boundary(int cpu, struct task_struct *p)
{
	struct ontime_dom *dom = get_dom(cpu, dom_list);

	if (!dom)
		return ULONG_MAX;

	return dom->lower_boundary;
}

/* Structure of ontime migration environment */
struct ontime_env {
	struct rq		*dst_rq;
	struct rq		*src_rq;
	struct task_struct	*target_task;
};
static struct ontime_env __percpu *ontime_env;

static inline int check_migrate_slower(int src, int dst)
{
	if (cpumask_test_cpu(src, cpu_coregroup_mask(dst)))
		return false;

	if (capacity_cpu(src) > capacity_cpu(dst))
		return true;
	else
		return false;
}

void ontime_select_fit_cpus(struct task_struct *p, struct cpumask *fit_cpus)
{
	struct ontime_dom *dom;
	int src_cpu = task_cpu(p);
	u32 util = ml_uclamp_task_util(p);
	struct cpumask mask;
	struct list_head *list = dom_list;

	dom = get_dom(src_cpu, list);
	if (!dom)
		return;

	/*
	 * If the task belongs to a group that does not support ontime
	 * migration, all active cpus are fit.
	 */
	if (!support_ontime(p)) {
		cpumask_copy(&mask, cpu_active_mask);
		goto done;
	}

	/*
	 * case 1) task util < lower boundary
	 *
	 * If task 'util' is smaller than lower boundary of current domain,
	 * do not target specific cpu because ontime migration is not involved
	 * in down migration. All active cpus are fit.
	 *
	 * fit_cpus = cpu_active_mask
	 */
	if (util < dom->lower_boundary) {
		cpumask_copy(&mask, cpu_active_mask);
		goto done;
	}

	cpumask_clear(&mask);

	/*
	 * case 2) lower boundary <= task util < upper boundary
	 *
	 * If task 'util' is between lower boundary and upper boundary of
	 * current domain, both current and faster domain are fit.
	 *
	 * fit_cpus = current cpus & faster cpus
	 */
	if (util < dom->upper_boundary) {
		cpumask_or(&mask, &mask, &dom->cpus);
		list_for_each_entry_continue(dom, list, node)
			cpumask_or(&mask, &mask, &dom->cpus);

		goto done;
	}

	/*
	 * case 3) task util >= upper boundary
	 *
	 * If task 'util' is greater than boundary of current domain, only
	 * faster domain is fit to gurantee cpu performance.
	 *
	 * fit_cpus = faster cpus
	 */
	list_for_each_entry_continue(dom, list, node)
		cpumask_or(&mask, &mask, &dom->cpus);

done:
	/*
	 * Check if the task is given a mulligan.
	 * If so, remove src_cpu from candidate.
	 */
	if (EMS_PF_GET(p) & EMS_PF_MULLIGAN)
		cpumask_clear_cpu(src_cpu, &mask);

	cpumask_copy(fit_cpus, &mask);
}

static struct task_struct *pick_heavy_task(struct rq *rq)
{
	struct task_struct *p, *heaviest_p = NULL;
	unsigned long util, max_util = 0;
	int task_count = 0;

	list_for_each_entry(p, &rq->cfs_tasks, se.group_node) {
		if (tex_task(p)) {
			heaviest_p = p;
			break;
		}

		if (!support_ontime(p))
			continue;

		/*
		 * Pick the task with the biggest util among tasks whose
		 * util is greater than the upper boundary.
		 */
		util = ml_uclamp_task_util(p);
		if (util < get_upper_boundary(task_cpu(p), p))
			continue;

		if (util > max_util) {
			heaviest_p = p;
			max_util = util;
		}

		if (++task_count >= TRACK_TASK_COUNT)
			break;
	}

	return heaviest_p;
}

static bool can_migrate(struct task_struct *p, struct ontime_env *env)
{
	struct rq *src_rq = env->src_rq, *dst_rq = env->dst_rq;

	if (p->exit_state)
		return false;

	if (!cpu_active(dst_rq->cpu))
		return false;

	if (unlikely(src_rq != task_rq(p)))
		return false;

	if (unlikely(src_rq->cpu != smp_processor_id()))
		return false;

	if (src_rq->nr_running <= 1)
		return false;

	if (!cpumask_test_cpu(dst_rq->cpu, p->cpus_ptr))
		return false;

	if (task_running(src_rq, p))
		return false;

	return true;
}

static void move_task(struct task_struct *p, struct ontime_env *env)
{
	p->on_rq = TASK_ON_RQ_MIGRATING;
	deactivate_task(env->src_rq, p, 0);
	set_task_cpu(p, env->dst_rq->cpu);

	activate_task(env->dst_rq, p, 0);
	p->on_rq = TASK_ON_RQ_QUEUED;
	check_preempt_curr(env->dst_rq, p, 0);
}

static int move_specific_task(struct task_struct *target, struct ontime_env *env)
{
	struct list_head *tasks = &env->src_rq->cfs_tasks;
	struct task_struct *p, *n;

	list_for_each_entry_safe(p, n, tasks, se.group_node) {
		if (p != target)
			continue;

		move_task(p, env);
		return 1;
	}

	return 0;
}

static int ontime_migration_cpu_stop(void *data)
{
	struct ontime_env *env = data;
	struct rq *src_rq, *dst_rq;
	struct task_struct *p;

	/* Initialize environment data */
	src_rq = env->src_rq;
	dst_rq = env->dst_rq;
	p = env->target_task;

	raw_spin_lock_irq(&src_rq->lock);

	/* Check task can be migrated */
	if (!can_migrate(p, env))
		goto out_unlock;

	BUG_ON(src_rq == dst_rq);

	/* Move task from source to destination */
	double_lock_balance(src_rq, dst_rq);
	if (move_specific_task(p, env)) {
		trace_ontime_migration(p, ml_uclamp_task_util(p),
				src_rq->cpu, dst_rq->cpu, "heavy");
	}
	double_unlock_balance(src_rq, dst_rq);

out_unlock:
	src_rq->active_balance = 0;

	raw_spin_unlock_irq(&src_rq->lock);
	put_task_struct(p);

	return 0;
}

static bool ontime_check_runnable(struct rq *rq)
{
	return rq->nr_running > 1 &&
		mlt_art_last_value(cpu_of(rq)) == SCHED_CAPACITY_SCALE;
}

static struct rq *ontime_find_mulligan_rq(struct task_struct *target_p,
					  struct ontime_env *env)
{
	struct rq *src_rq = env->src_rq, *dst_rq;
	int dst_cpu;

	/* Set flag to skip src_cpu when iterating candidates */
	EMS_PF_SET(target_p, EMS_PF_MULLIGAN);

	/* Find next cpu for this task */
	dst_cpu = ems_select_task_rq_fair(target_p, cpu_of(src_rq), 0, 0);

	/* Clear flag */
	EMS_PF_CLEAR(target_p, EMS_PF_MULLIGAN);

	if (dst_cpu < 0 || cpu_of(src_rq) == dst_cpu)
		return NULL;

	dst_rq = cpu_rq(dst_cpu);

	return dst_rq;
}

static int ontime_detach_task(struct task_struct *target_p,
			      struct ontime_env *env)
{
	struct rq *src_rq = env->src_rq, *dst_rq = env->dst_rq;
	int dst_cpu = cpu_of(dst_rq);

	/* Deactivate this task to migrate it */
	deactivate_task(src_rq, target_p, 0);
	set_task_cpu(target_p, dst_cpu);

	/* Returning 1 means a task is detached. */
	return 1;
}

static int ontime_attach_task(struct task_struct *target_p,
			      const struct ontime_env *env)
{
	struct rq *src_rq = env->src_rq, *dst_rq = env->dst_rq;
	struct rq_flags rf;

	rq_lock(dst_rq, &rf);
	activate_task(dst_rq, target_p, 0);
	check_preempt_curr(dst_rq, target_p, 0);
	rq_unlock(dst_rq, &rf);

	trace_ontime_migration(target_p, ml_uclamp_task_util(target_p),
			       cpu_of(src_rq), cpu_of(dst_rq), "mulligan");

	/* Returning 1 means a task is attached. */
	return 1;
}

static void ontime_mulligan(void)
{
	struct rq *src_rq = this_rq(), *dst_rq;
	struct list_head *tasks = &src_rq->cfs_tasks;
	struct task_struct *p, *target_p = NULL;
	struct ontime_env local_env = { .src_rq = src_rq };
	unsigned long flags;
	unsigned long min_util = ULONG_MAX, task_util;

	if (!ontime_check_runnable(src_rq))
		return;

	raw_spin_lock_irqsave(&src_rq->lock, flags);

	list_for_each_entry_reverse(p, tasks, se.group_node) {
		/* Current task cannot get mulligan */
		if (&p->se == src_rq->cfs.curr)
			continue;

		/* Find min util task */
		task_util = ml_uclamp_task_util(p);
		if (task_util < min_util) {
			target_p = p;
			min_util = task_util;
		}
	}

	/* No task is given a mulligan */
	if (!target_p) {
		raw_spin_unlock_irqrestore(&src_rq->lock, flags);
		return;
	}

	dst_rq = ontime_find_mulligan_rq(target_p, &local_env);

	/* No rq exists for the mulligan */
	if (!dst_rq) {
		raw_spin_unlock_irqrestore(&src_rq->lock, flags);
		return;
	}

	/* Fill ontime_env and check whether the task can be migrated */
	local_env.dst_rq = dst_rq;
	local_env.target_task = target_p;
	if (!can_migrate(target_p, &local_env)) {
		raw_spin_unlock_irqrestore(&src_rq->lock, flags);
		return;
	}

	/* Finally, the task can be moved! */
	ontime_detach_task(target_p, &local_env);

	raw_spin_unlock_irqrestore(&src_rq->lock, flags);

	/* Let's give a second chance to the task */
	ontime_attach_task(target_p, &local_env);
}

static struct cpu_stop_work __percpu *ontime_work;

static void ontime_heavy_migration(void)
{
	struct rq *rq = this_rq();
	struct ontime_env *env = per_cpu_ptr(ontime_env, rq->cpu);
	struct task_struct *p;
	int dst_cpu;
	unsigned long flags;
	struct ontime_dom *dom;
	struct list_head *list = dom_list;

	dom = get_dom(cpu_of(rq), list);
	if (!dom)
		return;

	/*
	 * No need to traverse rq to find a heavy task
	 * if this CPU utilization is under upper boundary
	 */
	if (ml_cpu_util(cpu_of(rq)) < dom->upper_boundary)
		return;

	raw_spin_lock_irqsave(&rq->lock, flags);

	/*
	 * Ontime migration is not performed when active balance is in progress.
	 */
	if (rq->active_balance) {
		raw_spin_unlock_irqrestore(&rq->lock, flags);
		return;
	}

	/*
	 * No need to migration if source cpu does not have cfs tasks.
	 */
	if (!rq->cfs.curr) {
		raw_spin_unlock_irqrestore(&rq->lock, flags);
		return;
	}

	/*
	 * Pick task to be migrated.
	 * Return NULL if there is no heavy task in rq.
	 */
	p = pick_heavy_task(rq);
	if (!p) {
		raw_spin_unlock_irqrestore(&rq->lock, flags);
		return;
	}

	/* Select destination cpu which the heavy task will be moved */
	dst_cpu = ems_select_task_rq_fair(p, rq->cpu, 0, 0);
	if (dst_cpu < 0 || rq->cpu == dst_cpu) {
		raw_spin_unlock_irqrestore(&rq->lock, flags);
		return;
	}

	get_task_struct(p);

	/* Set environment data */
	env->dst_rq = cpu_rq(dst_cpu);
	env->src_rq = rq;
	env->target_task = p;

	/* Prevent active balance to use stopper for migration */
	rq->active_balance = 1;

	raw_spin_unlock_irqrestore(&rq->lock, flags);

	/* Migrate task through stopper */
	stop_one_cpu_nowait(rq->cpu, ontime_migration_cpu_stop, env,
			per_cpu_ptr(ontime_work, rq->cpu));
}

/****************************************************************/
/*			External APIs				*/
/****************************************************************/
static bool skip_ontime;

void ontime_migration(void)
{
	if (skip_ontime)
		return;

	ontime_mulligan();
	ontime_heavy_migration();
}

int ontime_can_migrate_task(struct task_struct *p, int dst_cpu)
{
	int src_cpu = task_cpu(p);
	u32 util;

	if (!support_ontime(p))
		return true;

	/*
	 * Task is heavy enough but load balancer tries to migrate the task to
	 * slower cpu, it does not allow migration.
	 */
	util = ml_task_util(p);
	if (util >= get_lower_boundary(src_cpu, p) &&
	    check_migrate_slower(src_cpu, dst_cpu)) {
		/*
		 * However, only if the source cpu is overutilized, it allows
		 * migration if the task is not very heavy.
		 * (criteria : task util is under 75% of cpu util)
		 */
		if (cpu_overutilized(src_cpu) &&
			util * 100 < (ml_cpu_util(src_cpu) * 75)) {
			trace_ontime_can_migrate_task(p, dst_cpu, true, "src overutil");
			return true;
		}

		trace_ontime_can_migrate_task(p, dst_cpu, false, "migrate to slower");
		return false;
	}

	trace_ontime_can_migrate_task(p, dst_cpu, true, "n/a");

	return true;
}

/****************************************************************/
/*		   emstune mode update notifier			*/
/****************************************************************/
static int ontime_emstune_notifier_call(struct notifier_block *nb,
				unsigned long val, void *v)
{
	struct emstune_set *cur_set = (struct emstune_set *)v;
	int i;

	for (i = 0; i < CGROUP_COUNT; i++)
		ontime_enabled[i] = cur_set->ontime.enabled[i];

	if (cur_set->ontime.p_dom_list &&
	    !list_empty(cur_set->ontime.p_dom_list))
		dom_list = cur_set->ontime.p_dom_list;

	return NOTIFY_OK;
}

static struct notifier_block ontime_emstune_notifier = {
	.notifier_call = ontime_emstune_notifier_call,
};

/****************************************************************/
/*		  sysbusy state change notifier			*/
/****************************************************************/
static int ontime_sysbusy_notifier_call(struct notifier_block *nb,
					unsigned long val, void *v)
{
	enum sysbusy_state state = *(enum sysbusy_state *)v;

	if (val != SYSBUSY_STATE_CHANGE)
		return NOTIFY_OK;

	skip_ontime = (state > SYSBUSY_STATE1);

	return NOTIFY_OK;
}

static struct notifier_block ontime_sysbusy_notifier = {
	.notifier_call = ontime_sysbusy_notifier_call,
};

/****************************************************************/
/*			initialization				*/
/****************************************************************/

int ontime_init(struct kobject *ems_kobj)
{
	ontime_env = alloc_percpu(struct ontime_env);
	ontime_work = alloc_percpu(struct cpu_stop_work);

	dom_list = &default_dom_list;

	/* Explicitly assigned */
	skip_ontime = false;

	emstune_register_notifier(&ontime_emstune_notifier);
	sysbusy_register_notifier(&ontime_sysbusy_notifier);

	return 0;
}
