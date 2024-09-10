#ifndef PARSING_H
#define PARSING_H
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <assert.h>
#include <stdbool.h>

#define TAB         '\t'
#define NL          '\n'

#define OPAR        '('
#define CPAR        ')'
#define OCERLY      '{'
#define CCERLY      '}'

#define OBRAC       '['
#define CBRAC       ']'

#define DQUOTE      '\"'
#define SQUOTE      '\''
// TODO: IMPLEMET FUNCTIONS THAT CAN COLLECT THESE TOKENS..
typedef enum MITokenType {
    /* Groups */
    STR_LIT_INCOM,
    STR_LIT,
    NUMBER_LIT,
    KEYWORD,
    ID,
    /* Special tokens. */
    CALL,
    C_INCLUDE,
    C_TAG,
    C_INCLUDE_FILE,
    C_ENUM,
    C_TYPEDEF,
    C_STRUCT,
    C_UNION,
    TYPE,
    _GENERIC_NULL,
    COMMENT,
    /* Syms */
    EQ, // =
    GT, // >
    LT, // <
    AST, // *
    AND, // &
    PIPE, // |
    HASHTAG, // #
    OPAR_, // (
    CPAR_, // )
    OCERLY_, // {
    CCERLY_, // }
    OBRAC_, // [
    CBRAC_, // ]
    COMA, // ,
    PERIOD, // .
    DOLLARSIGN, // $
    AT, // @
    BANG, // !
    PLUS, // +
    MINUS, // -
    SEMICOLON, // ;
    COLON, // :
    FSLASH, // /
    BSLASH, // '\' /
    OTHER_PUNCT, // Any unknown punct.
} MITokenType;
typedef struct MIToken {
    MITokenType kind;
    int xstart, xend;
    char *data;
} MIToken;

typedef struct TokenList {
    MIToken *_list;
    int size, cap;
} TokenList;
typedef struct EditorConfig_s {
  bool autosave; // Unused
  char indent_char;
  int indent_count;
} EditorConfig_t;

typedef enum FileType {
    PYTHON = 0,
    C,
    JS,
    GO, 
    CNFG,
    MP3,
    UNSUP
} FileType;

EditorConfig_t *load_editor_config(char *file);
char	**split(char *str, char *charset, size_t *count);
char *read_next_line(int fd);
TokenList *retokenize_line(char *data);
void token_list_append(char *line, TokenList *list, MITokenType kind, int xstart, int xend);
#endif // PARSING_H