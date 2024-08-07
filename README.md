# PONG

Recreación del clásico juego PONG desarrollado en C++, además incluye opción de juego online. El objetivo de este proyecto es aprender los principios básicos de la programación de videojuegos, así como del desarrollo de aplicaciones online cliente/servidor.

El proyecto intenta mantener la mayor simpleza posible pero con una infraestructura principal que pueda servir como base para proyectos más grandes.

Este proyecto tiene dos dependencias principales:

- Allegro v5.2.6+ (sólo para el cliente)
- Boost v1.75+ (utilizado principalmente para la comunicación entre sockets TCP/IP)

## Entorno de desarrollo

Actualmente la forma más sencilla de trabajar en este proyecto es utilizar un devcontainer de VSCode, pues esto configurará todo automáticamente para poder desarrollar, compilar y depurar.

Requisitos:
- Instalar WSL (sólo si se está trabajando en Windows).
- Instalar Docker.
- Instalar VS Code.
- Instalar el pack de extensiones `Remote Development`.

Pasos para preparar el entorno en VS Code:
1. Pulsar F1 y buscar `Connect to WSL`.
1. Abrir un terminal y clonar este repositorio donde se desee.
1. Pulsar `Ctrl + K + O` y abrir el directorio de la carpeta clonada.
1. Pulsar el botón `Reopen in container` que aparecerá en una ventana abajo a la derecha. (Si no aparece, siempre se puede hacer: F1 y buscar `Reopen in container`).
1. Una vez se haya montado el devcontainer, abrir un terminal y lanzar los comandos:

    ```sh
    sudo bash .devcontainer/startup.sh
    ```

Y ya estaría todo listo, ya podemos incluso lanzar el depurador de VS Code donde tendremos tanto la opción `Launch client` como `Launch server`.

La próxima vez que queramos abrir el entorno, podemos acceder directamente desde `Remote Explorer > Dev Containers`. 

## Distribuir la aplicación

Para obtener información acerca de cómo compilar para distribuir en diferentes sistemas, consulte los documentos del directorio `docs`.
