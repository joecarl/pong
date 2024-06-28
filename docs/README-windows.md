# Compilar en Windows

1. Instalar MSYS2 de 64 bits descargado de <https://www.msys2.org/>.
1. Desde `MSYS2-MSYS` o `MSYS2-UCRT MinGW 64bit` actualizar los paquetes lanzando el comando `pacman -Syuu` tantas veces como sea necesario hasta que todos los paquetes estén actualizados (mirar el tutorial de instalación si es necesario).
1. Clonar o descargar este repositorio en alguna carpeta de nuestro equipo.
1. Desde `MSYS2-UCRT MinGW 64bit` navegar al directorio del repositorio y ejecutar `bash scripts/installdeps-ucrt.sh`.
1. Ejecutar `mingw32-make -j4 DEBUG=0` (incrementando el numero de `jobs` si fuera posible).

Las carpetas generadas en `build/release` contendrán todos los archivos necesarios para distribuir la aplicación en Windows.
