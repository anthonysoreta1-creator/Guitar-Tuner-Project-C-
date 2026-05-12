# Guitar-Tuner-Project-C++

## 🛠️ Dependencies Installation
*Before running Guitar Tuner, install these based on your OS:*Requirements

# Requirements

Before running the project, install the required dependencies for your operating system.

This project uses:

* [PortAudio](https://www.portaudio.com?utm_source=chatgpt.com) for real-time audio input
* [FFTW](https://www.fftw.org?utm_source=chatgpt.com) for frequency analysis

---

# Installation Guide

## 🍎 macOS

Install dependencies using [Homebrew](https://brew.sh?utm_source=chatgpt.com):

```bash id="mac1"
brew install portaudio
brew install fftw
```

Compile the program:

```bash id="mac2"
g++ -std=c++17 program.cpp -o program \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-lportaudio \
-lfftw3
```

Run:

```bash id="mac3"
./program
```

---

## 🪟 Windows

### Option 1 — Using MSYS2 (Recommended)

Install [MSYS2](https://www.msys2.org?utm_source=chatgpt.com).

Open the **MSYS2 UCRT64 Terminal** and install dependencies:

```bash id="win1"
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-portaudio
pacman -S mingw-w64-ucrt-x86_64-fftw
```

Compile:

```bash id="win2"
g++ -std=c++17 program.cpp -o program \
-lportaudio \
-lfftw3
```

Run:

```bash id="win3"
./program
```

---

### Option 2 — Visual Studio

Install:

* [Visual Studio Community](https://visualstudio.microsoft.com?utm_source=chatgpt.com)
* PortAudio
* FFTW

Then:

1. Create a new C++ Console Project
2. Add `program.cpp`
3. Configure:

   * Include directories
   * Library directories
   * Linker dependencies (`portaudio.lib`, `fftw3.lib`)
4. Build and run the project

---

## 🐧 Linux (Ubuntu/Debian)

Install dependencies:

```bash id="lin1"
sudo apt update
sudo apt install portaudio19-dev
sudo apt install libfftw3-dev
```

Compile:

```bash id="lin2"
g++ -std=c++17 program.cpp -o program \
-lportaudio \
-lfftw3 \
-pthread
```

Run:

```bash id="lin3"
./program
```

---

# Verify Installation

If installation succeeds, running the program should display:

```text id="verify1"
=== GUITAR TUNER ===
Showing example readings:
```

If microphone permissions are blocked, allow terminal or IDE microphone access in your operating system settings.
