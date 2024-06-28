NOTA: La información mostrada a continuación está obsoleta y sólo debe usarse a modo de referencia.

## Compilar en Windows

1. Instalar GIT for windows (solamente si se pretende participar en el desarrollo).
1. Instalar MSYS2 de 64 bits descargado de <https://www.msys2.org/>.
1. Desde MSYS2-MSYS actualizar los paquetes con `pacman -Syuu` hasta que no se pueda más (mirar el tutorial de instalación por si acaso).
1. Desde MSYS2-MSYS navegar al directorio del proyecto y ejecutar `bash scripts/installdeps.sh`.
1. Ejecutar `bash scripts/win-build.sh` (esto se puede hacer desde Git Bash).
1. Para lanzar la aplicación utilizar `bash scripts/win-run.sh` (también se puede hacer desde Git Bash).

Si tratamos de ejecutar la aplicación compilada haciendo doble click en ella nos dará error por falta de DLLs. Este último comando incluye en el PATH la carpeta `bin` de mingw64, lo que soluciona este problema. Por ahora, para distribuir la aplicación sería necesario copiar una gran cantidad de DLLs de dicha carpeta.

## Compilar el servidor en Linux

### Prerequisitos

1. Instalar el compilador de C++17 o superior.
1. Instalar las libs de Boost v1.75 o superior.

### Compilación

1. Ir a la carpeta del repositorio.

    Si aun no tenemos el repositorio en nuestro sistema:

    ```sh
    git clone https://github.com/joecarl/pong.git
    cd pong
    ```

    Si ya tenemos el repositorio en nuestro sistema simplemente, navegamos a él y lo actualizamos con:

    ```sh
    git pull
    ```

1. Finalmente ejecutar:

    ```sh
    bash scripts/build-server.sh
    ```

## Ejecutar el servidor en Linux

1. El servidor se compila en la carpeta `./build/server`, podemos navegar a esa carpeta y ejecutar:

    ```sh
    ./pongserver -p <DESIRED_PORT>
    ```

---

## Indicaciones para Ubuntu

- Para instalar el compilador de C++ se puede hacer con:
	```
	apt install build-essential
	```
	
- En caso de que las dependencias NO se hayan instalado en el directorio por defecto será necesario especificar dónde se han instalado antes de compilar:
	```
	export LIBRARY_PATH=/path/to/dep1/lib:/path/to/dep2/lib
	export CPATH=/path/to/dep1/include:/path/to/dep2/include
	```
	
## Indicaciones para CentOS 7

- Para instalar el compilador de C++ se puede hacer con:

    ```sh
    yum install devtoolset-7-gcc-c++ --enablerepo='centos-sclo-rh'
    ```

- Siempre que abramos una nueva sesión de consola será necesario ejecutar:

    Para habilitar g++:

    ```sh
    scl enable devtoolset-7 'bash'
    ```

    Para poder ejecutar la aplicación:

    ```sh
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
    ```

    Cambiando `/usr/local/lib` por el directorio donde se haya instalado boost si es necesario.
