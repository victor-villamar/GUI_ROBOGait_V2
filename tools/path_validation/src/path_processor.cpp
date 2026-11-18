#include <cmath>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QPointF>
#include <QSaveFile>
#include <QTextStream>
#include <QVector>

#include "Map/Interaction/Geometry/BezierSplineModel.hpp"
#include "Map/Interaction/Geometry/SplineFitter.hpp"
#include "Map/Interaction/Geometry/StrokeProcessor.hpp"

namespace
{
using ROBOGait::map::interaction::geometry::BezierSplineModel;
using ROBOGait::map::interaction::geometry::CubicBezierSegment;
using ROBOGait::map::interaction::geometry::SplineFitter;
using ROBOGait::map::interaction::geometry::StrokeProcessor;

struct ProcessingParameters
{
  double resample_spacing_m = 0.0;
  int smoothing_window_radius = 0;
  double simplify_tolerance_m = 0.0;
  int max_anchor_points = 0;
  double catmull_alpha = 0.0;
  double sample_spacing_m = 0.0;
};

bool parseDoubleOption(const QCommandLineParser& parser, const QCommandLineOption& option, double& value)
{
  bool valid = false;
  value = parser.value(option).toDouble(&valid);
  return valid && std::isfinite(value);
}

bool parseIntegerOption(const QCommandLineParser& parser, const QCommandLineOption& option, int& value)
{
  bool valid = false;
  value = parser.value(option).toInt(&valid);
  return valid;
}

bool readPoints(const QString& file_path, QVector<QPointF>& points, QString& error)
{
  QFile file(file_path);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    error = QString("No se pudo abrir el CSV de entrada: %1").arg(file_path);
    return false;
  }

  QTextStream stream(&file);
  const QString header = stream.readLine().trimmed();

  if (header != "index,x_m,y_m")
  {
    error = QString("La cabecera del CSV no es válida en %1").arg(file_path);
    return false;
  }

  int line_number = 1;

  while (!stream.atEnd())
  {
    ++line_number;
    const QString line = stream.readLine().trimmed();
    if (line.isEmpty())
    {
      continue;
    }

    const QStringList fields = line.split(',');
    bool valid_index = false;
    bool valid_x = false;
    bool valid_y = false;

    if (fields.size() != 3)
    {
      error = QString("Número de columnas no válido en la línea %1").arg(line_number);
      return false;
    }

    fields.at(0).toLongLong(&valid_index);
    const double x = fields.at(1).toDouble(&valid_x);
    const double y = fields.at(2).toDouble(&valid_y);

    if (!valid_index || !valid_x || !valid_y || !std::isfinite(x) || !std::isfinite(y))
    {
      error = QString("Valor no válido en la línea %1").arg(line_number);
      return false;
    }

    points.append(QPointF(x, y));
  }

  if (points.size() < StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    error = "El CSV debe contener al menos dos puntos válidos";
    return false;
  }

  return true;
}

bool writePoints(const QString& file_path, const QVector<QPointF>& points, QString& error)
{
  QSaveFile file(file_path);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    error = QString("No se pudo abrir el CSV de salida: %1").arg(file_path);
    return false;
  }

  QTextStream stream(&file);
  stream.setLocale(QLocale::c());
  stream.setRealNumberNotation(QTextStream::FixedNotation);
  stream.setRealNumberPrecision(10);
  stream << "index,x_m,y_m\n";

  for (int index = 0; index < points.size(); ++index)
  {
    stream << index << ',' << points.at(index).x() << ',' << points.at(index).y() << '\n';
  }

  if (!file.commit())
  {
    error = QString("No se pudo guardar el CSV de salida: %1").arg(file_path);
    return false;
  }

  return true;
}

bool writeStage(const QDir& output_directory, const QString& stage_name, const QVector<QPointF>& points, QString& error)
{
  return writePoints(output_directory.filePath(stage_name + ".csv"), points, error);
}
} // namespace

int main(int argc, char* argv[])
{
  QCoreApplication application(argc, argv);
  QCoreApplication::setApplicationName("path_validation_processor");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Procesa un trazado offline con la cadena geométrica de SplinePathEditor.");
  parser.addHelpOption();
  parser.addVersionOption();

  const QCommandLineOption input_option({"i", "input"}, "CSV original de entrada.", "csv");
  const QCommandLineOption output_option({"o", "output-dir"}, "Directorio para los CSV de las etapas.", "directorio");
  const QCommandLineOption resample_option("resample-spacing", "Separación de remuestreo en metros.", "metros");
  const QCommandLineOption smoothing_option("smoothing-window-radius", "Radio entero de la media móvil.", "radio");
  const QCommandLineOption simplify_option("simplify-tolerance", "Tolerancia Douglas-Peucker en metros.", "metros");
  const QCommandLineOption anchors_option("max-anchor-points", "Número máximo de puntos de anclaje.", "cantidad");
  const QCommandLineOption alpha_option("catmull-alpha", "Parámetro alfa de Catmull-Rom [0, 1].", "alfa");
  const QCommandLineOption sample_option("sample-spacing", "Separación de muestreo de la spline en metros.", "metros");

  parser.addOptions({input_option, output_option, resample_option, smoothing_option, simplify_option, anchors_option, alpha_option, sample_option});
  parser.process(application);

  const QList<QCommandLineOption> required_options = {input_option,    output_option,  resample_option, smoothing_option,
                                                      simplify_option, anchors_option, alpha_option,    sample_option};
  for (const QCommandLineOption& option : required_options)
  {
    if (!parser.isSet(option))
    {
      QTextStream(stderr) << "Falta la opción obligatoria --" << option.names().constLast() << "\n";
      parser.showHelp(2);
    }
  }

  ProcessingParameters parameters;
  const bool valid_parameters =
      parseDoubleOption(parser, resample_option, parameters.resample_spacing_m) &&
      parseIntegerOption(parser, smoothing_option, parameters.smoothing_window_radius) &&
      parseDoubleOption(parser, simplify_option, parameters.simplify_tolerance_m) && parseIntegerOption(parser, anchors_option, parameters.max_anchor_points) &&
      parseDoubleOption(parser, alpha_option, parameters.catmull_alpha) && parseDoubleOption(parser, sample_option, parameters.sample_spacing_m);

  if (!valid_parameters || parameters.resample_spacing_m <= 0.0 || parameters.smoothing_window_radius < 0 || parameters.simplify_tolerance_m <= 0.0 ||
      parameters.max_anchor_points < StrokeProcessor::MIN_VALID_PATH_POINTS || parameters.catmull_alpha < 0.0 || parameters.catmull_alpha > 1.0 ||
      parameters.sample_spacing_m <= 0.0)
  {
    QTextStream(stderr) << "Los parámetros de procesamiento no son válidos.\n";
    return 2;
  }

  QVector<QPointF> raw_points;
  QString error;

  if (!readPoints(parser.value(input_option), raw_points, error))
  {
    QTextStream(stderr) << error << '\n';
    return 1;
  }

  QDir output_directory(parser.value(output_option));

  if (!output_directory.exists() && !QDir().mkpath(output_directory.absolutePath()))
  {
    QTextStream(stderr) << "No se pudo crear el directorio de salida: " << output_directory.absolutePath() << '\n';
    return 1;
  }

  const QVector<QPointF> deduplicated = StrokeProcessor::removeDuplicatedPoints(raw_points, 1e-6);
  const QVector<QPointF> resampled = StrokeProcessor::resampleByArcLength(deduplicated, parameters.resample_spacing_m);
  const QVector<QPointF> smoothed = StrokeProcessor::smoothMovingAverage(resampled, parameters.smoothing_window_radius);
  const QVector<QPointF> simplified = StrokeProcessor::simplifyDouglasPeucker(smoothed, parameters.simplify_tolerance_m);
  const QVector<QPointF> anchors = StrokeProcessor::limitPointCount(simplified, parameters.max_anchor_points);
  const QVector<CubicBezierSegment> segments = SplineFitter::fitCentripetalCatmullRom(anchors, parameters.catmull_alpha);

  BezierSplineModel spline_model;

  if (anchors.size() < StrokeProcessor::MIN_VALID_PATH_POINTS || segments.isEmpty() || !spline_model.setFromSegments(segments))
  {
    QTextStream(stderr) << "La trayectoria no produjo una spline válida.\n";
    return 1;
  }

  const QVector<QPointF> spline = spline_model.sampleByDistance(parameters.sample_spacing_m);

  if (spline.size() < StrokeProcessor::MIN_VALID_PATH_POINTS)
  {
    QTextStream(stderr) << "La spline no produjo suficientes muestras.\n";
    return 1;
  }

  const QList<QPair<QString, QVector<QPointF>>> stages = {
      {"raw", raw_points},        {"deduplicated", deduplicated}, {"resampled", resampled}, {"smoothed", smoothed},
      {"simplified", simplified}, {"anchors", anchors},           {"spline", spline},
  };

  for (const auto& stage : stages)
  {
    if (!writeStage(output_directory, stage.first, stage.second, error))
    {
      QTextStream(stderr) << error << '\n';
      return 1;
    }
  }

  QTextStream(stdout) << "Procesamiento completado en: " << output_directory.absolutePath() << '\n';
  return 0;
}