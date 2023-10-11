#!/bin/bash

#global-deps
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-gcc
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-make
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-boost

#client-only-deps
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-freetype
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-allegro 

#allegro-deps(most of them will already be installed)
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-freeimage
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-cmake
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-dumb
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-flac
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-libjpeg-turbo
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-libpng
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-libvorbis
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-libwebp
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-openal
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-opusfile
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-physfs
pacman -S --noconfirm ucrt64/mingw-w64-ucrt-x86_64-pkgconf