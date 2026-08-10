#pragma once

#include <QPointF>
#include <QVector>

namespace ROBOGait
{
namespace map
{
namespace interaction
{
namespace geometry
{
/**
 * @brief Cubic Bezier segment representation
 */
struct CubicBezierSegment
{
  QPointF p0;
  QPointF c1;
  QPointF c2;
  QPointF p1;
};

/**
 * @brief Control point category for spline editing
 */
enum class ControlPointType
{
  Anchor = 0,
  HandleIn = 1,
  HandleOut = 2,
};

/**
 * @brief Editable cubic Bezier chain model
 */
class BezierSplineModel
{
public:
  /**
   * @brief Lightweight reference to an editable control point
   */
  struct ControlPointRef
  {
    ControlPointType type;
    int index;
    QPointF position;
  };

  /**
   * @brief Reset model contents
   */
  void clear();

  /**
   * @brief Build model from cubic Bezier segments
   */
  bool setFromSegments(const QVector<CubicBezierSegment>& segments);

  /**
   * @brief Build cubic Bezier segments from internal editable representation
   */
  QVector<CubicBezierSegment> getSegments() const;

  /**
   * @brief Retrieve all editable control points
   */
  QVector<ControlPointRef> getControlPoints() const;

  /**
   * @brief Update one control point position
   */
  bool setControlPoint(ControlPointType type, int index, const QPointF& position);

  /**
   * @brief Uniformly sample spline by target spacing
   */
  QVector<QPointF> sampleByDistance(double spacing_m) const;

  /**
   * @brief Check if model has a valid spline chain
   */
  bool isValid() const;

private:
  static QPointF evaluateCubic(const CubicBezierSegment& segment, double t);
  static double approximateLength(const CubicBezierSegment& segment, int subdivisions);

  QVector<QPointF> anchors_;     /**< Anchor points of the chain (N + 1 points for N segments) */
  QVector<QPointF> handles_in_;  /**< Incoming handles per anchor */
  QVector<QPointF> handles_out_; /**< Outgoing handles per anchor */

  static constexpr int MIN_VALID_SEGMENTS = 1;   /**< Minimum number of segments to be considered a valid chain */
  static constexpr int LENGTH_SUBDIVISIONS = 24; /**< Number of segments used to estimate cubic arc length */
};
} // namespace geometry
} // namespace interaction
} // namespace map
} // namespace ROBOGait
