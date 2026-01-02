# tcprecv - TCP Binary Data Receiver

A simple TCP client that receives binary data and outputs it to stdout.

**Author:** Edson Pereira, PY2SDR  
**Version:** 1.00

## Description

Connects to a TCP server, receives binary data, and writes it to stdout. Automatically reconnects if the connection drops.

## Build

```bash
git clone https://github.com/py2sdr/tcprecv
cd tcprecv
gcc -Wall -O2 -o tcprecv tcprecv.c
```

## Install

```bash
sudo cp tcprecv /usr/local/bin
```

## Usage

```bash
tcprecv ip port
```

## Examples

### Save to file
```bash
tcprecv 192.168.1.100 5000 > output.bin
```

### Pipe to another program
```bash
tcprecv 192.168.1.100 5000 | your_program
```

### Receive from remote nmux server
```bash
# Connect to nmux on remote SDR server
tcprecv 192.168.1.50 4950 | baudline -stdin -format s16 -channels 2 -samplerate 48000 -run
```

## Requirements

- Linux
- GCC compiler
- Port must be between 1024-65535

## Notes

- Press Ctrl+C to stop
- Data goes to stdout, status messages go to stderr
- IPv4 only

---
**73 de PY2SDR** 📻
