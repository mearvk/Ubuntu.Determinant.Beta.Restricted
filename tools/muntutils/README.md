<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# muntutils

`muntutils` is a native tool for the MEARVK Ubuntu.Determinant.Beta.Restricted tool set. It is written in both C and C++: a C11 core measures the filesystem, and a C++17 engine analyzes and trims source code. It has two capabilities exposed as subcommands.

1. `trim` studies a source tree, decides which functions are confidently unused under conservative reachability rules, and writes a slimmed copy of the code with those functions removed.
2. `report` measures the raw source, an optional slimmed source, and the compiled artifacts (.so, .dll, .a, .o, and executables), then writes a report that both an ordinary adult reader and a programmer can use.

Program based on Science at NCSU - Max Rupplin - MEARVK LLC 2026.

## Build with the derived GCC

The repository contains a GCC 16.2.0 source tree with the MEARVK-META metadata extension. Use `./build.sh` to prefer a locally built derived compiler:

```sh
./build.sh
```

The script searches the repository for a locally built GCC 16.2.0 C compiler first and, separately, for a locally built C++ compiler. `MEARVK_GCC=/path/to/gcc ./build.sh` selects a specific C compiler and `MEARVK_GXX=/path/to/g++ ./build.sh` selects a specific C++ compiler. If a derived compiler is not built yet, the script falls back to the platform `gcc` and `g++` and explicitly reports that metadata emission is unavailable.

When supported, the build uses per translation unit:

```text
-fme-metadata
-fme-metadata-edition=Ubuntu.Determinant.Beta.Restricted
-fme-metadata-version=1.00
-fme-metadata-company=MEARVK
-fme-metadata-source=tools/muntutils/<file>
```

The C translation unit is compiled with the C compiler, the C++ translation units with the C++ compiler, and the objects are linked together with the C++ compiler into `./muntutils`.

The metadata identifies build provenance; it is not a legal ownership, fiduciary, or execution authorization claim.

## Build directly

Linux/macOS:

```sh
cc  -std=c11   -Wall -Wextra -O2 -c muntutils_fs.c    -o muntutils_fs.o
c++ -std=c++17 -Wall -Wextra -O2 -c muntutils_trim.cpp -o muntutils_trim.o
c++ -std=c++17 -Wall -Wextra -O2 -c main.cpp          -o main.o
c++ -std=c++17 -Wall -Wextra -O2 -o muntutils muntutils_fs.o muntutils_trim.o main.o
```

You can also run `make` in this directory, which the repository `tools/Makefile` chains as the `muntutils` target.

### Intended build entry point

`make -C tools muntutils` is the intended way to build this tool from the repository root. That target is deliberately dependency-free: it builds `muntutils` on its own without touching the other tools.

The aggregate `make -C tools all` and `make -C tools install` targets are currently expected to fail. They route through the sibling `pcopy`/`pmove` tools, and `pcopy` has a pre-existing, unrelated build error (an invalid `PCOPY_IOCTL_MAGIC 0xPC` token in `pcopy.c`) that stops the aggregate build before it reaches anything else. That defect is outside the scope of this tool and is not fixed here. Until the unrelated `pcopy` fix lands, build and install `muntutils` through its own target (`make -C tools muntutils`, or `./build.sh` / `./install.sh` in this directory), not through the aggregate `all`/`install` targets.

## Behavior

`muntutils trim <src-tree> --out <dir>` walks the source tree, records the functions it can see, works out which ones are reachable, and writes a slimmed copy of the tree into the output directory with the confidently unused functions removed. `--strict` also treats non-static functions as trimmable when nothing references them.

`muntutils report <src-tree> [--slim <dir>] [--out <report>]` measures the raw source tree, an optional slimmed tree, and the compiled artifacts found under the source tree, then writes a report. Sizes are logical file sizes, not allocated filesystem blocks, and symbolic links are not followed.

## Safe by default

The trimmer never overwrites or deletes your originals on a default run. By default it writes the slimmed tree into a separate output directory and leaves every original file byte for byte unchanged. Rewriting originals in place happens only when you explicitly pass `--in-place` (alias `--apply`), and even then the tool first copies each file it is about to rewrite to `<file>.bak` so a backup always exists before any change.

## Reachability limits

Deciding what code is actually used is undecidable in general, and static analysis cannot fully resolve every path a program can take. `muntutils` cannot always see through dynamic dispatch, function pointers, `dlopen`/`dlsym` loading, reflection, or preprocessor conditionals. Because of this the tool is deliberately conservative: when it cannot prove a function is unused, it keeps that function, and it reports the constructs it could not fully resolve so a reviewer can check them by hand. Treat the removed-function list as a proposal to review, not as a guarantee.

Name-based dispatch is handled specifically. Many programs select a function by its textual name at runtime through a custom loader, a plugin or command registry, or config-driven dispatch, not only through `dlsym`/`GetProcAddress`. To stay conservative in that case, `muntutils` collects every identifier-shaped token that appears inside a string literal anywhere in the tree and, if such a token matches a defined function name, keeps that function alive as a root even when nothing else references it by symbol. In addition, whenever the tool proposes a removal while any string literal in the tree matches a defined function name, it records that as an unresolved construct and lowers the reported confidence below full certainty, because that is exactly the situation where a name could be assembled or referenced textually in a way the scanner cannot follow.

## Report audience

The `report` output is written for two readers at once. It opens with a plain-language summary that an ordinary adult user can read without a programming background, explaining in words how much source code there is, how much a slimmed copy removed, and how large the compiled program files are. It then gives a detailed section with exact file counts and byte totals for a programmer. The summary is careful to keep two different measurements apart: how much source text was removed is an estimate about the code as written, and it is not the same thing as the size of the compiled program, which is measured separately.

## Platform behavior

The implementation targets Linux, Windows, and macOS.

- Linux/macOS use POSIX filesystem enumeration.
- POSIX symbolic links are not followed.
- Windows uses native Unicode file enumeration APIs.
- Reported sizes are logical file sizes, not allocated filesystem blocks.

## Engineering records

- `muntutils.hss` — semantic specification
- `muntutils.hsss` — source/structure record
- `1-2-3-4.mmd` — ordered engineering/provenance record

## Scope

`muntutils` complements the read-only `size` and `limit` utilities. `size` measures byte extent and `limit` describes executable identity; `muntutils` adds reachability-based source trimming and a combined raw-vs-slim-vs-compiled report. The trimming capability is the only one that can write files, and it stays safe by default as described above. The NCSU and MEARVK attributions record the origin and provenance of the work; they are not a claim of legal ownership, fiduciary duty, or execution authorization over the analyzed code.

## Versioning

The initial utility version is **1.00**. Minor implementation changes increment the minor component; major architectural changes increment the major component.
