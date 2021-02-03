#include "context_metal.hpp"

#include <imgui/backends/imgui_impl_metal.h>
#include <imgui/backends/imgui_impl_osx.h>

std::shared_ptr<Context> Context::create()
{
  return std::make_shared<Context_Metal>();
}

Context_Metal::Context_Metal()
  : Context()
{
  m_device = MTLCreateSystemDefaultDevice();
  assert(m_device);

  m_commandQueue = [m_device newCommandQueue];
  m_renderPass = [MTLRenderPassDescriptor new];

  ImGui_ImplOSX_Init();
  ImGui_ImplMetal_Init(m_device);
}

Context_Metal::~Context_Metal()
{
  ImGui_ImplMetal_Shutdown();
  ImGui_ImplOSX_Shutdown();
}
