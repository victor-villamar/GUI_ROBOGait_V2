#pragma once

#include <QObject>
#include <QPointF>
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
   */
  void beginStroke();

  /**
   * @brief Get current path points as QVariantList of {x, y}
   */
  QVariantList getPathPoints() const;

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

signals:
  void pathChanged();    // Emitted when path points change
  void drawingChanged(); // Emitted when drawing state changes

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
   * @brief Compare two point vectors
   */
  static bool arePointVectorsEqual(const QVector<QPointF>& lhs, const QVector<QPointF>& rhs);

  ROBOGait::map::manager::MapVisualizationManager* map_visualization_manager_; /**< Map visualization manager pointer */
  QVector<QPointF> points_;                                                    /**< Current manual path points */
  bool drawing_;                                                               /**< True while drag stroke is active */
  bool stroke_blocked_by_outside_;                                             /**< True when pointer leaves map during a stroke */
  double min_point_distance_m_;                                                /**< Minimum distance between consecutive points */
};
} // namespace interaction
} // namespace map
} // namespace ROBOGait
