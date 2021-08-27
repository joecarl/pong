# pong

Pong recreation with online playing option.
Allego5 based.

## Para compilar el proyecto en windows es necesario preparar la siguiente configuración:

1. Instalar GIT for windows (no es totalmente necesario pero recomendable si se va a participar en el desarrollo).
1. Instalar MSYS2 de 64 bits descargado de https://www.msys2.org/ *[1]
1. Desde MSYS2-MSYS actualizar los paquetes con `pacman -Syuu` hasta que no se pueda más (mirar el tutorial de instalación por si acaso).
1. Desde MSYS2-MSYS navegar al directorio del proyecto y ejecutar `bash installdeps.sh`.
1. Ejecutar `bash win-build.sh` (esto se puede hacer desde Git Bash).
1. Por ahora, para ejecutar el archivo compilado es necesario incluir en el PATH la carpeta `bin` de mingw64.
