/* ReaImGui: ReaScript binding for Dear ImGui
 * Copyright (C) 2021-2025  Christian Fillion
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "renderer.hpp"

#include "context.hpp"
#include "error.hpp"
#include "import.hpp"
#include "texture.hpp"
#include "window.hpp"

#include <AppKit/AppKit.h>
#include <imgui/imgui.h>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/CATransaction.h>

static_assert(__has_feature(objc_arc),
  "This file must be built with automatic reference counting enabled.");

class MetalRenderer;
REGISTER_RENDERER(10, metal, "Metal", &Renderer::create<MetalRenderer>, []() -> char {
  if(@available(macOS 10.11, *))
    return RendererType::Available;
  else
    return 0;
}());

#define FPATH(name) \
  "/System/Library/Frameworks/" name ".framework/Versions/Current/" name

constexpr const char *METAL       {FPATH("Metal")},
                     *QUARTZ_CORE {FPATH("QuartzCore")};

constexpr uint8_t SHADER_LIBRARY[] {
#  include "metal_shader.metal.ipp"
};

enum Buffers { VertexBuf, IndexBuf };

class MetalRenderer final : public Renderer {
public:
  MetalRenderer(RendererFactory *, Window *);
  ~MetalRenderer();

  void setSize(ImVec2) override;
  void render(void *) override;
  void swapBuffers(void *) override;

private:
  struct Shared {
    Shared();
    ~Shared();

    void textureCommand(const TextureCmd &);

    id<MTLDevice> m_device;
    id<MTLCommandQueue> m_commandQueue;
    id<MTLDepthStencilState> m_depthStencilState;
    id<MTLRenderPipelineState> m_renderPipelineState;

    TextureCookie m_cookie;
    std::vector<id<MTLTexture>> m_textures;
  };

  void resizeBuffer(size_t buf,
    unsigned int wantSize, unsigned int reserveExtra, unsigned int stride);

  std::shared_ptr<Shared> m_shared;
  bool m_firstFrame;
  CAMetalLayer *m_layer;
  MTLRenderPassDescriptor *m_renderPass;
  id<MTLBuffer> m_buffers[2]; 
};

static id<MTLFunction> getShaderFunc(id<MTLLibrary> library, NSString *name)
{
  id<MTLFunction> func;
  if(@available(macOS 10.12, *)) {
    NSError *error {};
    MTLFunctionConstantValues *cv {};
    func = [library newFunctionWithName:name constantValues:cv error:&error];
    if(error)
      throw backend_error {[[error localizedDescription] UTF8String]};
  }
  else
    func = [library newFunctionWithName:name];

  if(!func)
    throw backend_error {"failed to load shader library functions"};

  return func;
}

MetalRenderer::Shared::Shared()
{
  static FuncImport<decltype(MTLCreateSystemDefaultDevice)>
    _MTLCreateSystemDefaultDevice {METAL, "MTLCreateSystemDefaultDevice"};
  if(!_MTLCreateSystemDefaultDevice)
    throw backend_error {"Metal is not available on this device"};
  m_device = _MTLCreateSystemDefaultDevice();

  m_commandQueue = [m_device newCommandQueue];

  static ClassImport _MTLDepthStencilDescriptor
    {METAL, "MTLDepthStencilDescriptor"};
  if(!_MTLDepthStencilDescriptor)
    throw backend_error {"could not import MTLDepthStencilDescriptor"};
  MTLDepthStencilDescriptor *depthDesc {[_MTLDepthStencilDescriptor new]};
  depthDesc.depthWriteEnabled = NO;
  depthDesc.depthCompareFunction = MTLCompareFunctionAlways;
  m_depthStencilState = [m_device newDepthStencilStateWithDescriptor:depthDesc];

  NSError *error {};
  dispatch_data_t shaderData
    {dispatch_data_create(SHADER_LIBRARY, sizeof(SHADER_LIBRARY), nil, nil)};
  id<MTLLibrary> library {[m_device newLibraryWithData:shaderData error:&error]};
  if(error)
    throw backend_error {[[error localizedDescription] UTF8String]};
  id<MTLFunction> vertexFunc   {getShaderFunc(library, @"vertex_main")},
                  fragmentFunc {getShaderFunc(library, @"fragment_main")};

  static ClassImport _MTLVertexDescriptor {METAL, "MTLVertexDescriptor"};
  if(!_MTLVertexDescriptor)
    throw backend_error {"failed to import MTLVertexDescriptor"};
  MTLVertexDescriptor *vertexDesc = [_MTLVertexDescriptor vertexDescriptor];
  vertexDesc.attributes[0].offset = offsetof(ImDrawVert, pos);
  vertexDesc.attributes[0].format = MTLVertexFormatFloat2;
  vertexDesc.attributes[0].bufferIndex = 0;
  vertexDesc.attributes[1].offset = offsetof(ImDrawVert, uv);
  vertexDesc.attributes[1].format = MTLVertexFormatFloat2;
  vertexDesc.attributes[1].bufferIndex = 0;
  vertexDesc.attributes[2].offset = offsetof(ImDrawVert, col);
  vertexDesc.attributes[2].format = MTLVertexFormatUChar4;
  vertexDesc.attributes[2].bufferIndex = 0;
  vertexDesc.layouts[0].stepRate = 1;
  vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
  vertexDesc.layouts[0].stride = sizeof(ImDrawVert);

  static ClassImport _MTLRenderPipelineDescriptor
    {METAL, "MTLRenderPipelineDescriptor"};
  if(!_MTLRenderPipelineDescriptor)
    throw backend_error {"failed to import MTLRenderPipelineDescriptor"};
  MTLRenderPipelineDescriptor *pipelineDesc {[_MTLRenderPipelineDescriptor new]};
  pipelineDesc.vertexFunction   = vertexFunc;
  pipelineDesc.fragmentFunction = fragmentFunc;
  pipelineDesc.vertexDescriptor = vertexDesc;
  // pipelineDesc.sampleCount = drawable.texture.sampleCount;
  MTLRenderPipelineColorAttachmentDescriptor *colorDesc
    {pipelineDesc.colorAttachments[0]};
  // colorDesc.pixelFormat = drawable.texture.colorPixelFormat;
  colorDesc.pixelFormat                 = MTLPixelFormatBGRA8Unorm;
  colorDesc.blendingEnabled             = YES;
  colorDesc.rgbBlendOperation           = MTLBlendOperationAdd;
  colorDesc.sourceRGBBlendFactor        = MTLBlendFactorSourceAlpha;
  colorDesc.destinationRGBBlendFactor   = MTLBlendFactorOneMinusSourceAlpha;
  colorDesc.alphaBlendOperation         = MTLBlendOperationAdd;
  colorDesc.sourceAlphaBlendFactor      = MTLBlendFactorOne;
  colorDesc.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

  m_renderPipelineState =
    [m_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
  if(error)
    throw backend_error {[[error localizedDescription] UTF8String]};
}

MetalRenderer::Shared::~Shared()
{
}

void MetalRenderer::Shared::textureCommand(const TextureCmd &cmd)
{
  switch(cmd.type) {
  case TextureCmd::Insert:
    m_textures.insert(m_textures.begin() + cmd.offset, cmd.size, nil);
    break;
  case TextureCmd::Update:
    break;
  case TextureCmd::Remove:
    m_textures.erase(m_textures.begin() + cmd.offset,
                     m_textures.begin() + cmd.offset + cmd.size);
    return;
  }

  static ClassImport _MTLTextureDescriptor
    {METAL, "MTLTextureDescriptor"};
  if(!_MTLTextureDescriptor)
    throw backend_error {"failed to import MTLTextureDescriptor"};

  for(size_t i {}; i < cmd.size; ++i) {
    int width, height;
    const unsigned char *pixels {cmd[i].getPixels(&width, &height)};

    // [m_device maxTexture{Width,Height}2D] is private undocumented API
    constexpr int metalMaxSize {16384};
    if(width > metalMaxSize || height > metalMaxSize)
      throw backend_error("texture size is greater than Metal limits");

    MTLTextureDescriptor *texDesc =
      [_MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                          width:width
                                                         height:height
                                                      mipmapped:NO];
    texDesc.storageMode = MTLStorageModeManaged;
    texDesc.usage = MTLTextureUsageShaderRead;

    id<MTLTexture> texture {[m_device newTextureWithDescriptor:texDesc]};
    if(!texture)
      throw backend_error {"failed to create texture"};
    [texture replaceRegion:MTLRegionMake2D(0, 0, width, height)
               mipmapLevel:0
                 withBytes:pixels
               bytesPerRow:width * 4];
    m_textures[cmd.offset + i] = texture;
  }
}

MetalRenderer::MetalRenderer(RendererFactory *factory, Window *window)
  : Renderer {window}, m_firstFrame {true}
{
  m_shared = factory->getSharedData<Shared>();
  if(!m_shared) {
    m_shared = std::make_shared<Shared>();
    factory->setSharedData(m_shared);
  }

  static ClassImport _MTLRenderPassDescriptor {METAL, "MTLRenderPassDescriptor"};
  if(!_MTLRenderPassDescriptor)
    throw backend_error {"could not import MTLRenderPassDescriptor"};
  m_renderPass = [_MTLRenderPassDescriptor new];

  static ClassImport _CAMetalLayer {QUARTZ_CORE, "CAMetalLayer"};
  if(!_CAMetalLayer)
    throw backend_error {"could not import CAMetalLayer"};
  m_layer = static_cast<CAMetalLayer *>([_CAMetalLayer layer]);
  // don't stretch on resize before the next redraw (bottom = top)
  m_layer.contentsGravity = kCAGravityBottomLeft;
  m_layer.device = m_shared->m_device;
  m_layer.opaque = NO;
  m_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  NSView *view {(__bridge NSView *)window->nativeHandle()};
  view.layer = m_layer;
  view.wantsLayer = YES;
}

MetalRenderer::~MetalRenderer()
{
}

void MetalRenderer::resizeBuffer(const size_t buf, const unsigned int wantSize,
  const unsigned int reserveExtra, const unsigned int stride)
{
  if(m_buffers[buf] && [m_buffers[buf] length] >= wantSize * stride)
    return;

  const unsigned int size {(wantSize + reserveExtra) * stride};
  constexpr auto options
    {MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined};
  m_buffers[buf] = [m_shared->m_device newBufferWithLength:size options:options];
}

void MetalRenderer::setSize(const ImVec2 size)
{
  const float scale {m_window->viewport()->DpiScale};

  [CATransaction begin];
  [CATransaction setDisableActions:YES]; // disable animation when scale changes
  m_layer.drawableSize = CGSizeMake(size.x * scale, size.y * scale);
  m_layer.contentsScale = scale;
  [CATransaction commit];
}

void MetalRenderer::render(void *)
{
  using namespace std::placeholders;
  m_window->context()->textureManager()->update(&m_shared->m_cookie,
    std::bind(&Shared::textureCommand, m_shared.get(), _1));

  const ImGuiViewport *viewport {m_window->viewport()};
  const ImDrawData *drawData {viewport->DrawData};
  const ImVec2 position {drawData->DisplayPos},
               scale    {viewport->DpiScale, viewport->DpiScale};

  id<CAMetalDrawable> drawable {};
  if(m_firstFrame) {
    if(m_layer.contentsScale != scale.x)
      setSize(drawData->DisplaySize);
    m_firstFrame = false;
    drawable = [m_layer nextDrawable];
  }
  else {
    // [CAMetalLayer nextDrawable] waits up 1 second when a window becomes
    // completely hidden. NSWindowOcclusionStateVisible is always unset for the
    // first frame.
    NSWindow *window {[(__bridge NSView *)m_window->nativeHandle() window]};
    if(window.occlusionState & NSWindowOcclusionStateVisible)
      drawable = [m_layer nextDrawable];
  }

  if(!drawable) {
    // execute queued texture operations even when the window is occluded
    [[m_shared->m_commandQueue commandBuffer] commit];
    return;
  }

  resizeBuffer(VertexBuf, drawData->TotalVtxCount, 5000, sizeof(ImDrawVert));
  resizeBuffer(IndexBuf, drawData->TotalIdxCount, 10000, sizeof(ImDrawIdx));

  m_renderPass.colorAttachments[0].texture = drawable.texture;
  m_renderPass.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);
  if(viewport->Flags & ImGuiViewportFlags_NoRendererClear)
    m_renderPass.colorAttachments[0].loadAction = MTLLoadActionDontCare;
  else
    m_renderPass.colorAttachments[0].loadAction = MTLLoadActionClear;

  id<MTLCommandBuffer> commandBuffer {[m_shared->m_commandQueue commandBuffer]};
  id<MTLRenderCommandEncoder> commandEncoder
    {[commandBuffer renderCommandEncoderWithDescriptor:m_renderPass]};
  [commandEncoder setCullMode:MTLCullModeNone];
  [commandEncoder setDepthStencilState:m_shared->m_depthStencilState];
  [commandEncoder setRenderPipelineState:m_shared->m_renderPipelineState];
  [commandEncoder setViewport:MTLViewport {
    .originX = 0.0,
    .originY = 0.0,
    .width   = drawData->DisplaySize.x * scale.x,
    .height  = drawData->DisplaySize.y * scale.y,
    .znear   = 0.0,
    .zfar    = 1.0,
  }];

  const ProjMtx projMatrix {drawData->DisplayPos, drawData->DisplaySize};
  [commandEncoder setVertexBuffer:m_buffers[VertexBuf] offset:0 atIndex:0];
  [commandEncoder setVertexBytes:&projMatrix length:sizeof(ProjMtx) atIndex:1];

  size_t vtxOffset {}, idxOffset {};
  for(int i {}; i < drawData->CmdListsCount; ++i) {
    const ImDrawList *cmdList {drawData->CmdLists[i]};

    memcpy(static_cast<char *>(m_buffers[VertexBuf].contents) + vtxOffset,
      cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(static_cast<char *>(m_buffers[IndexBuf].contents) + idxOffset,
      cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

    for(int j {}; j < cmdList->CmdBuffer.Size; ++j) {
      const ImDrawCmd *cmd {&cmdList->CmdBuffer[j]};
      if(cmd->UserCallback)
        continue; // no need to call the callback, not using them

      const ClipRect clipRect {cmd->ClipRect, position, scale};
      if(!clipRect)
        continue;
      [commandEncoder setScissorRect:MTLScissorRect {
        .x      = static_cast<NSUInteger>(clipRect.left),
        .y      = static_cast<NSUInteger>(clipRect.top),
        .width  = static_cast<NSUInteger>(clipRect.right - clipRect.left),
        .height = static_cast<NSUInteger>(clipRect.bottom - clipRect.top),
      }];

      [commandEncoder setFragmentTexture:m_shared->m_textures[cmd->GetTexID()] atIndex:0];
      [commandEncoder setVertexBufferOffset:vtxOffset + (cmd->VtxOffset * sizeof(ImDrawVert)) atIndex:0];
      [commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                 indexCount:cmd->ElemCount
                                  indexType:sizeof(ImDrawIdx) == 2 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32
                                indexBuffer:m_buffers[IndexBuf]
                          indexBufferOffset:idxOffset + (cmd->IdxOffset * sizeof(ImDrawIdx))];
    }

    vtxOffset += cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
    idxOffset += cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);
  }

  [commandEncoder endEncoding];
  // equivalent to [commandBuffer presentDrawable:drawable]; without slowing
  // down when presenting multiple windows sharing the same command queue
  [commandBuffer addScheduledHandler:^(id<MTLCommandBuffer>) { [drawable present]; }];
  [commandBuffer commit];
}

void MetalRenderer::swapBuffers(void *)
{
}
