# AGENTS.md

## What This Is

Tree-sitter grammar for ASP.NET Razor Pages (`.razor`, `.cshtml`). Single grammar, not a monorepo. Includes a Zed editor extension in `zed-razor/`.

## Build & Test

**Windows requires MSVC environment.** The bat files handle this:
```
.\run_test.bat        # sets up vcvars64 + runs tree-sitter test
.\run_parse.bat       # sets up vcvars64 + parses sample files
```

**Manual steps:**
```
npm install
npx tree-sitter generate          # regenerate src/ from grammar.js
npx tree-sitter test              # run 34 corpus tests
npx tree-sitter parse <file>      # inspect AST for a single file
```

**WASM for Zed extension:**
```
npx tree-sitter build --wasm
Copy-Item tree-sitter-razor.wasm zed-razor\grammars\razor.wasm
```

## Source of Truth

- **`grammar.js`** — the grammar definition. Everything in `src/` is generated from it.
- **`src/`** — generated C code (parser.c, scanner.c). Never edit manually; regenerate with `tree-sitter generate`.
- **`scanner.c`** — hand-written external scanner. Implements `_code_block_start` (captures `@{`) and `raw_text` (stops at `@`, `<`, `}`, and keywords `else`/`catch`/`finally`).
- **`queries/`** — highlights.scm and injections.scm.
- **`zed-razor/languages/razor/`** — Zed extension's copy of query files. Must stay in sync with `queries/`.

## Gotchas

- **Scanner keyword detection**: `raw_text` stops at `else`, `catch`, `finally` so the parser can match them as part of control structures. If you add new control flow keywords (e.g., `@using` block), update `check_full_keyword()` in `scanner.c`.
- **Scanner order matters**: `RAW_TEXT` is checked before `CODE_BLOCK_START` in the scan function. This prevents the position from being corrupted when both tokens are valid.
- **Query file sync**: Edit `queries/highlights.scm` first, then copy to `zed-razor/languages/razor/highlights.scm`.
- **WASM sync**: After rebuilding WASM, copy to `zed-razor/grammars/razor.wasm`.
- **Test corpus format**: Tests use tree-sitter's `===` separator format. Expected output is a partial S-expression (just the matched node, not the full document root).
- **No CI or lint**: No workflows, no pre-commit, no typecheck. Verify locally with `npx tree-sitter test`.
- **Editorconfig**: 2-space indent for `.js`, `.scm`, `.json`, `.gyp`; 4-space for `.c`/`.h`.
