# Desafío 1 - Informática II (UdeA, 2025-1)

Este proyecto implementa una solución para reconstruir imágenes encriptadas aplicando ingeniería inversa sobre transformaciones a nivel de bits (XOR, rotaciones circulares) y enmascaramientos validados mediante archivos `.txt`.

## Estructura del repositorio

- `/Caso1`, `/Caso2`: Carpetas con archivos de prueba (I_D.bmp, I_M.bmp, M.bmp y archivos M*.txt)
- `main.cpp`: Código principal con todas las funciones de transformación y validación.
- `Informe Desafio 1 Hugo y Angel.docx`: Documento técnico con análisis, metodología y pruebas.
- `Desafío I v1.pdf`: Enunciado original del problema.
- `README.md`: Este archivo.

## Requisitos

- Qt 5 o superior
- C++17
- Compilador compatible con `QImage` (proyecto creado como consola en Qt Creator)

## Ejecución

1. Abrir el proyecto en Qt Creator.
2. Ejecutar el programa.
3. Seleccionar el caso a probar (Caso1, Caso2 o carpeta personalizada).
4. El programa reconstruirá paso a paso la imagen original, generando archivos BMP intermedios.

## Autores
- Hugo Esteban Barrero García
- Juan Angel Omaña Montañez
