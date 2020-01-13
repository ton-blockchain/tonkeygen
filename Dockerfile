FROM ubuntu:14.04

RUN sudo apt-get update
RUN sudo apt-get install -y software-properties-common
RUN sudo apt-get install -y git libexif-dev liblzma-dev libz-dev libssl-dev libappindicator-dev libicu-dev libdee-dev libdrm-dev dh-autoreconf autoconf automake build-essential libass-dev
RUN sudo apt-get install -y libfreetype6-dev libgpac-dev libsdl1.2-dev libtheora-dev libtool libva-dev libvdpau-dev libvorbis-dev libxcb1-dev libxcb-image0-dev libxcb-shm0-dev libxcb-xfixes0-dev libxcb-keysyms1-dev 
RUN sudo apt-get install -y libxcb-icccm4-dev libxcb-render-util0-dev libxcb-util0-dev libxrender-dev libasound-dev libpulse-dev libxcb-sync0-dev libxcb-randr0-dev libx11-xcb-dev libffi-dev 
RUN sudo apt-get install -y libncurses5-dev pkg-config texi2html zlib1g-dev yasm xutils-dev bison python-xcbgen chrpath
RUN sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
RUN sudo apt-get update && sudo apt-get install gcc-8 g++-8 -y
RUN sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 && sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 60
RUN sudo update-alternatives --config gcc && sudo add-apt-repository --remove ppa:ubuntu-toolchain-r/test -y && MAKE_THREADS_CNT=-j8

RUN git clone --recursive https://github.com/ton-blockchain/tonkeygen.git && mkdir Libraries 
RUN cd Libraries && git clone https://github.com/desktop-app/patches.git
RUN cd Libraries && git clone --branch v3.15.3 https://github.com/Kitware/CMake cmake && cd cmake && ./bootstrap && make $MAKE_THREADS_CNT && sudo make install && cd ..
RUN cd Libraries && git clone --branch 0.9.1 https://github.com/ericniebler/range-v3
RUN cd Libraries && git clone https://github.com/madler/zlib.git && cd zlib && ./configure && make -j8 && sudo make install && cd ..
RUN cd Libraries && git clone https://github.com/openssl/openssl && cd openssl && git checkout OpenSSL_1_0_1-stable && ./config && make $MAKE_THREADS_CNT && sudo make install && cd ..
RUN cd Libraries && git clone https://github.com/xkbcommon/libxkbcommon.git && cd libxkbcommon && git checkout xkbcommon-0.8.4 && ./autogen.sh --disable-x11 && make $MAKE_THREADS_CNT && sudo make install && cd ..
RUN cd Libraries && git clone git://code.qt.io/qt/qt5.git qt5_6_2 && cd qt5_6_2 && perl init-repository --module-subset=qtbase,qtimageformats && git checkout v5.6.2 && cd qtimageformats && git checkout v5.6.2 && cd .. && cd qtbase && git checkout v5.6.2 && cd .. && cd qtbase && git apply ../../patches/qtbase_5_6_2.diff && cd .. && cd qtbase/src/plugins/platforminputcontexts && git clone https://github.com/desktop-app/fcitx.git &&  git clone https://github.com/desktop-app/hime.git && git clone https://github.com/desktop-app/nimf.git && cd ../../../.. && ./configure -prefix "/usr/local/desktop-app/Qt-5.6.2" -release -force-debug-info -opensource -confirm-license -qt-zlib -qt-libpng -qt-libjpeg -qt-freetype -qt-harfbuzz -qt-pcre -qt-xcb -qt-xkbcommon-x11 -no-opengl -no-gtkstyle -static -openssl-linked -nomake examples -nomake tests && make -j8 && sudo make install && cd ..

RUN cd Libraries && git clone https://chromium.googlesource.com/external/gyp && cd gyp && git checkout 9f2a7bb1 && git apply ../patches/gyp.diff && cd ..
RUN cd Libraries && git clone https://github.com/ton-blockchain/ton.git && cd ton && git checkout ecb3e06a06 && git submodule init && git submodule update third-party/crc32c && mkdir build-debug && cd build-debug && cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=/usr/local/include -DOPENSSL_CRYPTO_LIBRARY=/usr/local/ssl/lib/libcrypto.a -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=/usr/local/include -DZLIB_LIBRARIES=/usr/local/lib/libz.a -DTON_ARCH=`uname -m | sed --expression='s/_/-/g'` .. && make -j8 tonlib && cd .. && mkdir build && cd build && cmake -DTON_USE_ROCKSDB=OFF -DTON_USE_ABSEIL=OFF -DTON_ONLY_TONLIB=ON -DOPENSSL_FOUND=1 -DOPENSSL_INCLUDE_DIR=/usr/local/include -DOPENSSL_CRYPTO_LIBRARY=/usr/local/ssl/lib/libcrypto.a -DZLIB_FOUND=1 -DZLIB_INCLUDE_DIR=/usr/local/include -DZLIB_LIBRARIES=/usr/local/lib/libz.a -DTON_ARCH=`uname -m | sed --expression='s/_/-/g'` -DCMAKE_BUILD_TYPE=Release .. && make $MAKE_THREADS_CNT tonlib

#RUN /bin/bash

RUN cd /tonkeygen/Keygen && gyp/refresh.sh

#RUN ls /gyp/
#RUN cd / && ./gyp/refresh.sh

RUN cd tonkeygen/out/Release && make -j8
ENTRYPOINT /bin/bash
