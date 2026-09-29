<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# size

`size` is a small native C utility for quickly reporting the recursive logical size of a file, directory, or directory tree.

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
-fme-metadata-source=tools/size/size.c
```

The metadata identifies build provenance; it is not a legal ownership, fiduciary, or execution authorization claim.

## Behavior

For a directory, `size` walks all files and nested directories beneath the supplied parent and sums the byte length of regular files. Directory entries themselves are not counted.

It reports both exact bytes and a human-readable binary-unit value. The utility accepts multiple paths and supports `--help` and `--version`.

## Platform behavior

The implementation is intended for Linux, Windows, and macOS.

- Linux/macOS use POSIX `lstat` and directory traversal.
- POSIX symbolic links are not followed.
- Windows uses native Unicode file enumeration APIs.
- Windows reparse points are not recursively followed, preventing directory-junction cycles.
- The reported value is **logical file size**, not allocated filesystem blocks.

## Engineering records

- `size.hss` — semantic specification
- `size.hsss` — source/structure record
- `1-2-3-4.mmd` — ordered engineering/provenance record

## Scope

`size` intentionally remains a read-only measurement utility. It does not modify files, follow arbitrary links, or delete/trim data. It complements `limit`: `limit` describes executable identity and metadata, while `size` measures the logical byte extent of a filesystem tree.

## Versioning

The initial utility version is **1.00**. Minor implementation changes increment the minor component; major architectural changes increment the major component.
