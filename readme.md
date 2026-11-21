# Parse Pact: a compact parser generator

## Provides Autonomous Regex Scanners & Entire Parsers At Compile Time

Single header `C++-20` library to generate `lexer` and `parser`, at compile time or runtime!

## Key features
- __Single header__: immediate to streamline in your project:

```c++
#include <parsePact.hpp>

const auto parser=pp::generateParser(" ... grammar...");
parser.parse("...text to be parsed");
```
- Can generate the parser at compile time:
```c++
constexpr auto parser=pp::generateParser<" ... grammar...">();
```
- Plain standard `C++-20`, no extra dependency
- Supports `lalr(1)` grammar
- Can parse expressions at compile time!
```c++
constexpr auto parseResult=parser.parse<"...text to be parsed">();
```

**Status**: the grammar is completely created, parsing is still in
progress, no action is issued. Lexer do not track position yet. Compile time features requires wrapping the text in lambda, but we will improve on it.

**Based on:** The project is based on [lalr parser
generator](https://github.com/cwbaker/lalr/) by Charles Baker.

See the [development notes](doc/develop.md)

## Grammar source positions

The grammar reader tracks a `pp::SourceCursor` containing a zero-based byte
`offset` and one-based `line` and `column`. Columns count bytes, including one
column per tab; LF, CRLF, and lone CR are recognized as line endings. Tentative
matches restore the cursor when they roll back.

Grammar syntax errors include the position of the next byte or end of input:

```text
Error: line 3, column 1 (offset 13): Expected ';' after production
```

Symbols retain their first occurrence and production alternatives retain their
start position. Undefined or unreferenced symbols and parser conflicts report
those locations after grammar parsing has finished. These positions refer to
the supplied grammar string, including any leading newline in a raw literal.

Cursor updates work during constant evaluation. Invalid constexpr grammar
construction remains a compiler error: it reaches the non-constexpr diagnostic
function. Runtime input tokens, AST source spans, regex-internal errors, and
semantic-action argument errors do not yet carry grammar-wide source positions.

## Example - outdated
In the following example we show the regex parser in action, detecting a json real number, at compile time
```c++
#include <parserGenerator.hpp>

int main()
{
  /// Recognized token
  enum {JSON_NUMBER,JSON_REAL_NUMER,TEXT_NOT_CONTAINING_H};
  
  /// Pattern to recognize a number
  constexpr char jsonNumberPattern[]="(\\+|\\-)?[0-9]+";
  
  /// Pattern to recognize a real number
  constexpr char jsonRealNumberPattern[]="(\\+|\\-)?[0-9]+(\\.[0-9]+)?((e|E)(\\+|\\-)?[0-9]+)?";
  
  /// Pattern to recognize a string not containing char 'h'
  constexpr char testNotContainingHPattern[]="[^h]+";
  
  /////////////////////////////////////////////////////////////////
  
  /// Storage properties of the regex pattern matcher
  constexpr auto specs=estimateRegexParserSize(jsonNumberPattern,jsonRealNumberPattern,testNotContainingHPattern);
  
  /// Regex parser matcher
  constexpr auto parser=createParserFromRegex<specs>(jsonNumberPattern,jsonRealNumberPattern,testNotContainingHPattern);
  
  static_assert(parser.parse("-332.235e-34")==JSON_REAL_NUMER);
  static_assert(parser.parse("33")==JSON_NUMBER);
  static_assert(parser.parse("ello world!")==TEXT_NOT_CONTAINING_H);
  
  return 0;
}
```
