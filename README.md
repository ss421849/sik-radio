# Internet Radio Streamer (SIKRadio)

A lightweight, high-performance internet radio system consisting of a **sender (transmitter)** and a **receiver**, implemented in C++ using Linux BSD sockets. The project handles real-time audio streaming over UDP with custom buffering, reordering, and missing packet diagnostics.

This project was developed as a university assignment for the *Computer Networks (SIK)* course.

---

## 🛠️ Features

### Part A: Sender (`sikradio-sender`)
* **Live Streaming:** Reads raw audio bytes from `stdin` and streams them over UDP.
* **Stream Speed Control:** Operates at the speed of the incoming stream (e.g., synchronized via tools like `pv` or audio devices).
* **Graceful Shutdown:** Terminates with exit code `0` upon reaching `EOF`. Incomplete trailing packets (smaller than `PSIZE`) are safely discarded.

### Part B: Receiver (`sikradio-receiver`)
* **Custom Dynamic Buffer:** Implements a rolling buffer of `BSIZE` bytes to handle packet jitter, drops, and reordering.
* **Pre-buffering Control:** To ensure smooth playback, data delivery to `stdout` starts only after filling the buffer to a specific threshold ($\lfloor BSIZE \times 3/4 \rfloor$).
* **Session Management:** Tracks `session_id` (based on Unix epoch). Automatically restarts playback if a newer session is detected and drops stale packets.
* **Diagnostics:** Outputs missing packet alerts (`MISSING: BEFORE n EXPECTED i`) to `stderr`.

---

## 📐 Protocol Specification

Communication is strictly one-way (Sender $\rightarrow$ Receiver) over UDP. All binary data numbers are transmitted in **network byte order (big-endian)** using fixed-width integer types.

### Audio Packet Structure

| Field | Type | Description |
|---|---|---|
| `session_id` | `uint64_t` | Constant for the sender's runtime lifetime (initialized to current epoch seconds). |
| `first_byte_num` | `uint64_t` | 0-indexed position of the first byte of `audio_data` within the global stream. |
| `audio_data` | `byte[]` | Raw audio payload of exactly `PSIZE` bytes. |

---

## ⚙️ Configuration & Parameters

Both programs support flexible command-line configuration:

| Parameter | Used By | Default Value | Description |
|---|---|---|---|
| `-a` | Sender / Receiver | *Required* | **Sender:** Destination IP (`DEST_ADDR`).<br>**Receiver:** Source IP to connect to. |
| `-P` | Both | `20000 + (student_id % 10000)` | UDP data port (`DATA_PORT`). |
| `-p` | Sender | `512` | Audio payload size in bytes (`PSIZE`). |
| `-b` | Receiver | `65536` (64 kB) | Receiver buffer size in bytes (`BSIZE`). |
| `-n` | Sender | `"Nienazwany Nadajnik"` | Name of the transmitter (`NAZWA`). |

---

## 🚀 Building & Running

### Prerequisites
* GCC or Clang compiler with **C++20** support.
* CMake (version 3.20 or higher).

### Compilation
The project uses CMake for a robust cross-platform build system. You can compile both the sender and receiver by running:

```bash
# Create and navigate to the build directory
mkdir build && cd build

# Configure the project
cmake ..

# Build the binaries
cmake --build .

### Example Usage

#### 1. Streaming an MP3 File (CD Quality)

You can use `sox` to convert audio to raw streams and `pv` to throttle the transmission speed to match the required bit rate:

```bash
sox -S "MySong.mp3" -r 44100 -b 16 -e signed-integer -c 2 -t raw - | \
pv -q -L $((44100 * 4)) | \
./sikradio-sender -a 127.0.0.1 -n "My Local Radio"

```

#### 2. Streaming from a Microphone

```bash
arecord -t raw -f cd | ./sikradio-sender -a 127.0.0.1 -n "Live Podcast"

```

#### 3. Receiving and Playing the Stream

Pipe the receiver's output into the `play` utility (part of the `sox` package) to listen to the broadcast:

```bash
./sikradio-receiver -a 127.0.0.1 | play -t raw -c 2 -r 44100 -b 16 -e signed-integer --buffer 32768 -

```

