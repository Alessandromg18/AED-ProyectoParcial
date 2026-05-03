# AED-ProyectoParcial
# integrantes

## Operaciones de Agregación

Se han implementado las operaciones matemáticas cruzadas (agregación). Estas calcularán valores sobre filas, columnas o rangos bidimensionales de manera directa y optimizada.

### Funcionalidades

**En la clase estructural (`matrix/SparseMatrix.cpp` / `.h`):**
*   `getNumericValuesInRange(...)` **[NUEVO]:** Es la función "motor". Itera por las listas circulares dentro de un recuadro de coordenadas delimitado. Extrae e ignora texto (letra pura) y salta espacios vacíos, devolviendo solo los números puros.
*   `aggSum(...)`  Suma de todos los valores numéricos detectados.
*   `aggAverage(...)` Saca el promedio de todos los valores numéricos detectados (maneja casos borde para no dividirse entre cero si está vacío).
*  `aggMax(...)` Obtiene el número de valor más alto.
*   `aggMin(...)`  Obtiene el número de valor más bajo.

**En la Interfaz del Usuario (`main.cpp`):**
*   `parseExcelRange(...)` Una función traductora que lee lo que el usuario inserta en la consola (Ejemplo: `"A1:C4"`, `"A"` para una columna entera, o `"3"` para apuntar a una fila entera) y las adapta para mandarle las 4 coordenadas que requiere la matriz matemática internamente. 
*   **Comandos en consola** Integración en el *texto ingresado* en SFML de los comandos interactivos `sum`, `avg`, `max` y `min`.