#pragma once

#include <QObject>
#include <QPointF>
#include <QVariantList>
#include <QVector>

#include "Map/Interaction/Geometry/BezierSplineModel.hpp"

namespace ROBOGait
{
namespace map
{
namespace manager
{
class MapVisualizationManager;
}

namespace interaction
{
/**
 * @brief Handles raw freehand capture, spline smoothing and spline editing lifecycle
 */
class SplinePathEditor : public QObject
{
  Q_OBJECT

public:
  static constexpr double DEFAULT_SPLINE_RESAMPLE_SPACING_PX = 5.0;
  static constexpr int DEFAULT_SPLINE_SMOOTHING_WINDOW_RADIUS = 2;
  static constexpr double DEFAULT_SPLINE_CATMULL_ALPHA = 0.5;
  static constexpr double DEFAULT_SPLINE_SAMPLE_SPACING_PX = 4.0;
  static constexpr double DEFAULT_SPLINE_SIMPLIFY_TOLERANCE_PX = 8.0;
  static constexpr int DEFAULT_SPLINE_MAX_ANCHOR_POINTS = 28;

  /**
   * @brief Smoothing tuning in screen-space units
   */
  struct SmoothingTuningPx
  {
    explicit SmoothingTuningPx(double resample_spacing_px_value = DEFAULT_SPLINE_RESAMPLE_SPACING_PX,
                               int smoothing_window_radius_value = DEFAULT_SPLINE_SMOOTHING_WINDOW_RADIUS,
                               double catmull_alpha_value = DEFAULT_SPLINE_CATMULL_ALPHA, double sample_spacing_px_value = DEFAULT_SPLINE_SAMPLE_SPACING_PX) :
        resample_spacing_px(resample_spacing_px_value),
        smoothing_window_radius(smoothing_window_radius_value),
        catmull_alpha(catmull_alpha_value),
        sample_spacing_px(sample_spacing_px_value)
    {
    }

    double resample_spacing_px;
    int smoothing_window_radius;
    double catmull_alpha;
    double sample_spacing_px;
  };

  /**
   * @brief Reduction tuning for editable anchors density
   */
  struct EditReductionTuningPx
  {
    explicit EditReductionTuningPx(double simplify_tolerance_px_value = DEFAULT_SPLINE_SIMPLIFY_TOLERANCE_PX,
                                   int max_anchor_points_value = DEFAULT_SPLINE_MAX_ANCHOR_POINTS) :
        simplify_tolerance_px(simplify_tolerance_px_value), max_anchor_points(max_anchor_points_value)
    {
    }

    double simplify_tolerance_px;
    int max_anchor_points;
  };

  enum ControlPointType
  {
    Anchor = 0,
    HandleIn = 1,
    HandleOut = 2,
  };
  Q_ENUM(ControlPointType)

  // clang-format off
  Q_PROPERTY(bool hasPath
             READ hasPath
             NOTIFY pathChanged)

  Q_PROPERTY(bool isSmoothed
             READ isSmoothed
             NOTIFY smoothedChanged)

  Q_PROPERTY(bool isEditMode
             READ isEditMode
             WRITE setEditMode
             NOTIFY editModeChanged)

  Q_PROPERTY(bool hasEditablePath
             READ hasEditablePath
             NOTIFY editablePathChanged)
  // clang-format on

  /**
   * @brief Constructor of SplinePathEditor class
   */
  explicit SplinePathEditor(QObject* parent = nullptr);

  /**
   * @brief Check if active path has at least two points
   */
  bool hasPath() const;

  /**
   * @brief Check if raw path has already been converted to spline
   */
  bool isSmoothed() const;

  /**
   * @brief Check if spline edit mode is enabled
   */
  bool isEditMode() const;

  /**
   * @brief Check if current spline is valid to be used in compute action
   */
  bool hasEditablePath() const;

  /**
   * @brief Set edit mode from property assignment
   */
  void setEditMode(bool enabled);

  /**
   * @brief Set map visualization manager for coordinate conversion and rendering sync
   */
  void setMapVisualizationManager(ROBOGait::map::manager::MapVisualizationManager* manager);

  /**
   * @brief Override smoothing tuning in screen-space units
   */
  void setSmoothingTuningPx(const SmoothingTuningPx& tuning);

  /**
   * @brief Override editor reduction tuning in screen-space units
   */
  void setEditReductionTuningPx(const EditReductionTuningPx& tuning);

  /**
   * @brief Start a new stroke and clear previous spline/path
   */
  bool beginStroke();

  /**
   * @brief Clear current path/spline and stop drawing/edit mode
   */
  Q_INVOKABLE void clear();

  /**
   * @brief Load map-frame path points as an editable spline
   */
  Q_INVOKABLE bool loadPathPoints(const QVariantList& points);

  /**
   * @brief Clear cached points from external clear operation
   */
  void clearCachedPath();

  /**
   * @brief Start a new stroke and append first point from screen coordinates
   */
  Q_INVOKABLE void beginStrokeFromScreen(double screen_x, double screen_y);

  /**
   * @brief Append point from screen coordinates while dragging
   */
  Q_INVOKABLE void appendPointFromScreen(double screen_x, double screen_y);

  /**
   * @brief Finish current stroke
   */
  Q_INVOKABLE void endStroke();

  /**
   * @brief Smooth raw path into an editable spline
   */
  Q_INVOKABLE bool smoothPath();

  /**
   * @brief Enable or disable editing mode over the smoothed spline
   */
  Q_INVOKABLE bool setEditModeEnabled(bool enabled);

  /**
   * @brief Update a control point using map-frame coordinates
   */
  Q_INVOKABLE bool moveControlPoint(int type, int index, double map_x, double map_y);

  /**
   * @brief Return editable control points for QML overlays
   */
  Q_INVOKABLE QVariantList getControlPoints() const;

  /**
   * @brief Get active path points as QVariantList of {x, y}
   */
  Q_INVOKABLE QVariantList getPathPoints() const;

  /**
   * @brief Get points to be used by compute/calculate flow
   */
  Q_INVOKABLE QVariantList getPathPointsForCompute() const;

signals:
  void pathChanged();         // Emitted when raw path points change
  void drawingChanged();      // Emitted when drawing state changes (beginStroke/endStroke)
  void smoothedChanged();     // Emitted when smoothed spline points change
  void editModeChanged();     // Emitted when edit mode is enabled/disabled
  void editablePathChanged(); // Emitted when smoothed spline validity for compute changes (has enough points after reduction)

private:
  bool ensureMapVisualizationManager() const;
  void syncPathToVisualization();
  void setDrawing(bool drawing);
  void setSmoothedState(bool smoothed);
  void setEditablePathState(bool editable);
  void setEditModeState(bool enabled);
  bool appendRobotStartPoint();
  const QVector<QPointF>& getActivePathPoints() const;
  double pixelsToMetersDistance(double distance_px) const;
  static QVariantList toVariantList(const QVector<QPointF>& points);

  ROBOGait::map::manager::MapVisualizationManager* map_visualization_manager_; /**< Map visualization manager pointer */
  QVector<QPointF> raw_path_points_;                                           /**< Raw freehand points captured while drawing */
  QVector<QPointF> smoothed_path_points_;                                      /**< Sampled points generated from spline model */
  geometry::BezierSplineModel spline_model_;                                   /**< Editable spline model */
  bool is_smoothed_;                                                           /**< True when active path is the smoothed spline */
  bool is_edit_mode_;                                                          /**< True when edit mode is enabled */
  bool has_editable_path_;                                                     /**< True when smoothed spline is valid for compute */
  bool drawing_;                                                               /**< True while drag stroke is active */
  bool stroke_blocked_by_outside_;                                             /**< True when pointer leaves map during a stroke */
  double min_point_distance_m_;                                                /**< Minimum distance between consecutive points */
  SmoothingTuningPx smoothing_tuning_px_;                                      /**< Smoothing tuning values */
  EditReductionTuningPx edit_reduction_tuning_px_;                             /**< Anchor density reduction tuning values */
};
} // namespace interaction
} // namespace map
} // namespace ROBOGait
