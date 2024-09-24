# Dollar Price API Client

Este es un cliente de API en C que consulta el precio del dólar desde una API y procesa la respuesta en formato JSON.

## Requisitos

Para compilar y ejecutar este programa, necesitarás tener instaladas las siguientes bibliotecas:

- **libcurl**: para realizar solicitudes HTTP.
- **jansson**: para manejar datos en formato JSON.

## Instalación de Dependencias

Para instalar las bibliotecas necesarias en Debian/Ubuntu, ejecuta los siguientes comandos:

```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev libjansson-dev
```

## Compilación

Para compilar el programa en Linux (Debian/Ubuntu), utiliza el siguiente comando:

```bash
gcc -o dollar_price dollar.c -lcurl -ljansson
```
Este comando generará un archivo ejecutable llamado dollar_program.
(las flags son para llamar las librerias externas que instalamos previamente)

## Ejecución

Para ejecutar el programa, usa el siguiente comando:

```bash

./dollar_program
```

## Agregar el Ejecutable al PATH

Para ejecutar el programa desde cualquier lugar sin necesidad de especificar la ruta completa, agrega el directorio que contiene el ejecutable a tu PATH. Sigue estos pasos:

    Ubica el directorio del ejecutable (por ejemplo, si está en /home/alexis/desarrollo/personal/dollar_price/).

    Agrega el directorio al PATH. Abre tu archivo de configuración de shell (~/.bashrc o ~/.bash_profile) y añade la siguiente línea al final:

```bash

export PATH="$PATH:/directorio/en/donde/esta/el_compilado"
```

## Aplica los cambios ejecutando:

```bash

    source ~/.bashrc
```

## Ahora puedes ejecutar el programa simplemente escribiendo:

```bash

dollar_price
```

o como le hayas puesto al output al momento de compilar


## Notas

    Asegúrate de tener acceso a Internet, ya que el programa realiza una solicitud a una API externa para obtener los precios del dólar.
    Si tienes problemas con la asignación de memoria o errores al conectar con la API, asegúrate de revisar los mensajes de error en la terminal.

    Este archivo contiene las instrucciones necesarias para compilar y ejecutar el programa en un sistema Linux.
