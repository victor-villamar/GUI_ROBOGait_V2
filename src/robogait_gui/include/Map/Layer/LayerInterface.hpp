#pragma once

namespace ROBOGait
{
namespace map
{
namespace layer
{

/**
 * @brief Interface for layer updates and render requests
 */
class LayerInterface
{
public:
  virtual ~LayerInterface() = default;
  virtual void update() = 0;
  virtual bool needsRender() const = 0;
  virtual void clearRenderRequest() = 0;
};

} // namespace layer
} // namespace map
} // namespace ROBOGait
