#ifndef REAIMGUI_WIN32_HPP
#define REAIMGUI_WIN32_HPP

#ifdef _WIN32

#include <string>

namespace Win32 {

class Class {
public:
  Class(const wchar_t *name, WNDPROC proc)
    : m_name { name }
  {
    WNDCLASS wc {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = proc;
    wc.hInstance = Context::s_instance;
    wc.lpszClassName = m_name;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClass(&wc);
  }

  ~Class()
  {
    UnregisterClass(m_name, Context::s_instance);
  }

  const wchar_t *name() const { return m_name; }

private:
  const wchar_t *m_name;
};

inline std::wstring widen(const char *input, const UINT codepage = CP_UTF8)
{
  const int size = MultiByteToWideChar(codepage, 0, input, -1, nullptr, 0) - 1;

  std::wstring output(size, 0);
  MultiByteToWideChar(codepage, 0, input, -1, &output[0], size);

  return output;
}

}

#endif // _WIN32
#endif
