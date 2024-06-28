# Indicaciones para portar el cliente a Android

1. Compilar Allegro (seguir las indicaciones de este repositorio: <https://github.com/liballeg/android>)

1. Compilar Boost (seguir las indicaciones del apartado `Compiling` de este repositorio: <https://github.com/moritz-wundke/Boost-for-Android>). Es recomendable ejecutar el script de compilación en un entorno Linux.

1. Añadir los siguientes atributos a `AndroidManifest.xml`:

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

    El primero es necesario para poder hacer uso de internet, los dos siguientes sirven para indicar que el tipo de aplicación es un juego y el último es para que la aplicación no se reinicie al cambiar la orientación del dispositivo.

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
