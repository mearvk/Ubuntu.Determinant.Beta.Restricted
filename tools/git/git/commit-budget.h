/*
 * Native commit-part planning policy for the Ubuntu Determinant Git tree.
 *
 * This is the first implementation step for the integer commit method:
 *
 *     git commit N
 *
 * where N describes N ordered 50 MiB commit units.  The planner is deliberately
 * kept separate from builtin/commit.c while the policy is being established;
 * the normal Git commit path is not changed by this header alone.
 *
 * The corresponding push policy is four 50 MiB units (200 MiB) per transport
 * budget.  This header provides the shared arithmetic and the persistent-plan
 * record shape that the commit and push paths can consume once integrated.
 */
#ifndef GIT_COMMIT_BUDGET_H
#define GIT_COMMIT_BUDGET_H

#include "git-compat-util.h"

#include <stdint.h>
#include <inttypes.h>

#define GIT_COMMIT_PART_BYTES ((uintmax_t)50 * 1024 * 1024)
#define GIT_COMMIT_PUSH_PARTS 4U
#define GIT_COMMIT_PUSH_BYTES \
	(GIT_COMMIT_PART_BYTES * (uintmax_t)GIT_COMMIT_PUSH_PARTS)

/* A commit request is an integer number of 50 MiB logical units. */
struct git_commit_part_plan {
	uintmax_t requested_units;
	uintmax_t logical_bytes;
	uintmax_t part_bytes;
	unsigned int push_parts;
};

static inline int git_commit_part_plan_init(
	struct git_commit_part_plan *plan, uintmax_t units)
{
	if (!plan || !units)
		return -1;

	/* Protect the multiplication from wrapping on unusually large input. */
	if (units > UINTMAX_MAX / GIT_COMMIT_PART_BYTES)
		return -1;

	plan->requested_units = units;
	plan->logical_bytes = units * GIT_COMMIT_PART_BYTES;
	plan->part_bytes = GIT_COMMIT_PART_BYTES;
	plan->push_parts = GIT_COMMIT_PUSH_PARTS;
	return 0;
}

static inline uintmax_t git_commit_part_count_for_push(uintmax_t units)
{
	if (!units)
		return 0;
	return (units + GIT_COMMIT_PUSH_PARTS - 1) / GIT_COMMIT_PUSH_PARTS;
}

/*
 * Validate a fully-populated plan record. Returns 0 when the record is
 * internally consistent with the compiled policy, negative otherwise.
 * The authoritative definition lives in commit-budget.c.
 */
int git_commit_part_plan_validate(const struct git_commit_part_plan *plan);

/*
 * Return the number of 50 MiB units carried by push number push_index
 * (1-based) when a plan of the given size is split into 200 MiB pushes.
 * Returns 0 when push_index is beyond the last push. The authoritative
 * definition lives in commit-budget.c.
 */
uintmax_t git_commit_part_units_for_push(uintmax_t units,
					 unsigned int push_index);

#endif /* GIT_COMMIT_BUDGET_H */
