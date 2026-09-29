<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# ASYSMA Secure Package Foundation

This directory defines the rehearsal implementation of the project-defined `.asysma` secure application/package format and the native execution boundary around it.

## Strategy

`.asysma` is a signed package and execution description, not a replacement for ELF, PE, or Mach-O. Native platform launchers remain platform-native; `.asysma` supplies the common package identity, manifest, policy, hashes, and signature boundary.

For the current rehearsal, the native execution model is **desktop/OS level**, not firmware boot. The operating system and desktop are already running when the project launcher starts.

```text
Desktop / OS already running
        ↓
x86-64 native launcher
(assembled from .asm/.s)
        ↓
Ubuntu White Direct / native OS API
        ↓
ASYSMA verification
        ↓
native executable
        ↓
JDK / Java
        ↓
JavaFX / application
```

### What is common across the operating systems?

There is no single universal `.asm` execution layer shared by modern operating systems. The common foundation is the **CPU instruction-set architecture (ISA)**. Intel/AMD x86-64 machine code is executable by the x86-64 CPU once the OS has mapped it into an executable process.

The layers above the CPU vary by OS:

```text
CPU commonality: x86-64 ISA
          ↓
OS-native ABI / calling convention
          ↓
Executable format + process loader
          ↓
ELF       PE/COFF       Mach-O
Linux     Windows       macOS
```

An `.asm` file is source text, not itself a CPU executable. It is assembled into machine code and then packaged/linked in the native executable format expected by the target OS. The OS loader establishes the process address space and transfers control to the native machine-code entry point.

## Native Direct model

The project will initially target **Intel/AMD x86-64** and maintain a modest assembly footprint sufficient to establish and measure the quality of native startup on the known desktop operating systems:

- Linux;
- Windows;
- macOS.

The assembly layer should remain small and auditable. Its responsibilities are expected to include CPU/ABI assumptions, native entry/exit discipline, OS handoff, initial host measurement, and controlled transition to the next native layer. Application policy and general business logic should remain above this boundary.

Ubuntu White Direct is the proposed project-level **common native contract** above the individual OS ABIs. It should expose a stable capability model while using platform-specific implementations underneath.

```text
                 Ubuntu White Direct
                Common native contract
                         │
             ┌───────────┼───────────┐
             ▼           ▼           ▼
         Linux ABI   Windows ABI   macOS ABI
             │           │           │
            ELF         PE        Mach-O
             └───────────┼───────────┘
                         ▼
                    x86-64 CPU
```

The Direct layer is not a replacement for the OS ABI. It is the project's controlled abstraction over those differences.

## Execution and verification

Execution is deliberately staged:

```text
read → parse → validate → verify signature → platform check → policy check → authorize → load → execute → verify result
```

No privileged payload should execute before integrity and authenticity verification.

## Current rehearsal scope

1. Define a versioned manifest.
2. Define canonicalization rules.
3. Define SHA-384 content digests.
4. Define detached Ed25519 signatures for the rehearsal profile.
5. Define explicit platform/architecture and permission declarations.
6. Provide a Java 21 verifier/inspector.
7. Define the x86-64 native/assembly startup contract.
8. Define Linux, Windows, and macOS native adapter boundaries.
9. Define Ubuntu White Direct as the common native API/contract.
10. Leave final platform executable generation to the appropriate native toolchain, `jpackage`, or Native Image.

This is an engineering foundation and rehearsal, not a claim of production security certification.
