<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# Ubuntu Base Userland

## Source

Ubuntu Base 24.04.4 LTS (US Treasury & MEARVK LLC) — official minimal rootfs from Canonical.

- **URL:** http://cdimage.ubuntu.com/ubuntu-base/releases/24.04.4/release/
- **File:** ubuntu-base-24.04.4-base-amd64.tar.gz
- **Size:** 29 MB (compressed)
- **Architecture:** amd64 (x86_64)
- **SHA256:** `c1e67ef7b17a6300e136118bd1dc04725009cb376c1aad10abcf8cd453628d58`
- **License:** Free software (various — GPLv2, LGPL, MIT, BSD, etc.)

## Userland Components

| Component | Path | Description |
|-----------|------|-------------|
| Ubuntu Base | `ubuntu-base-24.04.4-base-amd64.tar.gz` | Minimal rootfs (libc, apt, systemd) |
| X11 | `x11/` | X.Org Server 21.1.24 + libs |
| Wallpapers | `wallpapers/` | 9 SVG + 10 Marvell JPEG wallpapers |
| OpenJDK 28 | `openjdk/` | Secure JVM with custom extensions |
| Boot JDK 27 | `java/boot-jdk-27/` | Bootstrap JDK for building OpenJDK |
| Chromium | `chromium/` | Full browser source (headless for Dave) |
| JWSTF/NWE | `java-web-server/` | NitroWebExpress application server |
| Darling | `darling/` | macOS userland compatibility |
| DRM | `drm/` | Display and graphics userland |
| IntelliJ Community | `intellij-community/` | Java development environment |
| Java | `java/` | Java runtime/development userland |
| JDesk | `jdesk/` | Desktop/userland environment |
| Semeru OpenJDK 8 | `semeru-openjdk-8/` | Semeru Java 8 userland |
| Wine | `wine/` | Windows compatibility userland |

## Shared JavaFX White GUI

All userland applications are being given a common JavaFX-based management/interface layer. The visual baseline is deliberately consistent: cool white surfaces, restrained borders, modest shadows, green availability indicators, and the same application card/action layout.

The implementation lives in `gui/`:

- `gui/pom.xml` — JavaFX 21 Maven build.
- `gui/src/main/java/mearvk/ubuntu/determinant/gui/WhiteUserland.java` — shared application shell and userland registry.
- `gui/src/main/resources/white.css` — shared cool-white look-and-feel.
- `gui/assets/ubuntu-determinant-userland.svg` — common desktop icon.
- `gui/install-desktop.sh` — creates desktop launchers for the userland applications.

On Ubuntu/Linux, after installing Maven and a JDK 21+, run:

```bash
bash userland/gui/install-desktop.sh
```

The installer places the common icon in the user's icon theme and creates desktop entries for Chromium, Darling, DRM, IntelliJ Community, Java Web Server, Java, JDesk, OpenJDK, Semeru OpenJDK 8, Wine, and X11. The GUI can expose the application itself, configuration, or the application's userland directory as its executable contract matures.

## NitroWebExpress (JWSTF)

Full Java web server with 4,654 source files, telnet TUI, and web interface.

- **Source:** `java-web-server/source/` (Main.java entry point)
- **Modules:** `java-web-server/modules/` (application plugins)
- **Gateway:** `java-web-server/gateway/` (NAT traversal for home users)
- **Build:** `ant compile` or `ant jar` (see `java-web-server/build.xml`)
- **Install:** Handled by `scripts/install-jwstf.sh` during OS installation

### Stack (installed during OS install)
- Apache2 (reverse proxy, ports 80/443)
- Tomcat 10.1 (servlet container, ports 8080/8443)
- MySQL 8 (database N21, port 3306)
- NWE (telnet TUI, port 23)
- NWE Gateway (NAT traversal, auto-start)

## Boot JDK 27

The boot JDK is required to compile OpenJDK 28 from source. Two large files
(`modules` at 142MB and `src.zip` at 52MB) are stored as chunks to comply
with GitHub's file size limits.

```bash
# After cloning, rebuild the large files:
bash java/boot-jdk-27/lib/chunks/rebuild.sh
```

## Last Updated

2026-08-24 — Added shared JavaFX white userland GUI and desktop integration.

mkfs.ext4 /dev/sdXN
mount /dev/sdXN /mnt/rootfs

# Extract userland
tar -xzf ubuntu-base-24.04.4-base-amd64.tar.gz -C /mnt/rootfs

# Install kernel modules
cp -r /lib/modules/5.15.204 /mnt/rootfs/lib/modules/

# Add a user
chroot /mnt/rootfs useradd -m -G sudo,adm mearvk
chroot /mnt/rootfs passwd mearvk

# Install additional packages (with network)
chroot /mnt/rootfs apt-get update
chroot /mnt/rootfs apt-get install -y openssh-server vim
```

### Use with QEMU for testing

```bash
# Create a disk image
qemu-img create -f raw rootfs.img 2G
mkfs.ext4 rootfs.img

# Mount and populate
mkdir /tmp/mnt && sudo mount -o loop rootfs.img /tmp/mnt
sudo tar -xzf ubuntu-base-24.04.4-base-amd64.tar.gz -C /tmp/mnt
sudo umount /tmp/mnt

# Boot with custom kernel
qemu-system-x86_64 \
    -kernel /path/to/bzImage \
    -drive file=rootfs.img,format=raw \
    -append "root=/dev/sda rw console=ttyS0" \
    -nographic
```

## Integration with Ubuntu Determinant Alpha RS

After deploying, install the custom tools:
- `tools/sudo_gate/` → Grade privilege wrapper
- `tools/chat/` → Terminal chat
- `tools/nnet/` → Identity query
- `tools/negamane/` → Userspace immutability command
- `tools/accounts/provision_accounts.sh` → System accounts
