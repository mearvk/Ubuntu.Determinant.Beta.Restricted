# Total — Three-Tier Native Moderator

**Total** is the project's native C/C++ moderation layer (the **Middle** tier).
It sits between Linux kernel state and ordinary userland policy, cooperating in
a controlled way with SecureJDK 28 and Graal. Source lives under
[`total/`](https://github.com/mearvk/Ubuntu.Determinant.Beta.Restricted/tree/main/total)
(`include/`, `src/`, `tests/`, `Makefile`, `total.conf.example`).

## What it is — and is not

Total is deliberately conservative. It:

- observes Linux memory state,
- maintains controlled admission accounting,
- provides a foundation for future systemd, cgroup, PSI, eBPF, SecureJDK/Graal
  IPC, provenance, and policy modules.

It **does not** replace Linux virtual memory, `malloc`/`free`, or JVM garbage
collection. It moderates and records; it does not take over the kernel or the
managed runtime.

## Evidence surface

A deployment may expose **3 through 1000 input channels at startup**, according
to configuration, hardware, policy, and service profile. Potential evidence
includes:

- process / thread state, memory pressure, allocation observations
- executable / library descriptors, package metadata
- JVM / Graal runtime events, trusted software descriptors, signed configuration
- filesystem provenance, service lifecycle events, application self-description
- cgroup / PSI observations, integrity measurements, resource requests
- diagnostic / test evidence

Every channel is subject to the [evidence pipeline](Architecture#evidence-pipeline):
an input's mere existence proves nothing until provenance, validation,
authorization, and policy admit it.

## Position in the stack

```text
SecureJDK 28 / Graal   (Top — managed semantics)
        │  authenticated evidence
      Total            (Middle — native moderation)  ← you are here
        │  kernel / OS evidence
Linux kernel / hardware (Ground — OS facts)
```

## Related pages

- **[Architecture](Architecture)** — the full three-tier model and pipeline.
- **[SecureJDK 28 & Graal](SecureJDK-and-Graal)** — the managed runtime Total cooperates with.

*Authoritative source: `README.md` (§ Total, § Evidence surface, § Root service
function and manager) and the `total/` source tree.*
