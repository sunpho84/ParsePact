#ifndef PARSE_CONST_PACT_HPP
#define PARSE_CONST_PACT_HPP

/* Optional compile-time construction and fixed-size storage
 *
 * Hosted at: https://github.com/sunpho84/parse-pact
 *
 * Copyright (C) 2024 Francesco Sanfilippo - fr.sanfilippo@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "parsePact.hpp"

namespace pp::internal
{
  /// Allows a C-string to be used as template argument
  template <size_t N>
  struct CtString
  {
    /// Stored string
    char str[N];
    
    /// Full length including possibly terminator
    static constexpr size_t size()
    {
      return N;
    }
    
    /// Creates the CtSring copying the passed string
    inline constexpr CtString(char const(&arr)[N])
    {
      for(size_t i=0;i<N;i++)
	this->str[i]=arr[i];
    }
    
    /// Creates empty CtSring
    inline constexpr CtString()
    {
      for(size_t i=0;i<N;i++)
	this->str[i]='\0';
    }
  };
  
  /// Creates the CtSring copying filling from a const char, not necessarily null terminated
  template <const std::pair<const char*,const char*> S>
  inline constexpr auto getCtString()
    {
      constexpr size_t N=
	S.second-S.first+1;
      
      /// Resulting string with a terminating null character
      CtString<N> res{};
      
      for(size_t i=0;i<N-1;i++)
	res.str[i]=S.first[i];
      
      res.str[N-1]='\0';
      
      return res;
    }
  
  /// A minimal struct version of the std::string_view, which can be passed as template argument
  struct CtStringView
  {
    /// Begin of the string
    const char* begin{};
    
    /// End of the string, past the last character
    const char* end{};
    
    /// Cast to std::string_view
    constexpr operator std::string_view() const
    {
      return {begin,end};
    }
    
    /// Default constructor
    constexpr CtStringView()=default;
    
    /// Construct from a std::string_view
    constexpr CtStringView(const std::string_view& s) :
      begin(s.begin()),end(s.end())
    {
    }
    
    /// Construct from a CtString
    template <size_t N>
    constexpr CtStringView(const CtString<N>& s) :
      begin(&s.str[0]),end(&s.str[N])
    {
    }
    
    /// Assigns from a std::string_view
    constexpr CtStringView& operator=(const std::string_view& s)
    {
      begin=s.begin();
      end=s.end();
      
      return *this;
    }
  };
  
  /// Structural array
  template <typename T,
	      size_t N>
  struct Array
  {
    /// Stored elements
    T data[N];
    
    /// Access an element without modifying it
    constexpr const T& operator[](const size_t& i) const
    {
      return data[i];
    }
    
    /// Access an element for modification
    constexpr T& operator[](const size_t& i)
    {
      return data[i];
    }
    
    /// Returns the beginning of the constant array
    constexpr const T* begin() const
    {
      return data;
    }
    
    /// Returns the beginning of the array
    constexpr T* begin()
    {
      return data;
    }
    
    /// Returns the end of the constant array
    constexpr const T* end() const
    {
      return data+N;
    }
    
    /// Returns the end of the array
    constexpr T* end()
    {
      return data+N;
    }
    
    /// Returns the number of elements
    constexpr const size_t size() const
    {
      return N;
    }
  };
  
  /// Parameters to hold a vector of vectors of heterogeneous size, on the stack
  struct Stack2DVectorPars
  {
    /// Total number of entries
    size_t nEntries;
    
    /// Number of rows
    size_t nRows;
    
    /// Detects if the table has no rows
    constexpr bool isNull() const
    {
      return nRows==0;
    }
  };
  
  /// Holds a 2D table with rows of heterogeneous length into a single
  /// array, allocated on stack
  template <typename T,
	    Stack2DVectorPars Pars>
  struct Stack2DVector
  {
    /// Parameters to specify a row of the table
    struct RowPars
    {
      /// Begin of the row
      size_t begin;
      
      /// Row length
      size_t size;
    };
    
    /// Holds the data
    Array<T,Pars.nEntries> data;
    
    /// Holds the parameters defining the rows positions
    Array<RowPars,Pars.nRows> rowPars;
    
    /// Access element (row, col)
    constexpr const T& operator()(const size_t& row,
				  const size_t& col) const
    {
      return data[rowPars[row].begin+col];
    }
    
    /// Returns the length of a row
    constexpr const size_t& rowSize(const size_t& row) const
    {
      return rowPars[row].size;
    }
    
    /// Returns the total size
    constexpr const size_t size() const
    {
      return rowPars.size();
    }
    
    /// Fills the entries using the function getRow
    template <typename F>
    constexpr void fillWith(const F& getRow)
    {
      for(size_t iEntry=0,iRow=0;iRow<Pars.nRows;iRow++)
	{
	  rowPars[iRow].begin=iEntry;
	  
	  for(const T& entry : getRow(iRow))
	    data[iEntry++]=entry;
	  
	  rowPars[iRow].size=iEntry-rowPars[iRow].begin;
	}
    }
  };
  
  /// Makes a reduction of the whole vector f, using the the function
  /// f for unary or binary reductions.
  ///
  /// N.B: the function takes the reduction as a second argument, as
  /// it might be optional
  template <typename T,
	    typename F>
  constexpr auto reduce(const std::vector<T>& v,
			const F& f) -> decltype(f(std::declval<T>()))
  {
    if(v.size())
      {
	/// First element
	auto t=f(v.front());
	
	for(size_t i=1;i<v.size();i++)
	  t=f(v[i],t);
	
	return t;
      }
    else
      return {};
  }
  
  /// Returns the total number of elements of a vector of vectors
  template <typename T>
  constexpr size_t vectorOfVectorsTotalEntries(const std::vector<std::vector<T>>& v)
  {
    return reduce(v,
		  [](const std::vector<T>& s,
		     std::optional<size_t> x=std::nullopt)
		  {
		    if(not x)
		      x=0;
		    
		    return *x+s.size();
		  });
  }
  
  /// Sizes defining the regex matcher
  struct RegexMatcherSizes
  {
    /// Number of DStates
    const size_t nDStates;
    
    /// Number of transitions
    const size_t nTransitions;
    
    /// Detects if the machine is empty
    constexpr bool isNull() const
    {
      return nDStates==0 and nTransitions==0;
    }
  };
  
  /// Gets the parameters needed to build the constexpr lexer
  constexpr RegexMatcherSizes getSizes(const RegexMatcher& matcher)
  {
    return {.nDStates=matcher.dStates.size(),
	      .nTransitions=matcher.transitions.size()};
  }

  /// Regex matcher storing the machine in fixed size tables
  template <RegexMatcherSizes Specs>
  struct RegexMatcherCt :
    BaseRegexMatcher<RegexMatcherCt<Specs>>
  {
    using BaseRegexMatcher<RegexMatcherCt<Specs>>::match;

    /// Match a string - alternative syntax which work only with compile-time string, included for consistency
    template <CtString str>
    constexpr std::optional<RegexMatchingResult> match() const
    {
      return match(str.str);
    }
    
    /// States of the machine
    Array<RegexMatcherDState,Specs.nDStates> dStates;
    
    /// Transitions among states
    Array<RegexMatcherDStateTransition,Specs.nTransitions> transitions;
    
    /// Create from dynamic-sized lexer
    constexpr RegexMatcherCt(const RegexMatcher& oth)
    {
      // for(size_t i=0;i<Specs.;i++)
      // 	regexes[i]=*(std::launder
      // 		     (&oth.regexes[i]));
      
      for(size_t i=0;i<Specs.nDStates;i++)
	dStates[i]=oth.dStates[i];
      
      for(size_t i=0;i<Specs.nTransitions;i++)
	transitions[i]=oth.transitions[i];
    }
    
    /// Default constructor
    constexpr RegexMatcherCt()=default;
  };
  
  /// Builds a fixed-size matcher using the shared runtime construction algorithm.
  template <RegexMatcherSizes RMS>
  constexpr auto createRegexMatcher(const std::vector<std::string_view>& regexes)
  {
    if constexpr(RMS.isNull())
      return createRegexMatcher(regexes);
    else
      return RegexMatcherCt<RMS>(createRegexMatcher(regexes));
  }
  
  template <RegexMatcherSizes RMS, typename...T>
    requires(std::is_same_v<char,T> and ...)
  constexpr auto createRegexMatcher(const T*...str)
  {
    return createRegexMatcher<RMS>(std::vector<std::string_view>{str...});
  }
  
  /// Estimates the compile time regex matcher size for a set of regex
  constexpr RegexMatcherSizes estimateRegexMatcherSize(const std::vector<std::string_view>& regexes)
  {
    return getSizes(createRegexMatcher(regexes));
  }
  
  /// Estimates the compile time regex matcher size for a list of regexes
  template <typename...T>
  requires(std::is_same_v<char,T> and ...)
  constexpr RegexMatcherSizes estimateRegexMatcherSize(const T*...str)
  {
    return estimateRegexMatcherSize(std::vector<std::string_view>{str...});
  }
  
  /// Create regex matcher from regexp, at constant time
  template <CtString...str>
    constexpr auto createRegexMatcher()
  {
    constexpr RegexMatcherSizes RMS=
      estimateRegexMatcherSize(str.str...);
    
    /// Needs to replace the matched regexes, which are cast to
    /// std::string_view and then to CtString and lose their
    /// consteprensess (seems to happen only on gcc)
    /// \todo verify
    auto res=createRegexMatcher<RMS>(str.str...);
    
    // res.regexes=std::array<CtStringView,sizeof...(str)>{(CtStringView)str...};
    
    return res;
  }
  
  /// Tokenizer at compile time, wrapping the regex matcher
  ///
  /// Wrapping the RegexMatcher in the template parameter allows to
  /// estimate the number of tokens and return them directly
  template <RegexMatcherCt _regexMatcher>
  struct TokenizerCt :
    BaseTokenizer<TokenizerCt<_regexMatcher>>
  {
    ///Regex matcher
    static constexpr RegexMatcherCt regexMatcher=_regexMatcher;
    
    /// Estimates the compile-time number of tokens
    template <CtString str>
    static constexpr size_t estimateNTokens()
    {
      return regexMatcher._tokenize(str.str).size();
    }
    
    /// Gives visibility to base tokenize
    using BaseTokenizer<TokenizerCt<_regexMatcher>>::tokenize;
    
    /// Break a constant time string into tokens
    template <CtString str>
    constexpr auto tokenize() const
    {
      constexpr size_t N=estimateNTokens<str>();
      
      const auto tokens=regexMatcher._tokenize(str.str);
      std::array<RegexMatchingResult,N> result{};
      std::copy(tokens.begin(),tokens.end(),result.begin());
      return result;
    }
  };
  
  /// Create tokenizer, at constant time
  template <CtString...str>
  constexpr TokenizerCt<createRegexMatcher<str...>()> createTokenizer()
  {
    return {};
  }
  
  /// Specifications of the grammar
  struct GrammarSpecs
  {
    /// Number of symbols
    const size_t nSymbols;
    
    const Stack2DVectorPars actionPars;
    
    const Stack2DVectorPars productionPars;
    
    const size_t nItems;
    
    const Stack2DVectorPars stateItemsPars;
    
    const Stack2DVectorPars transitionsOfStatesPars;
    
    const RegexMatcherSizes regexMachinePars;
    
    const size_t nRegexes;
    
    /// Detects if the grammar is empty
    constexpr bool isNull() const
    {
      return
	nSymbols==0 and
	actionPars.isNull() and
	productionPars.isNull() and
	nItems==0 and
	stateItemsPars.isNull() and
	transitionsOfStatesPars.isNull() and
	regexMachinePars.isNull();
    }
  };
  
  /// Gets the parameters needed to build the constexpr grammar
  constexpr GrammarSpecs getSizes(const Grammar& grammar)
  {
    return
	{.nSymbols=grammar.symbols.size(),
	 .actionPars{.nEntries=reduce(grammar.actionStrings,
				      [](const GrammarAction& a,
					 std::optional<size_t> x=std::nullopt)
				      {
					if(not x)
					  x=0;
					
					return *x+a.pars.size();
				      }),
	   .nRows=grammar.actionStrings.size()},
	 .productionPars{.nEntries=reduce(grammar.productions,
					  [](const GrammarProduction& p,
					     std::optional<size_t> x=std::nullopt)
					  {
					    if(not x)
					      x=0;
					    
					    return *x+p.iRhsList.size()+1;
					  }),
	   .nRows=grammar.productions.size()},
	 .nItems=grammar.items.size(),
	 .stateItemsPars{.nEntries=reduce(grammar.states,
					  [](const GrammarState& s,
					     std::optional<size_t> x=std::nullopt)
					  {
					    if(not x)
					      x=0;
					    
					    return *x+s.iItems.size();
					  }),
	   .nRows=grammar.states.size()},
	 .transitionsOfStatesPars{.nEntries=vectorOfVectorsTotalEntries(grammar.transitionsOfStates),
				  .nRows=grammar.transitionsOfStates.size()},
	 .regexMachinePars=getSizes(grammar.regexMatcher),
	.nRegexes=grammar.iSymbolOfRegex.size()};
  }

  /// Grammar in fixed size tables
  template <GrammarSpecs Specs>
  struct GrammarCt :
    BaseGrammar<GrammarCt<Specs>>
  {
    /// Symbols accepted by the grammar
    Array<BaseGrammarSymbol,Specs.nSymbols> symbols;
    
    /// Index of the symbols representing a production, for each state
    Stack2DVector<size_t,Specs.productionPars> productionsData;
    
    /// Action pars associated to each production
    Stack2DVector<GrammarActionPar,Specs.actionPars> actionsData;
    
    /// Action associated to each production
    Array<std::string_view,Specs.actionPars.nRows> actionsName;
    
    /// Original action annotations and per-production action indices.
    Array<std::string_view,Specs.actionPars.nRows> actionStrings;
    Array<std::optional<size_t>,Specs.productionPars.nRows> productionActions;
    
    /// Items representing the states, defined in term of index of production and position
    Array<GrammarItem,Specs.nItems> items;
    
    /// Index of the items representing a state, for each state
    Stack2DVector<size_t,Specs.stateItemsPars> stateIItemsData;
    
    /// Transitions for each state, defined in terms of the symbol, the
    /// type (shift/reduce), and target state (if shift) or the production to be
    /// applied (if reduce)
    Stack2DVector<GrammarTransition,Specs.transitionsOfStatesPars> transitionsOfStatesData;
    
    /// Matcher stored in fixed size tables
    RegexMatcherCt<Specs.regexMachinePars> regexMatcher;
    
    /// Grammar symbol corresponding to each regex token
    Array<size_t,Specs.nRegexes> iSymbolOfRegex;
    
    static_assert(Specs.transitionsOfStatesPars.nRows==Specs.stateItemsPars.nRows,"number of rows for transitionsOfStates and stateItems do not match");
    
    /// Returns the number of states
    static constexpr size_t nStates()
    {
      return Specs.transitionsOfStatesPars.nRows;
    }
    
    /// Returns the number of actions
    static constexpr size_t nActions()
    {
      return Specs.actionPars.nRows;
    }
    
    /// Returns the number of productions
    static constexpr size_t nProductions()
    {
      return Specs.productionPars.nRows;
    }
    
    /// Returns the number of nTranstitions
    constexpr size_t nTransitionsOfState(const size_t& iState) const
    {
      return transitionsOfStatesData.rowSize(iState);
    }
    
    /// Return the given transition for the required state
    constexpr const GrammarTransition& transitionOfState(const size_t& iState,
							 const size_t& iTransition) const
    {
      return transitionsOfStatesData(iState,iTransition);
    }
    
    /// Accessor to a production
    struct ProductionRef
    {
      /// Original grammar
      const GrammarCt* g;
      
      /// Index of the production
      const size_t iProduction;
      
      std::optional<size_t> iAction;
      
      /// Number of rhs symbols
      constexpr const size_t nRhs() const
      {
	return g->productionsData.rowSize(iProduction)-1;
      }
      
      /// Index of the lhs symbol
      constexpr const size_t& iLhs() const
      {
	return g->productionsData(iProduction,0);
      }
      
      /// Index of the iiRhs-th rhs symbol
      constexpr const size_t& iRhs(const size_t& iiRhs) const
      {
	return g->productionsData(iProduction,iiRhs+1);
      }
      
      /// Describes the production
      constexpr std::string describe(auto&&...) const
      {
	std::string out=(std::string)g->symbols[iLhs()].name+": ";
	
	for(size_t iiRhs=0;iiRhs<nRhs();iiRhs++)
	  out+=(std::string)g->symbols[iRhs(iiRhs)].name+" ";
	
	return out;
      }
    };
  
    /// Returns a reference to a production
    constexpr inline ProductionRef production(const size_t& iProduction) const
    {
      return {this,iProduction,productionActions[iProduction]};
    }
    
    /// Reference to an item
    struct ItemRef
    {
      /// Underlying grammar
      const GrammarCt* g;
      
      /// Index of the item
      const size_t iItem;
      
      /// Describes the item
      constexpr std::string describe(auto&&...)
      {
	/// Item referred
	const auto& item=g->items[iItem];
	
	/// Reference toi the production
	const ProductionRef p=g->production(item.iProduction);
	
	/// Resulting string
	std::string out=(std::string)g->symbols[p.iLhs()].name+": ";
	
	for(size_t iIRhs=0,max=p.nRhs();iIRhs<=max;iIRhs++)
	  {
	    if(iIRhs==item.position)
	      out+=" . ";
	    
	    if(iIRhs<max)
	      {
		out+=" ";
		out+=g->symbols[p.iRhs(iIRhs)].name;
	      }
	  }
	
	return out;
      }
    };
    
    /// Gets a reference to the iItem item of the grammar
    constexpr ItemRef item(const size_t& iItem) const
    {
      return {this,iItem};
    }
    
    /// Reference to a state
    struct StateRef
    {
      /// Underlying grammar
      const GrammarCt* g;
      
      /// Index of the state
      const size_t iState;
      
      /// Gets the number of item
      constexpr const size_t nItems() const
      {
	return g->stateIItemsData.rowSize(iState);
      }
      
      /// Returns the index of the iiItem-th item
      constexpr const size_t& iItem(const size_t& iiItem) const
      {
	return g->stateIItemsData(iState,iiItem);
      }
      
      /// Returns a reference to the iiItem-th item
      constexpr ItemRef item(const size_t& iiItem) const
      {
	return g->item(iItem(iiItem));
      }
      
      /// Gets the number of item
      constexpr const size_t nTransitions() const
      {
	return g->nTransitionsOfState(iState);
      }
      
      /// Returns a reference to the iTransition-th transition
      constexpr const GrammarTransition& transition(const size_t& iTransition) const
      {
	return g->transitionsOfStatesData(iState,iTransition);
      }
      
      /// Describes the state
      constexpr std::string describe(const std::string& pref="") const
      {
	/// Returned string
	std::string out;
	
	for(size_t iiItem=0;iiItem<nItems();iiItem++)
	  out+=pref+"| "+item(iiItem).describe()+"\n";
	
	out+=pref+"\n"+pref+"accepting:\n";
	
	for(size_t iTransition=0;iTransition<nTransitions();iTransition++)
	  {
	    const GrammarTransition& t=transition(iTransition);
	    out+=pref+"   symbol \""+(std::string)g->symbols[t.iSymbol].name+"\" ";
	    if(t.type==GrammarTransition::Type::SHIFT)
	      out+=" SHIFTING to state #";
	    else
	      out+=" REDUCING with production #";
	    out+=toString(t.iStateOrProduction)+"\n";
	  }
	
	return out;
      }
    };
    
    /// Returns a reference to the state
    constexpr StateRef state(const size_t& iState) const
    {
      return {this,iState};
    }
    
    /// Describes a state
    constexpr inline std::string describeState(const size_t& iState,
					       const std::string& pref="") const
    {
      return state(iState).describe(pref);
    }
    
    /// Describes a transition of the given state
    constexpr inline std::string describeStateTransition(const size_t& iState,
							 const size_t& iTransition,
							 const std::string& pref="") const
    {
      const GrammarTransition& transition=
	transitionsOfStatesData(iState,iTransition);
      
      /// Returned string
      std::string out;
      
      if(not std::is_constant_evaluated())
	{
	  out+=pref+"\"";
	  out+=symbols[transition.iSymbol].name;
	  out+="\" ";
	  if(const size_t& i=transition.iStateOrProduction;transition.type==GrammarTransition::SHIFT)
	    {
	      out+="transits to state "+std::to_string(i)+":\n";
	      out+=state(i).describe(pref+"       ");
	    }
	  else
	    out+=pref+"induces a reduce transition using production: "+production(i).describe(symbols)+"\n";
	}
      
      return out;
    }
    
    /// Create from dynamic-sized grammar
    constexpr GrammarCt(const Grammar& oth)
    {
      this->iStartSymbol=oth.iStartSymbol;
      
      this->iEndSymbol=oth.iEndSymbol;
      
      this->iErrorSymbol=oth.iErrorSymbol;
      
      this->iWhitespaceSymbol=oth.iWhitespaceSymbol;
      
      for(size_t iSymbol=0;iSymbol<Specs.nSymbols;iSymbol++)
	this->symbols[iSymbol]=(const BaseGrammarSymbol&)oth.symbols[iSymbol];
      
      productionsData.fillWith([&oth](const size_t& iProduction)
      {
	const GrammarProduction& p=oth.productions[iProduction];
	
	std::vector<size_t> res(p.iRhsList.size()+1);
	res[0]=p.iLhs();
	for(size_t iiRhs=0;iiRhs<p.iRhsList.size();iiRhs++)
	  res[iiRhs+1]=p.iRhsList[iiRhs];
	
	return res;
      });
      
      for(size_t iItem=0;iItem<Specs.nItems;iItem++)
	this->items[iItem]=oth.items[iItem];

      for(size_t iProduction=0;iProduction<nProductions();iProduction++)
        productionActions[iProduction]=oth.productions[iProduction].iAction;
      
      stateIItemsData.fillWith([&oth](const size_t& iState)->const std::vector<size_t>&{return oth.states[iState].iItems;});
      
      transitionsOfStatesData.fillWith([&oth](const size_t& iState)->const std::vector<GrammarTransition>&{return oth.transitionsOfStates[iState];});
      
      std::copy(oth.iSymbolOfRegex.begin(),oth.iSymbolOfRegex.end(),iSymbolOfRegex.begin());

      /// Parsed action annotations, used to fill the fixed-size action tables
      std::vector<GrammarAction> actions;
      for(size_t iAction=0;iAction<nActions();iAction++)
	{
	  actions.emplace_back(oth.actionStrings[iAction]);
	  actionStrings[iAction]=oth.actionStrings[iAction];
	  actionsName[iAction]=actions[iAction].name;
	}
      actionsData.fillWith([&actions](const size_t& iAction)->const std::vector<GrammarActionPar>&{return actions[iAction].pars;});
      
      regexMatcher=oth.regexMatcher;
    }
  };
  
  /// Create grammar from string
  template <GrammarSpecs GS>
  constexpr auto createGrammar(const std::string_view& str)
  {
    /// Creates the grammar
    Grammar grammar=str;
    
    if constexpr(GS.isNull())
      return grammar;
    else
      return GrammarCt<GS>(grammar);
  }
  
  /// Estimates the grammar size
  constexpr GrammarSpecs estimateGrammarSize(const std::string_view& str)
  {
    return getSizes(createGrammar(str));
  }
  
  /// Create grammar taking the grammar as template parameter
  template <CtString str>
  constexpr auto createGrammar()
  {
    constexpr GrammarSpecs GS=
      estimateGrammarSize(str.str);
    
    return createGrammar<GS>(str.str);
  }
  
  /// Specifications of the ParseTree
  struct ParseTreeSpecs
  {
    /// Total number of nodes
    const size_t nNodes;
    
    /// Detects if the ParseTree is empty
    constexpr bool isNull() const
    {
      return nNodes==0;
    }
  };
  
  /// Copies a runtime parse tree into fixed-size storage.
  template <ParseTreeSpecs Specs, typename G>
  constexpr auto createParseTree(const G& grammar,
				 std::string_view input)
  {
    const auto tree=createParseTree(grammar,input);
    if constexpr(Specs.isNull()) return tree;
    else
      {
        if(tree.size()!=Specs.nNodes) errorEmitter("Parse tree size mismatch");
        std::array<FlattenedParseTreeNode,Specs.nNodes> result{};
        std::copy(tree.begin(),tree.end(),result.begin());
        return result;
      }
  }
  
  template <typename G>
  constexpr ParseTreeSpecs estimateParseTreeSize(const G& grammar,
						 std::string_view input)
  {
    return {createParseTree(grammar,input).size()};
  }
  
  template <const auto& grammar, CtString input>
  constexpr auto createParseTree()
  {
    constexpr auto specs=estimateParseTreeSize(grammar,input.str);
    return createParseTree<specs>(grammar,input.str);
  }
}

namespace pp
{
  using internal::RegexMatcherCt;
  using internal::TokenizerCt;
  using internal::createRegexMatcher;
  using internal::createTokenizer;
  using internal::createGrammar;
}

#endif
