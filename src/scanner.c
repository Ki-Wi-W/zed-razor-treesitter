#include <tree_sitter/parser.h>
#include <wctype.h>
#include <string.h>

enum TokenType {
  CODE_BLOCK_START,
  RAW_TEXT,
};

typedef struct {
  int32_t depth;
  bool in_code_block;
  bool in_expression;
} Scanner;

void *tree_sitter_razor_external_scanner_create() {
  Scanner *scanner = calloc(1, sizeof(Scanner));
  scanner->depth = 0;
  scanner->in_code_block = false;
  scanner->in_expression = false;
  return scanner;
}

void tree_sitter_razor_external_scanner_destroy(void *payload) {
  Scanner *scanner = (Scanner *)payload;
  free(scanner);
}

unsigned tree_sitter_razor_external_scanner_serialize(void *payload, char *buffer) {
  Scanner *scanner = (Scanner *)payload;
  buffer[0] = (char)(scanner->depth & 0xFF);
  buffer[1] = (char)((scanner->depth >> 8) & 0xFF);
  buffer[2] = (char)scanner->in_code_block;
  buffer[3] = (char)scanner->in_expression;
  return 4;
}

void tree_sitter_razor_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {
  Scanner *scanner = (Scanner *)payload;
  if (length >= 4) {
    scanner->depth = (int32_t)((unsigned char)buffer[0] | ((unsigned char)buffer[1] << 8));
    scanner->in_code_block = (bool)buffer[2];
    scanner->in_expression = (bool)buffer[3];
  } else {
    scanner->depth = 0;
    scanner->in_code_block = false;
    scanner->in_expression = false;
  }
}

static void advance(TSLexer *lexer) {
  lexer->advance(lexer, false);
}

static void skip(TSLexer *lexer) {
  lexer->advance(lexer, true);
}

static bool is_keyword_start_char(int32_t c) {
  return c == 'e' || c == 'c' || c == 'f';
}

static bool check_full_keyword(TSLexer *lexer) {
  int32_t start = lexer->lookahead;

  if (start == 'e') {
    advance(lexer);
    if (lexer->lookahead == 'l') {
      advance(lexer);
      if (lexer->lookahead == 's') {
        advance(lexer);
        if (lexer->lookahead == 'e') {
          return true;
        }
      }
    }
    return false;
  }

  if (start == 'c') {
    advance(lexer);
    if (lexer->lookahead == 'a') {
      advance(lexer);
      if (lexer->lookahead == 't') {
        advance(lexer);
        if (lexer->lookahead == 'c') {
          advance(lexer);
          if (lexer->lookahead == 'h') {
            return true;
          }
        }
      }
    }
    return false;
  }

  if (start == 'f') {
    advance(lexer);
    if (lexer->lookahead == 'i') {
      advance(lexer);
      if (lexer->lookahead == 'n') {
        advance(lexer);
        if (lexer->lookahead == 'a') {
          advance(lexer);
          if (lexer->lookahead == 'l') {
            advance(lexer);
            if (lexer->lookahead == 'l') {
              advance(lexer);
              if (lexer->lookahead == 'y') {
                return true;
              }
            }
          }
        }
      }
    }
    return false;
  }

  return false;
}

static bool scan_code_block_start(Scanner *scanner, TSLexer *lexer) {
  while (iswspace(lexer->lookahead)) {
    skip(lexer);
  }

  if (lexer->lookahead == '{') {
    advance(lexer);
    scanner->in_code_block = true;
    scanner->depth++;
    lexer->mark_end(lexer);
    lexer->result_symbol = CODE_BLOCK_START;
    return true;
  }

  return false;
}

static bool scan_raw_text(TSLexer *lexer, Scanner *scanner) {
  bool has_content = false;

  while (true) {
    if (lexer->lookahead == '@' || lexer->lookahead == '<') {
      break;
    }

    if (lexer->lookahead == 0) {
      break;
    }

    if (lexer->lookahead == '}') {
      break;
    }

    if (is_keyword_start_char(lexer->lookahead)) {
      if (check_full_keyword(lexer)) {
        return false;
      }
      has_content = true;
      lexer->mark_end(lexer);
      continue;
    }

    has_content = true;
    advance(lexer);
  }

  if (has_content) {
    lexer->mark_end(lexer);
    lexer->result_symbol = RAW_TEXT;
    return true;
  }

  return false;
}

bool tree_sitter_razor_external_scanner_scan(void *payload, TSLexer *lexer, const bool *valid_symbols) {
  Scanner *scanner = (Scanner *)payload;

  if (valid_symbols[RAW_TEXT]) {
    if (scan_raw_text(lexer, scanner)) {
      return true;
    }
  }

  if (valid_symbols[CODE_BLOCK_START]) {
    if (lexer->lookahead == '@') {
      advance(lexer);
      if (scan_code_block_start(scanner, lexer)) {
        return true;
      }
    }
  }

  return false;
}
