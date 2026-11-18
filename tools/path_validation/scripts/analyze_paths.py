#!/usr/bin/env python3
"""Run the C++ path processor and analyze real captured trajectories."""

from __future__ import annotations

import argparse
import math
import re
import subprocess
from dataclasses import dataclass, replace
from pathlib import Path
from typing import Iterable

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.lines import Line2D
from matplotlib.ticker import FuncFormatter

plt.rcParams.update({"font.size": 10, "axes.titlesize": 11, "axes.labelsize": 10, "legend.fontsize": 8})


DOUGLAS_PEUCKER_VALUES_PX = (2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 16.0)
SMOOTHING_RADIUS_VALUES = (0, 1, 2, 3, 4, 5)
CATMULL_ALPHA_VALUES = (0.0, 0.25, 0.5, 0.75, 1.0)
RESAMPLE_SPACING_VALUES_PX = (2.0, 3.0, 5.0, 7.0, 10.0)
MAX_ANCHOR_VALUES = (12, 16, 20, 28, 36, 48)
SAMPLE_SPACING_VALUES_PX = (2.0, 4.0, 6.0, 8.0)

CURRENT_SIMPLIFY_TOLERANCE_PX = 8.0
CURRENT_SMOOTHING_RADIUS = 2
CURRENT_CATMULL_ALPHA = 0.5

RESULT_COLUMN_NAMES = {
    "trace": "identificador",
    "title": "titulo",
    "type": "tipo",
    "trial": "prueba",
    "original_file": "archivo_original",
    "input_path": "ruta_entrada",
    "simplify_tolerance_px": "tolerancia_simplificacion_px",
    "simplify_tolerance_m": "tolerancia_simplificacion_m",
    "points_before_simplification": "puntos_antes_simplificacion",
    "points_after_simplification": "puntos_despues_simplificacion",
    "final_anchor_count": "numero_puntos_anclaje",
    "reduction_percent": "reduccion_puntos_porcentaje",
    "reference_length_m": "longitud_referencia_m",
    "simplified_length_m": "longitud_simplificada_m",
    "spline_length_m": "longitud_trayectoria_m",
    "symmetric_rmse_m": "error_rmse_simetrico_m",
    "symmetric_max_deviation_m": "desviacion_maxima_simetrica_m",
    "smoothing_window_radius": "radio_ventana_suavizado",
    "smoothed_length_m": "longitud_suavizada_m",
    "orientation_variation_rad": "irregularidad_rad",
    "orientation_variation_rad_per_m": "irregularidad_rad_por_m",
    "catmull_alpha": "parametro_alpha",
    "anchor_count": "numero_puntos_anclaje",
    "anchor_polyline_rmse_m": "error_rmse_polilinea_anclajes_m",
    "anchor_polyline_max_deviation_m": "desviacion_maxima_polilinea_anclajes_m",
    "resample_spacing_px": "separacion_remuestreo_px",
    "resample_spacing_m": "separacion_remuestreo_m",
    "resampled_point_count": "numero_puntos_remuestreados",
    "spline_point_count": "numero_muestras_spline",
    "resampled_orientation_variation_rad": "irregularidad_remuestreo_rad",
    "resampled_orientation_variation_rad_per_m": "irregularidad_remuestreo_rad_por_m",
    "max_anchor_points": "maximo_puntos_anclaje",
    "limit_was_active": "limite_puntos_anclaje_activo",
    "sample_spacing_px": "separacion_muestreo_px",
    "sample_spacing_m": "separacion_muestreo_m",
}


@dataclass(frozen=True)
class ProcessingParameters:
    resample_spacing_m: float
    smoothing_window_radius: int
    simplify_tolerance_m: float
    max_anchor_points: int
    catmull_alpha: float
    sample_spacing_m: float


@dataclass(frozen=True)
class TraceContext:
    name: str
    title: str
    original_file: str
    trace_type: str | None
    trial: int | None
    input_path: Path
    metadata: dict[str, float]
    meters_per_pixel: float
    parameters: ProcessingParameters


def parse_arguments() -> argparse.Namespace:
    script_path = Path(__file__).resolve()
    repository_root = script_path.parents[3]
    validation_root = script_path.parents[1]
    parser = argparse.ArgumentParser(
        description="Procesa capturas reales y genera métricas y figuras de validación."
    )
    parser.add_argument(
        "--input",
        type=Path,
        nargs="*",
        default=None,
        help=(
            "Uno o varios CSV path_*_raw.csv. Si se omite, se analizan todos los "
            "raw de ~/.local/robogait/path_validation/raw/."
        ),
    )
    parser.add_argument(
        "--processor",
        type=Path,
        default=repository_root / "build/path_validation/path_validation_processor",
        help="Ruta al ejecutable C++ path_validation_processor.",
    )
    parser.add_argument(
        "--processed-dir",
        type=Path,
        default=validation_root / "data/processed",
        help="Directorio de salidas intermedias del procesador C++.",
    )
    parser.add_argument(
        "--results-dir",
        type=Path,
        default=validation_root / "results",
        help="Directorio raíz para CSV y figuras.",
    )
    parser.add_argument(
        "--trace-index",
        type=Path,
        default=None,
        help=(
            "CSV opcional archivo,tipo,prueba,titulo. Si se omite, se utiliza "
            "tools/path_validation/traces.csv cuando existe."
        ),
    )
    return parser.parse_args()


def read_points(path: Path) -> np.ndarray:
    frame = pd.read_csv(path)
    required_columns = ["index", "x_m", "y_m"]
    if list(frame.columns) != required_columns:
        raise ValueError(f"Cabecera no válida en {path}; se esperaba {required_columns}")
    points = frame[["x_m", "y_m"]].to_numpy(dtype=float)
    if len(points) < 2 or not np.isfinite(points).all():
        raise ValueError(f"El trazado {path} no contiene al menos dos puntos finitos")
    return points


def metadata_path_for(raw_path: Path) -> Path:
    suffix = "_raw.csv"
    if not raw_path.name.endswith(suffix):
        raise ValueError(f"El archivo debe llamarse path_*_raw.csv: {raw_path}")
    return raw_path.with_name(raw_path.name[: -len(suffix)] + "_metadata.csv")


def read_metadata(path: Path) -> dict[str, float]:
    if not path.is_file():
        raise FileNotFoundError(f"No se encontró el CSV de metadatos asociado: {path}")
    frame = pd.read_csv(path)
    if list(frame.columns) != ["parameter", "value"]:
        raise ValueError(f"Cabecera de metadatos no válida en {path}")
    metadata: dict[str, float] = {}
    for row in frame.itertuples(index=False):
        metadata[str(row.parameter)] = float(row.value)
    return metadata


def require_metadata(metadata: dict[str, float], keys: Iterable[str], path: Path) -> None:
    missing = [key for key in keys if key not in metadata]
    if missing:
        raise ValueError(f"Faltan parámetros en {path}: {', '.join(missing)}")


def read_trace_index(path: Path) -> dict[str, tuple[str, int, str]]:
    frame = pd.read_csv(path, dtype={"archivo": "string", "tipo": "string", "titulo": "string"})
    required_columns = ["archivo", "tipo", "prueba", "titulo"]
    if list(frame.columns) != required_columns:
        raise ValueError(f"Cabecera no válida en {path}; se esperaba {required_columns}")

    entries: dict[str, tuple[str, int, str]] = {}
    labels: set[str] = set()
    titles: set[str] = set()
    for row_number, row in enumerate(frame.itertuples(index=False), start=2):
        if pd.isna(row.archivo) or pd.isna(row.tipo) or pd.isna(row.prueba) or pd.isna(row.titulo):
            raise ValueError(f"Valor vacío en {path}, línea {row_number}")
        file_name = str(row.archivo).strip()
        trace_type = str(row.tipo).strip()
        title = str(row.titulo).strip()
        try:
            trial_value = float(row.prueba)
        except (TypeError, ValueError) as error:
            raise ValueError(
                f"Número de repetición no válido en {path}, línea {row_number}"
            ) from error
        if Path(file_name).name != file_name or not file_name.endswith("_raw.csv"):
            raise ValueError(f"Nombre de archivo no válido en {path}, línea {row_number}: {file_name}")
        if not re.fullmatch(r"[A-Za-z0-9_-]+", trace_type):
            raise ValueError(f"Tipo de trayectoria no válido en {path}, línea {row_number}: {trace_type}")
        if not title:
            raise ValueError(f"Título vacío en {path}, línea {row_number}")
        if not math.isfinite(trial_value) or not trial_value.is_integer() or trial_value < 1:
            raise ValueError(f"Número de repetición no válido en {path}, línea {row_number}")
        trial = int(trial_value)
        label = f"{trace_type}_{trial:02d}"
        if file_name in entries:
            raise ValueError(f"Archivo duplicado en {path}: {file_name}")
        if label in labels:
            raise ValueError(f"Identificador duplicado en {path}: {label}")
        if title in titles:
            raise ValueError(f"Título duplicado en {path}: {title}")
        entries[file_name] = (trace_type, trial, title)
        labels.add(label)
        titles.add(title)
    return entries


def build_trace_context(
    raw_path: Path,
    trace_index: dict[str, tuple[str, int, str]],
    fallback_number: int,
) -> TraceContext:
    raw_path = raw_path.resolve()
    if not raw_path.is_file():
        raise FileNotFoundError(f"No existe el trazado: {raw_path}")
    metadata_path = metadata_path_for(raw_path)
    metadata = read_metadata(metadata_path)
    required = (
        "resample_spacing_px",
        "resample_spacing_m",
        "smoothing_window_radius",
        "simplify_tolerance_px",
        "simplify_tolerance_m",
        "max_anchor_points",
        "catmull_alpha",
        "sample_spacing_px",
        "sample_spacing_m",
    )
    require_metadata(metadata, required, metadata_path)
    meters_per_pixel = metadata["resample_spacing_m"] / metadata["resample_spacing_px"]
    if not math.isfinite(meters_per_pixel) or meters_per_pixel <= 0.0:
        raise ValueError(f"La conversión píxel-metro no es válida en {metadata_path}")
    parameters = ProcessingParameters(
        resample_spacing_m=metadata["resample_spacing_m"],
        smoothing_window_radius=int(metadata["smoothing_window_radius"]),
        simplify_tolerance_m=metadata["simplify_tolerance_m"],
        max_anchor_points=int(metadata["max_anchor_points"]),
        catmull_alpha=metadata["catmull_alpha"],
        sample_spacing_m=metadata["sample_spacing_m"],
    )
    original_file = raw_path.name
    indexed_identity = trace_index.get(original_file)
    if indexed_identity is None:
        name = f"trayectoria_{fallback_number:02d}"
        title = f"Trayectoria {fallback_number}"
        trace_type = None
        trial = None
    else:
        trace_type, trial, title = indexed_identity
        name = f"{trace_type}_{trial:02d}"
    return TraceContext(
        name,
        title,
        original_file,
        trace_type,
        trial,
        raw_path,
        metadata,
        meters_per_pixel,
        parameters,
    )


def result_identity(trace: TraceContext) -> dict[str, object]:
    return {
        "trace": trace.name,
        "title": trace.title,
        "type": trace.trace_type,
        "trial": trace.trial,
        "original_file": trace.original_file,
    }


def run_processor(
    processor: Path,
    trace: TraceContext,
    parameters: ProcessingParameters,
    output_directory: Path,
) -> dict[str, np.ndarray]:
    output_directory.mkdir(parents=True, exist_ok=True)
    command = [
        str(processor),
        "--input",
        str(trace.input_path),
        "--output-dir",
        str(output_directory),
        "--resample-spacing",
        f"{parameters.resample_spacing_m:.17g}",
        "--smoothing-window-radius",
        str(parameters.smoothing_window_radius),
        "--simplify-tolerance",
        f"{parameters.simplify_tolerance_m:.17g}",
        "--max-anchor-points",
        str(parameters.max_anchor_points),
        "--catmull-alpha",
        f"{parameters.catmull_alpha:.17g}",
        "--sample-spacing",
        f"{parameters.sample_spacing_m:.17g}",
    ]
    completed = subprocess.run(command, text=True, capture_output=True, check=False)
    if completed.returncode != 0:
        details = completed.stderr.strip() or completed.stdout.strip()
        raise RuntimeError(f"Falló el procesador para {trace.name}: {details}")
    stage_names = (
        "raw",
        "deduplicated",
        "resampled",
        "smoothed",
        "simplified",
        "anchors",
        "spline",
    )
    return {name: read_points(output_directory / f"{name}.csv") for name in stage_names}


def polyline_length(points: np.ndarray) -> float:
    return float(np.linalg.norm(np.diff(points, axis=0), axis=1).sum())


def densify_polyline(points: np.ndarray, spacing: float) -> np.ndarray:
    dense_parts: list[np.ndarray] = []
    for start, end in zip(points[:-1], points[1:]):
        length = float(np.linalg.norm(end - start))
        subdivisions = max(1, int(math.ceil(length / spacing)))
        fractions = np.linspace(0.0, 1.0, subdivisions, endpoint=False)[:, None]
        dense_parts.append(start + fractions * (end - start))
    dense_parts.append(points[-1:])
    return np.vstack(dense_parts)


def nearest_point_distances(source: np.ndarray, target: np.ndarray) -> np.ndarray:
    distances = np.empty(len(source), dtype=float)
    chunk_size = 2048
    for start in range(0, len(source), chunk_size):
        chunk = source[start : start + chunk_size]
        squared = np.sum((chunk[:, None, :] - target[None, :, :]) ** 2, axis=2)
        distances[start : start + len(chunk)] = np.sqrt(np.min(squared, axis=1))
    return distances


def symmetric_geometric_error(
    reference: np.ndarray, candidate: np.ndarray, dense_spacing: float
) -> tuple[float, float]:
    dense_reference = densify_polyline(reference, dense_spacing)
    dense_candidate = densify_polyline(candidate, dense_spacing)
    forward = nearest_point_distances(dense_reference, dense_candidate)
    reverse = nearest_point_distances(dense_candidate, dense_reference)
    squared_sum = np.sum(forward**2) + np.sum(reverse**2)
    sample_count = len(forward) + len(reverse)
    return float(np.sqrt(squared_sum / sample_count)), float(max(forward.max(), reverse.max()))


def orientation_variation(points: np.ndarray) -> tuple[float, float]:
    segments = np.diff(points, axis=0)
    lengths = np.linalg.norm(segments, axis=1)
    segments = segments[lengths > 1e-12]
    if len(segments) < 2:
        return 0.0, 0.0
    angles = np.unwrap(np.arctan2(segments[:, 1], segments[:, 0]))
    total_variation = float(np.abs(np.diff(angles)).sum())
    length = polyline_length(points)
    return total_variation, total_variation / length if length > 0.0 else 0.0


def format_decimal(value: float) -> str:
    return f"{value:g}".replace(".", ",")


def write_results_csv(frame: pd.DataFrame, path: Path) -> None:
    visible_frame = frame.rename(columns=RESULT_COLUMN_NAMES).copy()
    for column in visible_frame.select_dtypes(include=["bool"]).columns:
        visible_frame[column] = visible_frame[column].map({True: "sí", False: "no"})
    visible_frame.to_csv(path, index=False)


def use_decimal_comma(ax: plt.Axes) -> None:
    formatter = FuncFormatter(lambda value, _: format_decimal(value))
    ax.xaxis.set_major_formatter(formatter)
    ax.yaxis.set_major_formatter(formatter)


def select_representative_trace(
    traces: list[TraceContext], preferred_type: str, purpose: str
) -> TraceContext:
    matching = [trace for trace in traces if trace.trace_type == preferred_type]
    if matching:
        return min(matching, key=lambda trace: trace.trial or 0)
    selected = traces[0]
    print(
        f"Aviso: no hay trazados de tipo '{preferred_type}' para {purpose}; "
        f"se utilizará '{selected.title}'."
    )
    return selected


def plot_path(ax: plt.Axes, points: np.ndarray, label: str, **kwargs: object) -> None:
    ax.plot(points[:, 0], points[:, 1], label=label, **kwargs)
    ax.set_xlabel("Coordenada x [m]")
    ax.set_ylabel("Coordenada y [m]")
    ax.set_aspect("equal", adjustable="datalim")
    ax.grid(True, alpha=0.25)
    use_decimal_comma(ax)


def save_figure(figure: plt.Figure, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    base_path = path.with_suffix("")
    figure.savefig(base_path.with_suffix(".pdf"), bbox_inches="tight")
    figure.savefig(base_path.with_suffix(".png"), dpi=220, bbox_inches="tight")
    plt.close(figure)


def create_documentation_raw_traces(
    traces: list[TraceContext], output_directory: Path
) -> None:
    column_definitions = (
        ("trayectoria_s", "Trayectoria en S"),
        ("giro_pronunciado", "Giro pronunciado"),
        ("trayectoria_sinuosa", "Trayectoria sinuosa"),
        ("trayectoria_casi_recta", "Trayectoria casi recta"),
    )
    expected_identities = {
        (trace_type, trial)
        for trace_type, _ in column_definitions
        for trial in range(1, 4)
    }
    indexed_traces = {
        (trace.trace_type, trace.trial): trace
        for trace in traces
        if trace.trace_type is not None and trace.trial is not None
    }
    if len(traces) != 12 or set(indexed_traces) != expected_identities:
        print(
            "Aviso: se omite la figura de los trazados manuales porque requiere "
            "las 12 capturas indexadas (cuatro tipos y tres pruebas por tipo)."
        )
        return

    figure, axes = plt.subplots(3, 4, figsize=(16.0, 10.0))
    for column, (trace_type, column_label) in enumerate(column_definitions):
        axes[0, column].set_title(column_label)
        for row, trial in enumerate(range(1, 4)):
            ax = axes[row, column]
            trace = indexed_traces[(trace_type, trial)]
            points = read_points(trace.input_path)
            ax.plot(points[:, 0], points[:, 1], color="tab:blue", linewidth=1.4)
            ax.set_xlabel("Coordenada x [m]")
            ax.set_ylabel("Coordenada y [m]")
            ax.axis("equal")
            ax.grid(True, alpha=0.25)
            use_decimal_comma(ax)

    figure.suptitle("Trazados manuales utilizados en el análisis")
    figure.subplots_adjust(left=0.08, right=0.985, bottom=0.07, top=0.91, hspace=0.38, wspace=0.32)
    for row, trial in enumerate(range(1, 4)):
        position = axes[row, 0].get_position()
        figure.text(
            0.018,
            (position.y0 + position.y1) / 2.0,
            f"Prueba {trial}",
            rotation=90,
            ha="center",
            va="center",
            fontsize=11,
            fontweight="bold",
        )

    save_figure(figure, output_directory / "trazados_manuales_utilizados.pdf")


def create_processing_panels(
    stages: dict[str, np.ndarray], output_path: Path, subtitle: str | None = None
) -> None:
    panels = (
        ("raw", "Trazado original"),
        ("resampled", "Remuestreo por longitud de arco"),
        ("smoothed", "Suavizado mediante media móvil"),
        ("simplified", "Simplificación Douglas-Peucker"),
        ("anchors", "Puntos de anclaje"),
        ("spline", "Trayectoria final mediante spline Bézier"),
    )
    figure, axes = plt.subplots(2, 3, figsize=(13.5, 8.0), constrained_layout=True)
    raw = stages["raw"]
    for ax, (stage_name, title) in zip(axes.flat, panels):
        plot_path(ax, raw, "Original", color="0.78", linewidth=1.0)
        points = stages[stage_name]
        if stage_name in {"simplified", "anchors"}:
            plot_path(
                ax,
                points,
                title,
                color="tab:red" if stage_name == "anchors" else "tab:blue",
                marker="o",
                markersize=4.0 if stage_name == "anchors" else 3.0,
                linewidth=1.2,
            )
        else:
            plot_path(ax, points, title, color="tab:blue", linewidth=1.6)
        ax.set_title(title)
    general_title = "Etapas del procesamiento de una trayectoria manual"
    if subtitle:
        general_title += f"\n{subtitle}"
    figure.suptitle(general_title)
    layout_engine = figure.get_layout_engine()
    if layout_engine is not None:
        layout_engine.set(rect=(0.0, 0.025, 1.0, 0.975))
    figure.legend(
        handles=[Line2D([0], [0], color="0.72", linewidth=1.5, label="Trazado original de referencia")],
        loc="lower center",
        bbox_to_anchor=(0.5, -0.025),
        frameon=False,
    )
    save_figure(figure, output_path)


def create_processing_figure(
    trace: TraceContext, stages: dict[str, np.ndarray], figures_directory: Path
) -> None:
    create_processing_panels(
        stages,
        figures_directory / f"processing_stages_{trace.name}.pdf",
        subtitle=trace.title,
    )


def create_metric_plot(
    frame: pd.DataFrame,
    x_column: str,
    y_columns: tuple[tuple[str, str], ...],
    current_value: float,
    output_path: Path,
    title: str,
    x_label: str,
) -> None:
    figure, axes = plt.subplots(1, len(y_columns), figsize=(6.0 * len(y_columns), 4.2), squeeze=False)
    for ax, (column, label) in zip(axes.flat, y_columns):
        for trace_title, group in frame.groupby("title"):
            ordered = group.sort_values(x_column)
            ax.plot(ordered[x_column], ordered[column], marker="o", label=trace_title)
        ax.axvline(
            current_value,
            color="tab:red",
            linestyle="--",
            label=f"Configuración seleccionada: {format_decimal(current_value)}",
        )
        ax.set_xlabel(x_label)
        ax.set_ylabel(label)
        ax.grid(True, alpha=0.25)
        use_decimal_comma(ax)
        ax.legend(fontsize="small")
    figure.suptitle(title)
    save_figure(figure, output_path)


def create_overlay_figure(
    runs: dict[str, list[tuple[float, dict[str, np.ndarray]]]],
    traces_by_name: dict[str, TraceContext],
    stage_name: str,
    current_value: float,
    output_path: Path,
    title: str,
    value_label: str,
    highlight_values: tuple[float, ...] | None = None,
) -> None:
    column_count = min(3, len(runs))
    row_count = math.ceil(len(runs) / column_count)
    figure, axes = plt.subplots(
        row_count,
        column_count,
        figsize=(5.4 * column_count, 4.5 * row_count),
        squeeze=False,
        constrained_layout=True,
    )
    colors = plt.cm.viridis(np.linspace(0.05, 0.95, max(len(values) for values in runs.values())))
    for ax, (trace_name, values) in zip(axes.flat, runs.items()):
        for color, (value, stages) in zip(colors, values):
            is_current = math.isclose(value, current_value)
            is_highlighted = is_current or (
                highlight_values is not None
                and any(math.isclose(value, highlighted) for highlighted in highlight_values)
            )
            plot_path(
                ax,
                stages[stage_name],
                f"{value_label}: {format_decimal(value)}" + (" (seleccionada)" if is_current else ""),
                color="tab:red" if is_current else color,
                linewidth=2.4 if is_highlighted else 1.1,
                alpha=1.0 if is_highlighted else 0.65,
            )
        ax.set_title(traces_by_name[trace_name].title)
        ax.legend(fontsize="x-small")
    for ax in axes.flat[len(runs) :]:
        ax.set_visible(False)
    figure.suptitle(title)
    save_figure(figure, output_path)


def study_douglas_peucker(
    processor: Path,
    traces: list[TraceContext],
    processed_directory: Path,
    csv_directory: Path,
    figures_directory: Path,
) -> tuple[pd.DataFrame, dict[str, list[tuple[float, dict[str, np.ndarray]]]]]:
    rows: list[dict[str, object]] = []
    overlay_runs: dict[str, list[tuple[float, dict[str, np.ndarray]]]] = {}
    for trace in traces:
        overlay_runs[trace.name] = []
        dense_spacing = max(min(trace.parameters.resample_spacing_m, trace.parameters.sample_spacing_m) / 4.0, 1e-6)
        for tolerance_px in DOUGLAS_PEUCKER_VALUES_PX:
            tolerance_m = tolerance_px * trace.meters_per_pixel
            parameters = replace(trace.parameters, simplify_tolerance_m=tolerance_m)
            stages = run_processor(
                processor,
                trace,
                parameters,
                processed_directory / trace.name / "douglas_peucker" / f"px_{tolerance_px:g}",
            )
            rmse, maximum = symmetric_geometric_error(stages["smoothed"], stages["simplified"], dense_spacing)
            before_count = len(stages["smoothed"])
            after_count = len(stages["simplified"])
            rows.append(
                {
                    **result_identity(trace),
                    "simplify_tolerance_px": tolerance_px,
                    "simplify_tolerance_m": tolerance_m,
                    "points_before_simplification": before_count,
                    "points_after_simplification": after_count,
                    "final_anchor_count": len(stages["anchors"]),
                    "reduction_percent": 100.0 * (before_count - after_count) / before_count,
                    "reference_length_m": polyline_length(stages["smoothed"]),
                    "simplified_length_m": polyline_length(stages["simplified"]),
                    "spline_length_m": polyline_length(stages["spline"]),
                    "symmetric_rmse_m": rmse,
                    "symmetric_max_deviation_m": maximum,
                }
            )
            overlay_runs[trace.name].append((tolerance_px, stages))
    frame = pd.DataFrame(rows)
    write_results_csv(frame, csv_directory / "douglas_peucker_metrics.csv")
    create_metric_plot(
        frame,
        "simplify_tolerance_px",
        (("final_anchor_count", "Número de puntos de anclaje"), ("reduction_percent", "Reducción de puntos [%]")),
        CURRENT_SIMPLIFY_TOLERANCE_PX,
        figures_directory / "douglas_peucker_anchor_count.pdf",
        "Reducción de puntos con Douglas-Peucker",
        "Tolerancia de simplificación [px]",
    )
    create_metric_plot(
        frame,
        "simplify_tolerance_px",
        (("symmetric_rmse_m", "RMSE simétrico [m]"), ("symmetric_max_deviation_m", "Desviación máxima [m]")),
        CURRENT_SIMPLIFY_TOLERANCE_PX,
        figures_directory / "douglas_peucker_geometric_error.pdf",
        "Error geométrico de Douglas-Peucker",
        "Tolerancia de simplificación [px]",
    )
    create_overlay_figure(
        overlay_runs,
        {trace.name: trace for trace in traces},
        "spline",
        CURRENT_SIMPLIFY_TOLERANCE_PX,
        figures_directory / "douglas_peucker_overlay.pdf",
        "Comparación de tolerancias Douglas-Peucker",
        "Tolerancia [px]",
    )
    return frame, overlay_runs


def study_smoothing(
    processor: Path,
    traces: list[TraceContext],
    processed_directory: Path,
    csv_directory: Path,
    figures_directory: Path,
) -> tuple[pd.DataFrame, dict[str, list[tuple[float, dict[str, np.ndarray]]]]]:
    rows: list[dict[str, object]] = []
    overlay_runs: dict[str, list[tuple[float, dict[str, np.ndarray]]]] = {}
    for trace in traces:
        overlay_runs[trace.name] = []
        dense_spacing = max(trace.parameters.resample_spacing_m / 4.0, 1e-6)
        for radius in SMOOTHING_RADIUS_VALUES:
            parameters = replace(trace.parameters, smoothing_window_radius=radius)
            stages = run_processor(
                processor,
                trace,
                parameters,
                processed_directory / trace.name / "smoothing" / f"radius_{radius}",
            )
            rmse, maximum = symmetric_geometric_error(stages["resampled"], stages["smoothed"], dense_spacing)
            variation, normalized_variation = orientation_variation(stages["smoothed"])
            rows.append(
                {
                    **result_identity(trace),
                    "smoothing_window_radius": radius,
                    "smoothed_length_m": polyline_length(stages["smoothed"]),
                    "symmetric_rmse_m": rmse,
                    "symmetric_max_deviation_m": maximum,
                    "orientation_variation_rad": variation,
                    "orientation_variation_rad_per_m": normalized_variation,
                }
            )
            overlay_runs[trace.name].append((float(radius), stages))
    frame = pd.DataFrame(rows)
    write_results_csv(frame, csv_directory / "smoothing_metrics.csv")
    create_metric_plot(
        frame,
        "smoothing_window_radius",
        (("symmetric_rmse_m", "RMSE simétrico [m]"), ("symmetric_max_deviation_m", "Desviación máxima [m]")),
        CURRENT_SMOOTHING_RADIUS,
        figures_directory / "smoothing_error.pdf",
        "Desviación introducida por la media móvil",
        "Radio de la ventana de suavizado",
    )
    create_metric_plot(
        frame,
        "smoothing_window_radius",
        (("orientation_variation_rad", "Variación de orientación [rad]"), ("orientation_variation_rad_per_m", "Variación [rad/m]")),
        CURRENT_SMOOTHING_RADIUS,
        figures_directory / "smoothing_roughness.pdf",
        "Irregularidad tras el suavizado",
        "Radio de la ventana de suavizado",
    )
    create_overlay_figure(
        overlay_runs,
        {trace.name: trace for trace in traces},
        "smoothed",
        CURRENT_SMOOTHING_RADIUS,
        figures_directory / "smoothing_overlay.pdf",
        "Comparación de radios de media móvil",
        "Radio",
    )
    return frame, overlay_runs


def study_catmull_rom(
    processor: Path,
    traces: list[TraceContext],
    processed_directory: Path,
    csv_directory: Path,
    figures_directory: Path,
) -> tuple[pd.DataFrame, dict[str, list[tuple[float, dict[str, np.ndarray]]]]]:
    rows: list[dict[str, object]] = []
    overlay_runs: dict[str, list[tuple[float, dict[str, np.ndarray]]]] = {}
    for trace in traces:
        overlay_runs[trace.name] = []
        reference_anchors: np.ndarray | None = None
        dense_spacing = max(trace.parameters.sample_spacing_m / 4.0, 1e-6)
        for alpha in CATMULL_ALPHA_VALUES:
            parameters = replace(trace.parameters, catmull_alpha=alpha)
            stages = run_processor(
                processor,
                trace,
                parameters,
                processed_directory / trace.name / "catmull_rom" / f"alpha_{alpha:g}",
            )
            if reference_anchors is None:
                reference_anchors = stages["anchors"]
            elif reference_anchors.shape != stages["anchors"].shape or not np.allclose(
                reference_anchors, stages["anchors"], rtol=0.0, atol=1e-10
            ):
                raise RuntimeError(f"Los anchors cambiaron durante el barrido Catmull-Rom de {trace.name}")
            rmse, maximum = symmetric_geometric_error(stages["anchors"], stages["spline"], dense_spacing)
            variation, normalized_variation = orientation_variation(stages["spline"])
            rows.append(
                {
                    **result_identity(trace),
                    "catmull_alpha": alpha,
                    "anchor_count": len(stages["anchors"]),
                    "spline_length_m": polyline_length(stages["spline"]),
                    "anchor_polyline_rmse_m": rmse,
                    "anchor_polyline_max_deviation_m": maximum,
                    "orientation_variation_rad": variation,
                    "orientation_variation_rad_per_m": normalized_variation,
                }
            )
            overlay_runs[trace.name].append((alpha, stages))
    frame = pd.DataFrame(rows)
    write_results_csv(frame, csv_directory / "catmull_rom_metrics.csv")
    create_overlay_figure(
        overlay_runs,
        {trace.name: trace for trace in traces},
        "spline",
        CURRENT_CATMULL_ALPHA,
        figures_directory / "catmull_rom_comparison.pdf",
        "Parametrizaciones Catmull-Rom con puntos de anclaje idénticos",
        "Parámetro α",
        highlight_values=(0.0, 0.5, 1.0),
    )
    return frame, overlay_runs


def study_other_parameters(
    processor: Path,
    traces: list[TraceContext],
    processed_directory: Path,
    csv_directory: Path,
) -> None:
    resample_rows: list[dict[str, object]] = []
    anchor_rows: list[dict[str, object]] = []
    sample_rows: list[dict[str, object]] = []
    for trace in traces:
        for spacing_px in RESAMPLE_SPACING_VALUES_PX:
            parameters = replace(trace.parameters, resample_spacing_m=spacing_px * trace.meters_per_pixel)
            stages = run_processor(
                processor,
                trace,
                parameters,
                processed_directory / trace.name / "resample_spacing" / f"px_{spacing_px:g}",
            )
            variation, normalized_variation = orientation_variation(stages["resampled"])
            resample_rows.append(
                {
                    **result_identity(trace),
                    "resample_spacing_px": spacing_px,
                    "resample_spacing_m": parameters.resample_spacing_m,
                    "resampled_point_count": len(stages["resampled"]),
                    "final_anchor_count": len(stages["anchors"]),
                    "spline_point_count": len(stages["spline"]),
                    "spline_length_m": polyline_length(stages["spline"]),
                    "resampled_orientation_variation_rad": variation,
                    "resampled_orientation_variation_rad_per_m": normalized_variation,
                }
            )
        for maximum_anchors in MAX_ANCHOR_VALUES:
            parameters = replace(trace.parameters, max_anchor_points=maximum_anchors)
            stages = run_processor(
                processor,
                trace,
                parameters,
                processed_directory / trace.name / "max_anchor_points" / f"count_{maximum_anchors}",
            )
            simplified_count = len(stages["simplified"])
            anchor_rows.append(
                {
                    **result_identity(trace),
                    "max_anchor_points": maximum_anchors,
                    "points_after_simplification": simplified_count,
                    "final_anchor_count": len(stages["anchors"]),
                    "limit_was_active": simplified_count > maximum_anchors,
                    "spline_length_m": polyline_length(stages["spline"]),
                }
            )
        for spacing_px in SAMPLE_SPACING_VALUES_PX:
            parameters = replace(trace.parameters, sample_spacing_m=spacing_px * trace.meters_per_pixel)
            stages = run_processor(
                processor,
                trace,
                parameters,
                processed_directory / trace.name / "sample_spacing" / f"px_{spacing_px:g}",
            )
            sample_rows.append(
                {
                    **result_identity(trace),
                    "sample_spacing_px": spacing_px,
                    "sample_spacing_m": parameters.sample_spacing_m,
                    "spline_point_count": len(stages["spline"]),
                    "spline_length_m": polyline_length(stages["spline"]),
                }
            )
    write_results_csv(pd.DataFrame(resample_rows), csv_directory / "resample_spacing_metrics.csv")
    write_results_csv(pd.DataFrame(anchor_rows), csv_directory / "max_anchor_points_metrics.csv")
    write_results_csv(pd.DataFrame(sample_rows), csv_directory / "sample_spacing_metrics.csv")


def plot_mean_with_deviation(
    ax: plt.Axes,
    frame: pd.DataFrame,
    x_column: str,
    y_column: str,
    selected_value: float,
    title: str,
    x_label: str,
    y_label: str,
    selected_label: str,
) -> None:
    grouped = frame.groupby(x_column)[y_column].agg(["mean", "std"]).reset_index()
    deviations = grouped["std"].fillna(0.0)
    ax.errorbar(
        grouped[x_column],
        grouped["mean"],
        yerr=deviations,
        marker="o",
        capsize=4,
        color="tab:blue",
        label="Media ± desviación estándar",
    )
    ax.axvline(selected_value, color="tab:red", linestyle="--", label=selected_label)
    ax.set_title(title)
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    ax.grid(True, alpha=0.25)
    use_decimal_comma(ax)
    ax.legend()


def create_documentation_douglas_peucker_quantitative(
    frame: pd.DataFrame, output_directory: Path
) -> None:
    figure, axes = plt.subplots(1, 2, figsize=(11.5, 4.4), constrained_layout=True)
    selected_label = "Configuración seleccionada: 8 px"
    plot_mean_with_deviation(
        axes[0],
        frame,
        "simplify_tolerance_px",
        "final_anchor_count",
        CURRENT_SIMPLIFY_TOLERANCE_PX,
        "Número medio de puntos de anclaje",
        "Tolerancia de simplificación [px]",
        "Número de puntos de anclaje",
        selected_label,
    )
    plot_mean_with_deviation(
        axes[1],
        frame,
        "simplify_tolerance_px",
        "symmetric_rmse_m",
        CURRENT_SIMPLIFY_TOLERANCE_PX,
        "Error geométrico medio",
        "Tolerancia de simplificación [px]",
        "RMSE geométrico [m]",
        selected_label,
    )
    figure.suptitle("Efecto cuantitativo de la simplificación Douglas-Peucker")
    save_figure(figure, output_directory / "douglas_peucker_cuantitativo.pdf")


def create_documentation_douglas_peucker_visual(
    runs: list[tuple[float, dict[str, np.ndarray]]],
    output_directory: Path,
) -> None:
    selected_values = (4.0, 8.0, 16.0)
    runs_by_value = {value: stages for value, stages in runs}
    figure, axes = plt.subplots(1, 3, figsize=(13.0, 4.2))
    for ax, value in zip(axes, selected_values):
        stages = runs_by_value[value]
        plot_path(ax, stages["spline"], "Spline Bézier", color="tab:blue", linewidth=1.8)
        ax.scatter(
            stages["anchors"][:, 0],
            stages["anchors"][:, 1],
            label="Puntos de anclaje",
            color="tab:red",
            s=22,
            zorder=3,
        )
        suffix = " (seleccionada)" if math.isclose(value, CURRENT_SIMPLIFY_TOLERANCE_PX) else ""
        ax.set_title(f"Tolerancia: {format_decimal(value)} px{suffix}")
    handles, labels = axes[0].get_legend_handles_labels()
    figure.legend(
        handles,
        labels,
        loc="lower center",
        bbox_to_anchor=(0.5, 0.015),
        ncol=2,
        frameon=False,
    )
    figure.suptitle("Efecto visual de Douglas-Peucker")
    figure.subplots_adjust(left=0.06, right=0.985, bottom=0.22, top=0.78, wspace=0.22)
    save_figure(figure, output_directory / "douglas_peucker_visual.pdf")


def create_documentation_smoothing(
    frame: pd.DataFrame, output_directory: Path
) -> None:
    documented = frame.copy()
    reference = (
        documented[documented["smoothing_window_radius"] == 0]
        .set_index("trace")["orientation_variation_rad_per_m"]
        .to_dict()
    )
    documented["roughness_reduction_percent"] = [
        100.0 * (reference[row.trace] - row.orientation_variation_rad_per_m) / reference[row.trace]
        if reference[row.trace] > 1e-12
        else 0.0
        for row in documented.itertuples()
    ]
    figure, axes = plt.subplots(1, 2, figsize=(11.5, 4.4), constrained_layout=True)
    selected_label = "Configuración seleccionada: 2"
    plot_mean_with_deviation(
        axes[0],
        documented,
        "smoothing_window_radius",
        "roughness_reduction_percent",
        CURRENT_SMOOTHING_RADIUS,
        "Reducción de la irregularidad [%]",
        "Radio de la ventana de suavizado",
        "Reducción de la irregularidad [%]",
        selected_label,
    )
    plot_mean_with_deviation(
        axes[1],
        documented,
        "smoothing_window_radius",
        "symmetric_rmse_m",
        CURRENT_SMOOTHING_RADIUS,
        "Error geométrico medio [m]",
        "Radio de la ventana de suavizado",
        "RMSE geométrico [m]",
        selected_label,
    )
    figure.suptitle("Efecto cuantitativo del suavizado mediante media móvil")
    save_figure(figure, output_directory / "suavizado_cuantitativo.pdf")


def create_documentation_catmull_rom(
    runs: list[tuple[float, dict[str, np.ndarray]]],
    output_directory: Path,
) -> None:
    selected_values = (0.0, 0.5, 1.0)
    runs_by_value = {value: stages for value, stages in runs}
    figure, axes = plt.subplots(1, 3, figsize=(13.0, 4.2))
    for ax, value in zip(axes, selected_values):
        stages = runs_by_value[value]
        plot_path(ax, stages["spline"], "Spline Bézier", color="tab:blue", linewidth=1.8)
        ax.scatter(
            stages["anchors"][:, 0],
            stages["anchors"][:, 1],
            label="Puntos de anclaje",
            color="tab:red",
            s=22,
            zorder=3,
        )
        suffix = " (seleccionado)" if math.isclose(value, CURRENT_CATMULL_ALPHA) else ""
        ax.set_title(f"α = {format_decimal(value)}{suffix}")
    handles, labels = axes[0].get_legend_handles_labels()
    figure.legend(
        handles,
        labels,
        loc="lower center",
        bbox_to_anchor=(0.5, 0.015),
        ncol=2,
        frameon=False,
    )
    figure.suptitle("Influencia de la parametrización de Catmull–Rom sobre una trayectoria sinuosa")
    figure.subplots_adjust(left=0.06, right=0.985, bottom=0.22, top=0.78, wspace=0.22)
    save_figure(figure, output_directory / "catmull_rom_documentacion.pdf")


def create_documentation_figures(
    traces: list[TraceContext],
    baseline_runs: dict[str, dict[str, np.ndarray]],
    douglas_peucker_frame: pd.DataFrame,
    douglas_peucker_runs: dict[str, list[tuple[float, dict[str, np.ndarray]]]],
    smoothing_frame: pd.DataFrame,
    catmull_rom_runs: dict[str, list[tuple[float, dict[str, np.ndarray]]]],
    output_directory: Path,
) -> None:
    output_directory.mkdir(parents=True, exist_ok=True)
    processing_trace = select_representative_trace(
        traces, "trayectoria_s", "las etapas de procesamiento"
    )
    turn_trace = select_representative_trace(
        traces, "giro_pronunciado", "la comparación visual Douglas-Peucker"
    )
    sinuous_trace = select_representative_trace(
        traces, "trayectoria_sinuosa", "la comparación Catmull-Rom"
    )

    create_processing_panels(
        baseline_runs[processing_trace.name],
        output_directory / "etapas_procesamiento.pdf",
    )
    create_documentation_douglas_peucker_quantitative(
        douglas_peucker_frame, output_directory
    )
    create_documentation_douglas_peucker_visual(
        douglas_peucker_runs[turn_trace.name], output_directory
    )
    create_documentation_smoothing(smoothing_frame, output_directory)
    create_documentation_catmull_rom(
        catmull_rom_runs[sinuous_trace.name], output_directory
    )


def main() -> int:
    arguments = parse_arguments()
    processor = arguments.processor.resolve()
    if not processor.is_file():
        raise FileNotFoundError(
            f"No se encontró el procesador C++: {processor}. Compila primero el target path_validation_processor."
        )
    default_trace_index = Path(__file__).resolve().parents[1] / "traces.csv"
    trace_index_path = arguments.trace_index.resolve() if arguments.trace_index else default_trace_index
    if arguments.trace_index and not trace_index_path.is_file():
        raise FileNotFoundError(f"No existe el índice indicado: {trace_index_path}")
    trace_index = read_trace_index(trace_index_path) if trace_index_path.is_file() else {}
    if trace_index_path.is_file():
        print(f"Índice de trazados: {trace_index_path}")

    if arguments.input:
        input_paths = sorted(arguments.input, key=lambda path: path.name)
    else:
        default_raw_directory = Path.home() / ".local/robogait/path_validation/raw"
        input_paths = sorted(default_raw_directory.glob("path_*_raw.csv"), key=lambda path: path.name)
        print(f"Capturas descubiertas automáticamente en: {default_raw_directory}")
    if not input_paths:
        raise FileNotFoundError("No se encontraron capturas raw para analizar")

    traces = [
        build_trace_context(path, trace_index, fallback_number)
        for fallback_number, path in enumerate(input_paths, start=1)
    ]
    if len({trace.original_file for trace in traces}) != len(traces):
        raise ValueError("Los nombres originales de las capturas deben ser únicos")
    if len({trace.name for trace in traces}) != len(traces):
        raise ValueError("Los identificadores legibles de las capturas deben ser únicos")
    if len({trace.title for trace in traces}) != len(traces):
        raise ValueError("Los títulos de las capturas deben ser únicos")
    unindexed_files = [trace.original_file for trace in traces if trace.trace_type is None]
    if trace_index and unindexed_files:
        print(
            "Aviso: se utilizarán títulos genéricos para capturas sin índice: "
            + ", ".join(unindexed_files)
        )

    processed_directory = arguments.processed_dir.resolve()
    csv_directory = arguments.results_dir.resolve() / "csv"
    figures_directory = arguments.results_dir.resolve() / "figures"
    csv_directory.mkdir(parents=True, exist_ok=True)
    figures_directory.mkdir(parents=True, exist_ok=True)

    create_documentation_raw_traces(traces, figures_directory / "documentation")

    manifest_rows = [
        {**result_identity(trace), "input_path": str(trace.input_path)} for trace in traces
    ]
    write_results_csv(pd.DataFrame(manifest_rows), csv_directory / "trace_manifest.csv")

    print(f"Procesando {len(traces)} captura(s) real(es)...")
    baseline_runs: dict[str, dict[str, np.ndarray]] = {}
    for trace in traces:
        baseline_runs[trace.name] = run_processor(
            processor,
            trace,
            trace.parameters,
            processed_directory / trace.name / "baseline",
        )
        create_processing_figure(trace, baseline_runs[trace.name], figures_directory)

    douglas_peucker_frame, douglas_peucker_runs = study_douglas_peucker(
        processor, traces, processed_directory, csv_directory, figures_directory
    )
    smoothing_frame, _ = study_smoothing(
        processor, traces, processed_directory, csv_directory, figures_directory
    )
    _, catmull_rom_runs = study_catmull_rom(
        processor, traces, processed_directory, csv_directory, figures_directory
    )
    study_other_parameters(processor, traces, processed_directory, csv_directory)
    create_documentation_figures(
        traces,
        baseline_runs,
        douglas_peucker_frame,
        douglas_peucker_runs,
        smoothing_frame,
        catmull_rom_runs,
        figures_directory / "documentation",
    )
    print(f"Resultados guardados en: {arguments.results_dir.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
