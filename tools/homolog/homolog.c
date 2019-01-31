#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <homolog.h>
#include <string.h>

#define LOG_INTERNAL 3
#define LOG_TRACE 4

// Some colors
#define COLOR_NORMAL "\x1B[0m"
#define COLOR_RED "\x1B[31m"
#define COLOR_YELLOW "\x1B[33m"
#define COLOR_CYAN "\x1B[36m"
#define COLOR_WHITE "\x1B[37m"
#define COLOR_GREY "\x1B[90m"
#define COLOR_LIGHT_RED "\x1B[91m"
#define COLOR_BLUE "\x1B[94m"
#define COLOR_MAGENTA "\x1B[95m"

// Colors For Printin
static char* HOMO_NORMAL = COLOR_NORMAL;

static char* HOMO_FATAL = COLOR_RED;
static char* HOMO_ERROR = COLOR_LIGHT_RED;
static char* HOMO_WARN = COLOR_YELLOW;
static char* HOMO_INFO = COLOR_WHITE;
static char* HOMO_DEBUG = COLOR_BLUE;
static char* HOMO_VERBOSE = COLOR_CYAN;
static char* HOMO_INTERNAL = COLOR_GREY;
static char* HOMO_TRACE = COLOR_MAGENTA;

// Basic Settings
static int log_level = LOG_DEBUG;
static char log_file[1024] = "homo.log";
static bool silent_mode = false;

// In module functions
static char* get_date(void);
static char** get_backtrace(void* addresses[], size_t size);

/**
 * Get the current time and return it as a string of
 * the following form:
 * [YYYY-MM-DD HH:MM:SS]
 *
 * Don't forget to free the pointer,
 */
static char* get_date(void) {
  time_t t = time(NULL);
  char* str = malloc(0x80);
  strftime(str, 0x80, "[%F %T]", localtime(t));
  return str;
}

void double_free(char** pointer, size_t size) {
  for (int i = 0; i < size; i++)
    free(pointer[i]);
  free(pointer);
}

/**
 * Get the printable version of backtrace.
 *
 * The returned string must be freed.
 */
static char** get_backtrace(void* addresses[], size_t size) {
  char** str = (char**)malloc(sizeof(char*) * size);
  for (int i = 0; i < size; i++)
    str[i] = malloc(sizeof(char) * 0xff);
  const int max_path = 0xf00;
  const int max_cmd = 0xfff;

  char command[max_cmd];
  char path[max_path];

  bool err;
  // For linux only
  if (system("which addr2line > /dev/null 2>&1")) {
    homo_log(LOG_INTERNAL,
             "Function 'addr2line' not found, please install binutils");
    err = true;
  } else {
    int path_length = readlink("/proc/self/exe", path, sizeof(path));
    if (path_length <= 0) {
      homo_log(LOG_INTERNAL, "Get execution path failed.");
      err = true;
    }
    path[path_length] = '\0';
  }
  if (err) {
    FREE_LOG(str, size);
    return NULL;
  }
  for (int i = 0; i < size; i++) {
    sprintf(command, "addr2line -Cfispe \"%s\" %X 2>/dev/null", path,
            (unsigned int)(uintptr_t)addresses[i]);

    FILE* out = popen(command, "r");
    if (out == NULL) {
      homo_log(LOG_INTERNAL, "Failed to execute %s", command);
      FREE_LOG(str, size);
      return NULL;
    }

    if (fgets(str[i], 0xff, out) == NULL) {
      homo_log(LOG_INTERNAL, "Failed to get formatted strings.");
      FREE_LOG(str, size);
      return NULL;
    }

    str[i][strlen(str[i]) - 1] = '\0';
    pclose(out);
  }
  return str;
}