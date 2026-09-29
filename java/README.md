<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# Coherent Professional Java Email Server

**Author:** Max Rupplin - MEARVK LLC 2026

A modular Java 21 SMTP mail server foundation. The design follows the classic SMTP command/session/store separation while applying contemporary Java service boundaries, explicit TLS policy, authentication, audit logging, bounded message handling, and filesystem-backed storage.

The project is intentionally assembled as small components (ASYSMA-style assembly): protocol parsing, session state, security policy, persistence, and transport are independently replaceable.

## Scope

- SMTP service with EHLO/HELO, MAIL, RCPT, DATA, RSET, NOOP, QUIT
- STARTTLS negotiation hook and explicit TLS policy
- AUTH PLAIN hook with constant-time credential comparison
- bounded message size and recipient count
- append-only audit logging
- durable filesystem message store
- clean Java 21 module/build structure

This is an initial professional foundation, not a claim of full RFC coverage or production certification. DKIM, SPF, DMARC, queue retry, DNS policy, spam filtering, and advanced MIME processing are deliberately separate future modules.

## Build

```sh
mvn -q test
mvn -q package
```

## Run

```sh
java -jar target/coherent-mail-server-0.1.0.jar
```

Configuration is supplied by system properties or the defaults in `ServerConfig`.

## References

The protocol model is based on the SMTP command/session tradition established by RFC 5321 and its predecessors. Security architecture uses modern Java TLS, bounded input, explicit authentication, and auditable state transitions.
