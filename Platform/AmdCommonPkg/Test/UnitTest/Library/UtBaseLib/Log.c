/*
 * Copyright (c) 2020 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
/* Copyright (C) 2021 - 2024 Advanced Micro Devices, Inc. All rights reserved. */
// SPDX-License-Identifier: MIT

#include <Library/PrintLib.h>
#include "Log.h"

#define MAX_CALLBACKS 32
#define MAX_LOG_MESSAGE_LENGTH  0x100

typedef struct {
  log_LogFn fn;
  void *udata;
  int level;
} Callback;

static struct {
  void *udata;
  log_LockFn lock;
  int level;
  bool quiet;
  Callback callbacks[MAX_CALLBACKS];
} L;


static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

static void print_message_with_format(log_Event *event) {
  switch (event->std) {
    case STRING_FMT_EDK2_PRINT_LIB:
      char buffer[MAX_LOG_MESSAGE_LENGTH];
      AsciiVSPrint(buffer, sizeof(buffer), event->fmt, event->ap);
      fputs(buffer, event->udata);
      break;

    case STRING_FMT_ANSI_C_STD:
    default:
      vfprintf(event->udata, event->fmt, event->ap);
      break;
  }

  if (event->line != 0) { // SA: Workaround for ids print
    fprintf(event->udata, "\n");
  }

  fflush(event->udata);
}


static void stdout_callback(log_Event *ev) {
  char buf[16];
  buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
  if (ev->line != 0) { // SA: Workaround for ids print
#ifdef LOG_USE_COLOR
    fprintf(
      ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
      buf, level_colors[ev->level], level_strings[ev->level],
      ev->file, ev->line);
#else
    fprintf(
      ev->udata, "%s %-5s %s:%d: ",
      buf, level_strings[ev->level], ev->file, ev->line);
#endif
  } else {
    fprintf(
      ev->udata, "%s %-5s ",
      buf, level_strings[ev->level]);
  }

  print_message_with_format(ev);
}


static void file_callback(log_Event *ev) {
  char buf[64];
  buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
  if (ev->line != 0) { // SA: Workaround for ids print
    fprintf(
      ev->udata, "%s %-5s %s:%d: ",
      buf, level_strings[ev->level], ev->file, ev->line);
  } else {
    fprintf(
      ev->udata, "%s %-5s ",
      buf, level_strings[ev->level]);
  }

  print_message_with_format(ev);
}


static void lock(void)   {
  if (L.lock) { L.lock(true, L.udata); }
}


static void unlock(void) {
  if (L.lock) { L.lock(false, L.udata); }
}


const char* log_level_string(int level) {
  return level_strings[level];
}


void log_set_lock(log_LockFn fn, void *udata) {
  L.lock = fn;
  L.udata = udata;
}


void log_set_level(int level) {
  L.level = level;
}


void log_set_quiet(bool enable) {
  L.quiet = enable;
}


int log_add_callback(log_LogFn fn, void *udata, int level) {
  for (int i = 0; i < MAX_CALLBACKS; i++) {
    if (!L.callbacks[i].fn) {
      L.callbacks[i] = (Callback) { fn, udata, level };
      return 0;
    }
  }
  return -1;
}


int log_add_fp(FILE *fp, int level) {
  return log_add_callback(file_callback, fp, level);
}


static void init_event(log_Event *ev, void *udata) {
  // SA: Workaround for using localtime_s
  time_t t = time(NULL);
  localtime_s(ev->time, &t);
  ev->udata = udata;
}


void log_log(int level, const char *file, int line, const char *fmt, ...) {
  log_Event ev = {
    .std   = STRING_FMT_ANSI_C_STD,
    .fmt   = fmt,
    .file  = file,
    .line  = line,
    .level = level,
  };

  struct tm _time;   // SA: Workaround for using localtime_s

  lock();

  ev.time = &_time;  // SA: Workaround for using localtime_s

  if (!L.quiet && level >= L.level) {
    init_event(&ev, stderr);
    va_start(ev.ap, fmt);
    stdout_callback(&ev);
    va_end(ev.ap);
  }

  for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
    Callback *cb = &L.callbacks[i];
    if (level >= cb->level) {
      init_event(&ev, cb->udata);
      va_start(ev.ap, fmt);
      cb->fn(&ev);
      va_end(ev.ap);
    }
  }

  unlock();
}

void log_log_sil(int level, const char *file, int line, const char *fmt, va_list ap) {
  log_Event ev = {
    .std   = STRING_FMT_EDK2_PRINT_LIB,
    .fmt   = fmt,
    .file  = file,
    .line  = line,
    .level = level,
  };

  struct tm _time;   // SA: Workaround for using localtime_s

  lock();

  ev.time = &_time;  // SA: Workaround for using localtime_s

  if (!L.quiet && level >= L.level) {
    init_event(&ev, stderr);
    ev.ap = ap;
    stdout_callback(&ev);
  }

  for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
    Callback *cb = &L.callbacks[i];
    if (level >= cb->level) {
      init_event(&ev, cb->udata);
      cb->fn(&ev);
    }
  }

  unlock();
}
