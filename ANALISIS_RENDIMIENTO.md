# Analisis De La Solucion Paralela

## 1. Diseno Del Screensaver

El programa es un screensaver en SDL2 con un ciclo de dia y noche y cuatro
estaciones en orden: primavera, verano, otono e invierno. Cada estacion usa
sprites propios para el arbol, cielo, suelo, nubes, flores, fauna y clima.

Las estaciones duran un ciclo completo de sol y luna (`60000 ms`). La
transicion visual ocupa los ultimos `15000 ms` y la nueva estacion entra al
amanecer. El renderizado SDL se conserva en el hilo principal; solo se
paralelizan calculos independientes sobre vectores de elementos.

Archivos principales:

- `src/season.cpp`: perfil y progreso de las estaciones.
- `src/flower.cpp`, `src/tulip.cpp`, `src/cloud.cpp`, `src/leaf.cpp`:
  actualizaciones secuenciales y paralelas.
- `src/weather.cpp`: particulas de lluvia.
- `src/wildlife_update.cpp`: actualizacion de aves, abeja y mariposa.
- `src/performance.cpp`: medicion de rendimiento.

## 2. Calculos Por Elemento

| Elemento | Calculo secuencial por elemento |
| --- | --- |
| Flor / tulipan | Convierte la posicion normalizada `(x, y)` a un `SDL_Rect` usando tamano de pantalla y suelo. |
| Nube | Suma `velocidad * deltaTime` a `x`, reinicia la nube al salir y calcula su rectangulo de destino. |
| Hoja | Decide si comienza a caer segun el progreso de otono; actualiza `y`, visibilidad, oscilacion y destino. |
| Lluvia | Actualiza `x` e `y` de cada particula con velocidad y deriva; reinicia la gota al abandonar la pantalla. |
| Fauna | Actualiza vuelo de aves, abeja y mariposa solo cuando la estacion y la luz permiten su presencia. |

Ejemplo del calculo de posicion de una flor:

```cpp
const int centerX = static_cast<int>(screenWidth * flower.horizontalPosition);
const int bottomY = groundY + static_cast<int>(
    flower.verticalPosition * (screenHeight - groundY - 2)
);
flower.dest = {centerX - width / 2, bottomY - height, width, height};
```

La misma formula se aplica a cada posicion del vector, por lo que no existe
dependencia entre flores.

## 3. Version Secuencial

La version secuencial recorre el vector completo en un solo hilo. Por ejemplo,
las flores delegan el trabajo al mismo rango que usa la version paralela:

```cpp
void updateFlowerPositionsSequential(...) {
    updateFlowerRange(flowers, textures, width, height, groundY,
                      0, flowers.size());
}
```

Esto permite comparar ambos modos con exactamente la misma logica, cambiando
solo la forma de distribuir el rango de indices.

## 4. Paralelizacion Con OpenMP

Las actualizaciones paralelas usan OpenMP. Cada hilo recibe un bloque exclusivo
del vector, por lo que no hay escrituras simultaneas sobre el mismo elemento.

```cpp
#pragma omp parallel num_threads(requestedThreads)
{
    const std::size_t chunk = (items.size() + threadCount - 1) / threadCount;
    const std::size_t begin = threadIndex * chunk;
    const std::size_t end = std::min(begin + chunk, items.size());
    updateRange(items, ..., begin, end);
}
```

En la lluvia se usa `#pragma omp parallel for schedule(static)`. Cuando se
acumulan particulas en un mismo monticulo, la operacion compartida usa
`#pragma omp atomic capture` para evitar condiciones de carrera.

El renderizado no se paraleliza porque SDL debe ejecutarse desde el hilo
principal.

## 4.1 Resumen Por Elemento Y Codigo Para Mostrar

| Elemento | Secuencial | Paralelo | Que demuestra |
| --- | --- | --- | --- |
| Flores | Recorre todo el vector con `updateFlowerRange`. | Divide el vector en bloques exclusivos por hilo. | Caso ideal: cada flor es independiente. |
| Tulipanes | Recorre `tulips` y calcula la posicion de su flor interna. | Usa el mismo particionado por bloques. | Misma estrategia que flores, pero con menos trabajo por elemento. |
| Nubes | Actualiza posicion, reinicio lateral y destino. | Cada hilo procesa un bloque de nubes. | Paralelizacion de movimiento continuo. |
| Hojas | Calcula inicio de caida, visibilidad y destino. | Bloques OpenMP con maximo de 8 hilos. | Limite de hilos para reducir sobrecarga interactiva. |
| Lluvia | Actualiza cada particula de lluvia en un ciclo. | `omp parallel for` con planificacion estatica. | Caso con sincronizacion atomica si se acumula en el suelo. |
| Fauna | Actualiza aves, abeja y mariposa en orden. | OpenMP `sections` separa esos tres grupos. | Paralelismo por tareas independientes. |

Lineas recomendadas para la exposicion:

- [flower.cpp](src/flower.cpp): 123-167. Es la comparacion mas clara entre
  un rango secuencial y el reparto manual de bloques OpenMP.
- [tulip.cpp](src/tulip.cpp): 49-80. Muestra que se reutiliza el mismo patron
  para una estructura diferente.
- [cloud.cpp](src/cloud.cpp): 98-132. Muestra el particionado de nubes por
  movimiento y la actualizacion de su destino de render.
- [leaf.cpp](src/leaf.cpp): 145-180. Destaca el limite de 8 hilos y el
  calculo de bloques para hojas.
- [weather.cpp](src/weather.cpp): 111-191 y 194-231. Permite mostrar el ciclo
  secuencial, `#pragma omp parallel for schedule(static)` y
  `#pragma omp atomic capture`.
- [wildlife_update.cpp](src/wildlife_update.cpp): 11-38. Es un ejemplo corto
  de `#pragma omp parallel sections`.

Para una diapositiva basta con mostrar las lineas de flores y lluvia: juntas
explican el caso sin datos compartidos y el caso que necesita sincronizacion.

## 5. Como Ejecutar

```bash
make
./screensaver --sequential
OMP_NUM_THREADS=16 ./screensaver --parallel
./screensaver --benchmark
```

El benchmark usa una carga alta para hacer visible el costo de los algoritmos:

| Algoritmo | Elementos | Iteraciones |
| --- | ---: | ---: |
| Flores | 120000 | 180 |
| Tulipanes | 120000 | 180 |
| Nubes | 60000 | 360 |
| Hojas | 150000 | 220 |
| Clima | 120000 | 180 |

## 6. Resultados Medidos

Medicion ejecutada con `./screensaver --benchmark` en este equipo. OpenMP tenia
hasta `16` hilos disponibles; la actualizacion de hojas se limita a `8` hilos
para evitar una region interactiva demasiado grande.

| Algoritmo | Secuencial (ms) | Paralelo (ms) | Speedup | Eficiencia |
| --- | ---: | ---: | ---: | ---: |
| Flores | 175.951 | 46.505 | x3.783 | 23.6% (16 hilos) |
| Tulipanes | 151.145 | 157.455 | x0.960 | 6.0% (16 hilos) |
| Nubes | 84.287 | 31.287 | x2.694 | 16.8% (16 hilos) |
| Hojas | 389.255 | 165.348 | x2.354 | 29.4% (8 hilos) |
| Clima | 108.413 | 46.066 | x2.353 | 14.7% (16 hilos) |

Formulas usadas:

```text
Speedup S = T_secuencial / T_paralelo
Eficiencia E = S / P
```

Donde `P` es la cantidad de hilos. Una eficiencia de 100% seria ideal; en la
practica disminuye por la creacion de hilos, sincronizacion, balance de carga,
cache y limites del hardware.

## 7. Interpretacion

- Flores, nubes, hojas y clima mejoran con OpenMP porque procesan muchos
  elementos independientes.
- Las hojas presentan la mejor eficiencia de la medicion porque concentran
  bastante calculo por elemento y usan un limite de 8 hilos.
- Los tulipanes obtuvieron `x0.960`: en esta medicion el paralelo fue levemente
  mas lento. Esto demuestra que paralelizar no siempre conviene cuando la
  sobrecarga de OpenMP supera el trabajo util.
- Los valores pueden cambiar con `OMP_NUM_THREADS`, el procesador y otros
  procesos del sistema. Para una comparacion justa se recomienda cerrar cargas
  pesadas y repetir la prueba varias veces.

## 8. Conclusion

La solucion mantiene una ruta secuencial como referencia y una ruta paralela
con OpenMP para los conjuntos mas costosos. El resultado valida que el
paralelismo es especialmente util cuando aumenta la cantidad de elementos,
pero debe evaluarse por algoritmo para evitar aplicar hilos donde no aportan
beneficio.
