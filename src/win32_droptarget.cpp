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

#include "win32_droptarget.hpp"

#include "context.hpp"

DropTarget::DropTarget(Context *ctx)
  : m_ctx {ctx}, m_refCount {0}
{
}

HRESULT DropTarget::QueryInterface(REFIID riid, void **object)
{
  if(riid == IID_IUnknown || riid == IID_IDropTarget) {
    *object = this;
    AddRef();
    return S_OK;
  }

  *object = nullptr;
  return E_NOINTERFACE;
}

ULONG DropTarget::AddRef()
{
  return InterlockedIncrement(&m_refCount);
}

ULONG DropTarget::Release()
{
  const LONG refCount {InterlockedDecrement(&m_refCount)};
  if(refCount == 0)
    delete this;
  return refCount;
}

HRESULT DropTarget::DragEnter(IDataObject *dataObj, DWORD, POINTL, DWORD *effect)
{
  FORMATETC format {CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  STGMEDIUM medium;

  if(SUCCEEDED(dataObj->GetData(&format, &medium))) {
    m_ctx->beginDrag(reinterpret_cast<HDROP>(medium.hGlobal));
    ReleaseStgMedium(&medium);
    *effect = DROPEFFECT_COPY;
  }
  else
    *effect = DROPEFFECT_NONE;

  return S_OK;
}

HRESULT DropTarget::DragLeave()
{
  m_ctx->endDrag(false);
  return S_OK;
}

HRESULT DropTarget::Drop(IDataObject *, DWORD, POINTL, DWORD *)
{
  m_ctx->endDrag(true);
  return S_OK;
}
