# pong

Pong recreation with online playing option.
Allego5 based.

## Para compilar el proyecto en windows es necesario preparar la siguiente configuración:

1. Instalar GIT for windows (no es totalmente necesario pero recomendable si se va a participar en el desarrollo).
1. Instalar MSYS2 de 64 bits descargado de https://www.msys2.org/ *[1]
1. Desde MSYS2-MSYS actualizar los paquetes con `pacman -Syuu` hasta que no se pueda más (mirar el tutorial de instalación por si acaso).
1. Desde MSYS2-MSYS navegar al directorio del proyecto y ejecutar `bash installdeps.sh`.
1. Ejecutar `bash win-build.sh` (esto se puede hacer desde Git Bash).
1. Para lanzar la aplicación utilizar `bash win-run.sh`. Este comando incluye en el PATH la carpeta `bin` de mingw64. Por ahora para distribuir la aplicación sería necesario copiar una gran cantidad de DLLs de dicha carpeta. 

## Compilar el servidor en CentOS 7

1. Instalar boost.
1. Instalar gcc g++ (en CentOs `yum install devtoolset-7-gcc-c++ --enablerepo='centos-sclo-rh'` y `scl enable devtoolset-7 'bash'`).
1. Clonar este repositorio en la carpeta deseada, y navegar a la carpeta clonada.
1. Ejecutar `bash ./build-server.sh`.

## Ejecutar el servidor en CentOS 7

1. Ejecutar `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib` o el directorio donde se haya instalado boost.
1. Ejecutar `./build/server/PONGSERVER -p DESIRED_PORT`