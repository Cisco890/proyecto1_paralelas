# Reporte de benchmark

Los valores son promedios de las mediciones ejecutadas por `scripts/benchmark.py`.

| Algoritmo | n | Sec. ms | Par. ms | Speedup | Eficiencia | Desv. sec. | Desv. par. |
|---|---:|---:|---:|---:|---:|---:|---:|
| Flores | 10 | 282.092 | 244.554 | 1.153 | 0.144 | 13.225 | 3.181 |
| Tulipanes | 10 | 400.131 | 251.487 | 1.591 | 0.199 | 2.743 | 5.011 |
| Nubes | 10 | 252.270 | 477.463 | 0.528 | 0.066 | 2.388 | 11.810 |
| Hojas | 10 | 836.058 | 208.470 | 4.010 | 0.501 | 5.259 | 16.633 |

Fórmulas: `speedup = secuencial / paralelo`; `eficiencia = speedup / workers`.
