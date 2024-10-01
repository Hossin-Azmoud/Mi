#include "parser.h"
#include <assert.h>
#include <fcntl.h>
// #include <math.h>
#include <stdio.h>
#include <string.h>

void array_free(char **toks) {
  if (toks) {
    for (int v = 0; toks[v]; v++)
      free(toks[v]);
    free(toks);
  }
}

EditorConfig_t *load_editor_config(char *file) {
  EditorConfig_t *cfg;
  char **toks;
  int stream;
  int i = 0;
  if (!file)
    return NULL;
  cfg = malloc(sizeof(*cfg));
  memset(cfg, 0, sizeof(*cfg));
  stream = open(file, O_RDONLY);
  if (stream < 0) {
    free(cfg);
    return (NULL);
  }
  for (char *next = read_next_line(stream), i = 0; next;
       (next = read_next_line(stream)), i++) {
    TokenList *tokens = retokenize_line(next);

    char *rhs = tokens->_list[0].data;
    if (tokens->_list[0].kind == HASHTAG || !*rhs)
      continue;

    char *expr = tokens->_list[1].data; // usually : or =
    assert(*expr == ':' || *expr == '=');
    char *lhs = tokens->_list[2].data;
    if (strcmp(rhs, "autosave") == 0) {
      cfg->autosave = atoi(lhs);
      continue;
    }
    if (strcmp(rhs, "indent_char") == 0) {
      assert(*lhs == '"');
      assert(*(lhs + 1) != 0);
      cfg->indent_char = *(lhs + 1); // The letter directly after the qoutes.
      continue;
    }
    if (strcmp(rhs, "indent_count") == 0) {
      cfg->indent_count = atoi(lhs);
      continue;
    }
    if (strcmp(rhs, "background") == 0) {
      cfg->theme.background = (int)np_atoi_base(lhs, NULL);
      continue;
    }

    if (strcmp(rhs, "foreground") == 0) {
      cfg->theme.foreground = (int)np_atoi_base(lhs, NULL);
      continue;
    }

    if (strcmp(rhs, "keyword_color") == 0) {
      cfg->theme.keyword_color = (int)np_atoi_base(lhs, NULL);
      continue;
    }
    if (strcmp(rhs, "type_color") == 0) {
      cfg->theme.type_color = (int)np_atoi_base(lhs, NULL);
      continue;
    }
    if (strcmp(rhs, "funcall_color") == 0) {
      cfg->theme.funcall_color = (int)np_atoi_base(lhs, NULL);
      continue;
    }
    if (strcmp(rhs, "special_token_color") == 0) {
      cfg->theme.special_token_color = (int)np_atoi_base(lhs, NULL);
      continue;
    }
    if (strcmp(rhs, "string_lit_color") == 0) {
      cfg->theme.string_lit_color = (int)np_atoi_base(lhs, NULL);
      continue;
    }
    if (strcmp(rhs, "comment_color") == 0) {
      cfg->theme.comment_color = (int)np_atoi_base(lhs, NULL);
      continue;
    }

  }
  // TODO: split it by : to get a key and a value.
  // free the allocated things.
  close(stream);
  array_free(toks);
  return (cfg);
}
