# TNFS - The Trivial Network Filesystem (Qt6 System Tray Edition)

![TNFS Badge](https://raw.githubusercontent.com/pjones1063/tnfsd/main/res/tnfs_badge.png)

This fork modernizes the classic TNFS daemon by wrapping it in a lightweight, cross-platform **Qt6 System Tray Application**. It makes managing and monitoring your retro-computing file server incredibly simple without needing to run terminal commands.

## New Qt6 GUI Features
- **System Tray Integration:** Run the server quietly in the background on Windows, Linux, or macOS.
- **One-Click Start/Stop:** Easily toggle the server on and off directly from the tray menu.
- **Live Console:** View server logs, mounting events, and errors in real-time via the built-in log window.
- **Settings UI:** Configure your root mount directory and stats output path without editing text files.
- **Safe Port Management:** Built-in network socket checking ensures the app fails gracefully with a UI warning if port 16384 is already in use.

## Usage Statistics
This fork includes a built-in usage logger.
- **Data:** All mounts and file opens are logged to `tnfsd_stats.csv` in the server directory.
- **Format:** `Timestamp, IP_Address, Filename`
- **Dashboard:** A Python script (`gen_stats.py`) is included to generate a visual HTML report of server activity.

**To run the dashboard:**
1. Generate stats: `python3 gen_stats.py`
2. Serve web page: `python3 -m http.server 8080`

## Rationale

Protocols such as NFS (Unix) or SMB (Windows) are overly complex for 8-bit systems. While NFS is well documented, it's a complex RPC based protocol. SMB is much worse. It is also a complex RPC based protocol, but it's also proprietary, poorly documented, and implementations differ so much that to get something that works with a reasonable subset of SMB would add a great deal of unwanted complexity. The Samba project has been going for *years* and they still haven't finished making it bug-for-bug compatible with the various versions of Windows!

At the other end, there's FTP, but FTP is not great for a general file system protocol for 8-bit systems - it requires two TCP sockets for each connection, and some things are awkward in FTP, even if they work.

So instead, TNFS provides a straightforward protocol that can easily be implemented on most 8-bit systems. It's designed to be a bit better than FTP for the purpose of a filesystem, but not as complex as the "big" network filesystem protocols like NFS or SMB. It is also designed to be usable with 'incomplete' TCP/IP stacks (e.g. ones that only support UDP).

## Security

This is not intended to be a proper, secure network file system. If you're storing confidential files on your vintage systems, you're barmy :) Encryption, for example, is not supported. However, servers that may be exposed to the internet should be coded in such a way they won't open up the host system to exploits.

## Supported systems

So far, the following 8-bit systems have a TNFS client available:

* Sinclair ZX Spectrum (with the Spectranet)
* Atari 8-bit (with the FujiNet)
* Coleco Adam (with the FujiNet)
* Tandy (with the FujiNet)
* Apple II (with the FujiNet)
* Commodore 64 (with the FujiNet)

## Supported server systems

The TNFS daemon has been built and tested via CMake and Qt6 for the following systems:

* Linux
* Microsoft Windows
* Mac OS X and successors

## Acknowledgements

The FujiNet team, for many improvements and enhancements to the basic TNFS daemon. See https://fujinet.online
