# Reporte de benchmark

Los valores son promedios de las mediciones ejecutadas por `scripts/benchmark.py`.

| Algoritmo | Mediciones | Sec. ms | Par. ms | Speedup | Eficiencia | Desv. sec. | Desv. par. |
|---|---:|---:|---:|---:|---:|---:|---:|
| Flores | 10 | 109.868 | 35.891 | 3.061 | 0.765 | 21.933 | 6.330 |
| Tulipanes | 10 | 140.359 | 39.354 | 3.567 | 0.892 | 1.899 | 4.252 |
| Nubes | 10 | 87.358 | 26.609 | 3.283 | 0.821 | 1.594 | 5.756 |
| Hojas | 10 | 401.165 | 113.440 | 3.536 | 0.884 | 11.441 | 5.286 |
| Clima | 10 | 111.878 | 28.711 | 3.897 | 0.974 | 2.204 | 3.138 |

Fórmulas: `speedup = secuencial / paralelo`; `eficiencia = speedup / workers`.
