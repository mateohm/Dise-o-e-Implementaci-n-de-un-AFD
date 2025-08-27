# Diseño e Implementación de un AFD

## Introducción

En esta práctica se diseñó e implementó un Autómata Finito Determinista (AFD) en dos lenguajes de programación: Python y C. El AFD permite verificar si un conjunto de cadenas son aceptadas o rechazadas de acuerdo con una configuración definida previamente. Para lograr esto, el programa lee dos archivos:

Conf.txt: contiene la configuración del autómata (estados, alfabeto, estado inicial, estados de aceptación y transiciones).

Cadenas.txt: incluye las cadenas de prueba a evaluar.


## Ejecución del programa

- Guardar en la misma carpeta los archivos dfa.py o dfa.c, junto con Conf.txt y Cadenas.txt.
- En Python:

   ``` 
  python dfa.py
   
   ```
- En C:

   ```
   gcc dfa.c -o dfa
   ./dfa
   
   ```

   El programa mostrará en pantalla cada cadena seguida de su resultado: ACEPTADA     o RECHAZADA.


## Gramática usada en la documentación

La descripción del AFD se basó en una gramática sencilla de tipo declarativo para expresar sus componentes:

- states: lista de estados separados por comas.

- alphabet: símbolos permitidos, cada uno de un carácter.

- start: estado inicial.

- accept: conjunto de estados de aceptación.

- transitions: reglas de la forma estado, símbolo -> destino.

Esta gramática asegura que el archivo de configuración sea legible, estructurado y fácil de procesar tanto por el programa en Python como en C.
