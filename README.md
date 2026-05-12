# Guitar-Tuner-Project-C-
## 🛠️ **Dependencies Installation** 
*Before running Guitar Tuner, install these based on your OS:*

### **🐧 LINUX (Ubuntu/Debian)**
```bash
sudo apt update
sudo apt install libsfml-dev libportaudio2 portaudio19-dev libfftw3-dev g++

# Optional: Arial font
sudo apt install fonts-liberation

### **For Mac 🍎**
# 1. Install Homebrew (one-time, if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 2. Install dependencies
brew install sfml portaudio fftw pkg-config

# Optional: Arial-like font
brew install font-liberation

### ** For Windows 🪟**
REM One-time vcpkg setup (if first time)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
