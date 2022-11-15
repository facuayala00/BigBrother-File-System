# Informe LAB 4 Grupo 18


## PONER LOS NOMBRES Y LOS MAILS ACA ARRIBA OJITO
- Ayala Facundo (facundo.ayala@mi.unc.edu.ar)
- Bonfils Gastón Tomás (gastonbonfils@mi.unc.edu.ar)
- Ordoñez Tomás (tomas.ordonez@mi.unc.ecu.ar)
- Lozano Benjamín (benjamin.lozano@mi.unc.edu.ar)

---

## Desarrollo
Brevemente discutiremos como se implementaron las 3 partes a desarrollar en este trabajo

### Parte 1
Creamos dos funciones, `fat_fuse_log_create` y`fat_fuse_write_log`. En la primera utilizamos la funcion `mknode` y definimos un path en `big_brother.c` usando lo previamente definido en dicho archivo. Luego para `fat_fuse_write_log` lo que hicimos fue escribir el archivo creado con `mknode` utilizando la funcion que nos daban de origen  `fat_fuse_log_activity` . Para escribir usamos `fat_file_pwrite`. A `fat_fuse_log_create` la utilizamos en la funcion `fat_fuse_readdir` al ultimo de todo.
A la funcion de escritura la utilizamos en `fat_fuse_write` y `fat_fuse_read`.
Por ultimo, para ocultar el archivo `fs.log` lo que hacemos es hacer en la funcion `fat_fuse_readdir` cuando lee los hijos, ignorar el path del archivo.

### Parte 2: escondiendo el archivo de logs
Modificamos la parte 1 para que cuando se monta el volumen se intente buscar el directorio secreto, identificandolo por el cluster marcado como `FAT_CLUSTER_BAD_SECTOR` y por tener nombre de entrada del path. Si se encuentra, entonces se carga el archivo `fs.log`. Si no se encuentra antes de los 10000 primeros clusters entonces se crea el directorio huérfano con el archivo de logs.   

### Parte 3: unlink y rmdir
Para implementar `unlink` copiamos la idea de la función previamente implementada `truncate` (una función de alto nivel con una de bajo nivel), pero eliminando todos los clusters del archivo. Para `rmdir` lo hicimos igual que `unlink` pero con chequeos pertinentes al caso (ver si el archivo es un directorio y si está vacío).  

### Puntos estrellas
El punto estrella que decidimos implementar fue el de agregar un registro de palabras censuradas para que al leer o escribir un archivo, registre también si alguna de las siguientes palabras se encontraba en el contenido. Dicho arreglo de palabras se encuentra en el archivo `big_brother.c` el cual se puede modificar a gusto.  
Para su implementacion decidimos definir una funcion en `big_brother.c` llamada `words_searcher` que como su nombre lo indica busca las palabras que se encuentran en el arreglo y de haber sido encontradas las agrega a una lista que se llama `words_found`, luego dicha lista se escribe en el archivo `fs.log`

--- 

## Preguntas:

1. **Cuando se ejecuta el main con la opción -d, ¿qué se está mostrando en la pantalla?**  
Se activa el modo **DEBUGGING** en el que se muestran mensajes de debugeo provenientes de FUSE que nos dan información útil como las funciones de bajo nivel que se van llamando o qué clusters se leen y escriben en la ejecución. Estos mensajes se pueden agregar con la llamada **DEBUG()**.  

2. **¿Hay alguna manera de saber el nombre del archivo guardado en el cluster 157?**  
No, si nos dan un cluster cualquiera, no podemos saber viendo su información a qué archivo pertenece. La única forma sería buscar alguna entrada de directorio que tenga como primer cluster al cluster que tenemos (o iterar por todas las cadenas de todos los archivos hasta encontrarlo).  

3. **¿Dónde se guardan las entradas de directorio? ¿Cuántos archivos puede tener adentro un directorio en FAT32?**  
a) Cuando el archivo es un directorio, sus entradas de directorio se guradan en el sector de datos, al principio.  
b) 16, esto lo descubrimos a fuerza bruta al trabajar con las imágenes, viendo que al intentar crear un archivo en un directorio con 16 archivos, el sistema de archivos no funionaba correctamente.

4. **Cuando se ejecuta el comando como ls -l, el sistema operativo, ¿llama a algún programa de usuario? ¿A alguna llamada al sistema? ¿Cómo se conecta esto con FUSE? ¿Qué funciones de su código se ejecutan finalmente?**  
Al hacer ls -l se ejecuta el programa de nombre ls, el cual realiza llamadas a sistema principalmente para leer los contenidos de un directorio y escribir en el archivo de salida. En FUSE, hacer ls -l llama a OPENDIR, GETATTR READDIR y CLOSEDIR, de todas estas funciones de FUSE, la operación de nuestro código que se ejecuta es READDIR, en la función fat_fuse_readdir.   


5. **¿Por qué tienen que escribir las entradas de directorio manualmente pero no tienen que guardar la tabla FAT cada vez que la modifican?**  
Escribimos manualmente las entradas de directorio porque las funciones que usamos para trabajar con la tabla FAT actualizan la tabla en memoria y en disco al mismo tiempo, la tabla FAT tiene su descriptor de archivo abierto que se pasa como parámetro en varias funciones de fat_table.c, estas funciones leen y escriben este archivo. Este no era el caso con las entradas de directorio, donde había disctincón entre el árbol de directorios guardado en memoria y las entradas de directorio guardadas en el disco.


6. **Para los sistemas de archivos FAT32, la tabla FAT, ¿siempre tiene el mismo tamaño? En caso de que sí, ¿qué tamaño tiene?**  
El tamaño de la tabla FAT depende del tamaño de los clusters, si tenemos clusters más chicos habrá más entradas para indexar (cada entrada de la tabla ocupa 4 bytes) y, por lo tanto, la tabla FAT será más grande.Una fórmula general para calcular la cantidad de bytes que ocupa la tabla FAT es: ((tamaño total / tamaño de cluster) + 2 (reservados)) * 4

    