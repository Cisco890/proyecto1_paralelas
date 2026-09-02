# Reporte de benchmark

Los valores son promedios de las mediciones ejecutadas por `scripts/benchmark.py`.

| Algoritmo | Mediciones | Sec. ms | Par. ms | Speedup | Eficiencia | Desv. sec. | Desv. par. |
|---|---:|---:|---:|---:|---:|---:|---:|
| Flores | 10 | 236.137 | 98.110 | 2.407 | 0.602 | 51.507 | 10.953 |
| Tulipanes | 10 | 259.220 | 121.657 | 2.131 | 0.533 | 10.801 | 17.632 |
| Nubes | 10 | 172.805 | 89.982 | 1.920 | 0.480 | 10.687 | 17.454 |
| Hojas | 10 | 839.661 | 396.719 | 2.117 | 0.529 | 65.989 | 94.071 |

Fórmulas: `speedup = secuencial / paralelo`; `eficiencia = speedup / workers`.
