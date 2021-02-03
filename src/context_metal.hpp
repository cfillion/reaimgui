#ifndef REAIMGUI_CONTEXT_METAL_HPP
#define REAIMGUI_CONTEXT_METAL_HPP

#include "context.hpp"

#include <Metal/Metal.h>

class Context_Metal : public Context {
public:
  Context_Metal();
  ~Context_Metal() override;

  auto device() const { return m_device; }
  auto commandQueue() const { return m_commandQueue; }
  auto renderPass() const { return m_renderPass; }

private:
  id<MTLDevice> m_device;
  id<MTLCommandQueue> m_commandQueue;
  MTLRenderPassDescriptor *m_renderPass;
};

#endif
