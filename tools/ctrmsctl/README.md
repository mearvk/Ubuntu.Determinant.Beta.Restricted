<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# ctrmsctl 1.00

`ctrmsctl` is a cautious Linux systemd filesystem-observation service. It maintains a light metadata index over a configured filesystem surface and exposes simple `find`, `search`, `locate`, and `status` commands.

The design principle is **procedural care, not an IQ score**. The service does not attempt to rank people, processes, files, or machines by intelligence. It records observable filesystem facts and keeps inference separate from observation.

## Observed surface

The initial implementation records:

- regular-file count
- directory count
- logical byte total
- path/name
- file mode
- modification time
- create/move/delete/close-write event names while monitoring

It does **not** read file contents, execute discovered files, execute commands found in metadata, follow symbolic links, or infer that a filename is safe or authoritative.

## Commands

```sh
ctrmsctl status
ctrmsctl find XmcDesktopProbe
ctrmsctl search testing-inputs
ctrmsctl locate /usr/local/bin/xmc
```

Build a fresh snapshot manually:

```sh
ctrmsctl --root / --index /var/lib/ctrmsctl/index.tsv
```

Run the monitor directly:

```sh
ctrmsctl --root / monitor
```

## systemd installation

On Ubuntu:

```sh
sudo ./install.sh
systemctl status ctrmsctl
```

The service is enabled for the default Ubuntu White Edition installation profile when that profile's provisioning system includes `tools/ctrmsctl/install.sh`.

The service runs as root because a complete filesystem observation requires permission to inspect many directories, but it uses `NoNewPrivileges`, `PrivateTmp`, `ProtectSystem=strict`, and a narrow writable state path. The index is metadata-only.

## Light and medium index

The initial `index.tsv` is the **light index**. It stores path and basic stat metadata and is intended for fast name/path queries.

A future medium index can add normalized extension/type, executable-format classification, package provenance, owner/group, stable file identity, content digest, and bounded event history. Those additions should remain opt-in because they increase I/O, storage, or privacy implications.

"New commands" and "idioms" are therefore treated as **observed metadata or declared relationships**, not as commands for the service to execute. `ctrmsctl` never turns a discovered filename into an instruction.

## Scope and safety

The filesystem root is configurable. Deployments that do not need a whole-disk view should use a narrower root such as `/usr/local`, `/opt`, or an application data directory. The service should not be used as an excuse to collect file contents or personal data.

The observation chain is:

```text
filesystem event
  → normalized metadata
  → indexed observation
  → query
  → human/system policy
```

An observation is evidence about what was observed at a time; it is not proof of ownership, authorization, authorship, intent, or trust.

## Versioning

`1.00` is the initial release. Minor implementation revisions increment the minor component; major architectural changes increment the major component.
