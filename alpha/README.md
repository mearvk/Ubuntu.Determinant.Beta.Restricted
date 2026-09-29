<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# UTF-32 Alpha Package

This directory contains a compact, implementation-oriented UTF-32 package for the repository. It is based on the Unicode Standard's UTF-32 definition and is intentionally written as an original conformance summary rather than a reproduction of the Unicode Standard text.

## Contents

- `UTF-32-SPEC.md` — normative implementation summary: scalar-value domain, 32-bit code units, validation, and encoding/decoding rules.
- `utf32.h` / `utf32.c` — small portable C implementation for scalar-value validation and UTF-32 code-unit conversion.
- `test_utf32.c` — executable tests covering ASCII, BMP, supplementary-plane values, surrogate rejection, and out-of-range rejection.
- `unicode-sources.md` — authoritative Unicode references and maintenance guidance.

## Upstream basis

UTF-32 is a fixed-width Unicode encoding form in which one 32-bit code unit corresponds to one Unicode scalar value. The scalar-value domain excludes surrogate code points and is bounded by U+10FFFF.

The package tracks the Unicode definition rather than treating arbitrary 32-bit integers as characters. Byte order is a property of an encoding scheme/serialization (UTF-32BE or UTF-32LE), not of the in-memory UTF-32 encoding form itself.

## Build

```sh
cc -std=c11 -Wall -Wextra -pedantic utf32.c test_utf32.c -o test_utf32
./test_utf32
```

The implementation has no external dependencies.