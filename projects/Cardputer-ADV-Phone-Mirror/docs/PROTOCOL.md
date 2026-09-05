# Cardputer Mirror wire protocol v0

Transport: TCP, default port `9000`.

Each video frame is one packet at the application layer:

| Bytes | Meaning |
|---|---|
| 0..3 | ASCII magic `CMIR` |
| 4..7 | JPEG byte length, unsigned 32-bit big-endian |
| 8.. | Baseline JPEG bytes |

Current receiver limits frames to 120 KiB and expects the sender to crop/resize to 240x135 before encoding.
The protocol deliberately uses TCP so the Cardputer does not need to implement UDP fragmentation/reassembly.

Version 1 can add a negotiated header for orientation, keyboard return traffic, quality, frame timing and LAN discovery.
