# JPIX Pixel Format (`.jpix`)

**JPIX** (JSpec Pixel Format, extension `.jpix`) is the project's deterministic
pixel/image model. It treats an image as a **logical pixel map** with explicit
extent, cohesion, and integrity, distinct from any particular raster or codec
encoding.

## Model summary

| Concept | Summary |
|---|---|
| **Pixel Map** | The canonical representation of an image as mapped pixels. |
| **Mapped vs. transparent** | Pixels are either mapped (present) or transparent (absent); both are explicit. |
| **Boundary & extent** | The map has a defined boundary and extent. |
| **Cohesion & movement** | Pixels have cohesion; movement is defined over the logical map. |
| **Logical vs. physical** | A logical map with an efficient physical representation — the two are separated. |
| **48-bit color baseline** | Color baseline of 48-bit with dimensional storage. |
| **Raster & codec** | Raster and codec representations derive from the logical map. |
| **Deterministic rendering** | Rendering is deterministic. |
| **Deterministic integrity** | Pixel integrity is verifiable and deterministic. |

## Why it matters

JPIX aligns images with the project's provenance/determinism discipline: an
image is a *logical* object with defined extent and integrity, and its physical
encodings are derivations — mirroring how [Total](Total) treats evidence and how
[Proffer](Proffer-and-Vocabulary) treats state.

## Related pages

- **[Proffer & Core Vocabulary](Proffer-and-Vocabulary)** — determinism and provenance discipline.
- **[Repository Map](Repository-Map)** — where image/wallpaper/icon assets live.

*Authoritative source: `README.md` (§ JSpec Pixel Format `.jpix` and its
subsections on canonical representation, extent, cohesion, 48-bit color, raster
/ codec, and deterministic rendering/integrity).*
