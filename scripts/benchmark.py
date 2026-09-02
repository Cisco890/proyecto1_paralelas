#!/usr/bin/env python3
"""Ejecuta mediciones repetidas del benchmark secuencial/paralelo.

Uso:
    python3 scripts/benchmark.py --binary ./screensaver --runs 10 --workers 4
"""

from __future__ import annotations

import argparse
import csv
import os
import re
import statistics
import subprocess
import sys
from collections import defaultdict
from pathlib import Path


ALGORITHMS = ("Flores", "Tulipanes", "Nubes", "Hojas", "Clima")
LINE_RE = re.compile(
    r"^(Flores|Tulipanes|Nubes|Hojas|Clima)\s+\|\s+elementos:\s+(\d+)\s+\|\s+"
    r"iteraciones:\s+(\d+)\s+\|\s+modo:\s+(secuencial|paralelo|sequential|parallel)\s+\|\s+"
    r"tiempo:\s+([0-9]+(?:\.[0-9]+)?)\s+ms\s*$"
)
MODE_MAP = {
    "secuencial": "sequential",
    "paralelo": "parallel",
    "sequential": "sequential",
    "parallel": "parallel",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", default="./screensaver",
                        help="ruta al ejecutable compilado")
    parser.add_argument("--runs", type=int, default=10,
                        help="mediciones por modo, mínimo recomendado: 10")
    parser.add_argument("--workers", type=int, default=4,
                        help="workers usados para calcular eficiencia")
    parser.add_argument("--output-dir", default="output/benchmark",
                        help="directorio de salida")
    return parser.parse_args()


def execute(binary: Path, mode: str, workers: int) -> tuple[dict[str, dict], str]:
    command = [str(binary), "--benchmark", f"--{mode}"]
    environment = os.environ.copy()
    environment["OMP_NUM_THREADS"] = str(workers)
    environment["OMP_DYNAMIC"] = "FALSE"
    completed = subprocess.run(
        command,
        cwd=binary.parent,
        env=environment,
        text=True,
        capture_output=True,
        check=False,
        timeout=300,
    )
    output = completed.stdout + completed.stderr
    if completed.returncode != 0:
        raise RuntimeError(
            f"El benchmark {mode} terminó con código {completed.returncode}:\n{output}"
        )

    parsed: dict[str, dict] = {}
    for line in output.splitlines():
        match = LINE_RE.match(line.strip())
        if match is None:
            continue
        label, elements, iterations, parsed_mode, milliseconds = match.groups()
        parsed[label] = {
            "elements": int(elements),
            "iterations": int(iterations),
            # El programa C++ imprime los nombres de modo en español; se
            # normalizan para que el agrupamiento sea independiente del idioma.
            "mode": MODE_MAP[parsed_mode],
            "milliseconds": float(milliseconds),
        }
    missing = set(ALGORITHMS) - set(parsed)
    if missing:
        raise RuntimeError(
            f"No se pudieron extraer {', '.join(sorted(missing))} del modo {mode}.\n{output}"
        )
    return parsed, output


def write_measurements(path: Path, rows: list[dict]) -> None:
    fields = ["run", "mode", "algorithm", "elements", "iterations", "milliseconds"]
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def write_summary(path: Path, rows: list[dict], workers: int) -> list[dict]:
    grouped: dict[tuple[str, str], list[float]] = defaultdict(list)
    metadata: dict[str, dict] = {}
    for row in rows:
        key = (row["algorithm"], row["mode"])
        grouped[key].append(float(row["milliseconds"]))
        metadata[row["algorithm"]] = row

    summary = []
    for algorithm in ALGORITHMS:
        sequential = grouped[(algorithm, "sequential")]
        parallel = grouped[(algorithm, "parallel")]
        seq_mean = statistics.mean(sequential)
        par_mean = statistics.mean(parallel)
        speedup = seq_mean / par_mean if par_mean else 0.0
        summary.append({
            "algorithm": algorithm,
            "runs": len(sequential),
            "elements": metadata[algorithm]["elements"],
            "iterations": metadata[algorithm]["iterations"],
            "sequential_mean_ms": seq_mean,
            "parallel_mean_ms": par_mean,
            "sequential_stddev_ms": statistics.stdev(sequential) if len(sequential) > 1 else 0.0,
            "parallel_stddev_ms": statistics.stdev(parallel) if len(parallel) > 1 else 0.0,
            "speedup": speedup,
            "efficiency": speedup / workers if workers > 0 else 0.0,
            "workers": workers,
        })

    fields = list(summary[0].keys())
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        writer.writerows(summary)
    return summary


def write_svg(path: Path, summary: list[dict], field: str, title: str,
              y_label: str, maximum: float | None = None) -> None:
    width, height = 900, 520
    left, bottom, chart_width, chart_height = 90, 80, 750, 350
    values = [float(row[field]) for row in summary]
    top = maximum if maximum is not None else max(values + [1.0]) * 1.20
    top = max(top, 1.0)
    bar_width = chart_width / len(summary) * 0.62
    chunks = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}">',
        '<rect width="100%" height="100%" fill="white"/>',
        f'<text x="{width / 2}" y="34" text-anchor="middle" font-size="22" font-family="sans-serif">{title}</text>',
        f'<text x="20" y="{height / 2}" transform="rotate(-90 20 {height / 2})" text-anchor="middle" font-size="14" font-family="sans-serif">{y_label}</text>',
        f'<line x1="{left}" y1="{height - bottom}" x2="{left + chart_width}" y2="{height - bottom}" stroke="black"/>',
        f'<line x1="{left}" y1="{height - bottom}" x2="{left}" y2="{height - bottom - chart_height}" stroke="black"/>',
    ]
    for index, (row, value) in enumerate(zip(summary, values)):
        x = left + (index + 0.5) * chart_width / len(summary) - bar_width / 2
        bar_height = value / top * chart_height
        y = height - bottom - bar_height
        chunks.append(f'<rect x="{x:.1f}" y="{y:.1f}" width="{bar_width:.1f}" height="{bar_height:.1f}" fill="#357edd"/>')
        chunks.append(f'<text x="{x + bar_width / 2:.1f}" y="{y - 8:.1f}" text-anchor="middle" font-size="13" font-family="sans-serif">{value:.3f}</text>')
        chunks.append(f'<text x="{x + bar_width / 2:.1f}" y="{height - bottom + 25}" text-anchor="middle" font-size="13" font-family="sans-serif">{row["algorithm"]}</text>')
    chunks.append("</svg>")
    path.write_text("\n".join(chunks), encoding="utf-8")


def write_report(path: Path, summary: list[dict]) -> None:
    lines = [
        "# Reporte de benchmark",
        "",
        "Los valores son promedios de las mediciones ejecutadas por `scripts/benchmark.py`.",
        "",
        "| Algoritmo | Mediciones | Sec. ms | Par. ms | Speedup | Eficiencia | Desv. sec. | Desv. par. |",
        "|---|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for row in summary:
        lines.append(
            f'| {row["algorithm"]} | {row["runs"]} | {row["sequential_mean_ms"]:.3f} | '
            f'{row["parallel_mean_ms"]:.3f} | {row["speedup"]:.3f} | '
            f'{row["efficiency"]:.3f} | {row["sequential_stddev_ms"]:.3f} | '
            f'{row["parallel_stddev_ms"]:.3f} |'
        )
    lines += [
        "",
        "Fórmulas: `speedup = secuencial / paralelo`; `eficiencia = speedup / workers`.",
    ]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    args = parse_args()
    if args.runs < 10:
        print("Advertencia: la bitácora solicita mínimo 10 mediciones.", file=sys.stderr)
    if args.workers <= 0:
        raise SystemExit("--workers debe ser positivo")

    binary = Path(args.binary).expanduser().resolve()
    if not binary.exists():
        raise SystemExit(f"No existe el ejecutable: {binary}")
    output_dir = Path(args.output_dir).expanduser().resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    rows: list[dict] = []
    console: list[str] = []
    for run in range(1, args.runs + 1):
        for mode in ("sequential", "parallel"):
            parsed, raw = execute(binary, mode, args.workers)
            console.append(f"===== run {run} {mode} =====\n{raw}")
            for algorithm in ALGORITHMS:
                result = parsed[algorithm]
                # `execute()` devuelve los datos agrupados por etiqueta, por
                # lo que el nombre del algoritmo debe agregarse explícitamente
                # antes de enviarlo a `write_summary()`.
                rows.append({"run": run, "algorithm": algorithm, **result})
            print(f"medición {run}/{args.runs}: {mode}")

    write_measurements(output_dir / "measurements.csv", rows)
    summary = write_summary(output_dir / "summary.csv", rows, args.workers)
    write_svg(output_dir / "speedup.svg", summary, "speedup", "Speedup por algoritmo", "speedup")
    write_svg(output_dir / "efficiency.svg", summary, "efficiency", "Eficiencia por algoritmo", "eficiencia", 1.0)
    write_report(output_dir / "report.md", summary)
    (output_dir / "console.txt").write_text("\n".join(console), encoding="utf-8")
    print(f"Resultados escritos en {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
