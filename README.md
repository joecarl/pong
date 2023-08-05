# PONG

Recreación del clásico juego PONG desarrollado en C++, además incluye opción de juego online. El objetivo de este proyecto es aprender los pricipios básicos de la programación de videojuegos, así como del desarrollo de aplicaciones online cliente/servidor.

El proyecto intenta mantener la mayor simpleza posible pero con una infraestructura principal que pueda servir como base para proyectos más grandes.

Este proyecto tiene dos dependencias principales:

- Allego v5.2.6+ (sólo para el cliente)
- Boost v1.75+ (utilizado principalmente para la comunicación entre sockets TCP/IP)

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

1. Instalar el compilador de C++14 o superior.
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

---

## Indicaciones para portar el cliente a Android

1. Compilar Allegro (seguir las indicaciones de este repositorio: <https://github.com/liballeg/android>)

1. Compilar Boost (seguir las indicaciones del apartado `Compiling` de este repositorio: <https://github.com/moritz-wundke/Boost-for-Android>). Es posible que haya que ejecutar el script de compilación en Linux.

1. Añadir dos lineas de configuración al `AndroidManifest.xml`:

    ```xml
    <manifest ...>
        <uses-permission android:name="android.permission.INTERNET" />
        <application 
            android:isGame="true"
            android:appCategory="game"
			...>
            <activity
                android:configChanges="orientation|keyboardHidden|screenLayout|uiMode|screenSize"
                ...>
                ...
            </activity>
        </application>
    </manifest>
    ```

    La primera es necesaria para poder hacer uso de internet, y la segunda es para que la aplicación no se reinicie al cambiar la orientación del dispositivo.

1. Editar el archivo `CMakeLists.txt` para establecer el siguiente contenido:

    ```cmake

    # Sets the minimum version of CMake required to build the native library.

    cmake_minimum_required(VERSION 3.10.2)

    # Declares and names the project.

    project("pong")

    # Creates and names a library, sets it as either STATIC
    # or SHARED, and provides the relative paths to its source code.
    # You can define multiple libraries, and CMake builds them for you.
    # Gradle automatically packages shared libraries with your APK.

    add_library( # Sets the name of the library.
            pong

            # Sets the library as a shared library.
            SHARED

            # Provides a relative path to your source file(s).
            client/main.cpp
            client/hengine.cpp
            ...)

    # Searches for a specified prebuilt library and stores the path as a
    # variable. Because CMake includes system libraries in the search path by
    # default, you only need to specify the name of the public NDK library
    # you want to add. CMake verifies that the library exists before
    # completing its build.

    find_library( # Sets the name of the path variable.
            log-lib

            # Specifies the name of the NDK library that
            # you want CMake to locate.
            log)

    # Specifies libraries CMake should link to your target library. You
    # can link multiple libraries, such as libraries you define in this
    # build script, prebuilt third-party libraries, or system libraries.

    target_link_libraries( # Specifies the target library.
            pong

            # Links the target library to the log library
            # included in the NDK.
            ${log-lib})

    set(NATIVE_LIB pong)

    set(JNI_FOLDER ${PROJECT_SOURCE_DIR}/../jni/allegro) # or wherever you put it
    include(${JNI_FOLDER}/allegro.cmake)


    set(BOOST_JNI_FOLDER ${PROJECT_SOURCE_DIR}/../jni/boost) # or wherever you put it

    include_directories(${BOOST_JNI_FOLDER}/${ABI}/include/boost-1_76)

    macro(boost_library NAME)
        string(TOUPPER ${NAME} UNAME)
        set(path ${BOOST_JNI_FOLDER}/${ABI}/lib/lib${NAME})

        if(EXISTS "${path}-clang-mt-x32-1_76.a")
            set(LIB_${UNAME} ${path}-clang-mt-x32-1_76.a)
        elseif(EXISTS "${path}-clang-mt-x64-1_76.a")
            set(LIB_${UNAME} ${path}-clang-mt-x64-1_76.a)
        elseif(EXISTS "${path}-clang-mt-a64-1_76.a")
            set(LIB_${UNAME} ${path}-clang-mt-a64-1_76.a)
        elseif(EXISTS "${path}-clang-mt-a32-1_76.a")
            set(LIB_${UNAME} ${path}-clang-mt-a32-1_76.a)
        else()
            message(SEND_ERROR "${path}*.a does not exist")
        endif()
        target_link_libraries(${NATIVE_LIB} ${LIB_${UNAME}})
    endmacro()

    boost_library(boost_chrono)
    boost_library(boost_system)
    boost_library(boost_thread)
    boost_library(boost_json)
    ```

    Cambiando las rutas que sean necesarias y añadiendo los archivos `.cpp` necesarios a la lista de `add_library`.
