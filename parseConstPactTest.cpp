#include "parseConstPact.hpp"

/// Compile-time regex matcher recognizing one or more 'a' characters
constexpr auto matcher=
  pp::createRegexMatcher<"a+">();

static_assert(matcher.match<"aaa">()->matchedString.size()==3);

/// Compile-time matching and tokenization use the same longest-prefix rule
constexpr auto prefixMatcher=
  pp::createRegexMatcher<"a","abc","b","x">();

static_assert(prefixMatcher.match<"abx">()->matchedString=="a");
static_assert(prefixMatcher.match<"ab">()->matchedString=="a");
static_assert(prefixMatcher.match<"abcx">()->matchedString=="abc");
static_assert(prefixMatcher.match<"abcx">()->iToken==1);
static_assert(not prefixMatcher.match<"z">());

constexpr auto prefixTokenizer=
  pp::createTokenizer<"a","abc","b","x">();

constexpr auto prefixTokens=
  prefixTokenizer.tokenize<"abx">();

static_assert(prefixTokens.size()==3);
static_assert(prefixTokens[0].matchedString=="a" and prefixTokens[0].iToken==0);
static_assert(prefixTokens[1].matchedString=="b" and prefixTokens[1].iToken==2);
static_assert(prefixTokens[2].matchedString=="x" and prefixTokens[2].iToken==3);

/// Compile-time tokenizer recognizing runs of 'a' characters and spaces
constexpr auto tokenizer=
  pp::createTokenizer<"a+"," +">();

/// Spaces are retained as tokens, so this input produces three entries
constexpr auto tokens=
  tokenizer.tokenize<"aa a">();

static_assert(tokens.size()==3);

/// Minimal fixed-size grammar with a named reduction action
/// Namespace scope allows the parse-tree factory to reference it as a template argument
constexpr auto grammar=
  pp::createGrammar<"smoke { start: 'a' [value]; }">();

/// Flattened tree: shifted 'a', the 'value' reduction, and the augmented start reduction
constexpr auto tree=
  pp::internal::createParseTree<grammar,"a">();

static_assert(tree.size()==3);
static_assert(std::get<pp::internal::ShiftNode>(tree[0]).txt=="a");
static_assert(std::get<pp::internal::ReduceNode>(tree[1]).iAction==0);

/////////////////////////////////////////////////////////////////

/// Checks agreement between runtime and compile-time parse trees
/// Returns 1 for a size mismatch, 2 for shifted text, or 3 for reduction metadata
int main()
{
  /// Runtime version of the same grammar and input
  const auto runtimeGrammar=
    pp::createGrammar("smoke { start: 'a' [value]; }");
  
  const auto runtimeTree=
    pp::internal::createParseTree(runtimeGrammar,"a");
  
  if(runtimeTree.size()!=tree.size())
    return 1;
  
  if(std::get<pp::internal::ShiftNode>(runtimeTree[0]).txt!=
     std::get<pp::internal::ShiftNode>(tree[0]).txt)
    return 2;
  
  // Compare both reductions, including the augmented start production.
  for(size_t i=1;i<tree.size();i++)
    {
      const auto& runtimeReduction=
	std::get<pp::internal::ReduceNode>(runtimeTree[i]);
      
      const auto& constantReduction=
	std::get<pp::internal::ReduceNode>(tree[i]);
      
      if(runtimeReduction.iAction!=constantReduction.iAction or
         runtimeReduction.nSubNodes!=constantReduction.nSubNodes)
	return 3;
    }
  
  return 0;
}
