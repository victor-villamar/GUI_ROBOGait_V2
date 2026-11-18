# Validación de trayectorias manuales

Esta herramienta procesa capturas reales con la misma implementación C++ que usa `SplinePathEditor`. El paquete independiente `path_validation` enlaza la biblioteca `robogait_path_geometry` exportada por `robogait_gui`. Python no reproduce Douglas-Peucker, Catmull-Rom, Bézier, el remuestreo ni la media móvil; únicamente ejecuta el procesador, calcula métricas y genera tablas y figuras.

La instrumentación está desactivada en una compilación normal. Activarla solo añade la escritura de la captura antes del procesamiento y no cambia los puntos utilizados por la GUI.

`tools/path_validation/` es también la raíz del paquete `colcon`; los comandos de compilación deben ejecutarse desde la raíz del workspace para que sea descubierto.

## 1. Compilar con captura habilitada

Desde la raíz del workspace:

```bash
cd $HOME/GUI_ROBOGait_V2
source /opt/ros/$ROS_DISTRO/setup.bash
colcon build --packages-select robogait_gui --cmake-args \
  -DENABLE_PATH_VALIDATION_EXPORT=ON
source install/setup.bash
colcon build --packages-select path_validation
source install/setup.bash
```

El ejecutable offline queda en:

```text
build/path_validation/path_validation_processor
```

Para volver a una compilación sin instrumentación:

```bash
colcon build --packages-select robogait_gui --cmake-args \
  -DENABLE_PATH_VALIDATION_EXPORT=OFF
```

## 2. Capturar trazados reales

Iniciar la aplicación compilada, dibujar una trayectoria manual y pulsar la acción que suaviza/procesa el trazado. Se escribe una sola pareja de archivos por acción:

```text
~/.local/robogait/path_validation/raw/path_YYYYMMDD_HHMMSS_raw.csv
~/.local/robogait/path_validation/raw/path_YYYYMMDD_HHMMSS_metadata.csv
```

El log muestra `Trazado de validación guardado en: ...`. Si hay dos capturas en el mismo segundo, se añade un sufijo numérico para no sobrescribir datos.

No es necesario copiar las capturas: el script busca por defecto en ese directorio. Si se copian a otra ubicación, deben mantenerse juntos el raw y sus metadatos. Por ejemplo:

```bash
cp ~/.local/robogait/path_validation/raw/path_20261114_153000_raw.csv \
  tools/path_validation/data/raw/
cp ~/.local/robogait/path_validation/raw/path_20261114_153000_metadata.csv \
  tools/path_validation/data/raw/
```

El análisis funciona con cualquier número de capturas, incluso una sola. Para que las figuras resumidas sean representativas conviene disponer, como mínimo, de una trayectoria en S, un giro pronunciado y una trayectoria sinuosa. Una trayectoria casi recta resulta útil como caso de control.

### Asociar capturas con tipos y repeticiones

La GUI conserva siempre los nombres con timestamp. Después de capturar, ordenar los raw cronológicamente con:

```bash
ls -1 ~/.local/robogait/path_validation/raw/path_*_raw.csv | sort
```

Rellenar manualmente `tools/path_validation/traces.csv` respetando el orden de captura. El formato es:

```csv
archivo,tipo,prueba,titulo
path_20261114_122537_raw.csv,trayectoria_s,1,Trayectoria en S - prueba 1
path_20261114_122652_raw.csv,trayectoria_s,2,Trayectoria en S - prueba 2
path_20261114_122900_raw.csv,giro_pronunciado,1,Giro pronunciado - prueba 1
```

Los tipos previstos son `trayectoria_s`, `giro_pronunciado`, `trayectoria_sinuosa` y `trayectoria_casi_recta`. El campo `titulo` es el único que se muestra en gráficas y leyendas. Los timestamps se conservan exclusivamente en los CSV de trazabilidad.

El orden acordado para rellenarlo es:

```text
trayectoria_s_01, trayectoria_s_02, trayectoria_s_03,
giro_pronunciado_01, giro_pronunciado_02, giro_pronunciado_03,
trayectoria_sinuosa_01, trayectoria_sinuosa_02, trayectoria_sinuosa_03,
trayectoria_casi_recta_01, trayectoria_casi_recta_02, trayectoria_casi_recta_03
```

## 3. Ejecutar una configuración manualmente

Las distancias que recibe el procesador están en metros. Deben tomarse del archivo de metadatos de la captura:

```bash
build/path_validation/path_validation_processor \
  --input tools/path_validation/data/raw/path_20261114_153000_raw.csv \
  --resample-spacing 0.05 \
  --smoothing-window-radius 2 \
  --simplify-tolerance 0.08 \
  --max-anchor-points 28 \
  --catmull-alpha 0.5 \
  --sample-spacing 0.04 \
  --output-dir tools/path_validation/data/processed/manual_run
```

Cada ejecución genera `raw.csv`, `deduplicated.csv`, `resampled.csv`, `smoothed.csv`, `simplified.csv`, `anchors.csv` y `spline.csv`.

## 4. Preparar Python

```bash
cd $HOME/GUI_ROBOGait_V2
python3 -m venv .venv-path-validation
source .venv-path-validation/bin/activate
python -m pip install -r tools/path_validation/requirements.txt
```

## 5. Ejecutar todos los barridos

La ejecución normal no necesita argumentos. Descubre y ordena todos los raw de `~/.local/robogait/path_validation/raw/`, usa el índice predeterminado y ejecuta todos los barridos:

```bash
python tools/path_validation/scripts/analyze_paths.py
```

Con una captura:

```bash
python tools/path_validation/scripts/analyze_paths.py \
  --processor build/path_validation/path_validation_processor \
  --input tools/path_validation/data/raw/path_20261114_153000_raw.csv
```

Con todas las capturas del directorio:

```bash
python tools/path_validation/scripts/analyze_paths.py \
  --processor build/path_validation/path_validation_processor \
  --input tools/path_validation/data/raw/path_*_raw.csv
```

El script busca automáticamente `tools/path_validation/traces.csv`. También se puede indicar otro índice explícitamente:

```bash
python tools/path_validation/scripts/analyze_paths.py \
  --processor build/path_validation/path_validation_processor \
  --trace-index tools/path_validation/traces.csv \
  --input ~/.local/robogait/path_validation/raw/path_*_raw.csv
```

El script exige el archivo `*_metadata.csv` asociado a cada raw y obtiene de él la conversión píxel-metro utilizada en la captura. No contiene ni genera trayectorias ficticias. Los archivos de entrada se ordenan por nombre antes del análisis, por lo que los timestamps se procesan cronológicamente. Si no existe un índice, o un archivo concreto no aparece en él, se asignan nombres visibles genéricos como `Trayectoria 1`; los timestamps nunca se muestran en figuras.

Los CSV quedan en `tools/path_validation/results/csv/`, las figuras en `tools/path_validation/results/figures/` y todas las etapas C++ de cada ejecución en `tools/path_validation/data/processed/<captura>/`. `trace_manifest.csv` registra la asociación efectiva entre identificadores legibles, tipos, repeticiones, nombres originales y rutas de entrada.

### Figuras para documentación

Las figuras resumidas se generan en `tools/path_validation/results/figures/documentation/`, siempre en PDF vectorial y PNG. Las figuras cuantitativas utilizan todas las capturas recibidas, ya sean 1, 4 o 12. Para las comparaciones visuales se elige la primera prueba disponible del tipo preferido:

- trayectoria en S para las etapas de procesamiento;
- giro pronunciado para Douglas-Peucker;
- trayectoria sinuosa para Catmull-Rom.

Si falta alguno de esos tipos, el script selecciona la primera captura disponible y muestra un aviso en la terminal. De este modo, una única captura también produce el conjunto completo de resultados.

## Métricas

- El error geométrico es un RMSE simétrico entre versiones densamente muestreadas, acompañado de la desviación máxima en ambas direcciones.
- La irregularidad es la suma de cambios absolutos de orientación entre segmentos; también se informa normalizada por la longitud (`rad/m`).
- El barrido Catmull-Rom comprueba que los anchors sean numéricamente idénticos para todos los valores de alfa y aborta si no se cumple.
- `max_anchor_points_metrics.csv` incluye `limite_puntos_anclaje_activo`, que solo es `sí` cuando Douglas-Peucker produjo más puntos que el límite probado.

Los PDF se guardan como gráficos vectoriales y todos los paneles de trayectoria usan metros y relación de aspecto X/Y igual.