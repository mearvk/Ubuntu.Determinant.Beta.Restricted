<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# limit

`limit` is a read-only local binary inventory utility for Linux, Windows, and macOS.

It scans the selected directory (non-recursively by default), recognizes common ELF, PE, and Mach-O binaries, and reports metadata fields useful for software provenance review.

## Build with the derived GCC

The repository now contains a GCC 16.2.0 source tree with the MEARVK-META metadata extension. Use `./build.sh` to prefer a locally built derived compiler:

```sh
./build.sh
```

The script searches the repository for a locally built GCC 16.2.0 compiler first. `MEARVK_GCC=/path/to/gcc ./build.sh` can select a specific compiler. If the derived compiler is not built yet, the script falls back to the platform `gcc` and explicitly reports that metadata emission is unavailable.

When supported, the build uses:

```text
-fme-metadata
-fme-metadata-edition=Ubuntu.Determinant.Beta.Restricted
-fme-metadata-version=1.01
-fme-metadata-company=MEARVK
-fme-metadata-source=tools/limit/limit.c
```

The metadata identifies build provenance; it is not a legal ownership, fiduciary, or execution authorization claim.

## Build directly

Linux/macOS:

```sh
cc -std=c11 -Wall -Wextra -O2 -o limit limit.c
```

Windows (MinGW):

```sh
gcc -std=c11 -Wall -Wextra -O2 -o limit.exe limit.c
```

## Use

```sh
./limit .
./limit /usr/local/bin
./limit --version
```

## Metadata contract

The implementation reports:

- binary name
- path
- size
- creation date when a portable API supplies it; otherwise explicitly `unavailable`
- modification time where available
- edition
- version
- company
- fiduciary
- metadata source

Values that are not present remain `unavailable`. The program never invents them.

`registry` is treated as platform metadata rather than one universal database. Future platform adapters may read Windows version resources/registry values, macOS bundle metadata, and Linux desktop/package metadata. The scanner remains read-only.

## Engineering records

- `limit.hss` — semantic specification
- `limit.hsss` — source/structure record
- `1-2-3-4.mmd` — ordered engineering/provenance record

## Versioning

`limit` starts at **1.00**. Minor implementation changes increment the minor component (`1.00` -> `1.01`); major architectural changes increment the major component.
