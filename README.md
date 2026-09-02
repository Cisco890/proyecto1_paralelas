# 🌳 Screen Saver – Árbol Estacional

## Estructura del proyecto

```
proyecto1_paralelas/
├── main.cpp              # Init SDL, loop principal y eventos
├── Makefile              # Compilación del proyecto
├── integrantes.c         # Nombres del equipo
├── README.md
├── .gitignore
├── assets/
│   ├── animals/
│   │   └── bird.png
│   └── tree/
│       └── tree.png
└── src/
    ├── bird.hpp / bird.cpp   # Pájaro (update + render)
    └── tree.hpp / tree.cpp   # Árbol estático (load + render)
```

## Compilación y ejecución

**Dependencias:** `SDL2` y `SDL2_image` (y un compilador C++ con soporte C++17).

```bash
# Compilar
make

# Ejecutar (desde la raíz del proyecto, para cargar assets/)
./screensaver

# Limpiar binarios y objetos
make clean
```

### Controles de estaciones

Las estaciones cambian automaticamente al completar un ciclo de sol y luna
(60 segundos). Durante los ultimos 15 segundos, que corresponden al tramo
nocturno, se mezclan gradualmente los colores antes del siguiente amanecer.
Tambien se pueden probar manualmente desde la ventana:

- `1`: primavera
- `2`: verano
- `3`: otono
- `4`: invierno
- `Esc`: salir

## Modos De Ejecucion

Se agregaron rutas secuenciales para las actualizaciones de:

- flores
- tulipanes
- nubes
- hojas
- lluvia
- fauna (aves, abeja y mariposa)

La animacion ahora puede ejecutarse en cualquiera de los dos modos:

```bash
./screensaver --parallel
./screensaver --sequential
```

## Benchmarks

Los benchmarks se ejecutan sin abrir la ventana SDL. Miden solo la logica de
actualizacion para que el renderizado, el monitor y la sincronizacion vertical
no alteren la comparacion. El benchmark principal evalua flores, tulipanes,
nubes y hojas con la misma cantidad de elementos e iteraciones en ambos modos.

| Comando | Proposito | Resultado |
| --- | --- | --- |
| `./screensaver --benchmark` | Comparacion base | Tiempo secuencial, tiempo paralelo y aceleracion por algoritmo. |
| `./screensaver --benchmark --detailed` | Analisis estadistico | Tres repeticiones, CSV y grafica SVG. |
| `./screensaver --benchmark --scalability` | Escalabilidad | Compara 20 000, 60 000 y 120 000 elementos. |
| `./screensaver --benchmark --validate` | Correccion | Verifica que ambos modos produzcan el mismo estado. |

### Comparacion base

Ejecuta una medicion de referencia:

```bash
./screensaver --benchmark
```

La salida usa estas metricas:

- `secuencial` y `paralelo`: tiempo total en milisegundos; menor es mejor.
- `aceleracion: xN`: cuantas veces es mas rapido el modo paralelo. Por ejemplo,
  `x2.0` significa que tarda aproximadamente la mitad.
- Una aceleracion menor que `x1.0` significa que el paralelo fue mas lento.
  Esto es normal con pocos elementos porque crear y coordinar hilos tiene costo.

### Analisis Detallado

Para reducir la variacion de una sola ejecucion:

```bash
./screensaver --benchmark --detailed
```

El comando ejecuta tres repeticiones de cada prueba y crea
`benchmark_results.csv` y `benchmark_chart.svg` en la raiz del proyecto. El
CSV incluye promedio, minimo, maximo y desviacion estandar; la grafica SVG
muestra el promedio, aceleracion y porcentaje de mejora. En la grafica, una
barra secuencial mas grande indica que ese modo tardo mas para la misma carga;
por tanto, una barra paralela mas corta representa mejor rendimiento. El CSV
puede abrirse con una hoja de calculo y la grafica SVG se abre directamente en
un navegador.

### Escalabilidad Y Validacion

```bash
./screensaver --benchmark --scalability
./screensaver --benchmark --validate
```

La prueba de escalabilidad mide como cambia la aceleracion con 20 000, 60 000
y 120 000 elementos, y genera `benchmark_scalability.csv` y
`benchmark_scalability.svg`. En esta grafica, una linea mas alta representa
mayor aceleracion del paralelo. La validacion compara el estado final de
flores, tulipanes, nubes, hojas y lluvia para confirmar que las rutas
secuencial y paralela son equivalentes.

Si quieres medir solo un modo:

```bash
./screensaver --benchmark --parallel
./screensaver --benchmark --sequential
```

Tambien se puede activar la comparacion base con
`SCREENSAVER_BENCHMARK=1 ./screensaver`.

Los tiempos dependen del procesador, cantidad de nucleos y carga del equipo.
Para un informe, conviene ejecutar el analisis detallado sin otros programas
pesados y reportar los promedios, no una sola ejecucion.

La configuracion compartida de cada estacion esta en `src/season.cpp`. Los
elementos nuevos deben consultar `SeasonSystem` y `SeasonProfile` para adaptar
su cantidad, color o comportamiento sin duplicar la logica de estaciones.
El ciclo se mantiene siempre en este orden: primavera, verano, otono e
invierno. Durante primavera las flores y tulipanes crecen gradualmente;
en otono caen las hojas; en invierno la lluvia es la precipitacion principal,
y primavera/verano usan mayor intensidad de luz solar. La presencia de aves,
abejas y mariposas tambien se calcula desde el perfil de cada estacion.

La logica queda separada por responsabilidad: `src/season.cpp` calcula el
estado estacional, `src/weather.cpp` actualiza las particulas y
`src/wildlife_update.cpp` actualiza la fauna. Los dos ultimos modulos exponen
rutas secuenciales y paralelas, seleccionadas con `--sequential` o
`--parallel`.
El arbol usa la textura base. Las hojas animadas de primavera y las hojas de
otono se administran desde `src/leaf.cpp`; su posicion y caida se actualizan
por bloques independientes.
Los animales animados reutilizables se implementan en `src/flying_animal.cpp`.
La lluvia usa el sistema de particulas de `src/weather.cpp`, y la grama
animada se administra desde `src/grass.cpp`. Sus intensidades se definen en
cada `SeasonProfile`.

Compilación manual equivalente:

```bash
g++ -std=c++17 -I. -o screensaver main.cpp src/bird.cpp src/tree.cpp \
  $(pkg-config --cflags --libs sdl2 SDL2_image)
```

---

## Descripción

El proyecto consiste en un **Screen Saver animado** cuyo elemento principal será un árbol que evolucionará dinámicamente según la **estación del año** y el **momento del día**. Mientras la computadora permanezca inactiva, el escenario cambiará gradualmente para simular el paso del tiempo, generando una experiencia visual relajante y diferente en cada ejecución.

Cada elemento del escenario podrá configurarse de forma independiente, permitiendo modificar su frecuencia de aparición y otros parámetros relacionados.

---

# Objetos del escenario

- 🌳 Árbol (tronco y follaje)
- 🍃 Hojas
- 🌱 Piso
- 🌸 Flores
- ❄️ Nieve
- 🌧️ Lluvia
- ☁️ Nubes
- ☀️ Sol
- 🌙 Luna
- ⭐ Estrellas
- 🐦 Pájaros
- 🦋 Mariposas
- 🐈 Gato
- ✈️ Aviones
- ☄️ Cometas o estrellas fugaces *(opcional)*

---

# Configuración

Cada objeto podrá configurarse de manera independiente mediante parámetros, permitiendo controlar aspectos como:

- Frecuencia de aparición.
- Cantidad máxima de elementos visibles.
- Intensidad de efectos (lluvia, nieve, viento, etc.).
- Activar o desactivar elementos específicos.

---

# Sistema de tiempo

El protector de pantalla contará con un sistema que simulará el paso del tiempo mientras permanezca activo.

## Estaciones

- 🌸 Primavera
- ☀️ Verano
- 🍂 Otoño
- ❄️ Invierno

## Ciclo del día

- ☀️ Día
- 🌙 Noche

Los cambios de estación y del ciclo día/noche ocurrirán automáticamente conforme transcurra el tiempo de inactividad. La velocidad de esta transición podrá configurarse para ofrecer una experiencia más dinámica.

---

# Comportamiento por estación

## 🌸 Primavera

- Árbol con follaje abundante.
- Colores vivos.
- Muy pocas hojas caerán.
- Aparecerán flores en el suelo.
- Las mariposas revolotearán sobre las flores.
- Mayor presencia de aves.

---

## ☀️ Verano

- Árbol completamente frondoso.
- Tonos verdes ligeramente más opacos.
- Caída ocasional de hojas.
- Cielo despejado con algunas nubes.
- Mayor actividad de aves.

---

## 🍂 Otoño

- Disminución del follaje.
- Caída constante de hojas.
- Colores café, amarillo y naranja.
- Ambiente más seco.

---

## ❄️ Invierno

- Árbol sin hojas.
- Caída de nieve.
- Suelo cubierto de nieve.
- Colores fríos y ambiente tranquilo.

---

# Ciclo día y noche

El comportamiento natural del árbol continuará independientemente del momento del día. Por ejemplo, si una estación contempla la caída de hojas, esta seguirá ocurriendo durante la noche.

## ☀️ Día

- Sol.
- Nubes.
- Iluminación natural.

## 🌙 Noche

- Luna.
- Estrellas.
- Posibilidad de cometas o estrellas fugaces.
- Iluminación tenue.

---

# Aparición de elementos

Los elementos decorativos aparecerán de forma aleatoria para dar vida al escenario. Algunos tendrán condiciones específicas para aparecer.

| Elemento | Condición |
|----------|-----------|
| 🦋 Mariposas | Solo cuando existan flores. |
| 🐦 Pájaros | Principalmente durante el día. |
| 🐈 Gato | Puede aparecer en cualquier momento y estación. |
| ✈️ Aviones | Pueden cruzar el cielo en cualquier momento. |
| ☄️ Estrellas fugaces | Solo durante la noche y con baja probabilidad. |

---

# Reglas de interacción

Algunos elementos dependerán de la presencia de otros para aparecer.

Ejemplos:

- Las mariposas solo aparecerán si existen flores.
- Las mariposas se moverán alrededor de las flores.
- Las estrellas fugaces únicamente podrán aparecer durante la noche.
- La nieve solo aparecerá durante el invierno.
- La lluvia podrá aparecer en cualquier estación, aunque será más frecuente durante primavera.

---

# Configuración sugerida

Cada elemento podría contar con parámetros similares a los siguientes:

```json
{
  "enabled": true,
  "spawnRate": 0.5,
  "maxInstances": 5
}
```

Donde:

- **enabled**: habilita o deshabilita el elemento.
- **spawnRate**: probabilidad de aparición.
- **maxInstances**: cantidad máxima visible simultáneamente.

---

# Posibles mejoras futuras

- 🌬️ Sistema de viento que afecte hojas, lluvia y nieve.
- 🌈 Aparición de arcoíris después de la lluvia.
- 🐿️ Animales adicionales (ardillas, conejos, etc.).
- 🌳 Variaciones del árbol según la estación.
- 🔆 Iluminación dinámica según la posición del sol o la luna.
- 🎵 Sonidos ambientales opcionales.
- 🌫️ Neblina o niebla en determinadas condiciones.
- 🌧️ Tormentas con relámpagos poco frecuentes.
- 🐦 Bandadas de aves cruzando el cielo.
- 🎄 Eventos especiales según fechas (Navidad, Halloween, etc.).

---

# Objetivo

Crear un protector de pantalla dinámico, relajante y visualmente atractivo que nunca se vea exactamente igual dos veces, gracias a la combinación de estaciones, ciclo día/noche y eventos aleatorios.


# Algunas Screenshots
![primavera](image.png)
![invierno](image-1.png)
