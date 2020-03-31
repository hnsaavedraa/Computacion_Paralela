# Algoritmo de Bluring

## Autores
* Johan Sebastian Salamanca Gonzalez (github.com/ssalamancag)
* Harold Nicolas Saavedra Alvarado (github.com/hnsaavedraa)
* Jose Fernando Morantes Flores (github.com/FernandoMorantes)


## Como usar

Para ejecutar el archivo blur-effect.c y aplicar el filtro sobre una imagen en especifico es necesario ejecutar el siguiente comando sobre una consola UNIX
```bash
gcc -Wall -pedantic blur-effect.c -o Nombrejecutable -pthread  -lm
time ./Nombrejecutable miImagen.jpg miImagen_blur.jpg kernel nHilos
```

Donde:
miImagen.jpg es la imagen original
miImagen_blur.jpg es el nombre de la imagen de salida
kernel es un valor entero entre 3 y 15
nHilos corresponde el numero de hilos en el que correra el programa

Para un correcto funcionamiento del script del que hablaremos a continuacion se recomienda que el Nombreejecutable sea "image", de lo contrario el script no funcionara de manera correcta.

En caso de desear ejecutar el script script_ejecutar_todo.sh, el cual ejecutara uno a uno los casos y almacenara los resultados de tiempo de respuesta en un archivo llamdo times.txt
Para correr el script es necesario ejecutar los comandos en una terminal UNIX

```bash
chmod +x script_ejecutar_todo.sh
./script_ejecutar_todo.sh
```
El script ejecutara los casos de prueba haciendo uso de las imagenes contenidas en este repositorio, en caso de querer usar otras imagenes se debe abrir el script con un editor de texto y cambiar los valores contenidos en el arreglo "img_init", asi mismo se deben indicar el nombre que tendran las imagenes procesadas en el arreglo "img_fin".

## Consideraciones
 
Hay que tener en cuenta que en algoritmo solo recibe imagenes en formato .jpg o .jpeg
Toda la informacion referente a la implementacion del algoritmo se encuentra en "reporte.pdf"
