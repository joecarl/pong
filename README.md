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


## Compilar el servidor en Linux

### Prerequisitos
1. Instalar el compilador de C++14 o superior.
1. Instalar las libs de Boost v1.75 o superior.

### Compilación
1. Ir a la carpeta del repositorio.

    Si aun no tenemos el repositorio en nuestro sistema:

    ```
    git clone https://github.com/joecarl/pong.git
    cd pong
    ```

    Si ya tenemos el repositorio en nuestro sistema simplemente, navegamos a él y lo actualizamos con: 

    ```
    git pull
    ```

1. Finalmente  ejecutar :

    ```
    bash ./build-server.sh
    ```

## Ejecutar el servidor en Linux

1. El servidor se compila en la carpeta `./build/server`, por tanto:

    ```
    cd build/server
    ./PONGSERVER -p DESIRED_PORT
    ```

---

## Indicaciones para CentOS 7

- Para instalar el compilador de C++ se puede hacer con:
    ```
    yum install devtoolset-7-gcc-c++ --enablerepo='centos-sclo-rh'
    ```

- Siempre que abramos una nueva sesión de consola será necesario ejecutar:

    Para habilitar g++:

    ```
    scl enable devtoolset-7 'bash'
    ```

    Para poder ejecutar la aplicación:

    ```
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
    ```

    Cambiando `/usr/local/lib` por el directorio donde se haya instalado boost si es necesario.