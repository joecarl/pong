#!/bin/bash

#global-deps
apt install -y libboost1.81-all-dev gfortran-
apt install -y libboost-json1.81-dev gfortran-

# El signo "-" después del nombre del paquete sirve para que no se instale ese paquete.
# Por eso aparece después de "gfortran" ya que ese paquete daba problemas con la imagen 
# de docker "gcc" utilizada para lanzar el server
