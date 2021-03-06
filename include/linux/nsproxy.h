#ifndef _LINUX_NSPROXY_H
#define _LINUX_NSPROXY_H

#include <linux/spinlock.h>
#include <linux/sched.h>

struct namespace;
struct uts_namespace;
struct ipc_namespace;

/*
 * A structure to contain pointers to all per-process
 * namespaces - fs (mount), uts, network, sysvipc, etc.
 *
 * 'count' is the number of tasks holding a reference.
 * The count for each namespace, then, will be the number
 * of nsproxies pointing to it, not the number of tasks.
 *
 * The nsproxy is shared by tasks which share all namespaces.
 * As soon as a single namespace is cloned or unshared, the
 * nsproxy is copied.
 */
struct nsproxy {
	atomic_t count;
	spinlock_t nslock;
	struct uts_namespace *uts_ns;
	struct ipc_namespace *ipc_ns;
	struct namespace *namespace;
};
extern struct nsproxy init_nsproxy;

struct nsproxy *dup_namespaces(struct nsproxy *orig);
int copy_namespaces(int flags, struct task_struct *tsk);
void get_task_namespaces(struct task_struct *tsk);
void free_nsproxy(struct nsproxy *ns);

static inline void put_nsproxy(struct nsproxy *ns)
{
	if (atomic_dec_and_test(&ns->count)) {
		free_nsproxy(ns);
	}
}

static inline void exit_task_namespaces(struct task_struct *p)
{
	struct nsproxy *ns = p->nsproxy;
	if (ns) {
		task_lock(p);
		p->nsproxy = NULL;
		task_unlock(p);
		put_nsproxy(ns);
	}
}
#endif
