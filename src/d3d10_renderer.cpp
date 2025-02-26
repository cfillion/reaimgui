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

#include <atlbase.h>
#include <d3d10.h>
#include <imgui/imgui.h>
#include <vector>

class D3D10Renderer;
REGISTER_RENDERER(10, d3d10, "Direct3D 10", &Renderer::create<D3D10Renderer>,
  RendererType::Available | RendererType::CanForceSoftware);

constexpr uint8_t VERTEX_SHADER[] {
#  include "d3d10_vertex.hlsl.ipp"
};

constexpr uint8_t PIXEL_SHADER[] {
#  include "d3d10_pixel.hlsl.ipp"
};

enum Buffers { ConstantBuf, VertexBuf, IndexBuf, };

class D3D10Renderer final : public Renderer {
public:
  D3D10Renderer(RendererFactory *, Window *);
  ~D3D10Renderer();

  void setSize(ImVec2) override;
  void render(void *) override;
  void swapBuffers(void *) override;

private:
  struct Shared {
    Shared(bool forceSoftware);
    ~Shared();

    void textureCommand(const TextureCmd &);

    CComPtr<ID3D10Device> m_device;
    CComPtr<IDXGIFactory> m_factory;
    CComPtr<ID3D10VertexShader> m_vertexShader;
    CComPtr<ID3D10PixelShader> m_pixelShader;
    CComPtr<ID3D10InputLayout> m_inputLayout;
    CComPtr<ID3D10BlendState> m_blendState;
    CComPtr<ID3D10RasterizerState> m_rasterizerState;
    CComPtr<ID3D10DepthStencilState> m_depthStencilState;
    CComPtr<ID3D10SamplerState> m_samplerState;

    TextureCookie m_cookie;
    std::vector<CComPtr<ID3D10ShaderResourceView>> m_textures;
  };

  struct Buffer {
    CComPtr<ID3D10Buffer> ptr;
    unsigned int size;

    operator ID3D10Buffer*()   const { return ptr; }
    ID3D10Buffer *operator->() const { return ptr; }
    ID3D10Buffer **operator&() { return &ptr; }
  };

  void createRenderTarget();
  bool setupBuffer(Buffer &, unsigned int wantSize,
    unsigned int reserveExtra, unsigned int stride, unsigned int bindFlags);

  std::shared_ptr<Shared> m_shared;
  CComPtr<IDXGISwapChain> m_swapChain;
  CComPtr<ID3D10RenderTargetView> m_renderTarget;
  std::array<Buffer, 3> m_buffers;
};


D3D10Renderer::Shared::Shared(const bool forceSoftware)
{
  static FuncImport<decltype(D3D10CreateDevice)>
    _D3D10CreateDevice {L"D3D10", "D3D10CreateDevice"};
  if(!_D3D10CreateDevice)
    throw backend_error {"Direct3D 10 is not installed on this system"};
  const auto createDevice = [&](const D3D10_DRIVER_TYPE driver) {
    return SUCCEEDED(_D3D10CreateDevice(
      nullptr, driver, nullptr, 0, D3D10_SDK_VERSION, &m_device));
  };
  if(forceSoftware || !createDevice(D3D10_DRIVER_TYPE_HARDWARE))
    createDevice(D3D10_DRIVER_TYPE_WARP); // software rasterizer
  if(!m_device)
    throw backend_error {"failed to create Direct3D 10 device"};

  {
    CComPtr<IDXGIDevice> dxgiDevice;
    CComPtr<IDXGIAdapter> dxgiAdapter;
    if(FAILED(m_device->QueryInterface(IID_PPV_ARGS(&dxgiDevice))))
      throw backend_error {"failed to get DXGI device"};
    if(FAILED(dxgiDevice->GetParent(IID_PPV_ARGS(&dxgiAdapter))))
      throw backend_error {"failed to get DXGI adapter"};
    if(FAILED(dxgiAdapter->GetParent(IID_PPV_ARGS(&m_factory))))
      throw backend_error {"failed to get DXGI factory"};
  }

  if(FAILED(m_device->CreateVertexShader(VERTEX_SHADER, sizeof(VERTEX_SHADER),
                                         &m_vertexShader)))
    throw backend_error {"failed to create vertex shader"};
  m_device->VSSetShader(m_vertexShader);

  if(FAILED(m_device->CreatePixelShader(PIXEL_SHADER, sizeof(PIXEL_SHADER),
                                        &m_pixelShader)))
    throw backend_error {"failed to create pixel shader"};
  m_device->PSSetShader(m_pixelShader);

  constexpr D3D10_INPUT_ELEMENT_DESC vertexInputs[] {
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0,
      offsetof(ImDrawVert, pos), D3D10_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0,
      offsetof(ImDrawVert, uv),  D3D10_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0,
      offsetof(ImDrawVert, col), D3D10_INPUT_PER_VERTEX_DATA, 0},
  };
  if(FAILED(m_device->CreateInputLayout(vertexInputs, std::size(vertexInputs),
                                        VERTEX_SHADER, sizeof(VERTEX_SHADER),
                                        &m_inputLayout)))
    throw backend_error {"failed to create layout of vertex shader inputs"};
  m_device->IASetInputLayout(m_inputLayout);
  m_device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  constexpr D3D10_BLEND_DESC blendDesc {
    .AlphaToCoverageEnable = false,
    .BlendEnable           = {true},
    .SrcBlend              = D3D10_BLEND_SRC_ALPHA,
    .DestBlend             = D3D10_BLEND_INV_SRC_ALPHA,
    .BlendOp               = D3D10_BLEND_OP_ADD,
    .SrcBlendAlpha         = D3D10_BLEND_ONE,
    .DestBlendAlpha        = D3D10_BLEND_INV_SRC_ALPHA,
    .BlendOpAlpha          = D3D10_BLEND_OP_ADD,
    .RenderTargetWriteMask = {D3D10_COLOR_WRITE_ENABLE_ALL},
  };
  if(FAILED(m_device->CreateBlendState(&blendDesc, &m_blendState)))
    throw backend_error {"failed to create blend state"};
  constexpr float blendFactor[] {0.f, 0.f, 0.f, 0.f};
  m_device->OMSetBlendState(m_blendState, blendFactor, 0xffffffff);

  constexpr D3D10_RASTERIZER_DESC rasterizerDesc {
    .FillMode        = D3D10_FILL_SOLID,
    .CullMode        = D3D10_CULL_NONE,
    .DepthClipEnable = true,
    .ScissorEnable   = true,
  };
  if(FAILED(m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState)))
    throw backend_error {"failed to create rasterizer state"};
  m_device->RSSetState(m_rasterizerState);

  constexpr D3D10_DEPTH_STENCILOP_DESC faceDesc {
    .StencilFailOp      = D3D10_STENCIL_OP_KEEP,
    .StencilDepthFailOp = D3D10_STENCIL_OP_KEEP,
    .StencilPassOp      = D3D10_STENCIL_OP_KEEP,
    .StencilFunc        = D3D10_COMPARISON_ALWAYS,
  };
  constexpr D3D10_DEPTH_STENCIL_DESC depthStencilDesc {
    .DepthEnable    = false,
    .DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL,
    .DepthFunc      = D3D10_COMPARISON_ALWAYS,
    .StencilEnable  = false,
    .FrontFace      = faceDesc,
    .BackFace       = faceDesc,
  };
  if(FAILED(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState)))
    throw backend_error {"failed to create depth stencil state"};
  m_device->OMSetDepthStencilState(m_depthStencilState, 0);

  constexpr D3D10_SAMPLER_DESC samplerDesc {
    .Filter         = D3D10_FILTER_MIN_MAG_MIP_LINEAR,
    .AddressU       = D3D10_TEXTURE_ADDRESS_WRAP,
    .AddressV       = D3D10_TEXTURE_ADDRESS_WRAP,
    .AddressW       = D3D10_TEXTURE_ADDRESS_WRAP,
    .ComparisonFunc = D3D10_COMPARISON_ALWAYS,
  };
  if(FAILED(m_device->CreateSamplerState(&samplerDesc, &m_samplerState)))
    throw backend_error {"failed to create sampler state"};
  m_device->PSSetSamplers(0, 1, &m_samplerState.p);
}

D3D10Renderer::Shared::~Shared()
{
}

void D3D10Renderer::Shared::textureCommand(const TextureCmd &cmd)
{
  switch(cmd.type) {
  case TextureCmd::Insert:
    m_textures.insert(m_textures.begin() + cmd.offset, cmd.size, nullptr);
    break;
  case TextureCmd::Update:
    // calls Release() on the textures in the range to replace
    std::fill_n(m_textures.begin() + cmd.offset, cmd.size, nullptr);
    break;
  case TextureCmd::Remove:
    m_textures.erase(m_textures.begin() + cmd.offset,
                     m_textures.begin() + cmd.offset + cmd.size);
    return;
  }

  for(size_t i {}; i < cmd.size; ++i) {
    int width, height;
    const unsigned char *pixels {cmd[i].getPixels(&width, &height)};

    CComPtr<ID3D10Texture2D> texture;
    const D3D10_TEXTURE2D_DESC textureDesc {
      .Width = static_cast<unsigned int>(width),
      .Height = static_cast<unsigned int>(height),
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .SampleDesc = {.Count = 1},
      .Usage = D3D10_USAGE_DEFAULT,
      .BindFlags = D3D10_BIND_SHADER_RESOURCE,
    };
    const D3D10_SUBRESOURCE_DATA subResourceDesc {
      .pSysMem = pixels,
      .SysMemPitch = textureDesc.Width * 4,
    };
    if(FAILED(m_device->CreateTexture2D(&textureDesc, &subResourceDesc, &texture)))
      throw backend_error {"failed to create texture"};

    const D3D10_SHADER_RESOURCE_VIEW_DESC resourceViewDesc {
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D,
      .Texture2D = {.MipLevels = textureDesc.MipLevels},
    };
    m_device->CreateShaderResourceView(texture,
      &resourceViewDesc, &m_textures[cmd.offset + i]);
  }
}

D3D10Renderer::D3D10Renderer(RendererFactory *factory, Window *window)
  : Renderer {window}
{
  m_shared = factory->getSharedData<Shared>();
  if(!m_shared) {
    m_shared = std::make_shared<Shared>(factory->wantSoftware());
    factory->setSharedData(m_shared);
  }

  DXGI_SWAP_CHAIN_DESC swapChainDesc {
    .BufferDesc = {
      .RefreshRate = {.Numerator = 60, .Denominator = 1},
      .Format      = DXGI_FORMAT_R8G8B8A8_UNORM,
    },
    .SampleDesc   = {.Count = 1, .Quality = 0},
    .BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount  = 1,
    .OutputWindow = window->nativeHandle(),
    .Windowed     = true,
    .SwapEffect   = DXGI_SWAP_EFFECT_DISCARD,
    .Flags        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,
  };
  if(FAILED(m_shared->m_factory->CreateSwapChain(m_shared->m_device, &swapChainDesc, &m_swapChain)))
    throw backend_error {"failed to create DXGI swap chain"};

  // disable Alt+Enter enabling fullscreen mode
  m_shared->m_factory->MakeWindowAssociation(window->nativeHandle(),
    DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

  createRenderTarget();

  if(!setupBuffer(m_buffers[ConstantBuf], 1, 0, sizeof(ProjMtx),
                  D3D10_BIND_CONSTANT_BUFFER))
    throw backend_error {"failed to create vertex constant buffer"};
}

D3D10Renderer::~D3D10Renderer()
{
}

void D3D10Renderer::createRenderTarget()
{
  CComPtr<ID3D10Texture2D> buffer;
  if(FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&buffer))))
    throw backend_error {"failed to get swap chain buffer"};
  if(FAILED(m_shared->m_device->CreateRenderTargetView(buffer, nullptr, &m_renderTarget)))
    throw backend_error {"failed to create render target view"};
}

bool D3D10Renderer::setupBuffer(Buffer &buffer, const unsigned int wantSize,
  const unsigned int reserveExtra, const unsigned int stride,
  const unsigned int bindFlags)
{
  if(buffer.ptr && buffer.size >= wantSize)
    return true;

  buffer.ptr = nullptr; // calls Release()

  const unsigned int newSize {wantSize + reserveExtra};
  const D3D10_BUFFER_DESC bufferDesc {
    .ByteWidth      = newSize * stride,
    .Usage          = D3D10_USAGE_DYNAMIC,
    .BindFlags      = bindFlags,
    .CPUAccessFlags = D3D10_CPU_ACCESS_WRITE,
  };
  if(FAILED(m_shared->m_device->CreateBuffer(&bufferDesc, nullptr, &buffer.ptr)))
    return false;
  buffer.size = newSize;

  return true;
}

void D3D10Renderer::setSize(const ImVec2 size)
{
  m_shared->m_device->OMSetRenderTargets(0, nullptr, nullptr);
  m_renderTarget = nullptr; // before resizing the swap chain buffer

  const float scale {m_window->viewport()->DpiScale};
  if(FAILED(m_swapChain->ResizeBuffers(0,
            size.x * scale, size.y * scale, DXGI_FORMAT_UNKNOWN, 0)))
    throw backend_error {"failed to resize swap chain buffer"};

  createRenderTarget();
}

void D3D10Renderer::render(void *)
{
  using namespace std::placeholders;
  m_window->context()->textureManager()->update(&m_shared->m_cookie,
    std::bind(&Shared::textureCommand, m_shared.get(), _1));

  const ImGuiViewport *viewport {m_window->viewport()};
  const ImDrawData *drawData {viewport->DrawData};

  ID3D10Device *device {m_shared->m_device};
  device->OMSetRenderTargets(1, &m_renderTarget.p, nullptr);

  if(!(viewport->Flags & ImGuiViewportFlags_NoRendererClear)) {
    constexpr float clearColor[] {0.f, 0.f, 0.f, 0.f};
    device->ClearRenderTargetView(m_renderTarget, clearColor);
  }

  const D3D10_VIEWPORT viewportDesc {
    .Width  =
      static_cast<unsigned int>(drawData->DisplaySize.x * viewport->DpiScale),
    .Height =
      static_cast<unsigned int>(drawData->DisplaySize.y * viewport->DpiScale),
    .MinDepth = 0.0f, .MaxDepth = 1.0f,
  };
  device->RSSetViewports(1, &viewportDesc);

  if(!setupBuffer(m_buffers[VertexBuf],
                  drawData->TotalVtxCount, 5000, sizeof(ImDrawVert),
                  D3D10_BIND_VERTEX_BUFFER))
    return;
  if(!setupBuffer(m_buffers[IndexBuf],
                  drawData->TotalIdxCount, 10000, sizeof(ImDrawIdx),
                  D3D10_BIND_INDEX_BUFFER))
    return;

  // copy data to the vertex/index buffers
  ImDrawVert *vertexData;
  ImDrawIdx *indexData;
  if(FAILED(m_buffers[VertexBuf]->Map(
      D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast<void **>(&vertexData))))
    return;
  if(FAILED(m_buffers[IndexBuf]->Map(
      D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast<void **>(&indexData))))
    return;
  for(int i {}; i < drawData->CmdListsCount; ++i) {
    const ImDrawList *cmdList {drawData->CmdLists[i]};
    memcpy(vertexData, cmdList->VtxBuffer.Data,
           cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(indexData, cmdList->IdxBuffer.Data,
           cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
    vertexData += cmdList->VtxBuffer.Size;
    indexData += cmdList->IdxBuffer.Size;
  }
  m_buffers[VertexBuf]->Unmap();
  m_buffers[IndexBuf]->Unmap();

  const ProjMtx projMatrix {drawData->DisplayPos, drawData->DisplaySize};
  void *constData;
  if(FAILED(m_buffers[ConstantBuf]->Map(D3D10_MAP_WRITE_DISCARD, 0, &constData)))
    return;
  memcpy(constData, &projMatrix, sizeof(ProjMtx));
  m_buffers[ConstantBuf]->Unmap();

  const unsigned int stride {sizeof(ImDrawVert)}, offset {};
  device->VSSetConstantBuffers(0, 1, &m_buffers[ConstantBuf].ptr.p);
  device->IASetVertexBuffers(0, 1, &m_buffers[VertexBuf].ptr.p, &stride, &offset);
  device->IASetIndexBuffer(m_buffers[IndexBuf],
    sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);

  const ImVec2 &clipOffset {drawData->DisplayPos},
               &clipScale  {viewport->DpiScale, viewport->DpiScale};
  int globalVtxOffset {}, globalIdxOffset {};
  for(int i {}; i < drawData->CmdListsCount; ++i) {
    const ImDrawList *cmdList {drawData->CmdLists[i]};
    for(int j {}; j < cmdList->CmdBuffer.Size; ++j) {
      const ImDrawCmd *cmd {&cmdList->CmdBuffer[j]};
      if(cmd->UserCallback)
        continue; // no need to call the callback, not using them

      const ClipRect clipRect {cmd->ClipRect, clipOffset, clipScale};
      static_assert(sizeof(ClipRect) == sizeof(D3D10_RECT));
      if(!clipRect)
        continue;
      device->RSSetScissorRects(1, reinterpret_cast<const D3D10_RECT *>(&clipRect));

      ID3D10ShaderResourceView *texture {m_shared->m_textures[cmd->GetTexID()]};
      device->PSSetShaderResources(0, 1, &texture);
      device->DrawIndexed(cmd->ElemCount, cmd->IdxOffset + globalIdxOffset,
                                          cmd->VtxOffset + globalVtxOffset);
    }

    globalVtxOffset += cmdList->VtxBuffer.Size;
    globalIdxOffset += cmdList->IdxBuffer.Size;
  }
}

void D3D10Renderer::swapBuffers(void *)
{
  m_swapChain->Present(0, 0); // present immediately (no vsync)
}
