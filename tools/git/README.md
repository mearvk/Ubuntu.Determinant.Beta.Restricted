<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# Git Tooling

This directory contains small, self-contained tooling for working with Git in
the Ubuntu Determinant build and source-management environment.

It has three layers:

1. **Source scripts** — `pull-source.sh` / `verify-source.sh` for acquiring and
   checking a source tree.
2. **The workflow surface** — `git-workflow.sh`, a safe, non-destructive command
   wrapper that also exposes the Ubuntu Determinant Git operations.
3. **Native policy source** — `git/`, the vendored Git tree carrying the native
   operation modules (headers + C/C++ companions) and their build.

Each operation is documented in a companion metadoc (see the index at the end)
and declared in the line-oriented contract `GIT_OPERATIONS.logic`.

## `pull-source.sh`

`pull-source.sh` provides a safe, reproducible wrapper for acquiring a Git
repository into a local source directory.

Design goals:

- Do not require interactive username/password prompts for public repositories.
- Use HTTPS by default.
- Disable configured Git credential helpers unless the caller explicitly opts in.
- Support a pinned branch, tag, or commit through `GIT_SOURCE_REF`.
- Support shallow clones for build environments through `GIT_CLONE_DEPTH`.
- Keep downloaded source separate from our customization/offset layers.
- Fail rather than silently continue when the source cannot be obtained.
- Record the resolved source commit in `.source-commit`.

## `verify-source.sh`

`verify-source.sh` is a read-only provenance and cleanliness check for a source
tree acquired by `pull-source.sh`.

It verifies:

- the destination is a Git repository;
- `HEAD` can be resolved;
- when `.source-commit` exists, it exactly matches `HEAD`;
- an optional expected branch/ref exists locally;
- the tracked working tree has no modifications; and
- no untracked files are present.

## `git-workflow.sh`

`git-workflow.sh` is a small, non-destructive command surface around Git for
repository maintenance. It intentionally refuses destructive operations such as
`reset --hard` and force pushes. Every command takes an optional repository path
as its first argument (defaulting to the current directory).

| Command | Purpose |
|---|---|
| `status [repo]` | short branch/status |
| `verify [repo]` | repository integrity (`fsck`, HEAD, object count) |
| `log [repo] [count]` | decorated graph log |
| `branches [repo]` | list all branches verbosely |
| `fetch [repo] [remote]` | prune-fetch a remote |
| `sync [repo] [remote] [branch]` | fast-forward-only sync (refuses a dirty tree) |
| `stage [repo] [pathspec...]` | `git add` the given paths |
| `commit [repo] <message>` | commit staged changes |
| `push [repo] [remote] [branch]` | set-upstream push |
| `premount [repo] [--add\|--commit\|--both]` | read-only inventory of newly added/committed files |
| `premount [repo] push [remote] [branch] [--sha512] [--attempts N]` | referenced 200 MiB ordered add/commit sequence |
| `commit-parts [repo] <message>` | ordered, non-destructive part commit |
| `push-resume [repo] [remote] [branch] [--attempts N]` | resumable push for lossy links |
| `temperature [repo] [--recandle] [--min-idle DAYS]` | advisory project staleness / recandle scan |

## Ubuntu Determinant Git operations

The Ubuntu Determinant "Git method" adds a set of operations that supplement
Git without changing object identity or history. Each has native policy source
in `git/` (a `<op>.h` contract plus C and C++ companions), a metadoc, and a
block in `GIT_OPERATIONS.logic`. The current operation set is:

```
premount, add, commit, push, resume, merge, rebase, restage, autocheck, moral, temperature, alter-comment, propath, messages
```

Highlights:

- **add / commit / push budgets** — deterministic 100 MiB add blocks, 50 MiB
  commit units, and a hard 200 MiB push ceiling (`add-budget.h`,
  `commit-budget.h`, `push-budget.h`).
- **premount** — a read-only inventory of newly added/committed files, and
  `premount push`, a referenced ordered-transfer plan (see below).
- **resume** — checkpoint/resume for slow or lossy connections (see below).
- **temperature** — an advisory AI scan for stale / recandle-worthy projects
  (see below).
- **autocheck / restage / merge / rebase / moral** — pull-then-prepare,
  two-level index history, future-base merge metadata, schedule-relative rebase
  metadata, and the symbolic blessing operation, respectively.
- **alter-comment** — `git alter-comment X "message"` alters only the messages
  of the last `X` commits, addressed by an integer count; trees, authorship, and
  dates are preserved (`alter-comment.h`, see `ALTER_COMMENT.md`).
- **propath** — `git propath URL [PATH ...]` clones only a tuple of paths and
  fetches the rest on demand (shallow/iterative or recursive), evicting unused
  local copies after ~2 days; config-file driven (`propath.h`, see
  `PROPATH.md`).
- **messages** — a centralized catalog of the stdout/stderr text the Edition
  Git emits and the concern classes it reports (memory bloat, disk space,
  missing files, size ceiling, overflow, permission, corruption, …). Seeded
  from a per-repository config document (recommended `.gitmessages`) that
  mirrors `messages.h`; the command surface reads it as a reference/input and
  falls back to compiled defaults. Advisory presentation only — it never
  changes behaviour or hides a diagnostic (`messages.h`, see `MESSAGES.md`).
  A tree-wide print listener (`gitmsg-listen.h`/`.c`, force-included via the
  opt-in `git-listen` build) brings the compiled `git` binary under the same
  catalog: every stdout/stderr print is intercepted with its call site and the
  `[MAP]` rules decide the real message, defaulting to verbatim pass-through so
  Git's data output is unchanged unless explicitly mapped. A native config
  loader (`gitmsg-config.h`/`.c`) reads `.gitmessages` at startup (overriding
  wordings and adding `[MAP]` rules), and a diagnostic hook (`gitmsg-diag.c`,
  installed in `git.c`) routes `die`/`error`/`warning` through the catalog too —
  active in every build, not just `git-listen`. The `gitmsg` inspector (and
  `git-workflow.sh messages`) reports the resolved catalog, rules, and config
  path; `gitmessages.example` is a ready-to-copy starting point.

## Native push policy

Push-size enforcement is implemented in the native Git source rather than as a
Bash wrapper.

`git push` now enters a guarded front-end in `git/push-budget.h` for builtin
push callers. The policy uses the actual Git ref/object graph to determine the
candidate local tips selected by the push refspec, obtains the remote's
advertised tips, and asks Git's own `rev-list` object traversal to calculate the
disk usage of objects reachable from the candidate tips but not already
reachable from the remote tips.

The default maximum is a compiled **200 MiB (209,715,200 bytes)** per push
effort:

```text
GIT_PUSH_MAX_BYTES = 200 MiB
```

When the calculated object budget exceeds that ceiling, the push is rejected
before the normal transport push is entered. No shell wrapper or
environment-variable override is required. Dry runs perform the same analysis so
a user can see whether the corresponding real push would fit the policy.

The calculation deliberately operates on commits, refs, and Git objects rather
than summing working-tree files. Multiple selected refs are considered together
and shared reachable objects are naturally deduplicated by Git's revision/object
traversal. Remote object reachability is used as the exclusion set, so objects
already represented by advertised remote refs do not count toward the new-object
budget.

The measured value is an on-disk object-size estimate, not a prediction of the
final network packet size. Git transport compression can change the wire size;
the 200 MiB policy therefore acts as a conservative pre-transfer object budget.

The underlying upstream `transport_push()` implementation remains responsible
for normal ref matching, hooks, status reporting, transport selection,
negotiation, and actual transfer. The native policy is a front-end safety
decision before that operation.

### Resumable push on slow or lossy connections

The push front-end is resume-aware. `builtin/push.c` routes its transport call
through `push-budget.h`'s `transport_push_resume()`, which drives the checkpoint
model in `resume-budget.h`. Interruption is treated as a normal condition: the
still-unacknowledged remainder is retried rather than restarted, progress is
measured only by the remote's acknowledged tips, and the effort halts instead of
looping once retries are exhausted. The 200 MiB object guard re-runs before
every attempt, so resume never weakens the ceiling. The retry ceiling is
configurable through `push.resumeAttempts` (default 5; `0` means unlimited by
policy). See `RESUME.md` for the full model.

### `premount push`

`git-workflow.sh premount [repo] push [remote] [branch] [--sha512] [--attempts N]`
produces a deterministic premount document, computes a SHA-256 (or, with
`--sha512`, SHA-512) reference over the document body only — so the same
candidate set always yields the same reference — and then transfers the ordered
candidate set as an ordered 200 MiB add/commit sequence. The commits form a
chain that the resume policy pushes with partial-commit/resume support, and no
file is split across the 200 MiB boundary. The reference proves byte-level
integrity only (see `SHA256.md`). Full model in `PREMOUNT.md`.

### `temperature`

`git-workflow.sh temperature [repo] [--recandle] [--min-idle DAYS]` is a
read-only advisory scan that finds projects (subtrees) that were started and may
since have been left alone, and reports which could be improved or "recandled".
Each project gets a thermal band by idle age (hot/warm/cool/cold) and three
learner strips: **quality & intention**, **relative importance**, and **total
achievable value** (`(100 - quality) * importance / 100`). A cold/cool project
that is important and improvable is flagged as a recandle candidate. Scores are
advisory and use observable project signals only. Full model in
`TEMPERATURE.md`.

## Running on Standard Git and the Edition Git

The Ubuntu Determinant operations are designed to run identically under stock
upstream Git and under the Edition (vendored) Git; only the packaging differs.

- **Standard Git** (recent upstream; tested with 2.50.x) dispatches an unknown
  `git <name>` to a `git-<name>` executable on `PATH`. `git-temperature` is such
  a shim: put it on `PATH` and `git temperature` works with stock Git. It
  locates the sibling `git-workflow.sh` and forwards to it.
- **Edition Git** registers operations in `git/command-list.txt` and compiles
  their native policy modules through the build in `build/`.

## Building the native policy

`build/Makefile` compiles the C and C++ policy companions together, checks the
header/source set, and archives them:

```sh
make -C build          # compile + archive libgit-native-policy.a
make -C build check    # verify the source/header set is complete
make -C build clean
```

The policy headers are both C and C++ clean: under C they use Git's compat
layer, and under C++ they pull the fixed-width integer/size/string types
directly (git-compat-util.h is not C++-clean standalone), so the C++ companions
build as ordinary translation units. Command registration and any deeper builtin
integration remain separate, deliberate steps.

## Building the full Edition git binary

The whole vendored `git` binary (with the native operations wired in) builds
from `git/` via Git's own Makefile. The known-good flag set and the sandbox
prerequisites are captured so the build does not have to be rediscovered:

```sh
tools/git/build-edition-git.sh            # provision shims, build, verify
tools/git/build-edition-git.sh -j 8       # choose parallelism
tools/git/build-edition-git.sh --clean    # force a full rebuild first
tools/git/build-edition-git.sh --verify-only   # just re-run the checks
```

The wrapper:

1. provisions tiny `cmp`/`diff` shims when GNU diffutils is absent (Git's build
   scripts call `cmp`, and the restricted/CI sandbox may lack it with no network
   to install it);
2. applies the captured flag set from `build/git-full.mk`
   (`NO_EXPAT`, `NO_LIBPCRE`, `NO_GETTEXT`, `NO_TCLTK`, `NO_PYTHON` — all stock
   upstream Git knobs; the missing libs are optional and only affect
   `git-http-push`, `grep -P`, i18n, the Tcl/Tk GUI, and `git-p4`);
3. builds in stages (`libgit.a`, then the `git` binary) so progress is visible
   and incremental relinks stay fast;
4. verifies the binary runs and that the native `temperature` command is
   registered.

To drive Git's Makefile directly with the same flags:

```sh
make -f tools/git/build/git-full.mk git     # build with the captured flags
make -f tools/git/build/git-full.mk clean
make -f tools/git/build/git-full.mk print-flags
```

Note that `resume-budget.o` is part of `LIB_OBJS` in the git Makefile so the
resume lifecycle symbols link into the binary, and `temperature` is registered
in `command-list.txt` with a synopsis from `Documentation/git-temperature.adoc`.

## Repository policy

These tools are infrastructure, not application source. They should remain
independent of GNOME, MATE, Ubuntu White Edition, and individual upstream
projects so they can be reused by the ISO build system.

Source acquisition, verification, and push are separate stages: acquisition
obtains and records a source snapshot; verification independently checks that
snapshot; native push policy evaluates the object graph before transport and
prevents an oversized push effort from being attempted.

## Metadoc index

| Metadoc | Operation / topic |
|---|---|
| `ADD_BLOCKS.md` | 100 MiB add block planning |
| `COMMIT_PARTS.md` | 50 MiB commit-part method |
| `RESUME.md` | resumable / partial push checkpoint model |
| `PREMOUNT.md` | premount inventory + `premount push` |
| `TEMPERATURE.md` | temperature advisory scan |
| `AUTOCHECK.md` | pull-then-prepare preflight |
| `RESTAGE.md` | two-level restage / index history |
| `REBASE_METADOC.md` / `REBASE_MERGE_METADOC.md` | rebase / merge metadata |
| `MORAL.md` | symbolic blessing operation |
| `ALTER_COMMENT.md` | integer-addressed commit-message alteration |
| `PROPATH.md` | path-scoped lazily-expanding partial clone + idle eviction |
| `MESSAGES.md` | centralized stdout/stderr message + concern catalog and its config |
| `FOUNDING.md`, `EXPLANATIONS.md` | project rationale and detailed notes |
| `GIT_OPERATIONS.logic` | the line-oriented operation contract |
