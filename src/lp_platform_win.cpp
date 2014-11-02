#include "lp_platform.h"
#include "lp_constants.h"
#include "lp_resource.h"
#include "lp_utils.h"
#include "lpopup.h"

#if defined LP_OS_WIN64
#  define LP_GWL_WNDPROC GWLP_WNDPROC
#  define SetWindowLongPtrW64( w, i, l ) \
                SetWindowLongPtrW( w, i, (LONG_PTR)(l) )
#  define GetWindowLongPtrW64( w, i) \
                GetWindowLongPtrW( w, i)
#else
#  define LP_GWL_WNDPROC GWL_WNDPROC
#  define SetWindowLongPtrW64( w, i, l ) \
                SetWindowLongPtrW( w, i, (LONG)(l) )
#  define GetWindowLongPtrW64( w, i) \
                GetWindowLongPtrW( w, i)
#endif

#define WM_SET_KILL_TIMER (WM_APP+10)

static const int TIMER_STATE_NONE = 0;
static const int TIMER_STATE_CANCELED = -1;
static const int TIMER_STATE_ACTIVATED = 1;

static HINSTANCE                              g_hinst;
static HWND                                   g_hwnd;
static HWND                                   g_caller_hwnd;
static unsigned int                           g_menu_seq = 400;
static std::unordered_map<int, HMENU>         g_menu_map;
static std::unordered_map<int, lp::menu_func> g_menu_cb_map;
static std::unordered_map<int, void*>         g_menu_cb_data_map;
static int                                    g_vbreak;
static HMENU                                  g_context_hmenu;
static IContextMenu                          *g_context_icontext;

static void lp_platform_split_cmdline(lp::oschar *cmdline, lp::oschar **args) { // {{{
  int    state = 0;
  size_t pos = 0;
  size_t len = _tcslen(cmdline);
  while(pos < len) {
    lp::oschar ch = cmdline[pos];
    if(state == 0) {
      if(ch == L'"') {
        state = 1;
      } else if(!iswspace(ch)) {
        state = 2;
      }
    }else if(state == 1) {
      if(ch == L'"' && cmdline[pos-1] != L'\\' && pos != (len-1)) {
        pos += 1;
        while(pos < len && iswspace(cmdline[pos])) pos++; 
        *args = cmdline + pos;
        return;
      }
    }else if(state == 2) {
      if(iswspace(ch)) {
        while(pos < len && iswspace(cmdline[pos])) pos++; 
        *args = cmdline + pos;
        return;
      }
    }
    pos += 1;
  }
  *args = NULL;
} // }}}

static char* lp_platform_get_last_error_message(){ // {{{
  void* msg_buf;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
    (LPTSTR) &msg_buf, 0, NULL);
  char *ret = lp::platform::oschar2utf8((const lp::oschar*)msg_buf);
  ret[strlen(ret)-2] = '\0';
  LocalFree(msg_buf);
  return ret;
} // }}}

static void lp_platform_set_error(lp::Error &error){ // {{{
  error.setCode(GetLastError());
  lp::unique_char_ptr msg(lp_platform_get_last_error_message());
  error.setMessage(msg.get());
} // }}}

static void lp_platform_add_menu_item(HMENU hmenu, int id, const char *text, HMENU subhmenu, HBITMAP hbmp) { // {{{
    MENUITEMINFO mii = {0};
    LP_OSSTRV(ostext, text);
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_ID|MIIM_STRING|MIIM_FTYPE;
    mii.wID   = (UINT)id;
    if(text != NULL) {
      mii.fType = MFT_STRING;
      mii.dwTypeData = ostext.get();
      if(g_vbreak) {
        mii.fType |= MFT_MENUBARBREAK;
        g_vbreak = 0;
      }
    }else {
      mii.fType = MFT_SEPARATOR;
    }
    if(subhmenu != 0) {
      mii.hSubMenu = subhmenu;
      mii.fMask |= MIIM_SUBMENU;
    }
    if (hbmp != NULL) {
      mii.fMask |= MIIM_BITMAP;
      mii.hbmpItem = hbmp;
    }
    InsertMenuItem(hmenu, id, true, &mii);
} // }}}

static HBITMAP lp_platform_get_icon(const char *path, const int size){ // {{{
   SHFILEINFO shinfo;
   HDC hdc1 = 0;
   HDC hdc2 = 0;
   HBITMAP hbmp1 = 0;
   HBITMAP hbmp2 = 0;
   HICON hicon = 0;
   LPVOID bits;
   BITMAPINFO bmi;

   LP_OSSTRV(ospath, path);

   int special_folder = -1;
   if(_tcscmp(ospath.get(), L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}") == 0){
     special_folder = CSIDL_DRIVES;
   }else if(_tcscmp(ospath.get(), L"::{208D2C60-3AEA-1069-A2D7-08002B30309D}") == 0){
     special_folder = CSIDL_NETWORK;
   }else if(_tcscmp(ospath.get(), L"::{645FF040-5081-101B-9F08-00AA002F954E}") == 0){
     special_folder = CSIDL_BITBUCKET;
   }else if(_tcscmp(ospath.get(), L"::{450D8FBA-AD25-11D0-98A8-0800361B1103}") == 0){
     special_folder = CSIDL_PERSONAL;
   }else if(_tcscmp(ospath.get(), L"::{2227A280-3AEA-1069-A2DE-08002B30309D}") == 0){
     special_folder = CSIDL_PRINTERS;
   }

   const int size_flag = size > 16 ? SHGFI_LARGEICON : SHGFI_SMALLICON;

   if(special_folder > -1){
     LPITEMIDLIST pidl;
     if(SHGetSpecialFolderLocation(NULL, special_folder, &pidl) != S_OK) {
       goto label_finalize;
     }
     if(!SHGetFileInfo((LPCWSTR)pidl, 0, &shinfo, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_ICON | size_flag)) {
       goto label_finalize;
     }
   }else{
     const int attr = PathIsDirectory(ospath.get()) ? FILE_ATTRIBUTE_DIRECTORY
                                                    : FILE_ATTRIBUTE_NORMAL;
     const int flags = SHGFI_ICON | size_flag | SHGFI_USEFILEATTRIBUTES;
     if(!SHGetFileInfo(ospath.get(), attr, &shinfo, sizeof(SHFILEINFO), flags)) {
       if(ExtractIconEx(ospath.get(), 0, &hicon, 0, 1) == 0) {
         goto label_finalize;
       }
     }
   }
   if(hicon == 0) hicon = shinfo.hIcon;
   hdc1 = GetDC(0);
   memset(&bmi, 0, sizeof(BITMAPINFO));
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = size;
   bmi.bmiHeader.biHeight = size;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 32;

   hbmp1 = CreateDIBSection(NULL, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, &bits, NULL, 0);
   hdc2 = CreateCompatibleDC(hdc1);
   hbmp2 = (HBITMAP)SelectObject(hdc2, hbmp1);
   DrawIconEx(hdc2, 0, 0, hicon, size, size, 0, 0, DI_NORMAL); 
   SelectObject(hdc2, hbmp2);

label_finalize:
   if(hbmp2 != 0){ DeleteObject(hbmp2); }
   if(hicon != 0){ DestroyIcon(hicon); }
   if(hdc1 != 0){ DeleteDC(hdc1); }
   if(hdc2 != 0){ DeleteDC(hdc2); }
   return hbmp1;
} /* }}} */

lp::oschar* lp::platform::utf82oschar(const char *src) { // {{{
  int size = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
  size++;
  lp::oschar *buff = new lp::oschar[size];
  size = MultiByteToWideChar(CP_UTF8, 0, src, -1, buff, size);
  buff[size] = 0;
  return buff;
} // }}}

void lp::platform::utf82oschar_b(lp::oschar *buf, const unsigned int bufsize, const char *src) { // {{{
  int size = MultiByteToWideChar(CP_UTF8, 0, src, -1, buf, bufsize);
  buf[size] = 0;
} // }}}

char* lp::platform::oschar2utf8(const lp::oschar *src) { // {{{
  unsigned int size = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
  size++;
  char *buff = new char[size];
  size = WideCharToMultiByte(CP_UTF8, 0, src, -1, buff, size, NULL, NULL);
  buff[size] = 0;
  return buff;
} // }}}

void lp::platform::oschar2utf8_b(char *buf, const unsigned int bufsize, const lp::oschar *src) { // {{{
  int size = WideCharToMultiByte(CP_UTF8, 0, src, -1, buf, bufsize, NULL, NULL);
  buf[size] = 0;
} // }}}

char* lp::platform::oschar2local(const lp::oschar *src) { // {{{
  int size = WideCharToMultiByte(GetACP(), 0, src, -1, NULL, 0, NULL, NULL);
  size++;
  char *buff = new char[size];
  size = WideCharToMultiByte(GetACP(), 0, src, -1, buff, size, NULL, NULL);
  buff[size] = 0;
  return buff;
} // }}}

void lp::platform::oschar2local_b(char *buf, const unsigned int bufsize, const lp::oschar *src) { // {{{
  int size = WideCharToMultiByte(GetACP(), 0, src, -1, buf, bufsize, NULL, NULL);
  buf[size] = 0;
} // }}}

char* lp::platform::utf82local(const char *src) { // {{{
  lp::unique_oschar_ptr osstring(lp::platform::utf82oschar(src));
  return lp::platform::oschar2local(osstring.get());
} // }}}

char* lp::platform::local2utf8(const char *src, int sz) { // {{{
  int size = MultiByteToWideChar(GetACP(), 0, src, sz ? sz : -1, NULL, 0);
  size++;
  lp::oschar *buff = new lp::oschar[size];
  size = MultiByteToWideChar(GetACP(), 0, src, sz ? sz : -1, buff, size);
  buff[size] = '\0';
  char *ret = lp::platform::oschar2utf8(buff);
  delete[] buff;
  return ret;
} // }}}

LRESULT CALLBACK lp_platform_winproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){ // {{{
  const unsigned int APP_EXIT = 10;
  static int timer_state = TIMER_STATE_NONE;

  switch(uMsg) {
    case WM_CREATE : {
        g_menu_map[0] = CreatePopupMenu();
        SetWindowLongPtrW64(hWnd, GWL_EXSTYLE,(GetWindowLongPtrW64(hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW) & ~WS_EX_APPWINDOW);
      }
      break;
killtimer:
    case WM_SET_KILL_TIMER : {
        timer_state = TIMER_STATE_ACTIVATED;
        KillTimer(hWnd , APP_EXIT);
        SetTimer(hWnd, APP_EXIT, 500, NULL);
      }
      break;
    case WM_TIMER: {
        if(wParam == APP_EXIT){
          KillTimer(hWnd , APP_EXIT);
          if(timer_state != TIMER_STATE_CANCELED){
            lp::utils::exit_application(0);
          }
          timer_state = TIMER_STATE_NONE;
        }
      }
      break;
    case WM_COMMAND: {
        if(g_menu_cb_map.find(LOWORD(wParam)) != g_menu_cb_map.end()) {
          timer_state = TIMER_STATE_CANCELED;
          g_menu_cb_map[LOWORD(wParam)](g_menu_cb_data_map[LOWORD(wParam)]);
          goto killtimer;
        }
      }
      break;

    case WM_UNINITMENUPOPUP: {
        if((HMENU)wParam == g_menu_map[0]) {
          goto killtimer;
        }
      }
      break;
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
} // }}}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) { // {{{
  g_hinst = hInstance;
  char **ar = (char**) malloc(sizeof(char*) * (__argc + 1));
  int l;
  for(int i = 0; i <  __argc; i++){
    for (l = 0; __argv[i] && __argv[i][l]; l++) {};
    ar[i] = lp::platform::local2utf8(__argv[i], l);
  }
  ar[__argc] = 0;
  int ret = _main(__argc, ar);
  for(int i = 0; i <  __argc; i++){
    delete[] ar[i];
  }
  return ret;
}
// }}}

void lp::platform::new_menu(int id) { // {{{
  // root menu
  if(id == 0) return;
  HMENU subhmenu = CreatePopupMenu();
  g_menu_map[id] = subhmenu;
} // }}}

void lp::platform::add_item(int id, const char *text, const char *icon, lp::menu_func callback, void *data) { // {{{
  if(g_menu_map.find(id) == g_menu_map.end()){
    return;
  }
  HMENU hmenu = g_menu_map[id];
  g_menu_seq++;
  g_menu_cb_map[g_menu_seq] = callback;
  g_menu_cb_data_map[g_menu_seq] = data;
  HBITMAP hbmp = 0;
  if(icon != 0 && strlen(icon) > 0) {
    hbmp = lp_platform_get_icon(icon, 16);
  }
  lp_platform_add_menu_item(hmenu, g_menu_seq, text, 0, hbmp);
  //TODO DeleteObject(hbmp)
} // }}}

void lp::platform::add_submenu(int id, int otherid, const char *text, const char *icon) { // {{{
  if(g_menu_map.find(id) == g_menu_map.end() || 
     g_menu_map.find(otherid) == g_menu_map.end()) {
    return;
  }
  HMENU hmenu = g_menu_map[id];
  HMENU subhmenu = g_menu_map[otherid];
  HBITMAP hbmp = 0;
  if(icon != 0 && strlen(icon) > 0) {
    hbmp = lp_platform_get_icon(icon, 16);
  }

  g_menu_seq++;
  lp_platform_add_menu_item(hmenu, g_menu_seq, text, subhmenu, hbmp);
} // }}}

void lp::platform::add_hseparator(int id) { // {{{
  if(g_menu_map.find(id) == g_menu_map.end()){
    return;
  }
  HMENU hmenu = g_menu_map[id];
  g_menu_seq++;
  lp_platform_add_menu_item(hmenu, g_menu_seq, NULL, 0, 0);
} // }}}

void lp::platform::add_vseparator(int id) { // {{{
  if(g_menu_map.find(id) == g_menu_map.end()){
    return;
  }
  g_vbreak = 1;
} // }}}

void lp::platform::add_file_context(int id, const char *text, const char *path, const char *icon) { // {{{
  if(g_menu_map.find(id) == g_menu_map.end()){
    return;
  }
  HMENU hmenu = g_menu_map[id];
  LP_OSSTRV(ospath, path);

  IShellFolder        *shell_folder = NULL;
  ITEMIDLIST          *childlist;
  ITEMIDLIST          *abslist;
  HBITMAP hbmp = 0;

  abslist = ILCreateFromPath(ospath.get());
  if(abslist == 0) {
    goto finalize;
  }
  SHBindToParent(abslist, IID_IShellFolder, (void **)(&shell_folder), NULL);
  childlist = ILFindLastID(abslist);

  if(shell_folder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&childlist, IID_IContextMenu, NULL, (void **)&g_context_icontext) != S_OK){
    goto finalize;
  }

  g_context_hmenu = CreatePopupMenu();
  g_context_icontext->QueryContextMenu(g_context_hmenu, 0, 1, 0x7fff, CMF_NORMAL);

  for(int i = 0, l = GetMenuItemCount(g_context_hmenu); i < l; i++) {
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask  = MIIM_ID;
    GetMenuItemInfo(g_context_hmenu, i, true, &mii);
    g_menu_cb_map[mii.wID] = [](void *data) {
      int context_id = (int)((intptr_t)data);
      CMINVOKECOMMANDINFO ici;
      ici.cbSize       = sizeof(CMINVOKECOMMANDINFO);
      ici.fMask        = 0;
      ici.hwnd         = g_hwnd;
      ici.lpVerb       = (LPCSTR)MAKEINTRESOURCE(context_id - 1);
      ici.lpParameters = NULL;
      ici.lpDirectory  = NULL;
      ici.nShow        = SW_SHOW;
      
      g_context_icontext->InvokeCommand(&ici);
      g_context_icontext->Release();
      DestroyMenu(g_context_hmenu);
      g_context_hmenu = NULL;
    };
    g_menu_cb_data_map[mii.wID] = (void*)(intptr_t)mii.wID;
  }

  if(icon != 0 && strlen(icon) > 0) {
    hbmp = lp_platform_get_icon(icon, 16);
  }

  g_menu_seq++;
  lp_platform_add_menu_item(hmenu, g_menu_seq, text, g_context_hmenu, hbmp);


finalize:
  if(shell_folder != NULL) shell_folder->Release();
  if(abslist != NULL) ILFree(abslist);
} // }}}

int lp::platform::menu_item_count(int id) { // {{{
  if(g_menu_map.find(id) == g_menu_map.end()){
    return 0;
  }
  HMENU hmenu = g_menu_map[id];
  int ret = GetMenuItemCount(hmenu);
  return ret < 0 ? 0 : ret;
} // }}}

void lp::platform::show_menu(int type, int x, int y){ // {{{
  HMENU hmenu = g_menu_map[0];
  int rx, ry;
  switch(type) {
    case lp::SHOW_CENTER: {
default_type:
      RECT rc;
      SystemParametersInfo(SPI_GETWORKAREA,0,&rc,0);
      rx = rc.right/2 + x;
      ry = rc.bottom/2 + y;
    }
    break;
    case lp::SHOW_COORD: {
      rx = x;
      ry = y;
    }
    break;
    case lp::SHOW_MOUSE: {
      POINT po;
      GetCursorPos(&po);
      rx = po.x + x;
      ry = po.y + y;
    }
    break;
    case lp::SHOW_WINDOW: {
      WINDOWPLACEMENT placement;
      GetWindowPlacement(g_caller_hwnd, &placement);
      RECT pos = placement.rcNormalPosition;
      rx = pos.left + (pos.right - pos.left)/2;
      ry = pos.top + (pos.bottom-pos.top)/2;
    }
    break;

    default:
      goto default_type;
  }

  while(GetKeyState(VK_LBUTTON) < 0 || GetKeyState(VK_RBUTTON) < 0){ Sleep(10); }
  SetForegroundWindow(g_hwnd);
  TrackPopupMenu(hmenu, 0, rx, ry, 0, g_hwnd, NULL);
} // }}}

int lp::platform::run(int argc, char **argv) { // {{{
  MSG    msg = {0};
  while(GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg); 
    DispatchMessage(&msg);
  }
  return (int)(msg.wParam);
} // }}}

void lp::platform::init(int argc, char **argv) { // {{{
  CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  g_caller_hwnd = GetForegroundWindow();

  WNDCLASSEX wc = {0};
  wc.cbSize        = sizeof(WNDCLASSEX);
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = lp_platform_winproc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = DLGWINDOWEXTRA;
  wc.hInstance     = g_hinst;
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = L"lpopupmain";
  RegisterClassEx(&wc);
  g_hwnd = CreateWindow(L"lpopupmain", L"lpopup", 
               WS_OVERLAPPEDWINDOW,
               CW_USEDEFAULT, CW_USEDEFAULT,
               1, //width
               1, //height
               NULL, NULL, g_hinst, NULL);
  SetWindowPos(g_hwnd,HWND_TOP, 30000,30000,1,1, SWP_SHOWWINDOW);
} // }}}

void lp::platform::exit(int code) { // {{{
  for(auto it = g_menu_map.begin(), last = g_menu_map.end(); it != last; ++it){
    DestroyMenu((*it).second);
  }
  CoUninitialize();
} // }}}
 
int lp::platform::message(const char *title, const char *text, int type) { // {{{
  int flag = 0;
  if(type == lp::MESSAGE_INFO)  flag |= MB_OK | MB_ICONINFORMATION;
  if(type == lp::MESSAGE_WARN)  flag |= MB_OK | MB_ICONWARNING;
  if(type == lp::MESSAGE_ERROR) flag |= MB_OK | MB_ICONERROR;
  if(type == lp::MESSAGE_ASK)   flag |= MB_YESNO | MB_ICONQUESTION;
  LP_OSSTRV(ostext, text);
  LP_OSSTRV(ostitle, title);

  int ret = MessageBoxW(g_hwnd, ostext.get(), ostitle.get(), flag);
  if(ret == IDOK || ret == IDYES) return lp::MESSAGE_OK;
  if(ret == IDNO) return lp::MESSAGE_NO;
  return 0;
} // }}}

void lp::platform::config_path(lp::oschar* buf, size_t buflen) { // {{{
  GetModuleFileNameW(g_hinst, buf, buflen);
  PathRenameExtensionW(buf, L".conf.lua");
} // }}}

int lp::platform::call(const char *cmdline, const char *workdir, bool hide, bool async, std::string &sstdout, std::string &sstderr, lp::Error &error) { // {{{
  lp::oschar command[MAX_PATH*10];
  lp::platform::utf82oschar_b(command, MAX_PATH*10, cmdline);
  LP_OSSTRV(wdir, workdir);
  int funcret = 0;
  
  HANDLE read_pipe = 0, write_pipe = 0;
  HANDLE err_read_pipe = 0, err_write_pipe = 0;
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = 0;
  sa.bInheritHandle = true;
  CreatePipe(&read_pipe, &write_pipe, &sa, 8192);
  CreatePipe(&err_read_pipe, &err_write_pipe, &sa, 8192);
  STARTUPINFO startup_info;
  PROCESS_INFORMATION process_info;
  ZeroMemory(&startup_info, sizeof(STARTUPINFO));
  startup_info.cb = sizeof(STARTUPINFO);
  startup_info.dwFlags = STARTF_USESTDHANDLES;
  startup_info.hStdOutput = write_pipe;
  startup_info.hStdError  = err_write_pipe;
  DWORD creationFlag = 0;
  if(hide) {
    startup_info.wShowWindow = SW_HIDE;
    creationFlag = CREATE_NO_WINDOW |DETACHED_PROCESS;
  }else{
    startup_info.wShowWindow = SW_SHOW;
    creationFlag = CREATE_NEW_CONSOLE;
  }
  SetLastError(NO_ERROR);
  if (CreateProcess(0,command,0,0,true,creationFlag, 0, 
      _tcslen(wdir.get()) > 0 ? wdir.get() : 0, &startup_info, &process_info)) {
    if(async) goto finalize;
    char buf_stdout[8192], buf_errout[8192];
    DWORD dstdout = 0, derrout = 0;
    DWORD ret;
  
    while ( (ret = WaitForSingleObject(process_info.hProcess, 0)) != WAIT_ABANDONED) {
      memset(buf_stdout, 0, sizeof(buf_stdout));
      memset(buf_errout, 0, sizeof(buf_errout));

      PeekNamedPipe(read_pipe, 0, 0, 0, &dstdout, 0);
      if (dstdout > 0) {
        ReadFile(read_pipe, buf_stdout, sizeof(buf_stdout) - 1, &dstdout, 0);
      }
  
      PeekNamedPipe(err_read_pipe, 0, 0, 0, &derrout, 0);
      if (derrout > 0) {
        ReadFile(err_read_pipe, buf_errout, sizeof(buf_errout) - 1, &derrout, 0);
      }
      sstdout += buf_stdout;
      sstderr += buf_errout;
  
      MSG msg;
      if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      if (ret == WAIT_OBJECT_0) break;

    }
  
    DWORD res;
    GetExitCodeProcess(process_info.hProcess, &res);
    if(res != 0) {
      error.setMessage("Command failed.");
      error.setCode(1);
      funcret = res;
    }
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
  }else{
    lp_platform_set_error(error);
    funcret = 2;
  }
  
finalize:

  CloseHandle(write_pipe);
  CloseHandle(read_pipe);
  CloseHandle(err_write_pipe);
  CloseHandle(err_read_pipe);
  return funcret;
} // }}}

int lp::platform::shell_execute(const char *cmdline, const char *workdir, lp::Error &error) { // {{{
  lp::oschar wcmdline[MAX_PATH*10];
  lp::platform::utf82oschar_b(wcmdline, MAX_PATH*10, cmdline);
  LP_OSSTRV(wworkdir, workdir);

  lp::oschar *wargs = NULL;
  lp_platform_split_cmdline(wcmdline, &wargs);
  if(wargs != NULL){
    wcmdline[wargs-wcmdline-1] = L'\0';
    PathUnquoteSpaces(wcmdline);
  }
  
#ifdef LP_OS_WIN64
#define RET_TYPE  long long 
#else
#define RET_TYPE  long
#endif
  RET_TYPE ret = (RET_TYPE)(ShellExecute(g_hwnd, L"open", wcmdline, wargs, _tcslen(wworkdir.get()) > 0 ? wworkdir.get() : NULL, SW_SHOWNORMAL));
#undef RET_TYPE

  if(ret > 32){ return 0; }

  char buf[1024];
  error.setCode((int)ret);
  switch(ret) {
    case 0:
      sprintf(buf, "%d: %s", error.getCode(), "The operating system is out of memory or resources.");
      break;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
      sprintf(buf, "%d: %s", error.getCode(), "The specified path was not found.");
      break;
    case SE_ERR_ACCESSDENIED:
      sprintf(buf, "%d: %s", error.getCode(), "The operating system denied access to the specified file.");
      break;
    default:
      sprintf(buf, "%d: %s", error.getCode(), "Error.");
  }
  error.setMessage(buf);

  return 1;
} // }}}

