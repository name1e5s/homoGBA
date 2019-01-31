#ifndef HOMOLOG_H
#define HOMOLOG_H

#define LOG_FATAL -3
#define LOG_ERROR -2
#define LOG_WARN -1
#define LOG_INFO 0
#define LOG_DEBUG 1
#define LOG_VERBOSE 2

#define FREE_LOG(X, ...) \
  _Generic((X), char* : free, char** : double_free)(X, ##__VA_ARGS__)

void homo_log(int log_level, const char* str, ...);
void homo_trace(void);
void homo_flush();
void homo_set_dbg_level(int level);
void homo_set_log_file(const char* file);
void homo_set_silent_mode(int silent);  // 1 as on, 0 as off

void homo_load_config(const char* file);

void double_free(char** pointer, size_t size);
#endif