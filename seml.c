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

typedef struct Buf {
  char* data;
  unsigned capacity;
  unsigned size;
} Buf;

void buf_append(Buf* buf, char const* data, unsigned size) {
  unsigned cap = buf->capacity;
  unsigned needed = buf->size + size;
  if (!cap)
    cap = size;
  else
    while (cap < needed)
      cap *= 2;
  buf->data = realloc(buf->data, cap);
  buf->capacity = cap;
  memcpy(buf->data + buf->size, data, size);
  buf->size = needed;
}

void buf_free(Buf* buf) {
  free(buf);
  buf->data = NULL;
  buf->size = 0;
}

char in_list = 0;
char bold = 0;
char buf[16 * 1024 * 1024];
int count = 0;
int ofs = 0;
Buf cur_link_text = {};
Buf cur_link_url = {};

enum Mode {
  LineReady,
  LineStart,
  ParReady,
  ParStart,
  ParEnd,
  HeadReady,
  HeadStart,
  LinkText,
  LinkTextPost,
  LinkUrl,
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
    if (buf[ofs] == '[') {
      writeo(&buf[checkpoint], ofs - checkpoint);
      mode = LinkText;
      next();
      tail dispatch();
    }
    if (buf[ofs] == '*') {
      writeo(&buf[checkpoint], ofs - checkpoint);
      output(bold ? "</strong>" : "<strong>");
      bold = !bold;
      next();
      tail dispatch();
    }
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

void link_text(void) {
  int checkpoint = ofs;
  while (buf[ofs] != ']') {
    if (++ofs >= count) {
      buf_append(&cur_link_text, &buf[checkpoint], count - checkpoint);
      return;
    }
  }
  buf_append(&cur_link_text, &buf[checkpoint], ofs - checkpoint);
  mode = LinkTextPost;
  next();
  tail dispatch();
}

void link_text_post(void) {
  if (buf[ofs] == '(') {
    mode = LinkUrl;
    next();
    tail dispatch();
  } else {
    output("[");
    writeo(cur_link_text.data, cur_link_text.size);
    output("]");
    mode = ParStart;
    tail dispatch();
  }
}

void link_url(void) {
  int checkpoint = ofs;
  while (buf[ofs] != ')') {
    if (++ofs >= count) {
      buf_append(&cur_link_url, &buf[checkpoint], count - checkpoint);
      return;
    }
  }
  buf_append(&cur_link_url, &buf[checkpoint], ofs - checkpoint);
  output("<a href=\"");
  writeo(cur_link_url.data, cur_link_url.size);
  output("\">");
  writeo(cur_link_text.data, cur_link_text.size);
  output("</a>");
  cur_link_text.size = cur_link_url.size = 0;
  mode = ParStart;
  next();
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
    case LinkText:
      tail link_text();
    case LinkTextPost:
      tail link_text_post();
    case LinkUrl:
      tail link_url();
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
