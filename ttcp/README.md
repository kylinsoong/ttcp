# TTCP - Test TCP Connection Performance

A network performance testing tool for measuring TCP and UDP throughput.

## Overview

TTCP (Test TCP) is a classic network benchmarking utility that measures the performance of TCP and UDP connections. It can be used to test network throughput between two systems by transferring fabricated buffers of data.

## Features

- **TCP and UDP Support**: Test both TCP and UDP protocol performance
- **IPv4 and IPv6**: Full support for both IP versions
- **Multicast Support**: UDP multicast testing capabilities
- **Configurable Parameters**:
  - Buffer size
  - Number of buffers
  - Port number
  - Socket buffer sizes
  - Buffer alignment and offset
- **Performance Metrics**:
  - Transfer rate (KB/s, MB/s, Kbit/s, Mbit/s, Gbit/s)
  - Real time and CPU time
  - Number of I/O calls
- **Advanced Options**:
  - TCP_NODELAY socket option
  - Socket keepalive configuration
  - Write interval control
  - Verbose output mode
- **Configuration File**: Support for `/etc/ttcp.conf` configuration

## Building

```bash
make
```

This will produce the `ttcp.out` executable.

### Clean Build

```bash
make clean && make
```

## Usage

### Basic Syntax

```bash
# Transmitter mode
ttcp -t [-options] host

# Receiver mode
ttcp -r [-options] [multicast-group]
```

### Common Options

| Option | Description |
|--------|-------------|
| `-4` | Use IPv4 |
| `-6` | Use IPv6 |
| `-l ##` | Length of buffers read from or written to network (default 8192) |
| `-u` | Use UDP instead of TCP |
| `-p ##` | Port number to send to or listen at (default 5001) |
| `-s` | `-t`: source a pattern to network; `-r`: sink (discard) all data from network |
| `-A ##` | Align the start of buffers to this modulus (default 16384) |
| `-O ##` | Start buffers at this offset from the modulus (default 0) |
| `-v` | Verbose: print more statistics |
| `-d` | Set SO_DEBUG socket option |
| `-b ##` | Set socket buffer size (if supported) |
| `-f X` | Format for rate: k,K = kilo{bit,byte}; m,M = mega; g,G = giga |

### Transmitter-Specific Options

| Option | Description |
|--------|-------------|
| `-n ##` | Number of source buffers written to network (default 2048) |
| `-D` | Don't buffer TCP writes (sets TCP_NODELAY socket option) |
| `-w ##` | Number of microseconds to wait between each write |
| `-P ##` | Port number for client source port |

### Receiver-Specific Options

| Option | Description |
|--------|-------------|
| `-B` | For `-s`, only output full blocks as specified by `-l` (for TAR) |
| `-T` | "Touch": access each byte as it's read |
| `-L` | Enable receive side to sleep randomly (1-3 seconds) to simulate application latency |
| `-I if` | Specify the network interface (e.g. eth0) to use |

## Examples

### Basic TCP Test

**Receiver (Server):**
```bash
./ttcp.out -r -s
```

**Transmitter (Client):**
```bash
./ttcp.out -t -s -n 1000 192.168.1.100
```

### UDP Test

**Receiver:**
```bash
./ttcp.out -r -s -u
```

**Transmitter:**
```bash
./ttcp.out -t -s -u -n 1000 192.168.1.100
```

### Custom Buffer Size and Port

**Receiver:**
```bash
./ttcp.out -r -s -l 16384 -p 6000
```

**Transmitter:**
```bash
./ttcp.out -t -s -l 16384 -p 6000 -n 500 192.168.1.100
```

### IPv4 Only with TCP_NODELAY

**Receiver:**
```bash
./ttcp.out -r -s -4
```

**Transmitter:**
```bash
./ttcp.out -t -s -4 -D -n 2000 192.168.1.100
```

### Verbose Mode with Custom Output Format

**Receiver:**
```bash
./ttcp.out -r -s -v
```

**Transmitter:**
```bash
./ttcp.out -t -s -v -f M -n 1000 192.168.1.100
```

### With Socket Buffer Size

**Receiver:**
```bash
./ttcp.out -r -s -b 131072
```

**Transmitter:**
```bash
./ttcp.out -t -s -b 131072 -n 1000 192.168.1.100
```

## Output Format

The output format (`-f` option) supports the following:

| Format | Description |
|--------|-------------|
| `k` | Kilobits per second |
| `K` | Kilobytes per second (default) |
| `m` | Megabits per second |
| `M` | Megabytes per second |
| `g` | Gigabits per second |
| `G` | Gigabytes per second |

## Sample Output

```
ttcp-t: buflen=8192, nbuf=1000, align=16384/0, port=5001  tcp  -> localhost
ttcp-t: socket
ttcp-t: connect
ttcp-r: 8192000 bytes in 0.02 real seconds = 381898.03 KB/sec +++
ttcp-r: 1000 I/O calls, msec/call = 0.02, calls/sec = 47737.25
ttcp-r: 0.0user 0.0sys 0:00real 50% 0i+0d 712704maxrss 0+1pf 0+134csw
```

## Configuration File

TTCP can read configuration from `/etc/ttcp.conf`. The file uses `key=value` format:

```
# TTCP Configuration File
ip_version=4
buffer_length=8192
protocol=tcp
port=5001
silent=1
verbose=1
nbuf=2048
format=K
```

### Configuration Options

| Key | Description |
|-----|-------------|
| `ip_version` | IP version: 4 or 6 |
| `buffer_length` | Buffer length in bytes |
| `protocol` | Protocol: tcp or udp |
| `port` | Port number (1024-65535) |
| `silent` | Enable sink/source mode: 1 |
| `bufalign` | Buffer alignment |
| `bufoffset` | Buffer offset |
| `verbose` | Verbose output: 1 |
| `ttcp.sock.debug` | Enable SO_DEBUG: 1 |
| `ttcp.sock.sndbuf.size` | Send buffer size |
| `ttcp.sock.rcvbuf.size` | Receive buffer size |
| `ttcp.sock.snd.timeout` | Send timeout in seconds |
| `ttcp.sock.rcv.timeout` | Receive timeout in seconds |
| `format` | Output format: k, K, m, M, g, G |
| `nbuf` | Number of buffers |
| `latency` | Enable latency simulation |
| `ttcp.tcp.nodelay` | Enable TCP_NODELAY: 1 |
| `write_interval` | Microseconds between writes |
| `source_port` | Source port for transmitter |
| `blocks_output` | Full blocks output mode: 1 |
| `touch` | Touch data mode: 1 |
| `device` | Network interface name |
| `ttcp.sock.keepalive` | Enable keepalive: 1 |
| `ttcp.tcp.tcp_keepalive_time` | Keepalive idle time |
| `ttcp.tcp.tcp_keepalive_intvl` | Keepalive interval |
| `ttcp.tcp.tcp_keepalive_probes` | Keepalive probes count |

## Testing

A comprehensive test script is included to verify functionality:

```bash
./test_ttcp.sh
```

The test script includes:
- 18 positive test cases covering all major features
- 4 negative test cases for error handling
- 2 forced failure tests to demonstrate failure output

### Test Categories

1. **Basic Tests**: TCP and UDP transfers
2. **Protocol Tests**: IPv4 only mode
3. **Port Tests**: Custom port numbers
4. **Buffer Tests**: Custom buffer sizes, alignment, offset
5. **Socket Tests**: TCP_NODELAY, socket buffer sizes
6. **Output Tests**: Verbose mode, different output formats
7. **Timing Tests**: Write intervals
8. **Error Handling**: Invalid options, missing arguments

## License

Public Domain. Distribution Unlimited.

## History

- Originally developed for 4.2BSD systems
- Modified for various UNIX systems (BSD43, SYSV)
- Enhanced at Silicon Graphics, Inc. (1989-1991)
- IPv6 support added by Pekka Savola (2001-2002)
- Multicast support added by Stig Venaas (2002-2004)
- Additional features by Kylin Soong (2022-2024)
