# waifu2x-qtgui
Qt GUI for waifu2x-converter-cpp

Licensed under GPLv3 or later.

/!\ Currently under development, some features have not been implemented yet.

![screenshot](https://github.com/nastys/waifu2x-qtgui/raw/master/screenshot.png)
(Hatsune Miku character by Piapro, licensed under CC BY-NC)

# Building [waifu2x-converter-cpp](https://github.com/DeadSix27/waifu2x-converter-cpp) (Linux Mint/Ubuntu/Debian)
If you have an NVIDIA GPU, install [CUDA](https://developer.nvidia.com/cuda-downloads?target_os=Linux&target_arch=x86_64&target_distro=Ubuntu).

Open a terminal window (CTRL+ALT+T) and run:
    
    sudo apt install -y git build-essential gcc cmake libopencv-dev beignet-opencl-icd mesa-opencl-icd opencl-headers
    
    git clone "https://github.com/DeadSix27/waifu2x-converter-cpp"
    
    cd waifu2x-converter-cpp
    
    mkdir out && cd out
    
    cmake ..
    
    make -j4
    
    sudo make install
    
    sudo ldconfig

# Building waifu2x-qtgui (Linux Mint/Ubuntu/Debian)
- Download and install the latest version of [Qt](https://www.qt.io/download) \(Qt >=5.13.0-beta1 Desktop gcc 64-bit and Qt Creator >=4.9.0-rc1\);
- Open a new terminal window (CTRL+ALT+T) and run:
    
      sudo apt install -y qt5-image-formats-plugins imagemagick
    
      git clone "https://github.com/nastys/waifu2x-qtgui.git"
    
- Start Qt Creator >=4.9.0-rc1 and open waifu2x-qtgui/waifu2x-qtgui.pro;
- Click on Configure Project;
- Wait a few seconds, then run the project (CTRL+R).
