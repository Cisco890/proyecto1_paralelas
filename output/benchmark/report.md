# Reporte de benchmark

Los valores son promedios de las mediciones ejecutadas por `scripts/benchmark.py`.

| Algoritmo | Mediciones | Sec. ms | Par. ms | Speedup | Eficiencia | Desv. sec. | Desv. par. |
|---|---:|---:|---:|---:|---:|---:|---:|
| Flores | 10 | 287.489 | 76.195 | 3.773 | 0.943 | 1.631 | 3.088 |
| Tulipanes | 10 | 400.061 | 106.282 | 3.764 | 0.941 | 3.306 | 1.701 |
| Nubes | 10 | 253.808 | 68.158 | 3.724 | 0.931 | 1.798 | 1.759 |
| Hojas | 10 | 836.333 | 217.459 | 3.846 | 0.961 | 10.054 | 2.101 |
| Clima | 10 | 322.524 | 77.425 | 4.166 | 1.041 | 3.272 | 2.011 |

Fórmulas: `speedup = secuencial / paralelo`; `eficiencia = speedup / workers`.
