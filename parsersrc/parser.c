#include <assert.h>
#include "parser.h"
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void array_free(char **toks)
{
  if (toks) {
    for (int v = 0; toks[v]; v++)
      free(toks[v]);
    free(toks);
  }
}

EditorConfig_t *load_editor_config(char *file)
{
  EditorConfig_t *cfg;
  char **toks;
  int stream;
  int i = 0;
  if (!file) return NULL;
  cfg = malloc(sizeof(*cfg));
  stream = open(file, O_RDONLY);
  if (stream < 0)
  {
    free(cfg);
    return (NULL);
  }
  // BUG: There is a problem. u can not identify string literals that has spaces in em using this method
  // TODO: Introduce a tokinizer to solve the problem..
  // TODO: Load every line of the file.
  for (char *next = read_next_line(stream), i = 0;
  next;
  (next = read_next_line(stream)), i++)
  {
    // size_t count;
    // toks = split(next, ": \t\n", &count);
    TokenList *tokens = retokenize_line(next);

    if (tokens->size != 3)
    {
      fprintf(stderr, "[ERROR] line %i <%s> has a syntax error\n", i + 1, next);
      free(cfg);
      array_free(toks);
      exit(1);
    }
    char *rhs = tokens->_list[0].data;
    char *expr = tokens->_list[1].data; // usually : or =
    assert(*expr == ':' || *expr == '=');
    char *lhs = tokens->_list[2].data;
    printf("%i: %s | %s\n", i, rhs, lhs);
  }
  // TODO: split it by : to get a key and a value.
  // free the allocated things.
  close (stream);
  array_free(toks);
  return (cfg);
} 