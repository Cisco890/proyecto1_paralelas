# 🌳 Screen Saver – Árbol Estacional

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
