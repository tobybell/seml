#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define tail __attribute__((musttail)) return
#define next() if (++ofs >= count) return

void output(char const* data) {
  write(1, data, strlen(data));
}

void writeo(char const* data, unsigned len) {
  write(1, data, len);
}

char in_list = 0;
char buf[16 * 1024 * 1024];
int count = 0;
int ofs = 0;

enum Mode {
  LineReady,
  LineStart,
  ParReady,
  ParStart,
  ParEnd,
  HeadReady,
  HeadStart,
};

enum Mode mode = LineReady;

void dispatch(void);

void ensure_in_list() {
  if (!in_list) {
    in_list = 1;
    output("\n<ul>");
  }
}

void ensure_not_in_list() {
  if (in_list) {
    in_list = 0;
    output("\n</ul>");
  }
}

void par_end(void) {
  while (buf[ofs] == ' ')
    next();
  if (buf[ofs] == '\n') {
    mode = LineReady;
    next();
    tail dispatch();
  }
  if (in_list && buf[ofs] == '*') {
    mode = ParReady;
    next();
    tail dispatch();
  }
  output("<br>");
  mode = ParStart;
  tail dispatch();
}

void par_start(void) {
  int checkpoint = ofs;
  while (buf[ofs] != '\n') {
    if (++ofs >= count) {
      writeo(&buf[checkpoint], count - checkpoint);
      return;
    }
  }
  writeo(&buf[checkpoint], ofs - checkpoint);
  mode = ParEnd;
  next();
  tail dispatch();
}

void head_ready(void) {
  while (buf[ofs] == ' ')
    next();
  if (buf[ofs] == '\n') {
    mode = LineStart;
    next();
    tail dispatch();
  }
  output("\n<p><strong>");
  mode = HeadStart;
  tail dispatch();
}

void head_start(void) {
  int checkpoint = ofs;
  while (buf[ofs] != '\n') {
    if (++ofs >= count) {
      writeo(&buf[checkpoint], count - checkpoint);
      return;
    }
  }
  writeo(&buf[checkpoint], ofs - checkpoint);
  output("</strong>");
  mode = LineReady;
  next();
  tail dispatch();
}

void par_ready(void) {
  while (buf[ofs] == ' ' || buf[ofs] == '\n')
    next();
  output(in_list ? "\n<li>" : "\n<p>");
  mode = ParStart;
  tail dispatch();
}

void line_start(void) {
  if (buf[ofs] == '!') {
    ensure_not_in_list();
    mode = HeadReady;
    next();
    tail dispatch();
  }
  if (buf[ofs] == '*') {
    ensure_in_list();
    next();
  } else {
    ensure_not_in_list();
  }
  mode = ParReady;
  tail dispatch();
}

void line_ready(void) {
  while (buf[ofs] == ' ' || buf[ofs] == '\n')
    next();
  mode = LineStart;
  tail dispatch();
}

void dispatch(void) {
  switch (mode) {
    case LineReady:
      tail line_ready();
    case LineStart:
      tail line_start();
    case ParReady:
      tail par_ready();
    case HeadReady:
      tail head_ready();
    case ParStart:
      tail par_start();
    case ParEnd:
      tail par_end();
    case HeadStart:
      tail head_start();
  }
  abort();
}

int main() {
  output("Content-Type: text/html; charset=\"utf-8\"\n\n<html>\n<head>\n</head>\n<body>");
  
  while (1) {
    ofs = 0;
    count = read(0, buf, sizeof(buf));
    if (count == -1)
      return 1;
    if (count == 0)
      break;

    dispatch();
  }

  output("\n</body>\n</html>\n");
}
