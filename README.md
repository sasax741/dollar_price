APLICACION PARA VERIFICAR EL PRECIO DEL DOLAR

un programa que ejecuta un curl a la api : https://dolarapi.com
api usada por dolarHoy y otras aplicaciones web muestran el precio del dolar


mi objectivo con esta app es integrarla en el path de mi maquina linux y poder verificar el precio del dolar desde mi PC

para compilar el programas uso gcc

comando de compilacion : gcc -o dollar2 dollar.c -lcurl -ljansson -L/ruta/a/libjansson