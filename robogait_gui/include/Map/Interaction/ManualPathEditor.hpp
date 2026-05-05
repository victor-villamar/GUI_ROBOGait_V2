#pragma once

#include <QObject>
#include <QPointF>
#include <QSet>
#include <QVariantList>
#include <QVector>

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
 * @brief Handles user path sketching/editing by drag over the map
 */
class ManualPathEditor : public QObject
{
  Q_OBJECT
public:
  // clang-format off
  Q_PROPERTY(bool hasPath
             READ hasPath
             NOTIFY pathChanged)

  Q_PROPERTY(bool isSegmented
             READ isSegmented
             NOTIFY segmentedChanged)
  // clang-format on

  /**
   * @brief Constructor of ManualPathEditor class
   */
  explicit ManualPathEditor(QObject* parent = nullptr);

  /**
   * @brief Check if path has at least one point
   */
  bool hasPath() const;

  /**
   * @brief Check if current active path is already segmented
   */
  bool isSegmented() const;

  /**
   * @brief Set MapVisualizationManager for coordinate conversion and rendering
   */
  void setMapVisualizationManager(ROBOGait::map::manager::MapVisualizationManager* manager);

  /**
   * @brief Sync cached points from external path updates
   *
   * This method only updates local editor state. It does not push changes back to map rendering.
   */
  void setCachedPath(const QVariantList& points);

  /**
   * @brief Clear cached points from an external path clear operation
   *
   * This method only updates local editor state. It does not clear map rendering.
   */
  void clearCachedPath();

  /**
   * @brief Start a new stroke and clear previous path
   *
   * Initializes the first point from current robot pose.
   *
   * @return true if stroke started with a valid robot-start point, false otherwise
   */
  bool beginStroke();

  /**
   * @brief Get current path points as QVariantList of {x, y}
   */
  Q_INVOKABLE QVariantList getPathPoints() const;

  /**
   * @brief Start a new stroke and append first point from screen coordinates
   */
  Q_INVOKABLE void beginStrokeFromScreen(double screen_x, double screen_y);

  /**
   * @brief Append a point from screen coordinates while dragging
   */
  Q_INVOKABLE void appendPointFromScreen(double screen_x, double screen_y);

  /**
   * @brief Finish current stroke
   */
  Q_INVOKABLE void endStroke();

  /**
   * @brief Clear current path and stop drawing mode
   */
  Q_INVOKABLE void clear();

  /**
   * @brief Build and activate the segmented polyline from current raw path
   *
   * @return true when segmentation succeeded and state switched to segmented
   */
  Q_INVOKABLE bool segmentPath();

signals:
  void pathChanged();      // Emitted when path points change
  void drawingChanged();   // Emitted when drawing state changes
  void segmentedChanged(); // Emitted when segmented state changes

private:
  /**
   * @brief Ensure manager is set before using map conversions/render sync
   */
  bool ensureMapVisualizationManager() const;

  /**
   * @brief Push internal cached points to map visualization layer
   */
  void syncPathToVisualization();

  /**
   * @brief Set drawing state and emit signal when changed
   */
  void setDrawing(bool drawing);

  /**
   * @brief Set segmented state and emit signal when changed
   */
  void setSegmentedState(bool segmented);

  /**
   * @brief Compare two point vectors
   */
  static bool arePointVectorsEqual(const QVector<QPointF>& lhs, const QVector<QPointF>& rhs);

  /**
   * @brief Get the currently active path points for visualization
   *
   * Returns segmented points when available and selected, otherwise raw points.
   */
  const QVector<QPointF>& getActivePathPoints() const;

  /**
   * @brief Convert screen-space pixel distances to map-space meters
   */
  double pixelsToMetersDistance(double distance_px) const;

  /**
   * @brief Resample a polyline at near-uniform spacing
   */
  QVector<QPointF> uniformResample(const QVector<QPointF>& points, double spacing_m) const;

  /**
   * @brief Detect corner indices using the ShortStraw heuristic
   */
  QVector<int> detectShortStrawCorners(const QVector<QPointF>& points, int window, double median_factor, double line_threshold) const;

  /**
   * @brief Simplify polyline with RDP while forcing locked corner indices
   */
  QVector<QPointF> simplifyWithLockedCorners(const QVector<QPointF>& points, const QVector<int>& locked_corner_indices, double epsilon_m) const;

  /**
   * @brief Build a segmented polyline from raw freehand points
   *
   * This is the geometric commit pipeline (resample -> corners -> locked-RDP).
   */
  QVector<QPointF> buildSegmentedPolyline(const QVector<QPointF>& points) const;

  /**
   * @brief Distance from point to segment
   */
  static double pointToSegmentDistance(const QPointF& point, const QPointF& seg_a, const QPointF& seg_b);

  /**
   * @brief Per-segment RDP returning kept indices in [first,last]
   */
  QVector<int> collectRdpKeptIndices(const QVector<QPointF>& points, int first, int last, double epsilon_m) const;

  /**
   * @brief Line test used by ShortStraw refinement
   */
  static bool isLineSegmentApproximation(const QVector<QPointF>& points, int first, int last, double threshold_ratio);

  /**
   * @brief Append the current robot position as first point of the stroke
   *
   * @return true if robot pose was valid and point added
   */
  bool appendRobotStartPoint();

  /**
   * @brief Configuration parameters for geometric segmentation
   *
   * @param resample_spacing_px Target resampling spacing in screen pixels
   * @param rdp_epsilon_px RDP tolerance in screen pixels
   * @param short_straw_window ShortStraw half-window size
   * @param short_straw_median_factor ShortStraw median factor
   * @param short_straw_line_threshold ShortStraw line test threshold
   */
  struct SegmentationConfig
  {
    double resample_spacing_px = 5.0;
    double rdp_epsilon_px = 3.0;
    int short_straw_window = 3;
    double short_straw_median_factor = 0.95;
    double short_straw_line_threshold = 0.95;
  };

  ROBOGait::map::manager::MapVisualizationManager* map_visualization_manager_; /**< Map visualization manager pointer */
  QVector<QPointF> raw_path_points_;                                           /**< Raw freehand points captured while drawing */
  QVector<QPointF> segmented_path_points_;                                     /**< Segmented polyline points (future commit step) */
  bool is_segmented_;                                                          /**< True when active visualization should use segmented_path_points_ */
  bool drawing_;                                                               /**< True while drag stroke is active */
  bool stroke_blocked_by_outside_;                                             /**< True when pointer leaves map during a stroke */
  double min_point_distance_m_;                                                /**< Minimum distance between consecutive points */
  SegmentationConfig segmentation_config_;                                     /**< Parameters for geometric segmentation */

  static constexpr int MIN_VALID_PATH_POINTS = 2; /**< Minimum number of valid points required for a path */
};
} // namespace interaction
} // namespace map
} // namespace ROBOGait
