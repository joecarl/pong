#!/bin/bash

#global-deps
pacman -S --noconfirm mingw64/mingw-w64-x86_64-gcc
pacman -S --noconfirm mingw64/mingw-w64-x86_64-boost

#client-only-deps
pacman -S --noconfirm mingw64/mingw-w64-x86_64-freetype
pacman -S --noconfirm mingw64/mingw-w64-x86_64-allegro 

#allegro-deps

pacman -S --noconfirm mingw64/mingw-w64-x86_64-freeimage

pacman -S --noconfirm mingw64/mingw-w64-x86_64-cmake
pacman -S --noconfirm mingw64/mingw-w64-x86_64-dumb
pacman -S --noconfirm mingw64/mingw-w64-x86_64-flac
pacman -S --noconfirm mingw64/mingw-w64-x86_64-freetype
pacman -S --noconfirm mingw64/mingw-w64-x86_64-gcc
pacman -S --noconfirm mingw64/mingw-w64-x86_64-libjpeg-turbo
pacman -S --noconfirm mingw64/mingw-w64-x86_64-libpng
pacman -S --noconfirm mingw64/mingw-w64-x86_64-libvorbis
pacman -S --noconfirm mingw64/mingw-w64-x86_64-libwebp
pacman -S --noconfirm mingw64/mingw-w64-x86_64-openal
pacman -S --noconfirm mingw64/mingw-w64-x86_64-opusfile
pacman -S --noconfirm mingw64/mingw-w64-x86_64-physfs
pacman -S --noconfirm mingw64/mingw-w64-x86_64-pkg-config
