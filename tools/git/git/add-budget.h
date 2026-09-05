/*
 * Ubuntu Determinant native add planner.
 *
 * This header establishes the presumed policy for an intelligent native
 * `git add` implementation. The policy is intentionally planning-oriented:
 * staged paths are considered in deterministic breadth-first pathname order
 * and grouped into 100 MiB blocks. It does not replace Git's normal index
 * semantics until the corresponding builtin/add.c integration is installed.
 */

#ifndef GIT_ADD_BUDGET_H
#define GIT_ADD_BUDGET_H

#include "git-compat-util.h"
#include "object.h"

/*
 * The project calls these "100 MB blocks". We use MiB here so that the
 * boundary is deterministic and consistent with the existing 200 MiB push
 * ceiling: four 50 MiB commit units form one 200 MiB push budget.
 */
#define GIT_ADD_BLOCK_BYTES ((uintmax_t)100 * 1024 * 1024)
#define GIT_ADD_BLOCKS_PER_PUSH ((uintmax_t)2)
#define GIT_ADD_BLOCKS_PER_200M_PUSH ((uintmax_t)2)

struct git_add_block_plan {
	uintmax_t block_number;
	uintmax_t bytes;
	uintmax_t path_count;
	int complete;
};

/*
 * Return the 1-based block number that a cumulative byte position falls in.
 * A cumulative size of 0 bytes is reported as block 1. Callers must perform
 * the actual pathname traversal and object-size accounting; this API
 * deliberately contains no filesystem assumptions.
 *
 * The single authoritative definition lives in add-budget.c.
 */
uintmax_t git_add_block_for_bytes(uintmax_t cumulative_bytes);

/*
 * Return non-zero when appending object_bytes to a block that already holds
 * current_bytes would push the running total across a 100 MiB block boundary.
 * Overflow of the running total is treated as crossing a boundary.
 */
int git_add_block_would_cross_boundary(uintmax_t current_bytes,
					uintmax_t object_bytes);

#endif /* GIT_ADD_BUDGET_H */
