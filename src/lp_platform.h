#ifndef __LP_PLATFORM_H__
#define __LP_PLATFORM_H__

#include "lp_constants.h"
#include "lp_utils.h"

#ifdef LP_OS_WIN
#  include "lp_platform_win.h"
#endif

namespace lp {
  namespace platform {
    lp::oschar* utf82oschar(const char *src);
    void utf82oschar_b(lp::oschar *buf, const unsigned int bufsize, const char *src);
    char* oschar2utf8(const lp::oschar *src);
    void oschar2utf8_b(char *buf, const unsigned int bufsize, const lp::oschar *src);
    char* oschar2local(const lp::oschar *src);
    void oschar2local_b(char *buf, const unsigned int bufsize, const lp::oschar *src);
    char* utf82local(const char *src);
    char* local2utf8(const char *src, int sz=0);

    void init(int argc, char **argv);
    void exit(int code);
    int run(int argc, char **argv);

    void config_path(lp::oschar* buf, size_t buflen);
    int call(const char *cmdline, const char *workdir, bool hide, bool async, std::string &sstdout, std::string &sstderr, lp::Error &error);
    int shell_execute(const char *cmdline, const char *workdir, lp::Error &error);

    /* GUI stuff */
    int message(const char *title, const char *text, int type=0);
    void new_menu(int id);
    void add_item(int id, const char *text, const char *icon, lp::menu_func callback, void *data);
    void add_submenu(int id, int otherid, const char *text, const char *icon);
    void add_file_context(int id, const char *text, const char *path, const char *icon);
    void add_hseparator(int id);
    void add_vseparator(int id);
    void show_menu(int type, int x=0, int y=0);
    int menu_item_count(int id);
  }
}

#endif
