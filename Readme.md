### (EN CONSTRUCCION)

## Programa de derivación de variables Netcdf de tres dimensiones

#### Objetivos.

El objetivo de este programa es la realización de una interpolación mediante una spline cúbica de los valores de un variable vector netcdf de forma V(d1=t,d2,d3), donde la dimensión más lenta corresponde al tiempo y la obtención de los valores de su derivada en ese tiempo, que será la salida del programa en un nuevo fichero con identicas dimensiones.

Esto puede ser útil para la obtención de un fichero preparado para una representación cartográfica si las dimensiones restantes corresponden a latitud y longitud, por ejemplo.

---

#### Programación y medios.

Se utiliza para la programación el lenguaje C++ en entorno linux y con recursos de compilacion GCC y CMake. 

La IDE utilizada es ***VSCodium*** y extensiones de ***CLang*** y depuración habituales.  La aplicación de cálculo algebraico e interpolación es la versión gratuita del programa ***Alglib*** para C++, bajo licencia GPL2+, que se adjunta al programa tal como se obtiene, sin modificación alguna. La librería de API de Netcdf es la de ***Unidata*** en la versión del gestor de paquetes de  Fedora 44. 

---

#### Declaración de uso personal y sin rendimiento económico.

Esta programa es un desarrollo realizado por una persona física y con solo un interés de presentación de un método alternativo de cálculo de magnitudes sometidas a cambio. No hay intención de ser la mejor muestra de programación posible, sino un intento ptopio de obtener un resultado. Cualquier uso que se pueda hacer en otro sentido, debe cumplir con las exigencias de los medios utilizados y expresados en las condiciones que dispongan para ello. No se responde del uso por terceros que puedan acceder a ello.

---

#### Detalles del método.

En la derivación se utiliza por eficiencia una función que realiza en el mismo proceso interpolación y derivación, en el que aún siendo posible que el resultado posea otra granularidad distinta a la de entrada, se utiliza la misma. 

En cuanto al necesario array de valores de tiempo como abscisas, se realiza una simplificación a valores de conteo consecutivo con incremento de la unidad, desde *n=0* hasta el valor final temporal del fichero. Se incorporará la opción de solucionar los límites de la spline de forma natural,parabólica y por valores restringidos conocidos de primera y segunda derivada.

La entrada será el fichero Netcdf con al menos una variable con esas tres dimensiones y un fichero de definición que se puede rellenar con cualquier editor de texto.  En este fichero, de nombre *diffmotion.cfig* , en el mismo directorio del programa, se incluirá al menos estos valores en filas terminadas por caracter de linea:

1. Ruta del fichero.

2. Nombre del fichero con extensión incluida.

3. Si se quiere limitar el alcance temporal del fichero, un valor entero indicando el número de datos de la dimensión tiempo a considerar. Deben ser más de tres y no deben superar el alcance máximo definido en él. **Si no se quiere limitar -1**.

4. Nombre de la variable contenida en el fichero.

5. Tipo de limitación de bordes: *natural* | *parabolic* | *clamped_1d* | *clamped_2d*  (una a elegir) . 

6. Si se elige *clamped*, el valor del borde izquierdo y derecho separado por punto y coma. En otro caso cualquier valor o ninguno.

Se incluye un fichero con este contenido para poder servir de plantilla.
La salida del programa sera el nombre del de entrada con un sufijo añadido "_\_Dif_" en la misma ruta.

### Todo   

1. Se va a revisar la gestión de Fillvalues.

2. Se incluirá un fichero de información de lo que se realiza en el fivchero de salida desde el de entrada.

3. Se permitirá una cuarta dimensión con un solo valor asignado.

---

J.R.Fuentes Martinez   (Spain)    July 2026
