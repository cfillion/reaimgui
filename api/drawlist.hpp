#ifndef REAIMGUI_DRAWLIST_HPP
#define REAIMGUI_DRAWLIST_HPP

#include "resource.hpp"

struct ImGui_DrawList;

class DrawListSplitter : public Resource {
public:
  static constexpr const char *api_type_name { "ImGui_DrawListSplitter" };

  DrawListSplitter(ImGui_DrawList *);
  ImDrawList *drawList() const;
  ImDrawListSplitter *operator->();

protected:
  bool isValid() const override;

private:
  ImGui_DrawList *m_drawlist;
  ImDrawListSplitter m_splitter;
};

using ImGui_DrawListSplitter = DrawListSplitter;

#endif
