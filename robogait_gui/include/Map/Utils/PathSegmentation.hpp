#pragma once

#include <QPointF>
#include <QVector>

namespace ROBOGait
{
namespace map
{
namespace utils
{
/**
 * @brief Stateless geometric segmentation for freehand polylines
 */
class PathSegmentation
{
public:
  /**
   * @brief Configuration for geometric segmentation
   *
   * @param resample_spacing_m Uniform resampling spacing in meters
   * @param rdp_epsilon_m RDP tolerance in meters
   * @param short_straw_window ShortStraw half-window size
   * @param short_straw_median_factor ShortStraw median factor
   * @param short_straw_line_threshold ShortStraw line test threshold
   */
  struct Config
  {
    double resample_spacing_m = 0.05;
    double rdp_epsilon_m = 0.03;
    int short_straw_window = 3;
    double short_straw_median_factor = 0.95;
    double short_straw_line_threshold = 0.95;
  };

  /**
   * @brief Build segmented polyline from raw freehand points
   *
   * @param points Input raw freehand points
   * @param config Configuration parameters for segmentation
   *
   * @return Segmented polyline points
   */
  static QVector<QPointF> segmentPolyline(const QVector<QPointF>& points, const Config& config);

private:
  /**
   * @brief Uniformly resample a polyline to a specified spacing
   *
   * @param points Input polyline points
   * @param spacing_m Desired spacing between resampled points in meters
   *
   * @return Uniformly resampled polyline points
   */
  static QVector<QPointF> uniformResample(const QVector<QPointF>& points, double spacing_m);

  /**
   * @brief Detect corners using the ShortStraw algorithm
   *
   * @param points Input polyline points
   * @param window ShortStraw half-window size
   * @param median_factor ShortStraw median factor
   * @param line_threshold ShortStraw line test threshold
   *
   * @return Indices of detected corner points
   */
  static QVector<int> detectShortStrawCorners(const QVector<QPointF>& points, int window, double median_factor, double line_threshold);

  /**
   * @brief Check if a segment of the polyline can be approximated by a line within a threshold
   *
   * @param points Input polyline points
   * @param first Index of the first point in the segment
   * @param last Index of the last point in the segment
   * @param threshold_ratio Maximum allowed ratio of point-to-segment distance to segment length for line approximation
   *
   * @return true if the segment can be approximated by a line, false otherwise
   */
  static bool isLineSegmentApproximation(const QVector<QPointF>& points, int first, int last, double threshold_ratio);

  /**
   * @brief Simplify a polyline using Ramer-Douglas-Peucker while keeping specified corner points
   *
   * @param points Input polyline points
   * @param locked_corner_indices Indices of points to keep as corners (in addition to start and end)
   * @param epsilon_m RDP tolerance in meters
   *
   * @return Simplified polyline points
   */
  static QVector<QPointF> simplifyWithLockedCorners(const QVector<QPointF>& points, const QVector<int>& locked_corner_indices, double epsilon_m);

  /**
   * @brief Collect indices of points kept by Ramer-Douglas-Peucker algorithm for a segment of the polyline
   *
   * @param points Input polyline points
   * @param first Index of the first point in the segment
   * @param last Index of the last point in the segment
   * @param epsilon_m RDP tolerance in meters
   *
   * @return Indices of points kept by RDP (including first and last)
   */
  static QVector<int> collectRdpKeptIndices(const QVector<QPointF>& points, int first, int last, double epsilon_m);

  /**
   * @brief Compute the distance from a point to a line segment defined by two points
   *
   * @param point The point to measure the distance from
   * @param seg_a The first endpoint of the line segment
   * @param seg_b The second endpoint of the line segment
   *
   * @return The distance from the point to the line segment
   */
  static double pointToSegmentDistance(const QPointF& point, const QPointF& seg_a, const QPointF& seg_b);

  static constexpr int MIN_VALID_PATH_POINTS = 2; /** Minimum number of valid points in a path */
};
} // namespace utils
} // namespace map
} // namespace ROBOGait
