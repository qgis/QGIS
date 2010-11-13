/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   astyle.h
 *
 *   This file is a part of "Artistic Style" - an indentation and
 *   reformatting tool for C, C++, C# and Java source files.
 *   http://astyle.sourceforge.net
 *
 *   The "Artistic Style" project, including all files needed to
 *   compile it, is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this project; if not, write to the
 *   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA  02110-1301, USA.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#ifndef ASTYLE_H
#define ASTYLE_H

#ifdef __VMS
#define __USE_STD_IOSTREAM 1
// #include <sstream>
#include <assert>
#else
#include <cassert>
#endif

#include <string.h>
#include <string>
#include <vector>
#include <cctype>

#ifdef _WIN32
#define STDCALL __stdcall
#define EXPORT  __declspec(dllexport)
#else
#define STDCALL
#define EXPORT
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996)  // secure version deprecation warnings for .NET 2005
#pragma warning(disable: 4267)  // 64 bit signed/unsigned loss of data
#endif

#ifdef __INTEL_COMPILER
#pragma warning(disable:  383)  // value copied to temporary, reference to temporary used
#pragma warning(disable:  444)  // destructor for base class is not virtual
#pragma warning(disable:  981)  // operands are evaluated in unspecified order
//#pragma warning(disable: 1418)  // external function with no prior declaration
#pragma warning(disable: 1419)  // external declaration in primary source file
#endif

using namespace std;

namespace astyle
{

enum FileType      { C_TYPE=0, JAVA_TYPE=1, SHARP_TYPE=2 };

/* The enums below are not recognized by 'vectors' in Microsoft Visual C++
   V5 when they are part of a namespace!!!  Use Visual C++ V6 or higher.
*/
enum BracketMode   { NONE_MODE, ATTACH_MODE, BREAK_MODE, BDAC_MODE };

enum BracketType   { NULL_TYPE = 0,
                     NAMESPACE_TYPE = 1,		// also a DEFINITION_TYPE
                     CLASS_TYPE = 2,			// also a DEFINITION_TYPE
                     DEFINITION_TYPE = 4,
                     COMMAND_TYPE = 8,
                     ARRAY_TYPE  = 16,          // arrays and enums
                     SINGLE_LINE_TYPE = 32
                   };

class ASSourceIterator
{
	public:
		ASSourceIterator() {}
		virtual ~ASSourceIterator() {}
		virtual bool hasMoreLines() const = 0;
		virtual string nextLine() = 0;
		virtual string peekNextLine() = 0;
		virtual void peekReset() = 0;
};

class ASResource
{
	public:
		void buildAssignmentOperators(vector<const string*> &assignmentOperators);
		void buildCastOperators(vector<const string*> &castOperators);
		void buildHeaders(vector<const string*> &headers, int fileType, bool beautifier=false);
		void buildNonAssignmentOperators(vector<const string*> &nonAssignmentOperators);
		void buildNonParenHeaders(vector<const string*> &nonParenHeaders, int fileType, bool beautifier=false);
		void buildOperators(vector<const string*> &operators);
		void buildPreBlockStatements(vector<const string*> &preBlockStatements);
		void buildPreCommandHeaders(vector<const string*> &preCommandHeaders);
		void buildPreDefinitionHeaders(vector<const string*> &preDefinitionHeaders);

	public:
		static const string AS_IF, AS_ELSE;
		static const string AS_DO, AS_WHILE;
		static const string AS_FOR;
		static const string AS_SWITCH, AS_CASE, AS_DEFAULT;
		static const string AS_TRY, AS_CATCH, AS_THROWS, AS_FINALLY;
		static const string AS_PUBLIC, AS_PROTECTED, AS_PRIVATE;
		static const string AS_CLASS, AS_STRUCT, AS_UNION, AS_INTERFACE, AS_NAMESPACE, AS_EXTERN;
		static const string AS_STATIC;
		static const string AS_CONST;
		static const string AS_SYNCHRONIZED;
		static const string AS_OPERATOR, AS_TEMPLATE;
		static const string AS_OPEN_BRACKET, AS_CLOSE_BRACKET;
		static const string AS_OPEN_LINE_COMMENT, AS_OPEN_COMMENT, AS_CLOSE_COMMENT;
		static const string AS_BAR_DEFINE, AS_BAR_INCLUDE, AS_BAR_IF, AS_BAR_EL, AS_BAR_ENDIF;
		static const string AS_RETURN;
		static const string AS_CIN, AS_COUT, AS_CERR;
		static const string AS_ASSIGN, AS_PLUS_ASSIGN, AS_MINUS_ASSIGN, AS_MULT_ASSIGN;
		static const string AS_DIV_ASSIGN, AS_MOD_ASSIGN, AS_XOR_ASSIGN, AS_OR_ASSIGN, AS_AND_ASSIGN;
		static const string AS_GR_GR_ASSIGN, AS_LS_LS_ASSIGN, AS_GR_GR_GR_ASSIGN, AS_LS_LS_LS_ASSIGN;
		static const string AS_EQUAL, AS_PLUS_PLUS, AS_MINUS_MINUS, AS_NOT_EQUAL, AS_GR_EQUAL, AS_GR_GR_GR, AS_GR_GR;
		static const string AS_LS_EQUAL, AS_LS_LS_LS, AS_LS_LS, AS_ARROW, AS_AND, AS_OR;
		static const string AS_COLON_COLON, AS_PAREN_PAREN, AS_BLPAREN_BLPAREN;
		static const string AS_PLUS, AS_MINUS, AS_MULT, AS_DIV, AS_MOD, AS_GR, AS_LS;
		static const string AS_NOT, AS_BIT_XOR, AS_BIT_OR, AS_BIT_AND, AS_BIT_NOT;
		static const string AS_QUESTION, AS_COLON, AS_SEMICOLON, AS_COMMA;
		static const string AS_ASM;
		static const string AS_FOREACH, AS_LOCK, AS_UNSAFE, AS_FIXED;
		static const string AS_GET, AS_SET, AS_ADD, AS_REMOVE;
		static const string AS_CONST_CAST, AS_DYNAMIC_CAST, AS_REINTERPRET_CAST, AS_STATIC_CAST;
};

class ASBeautifier : protected ASResource
{
	public:
		ASBeautifier();
		virtual ~ASBeautifier();
		virtual void init(ASSourceIterator* iter); // pointer to dynamically created iterator.
		void init();
		virtual bool hasMoreLines() const;
		virtual string nextLine();
		virtual string beautify(const string &line);
		void setTabIndentation(int length = 4, bool forceTabs = false);
		void setSpaceIndentation(int length = 4);
		void setMaxInStatementIndentLength(int max);
		void setMinConditionalIndentLength(int min);
		void setClassIndent(bool state);
		void setSwitchIndent(bool state);
		void setCaseIndent(bool state);
		void setBracketIndent(bool state);
		void setBlockIndent(bool state);
		void setNamespaceIndent(bool state);
		void setLabelIndent(bool state);
		void setCStyle();
		void setJavaStyle();
		void setSharpStyle();
		void setEmptyLineFill(bool state);
		void setPreprocessorIndent(bool state);
		int  getIndentLength(void);
		string getIndentString(void);
		char peekNextChar(const string &line, int i) const;
		bool getCaseIndent(void);
		bool getCStyle(void);
		bool getJavaStyle(void);
		bool getSharpStyle(void);
		bool getEmptyLineFill(void);

	protected:
		int getNextProgramCharDistance(const string &line, int i);
		const string *findHeader(const string &line, int i,
		                         const vector<const string*> &possibleHeaders,
		                         bool checkBoundry = true);
		string trim(const string &str);
		int  indexOf(vector<const string*> &container, const string *element);
		int  fileType;
		bool isCStyle;
		bool isJavaStyle;
		bool isSharpStyle;

		// variables set by ASFormatter - must be updated in activeBeautifierStack
		int  inLineNumber;
		bool lineCommentNoBeautify;
		bool isNonInStatementArray;
		bool isSharpAccessor;

	private:
		ASBeautifier(const ASBeautifier &copy);
		void operator=(ASBeautifier&); // not to be implemented

		void initStatic();
		void registerInStatementIndent(const string &line, int i, int spaceTabCount,
		                               int minIndent, bool updateParenStack);
		string preLineWS(int spaceTabCount, int tabCount);

		static vector<const string*> headers;
		static vector<const string*> nonParenHeaders;
		static vector<const string*> preBlockStatements;
		static vector<const string*> assignmentOperators;
		static vector<const string*> nonAssignmentOperators;

		ASSourceIterator *sourceIterator;
		vector<ASBeautifier*> *waitingBeautifierStack;
		vector<ASBeautifier*> *activeBeautifierStack;
		vector<int> *waitingBeautifierStackLengthStack;
		vector<int> *activeBeautifierStackLengthStack;
		vector<const string*> *headerStack;
		vector< vector<const string*>* > *tempStacks;
		vector<int> *blockParenDepthStack;
		vector<bool> *blockStatementStack;
		vector<bool> *parenStatementStack;
		vector<int> *inStatementIndentStack;
		vector<int> *inStatementIndentStackSizeStack;
		vector<int> *parenIndentStack;
		vector<bool> *bracketBlockStateStack;
		string indentString;
		const string *currentHeader;
		const string *previousLastLineHeader;
		const string *immediatelyPreviousAssignmentOp;
		const string *probationHeader;
		bool isInQuote;
		bool isInVerbatimQuote;
		bool haveLineContinuationChar;
		bool isInComment;
		bool isInCase;
		bool isInQuestion;
		bool isInStatement;
		bool isInHeader;
		bool isInOperator;
		bool isInTemplate;
		bool isInDefine;
		bool isInDefineDefinition;
		bool classIndent;
		bool isInClassHeader;
		bool isInClassHeaderTab;
		bool switchIndent;
		bool caseIndent;
		bool namespaceIndent;
		bool bracketIndent;
		bool blockIndent;
		bool labelIndent;
		bool preprocessorIndent;
		bool isInConditional;
		bool isMinimalConditinalIndentSet;
		bool shouldForceTabIndentation;
		bool emptyLineFill;
		bool backslashEndsPrevLine;
		bool blockCommentNoIndent;
		bool blockCommentNoBeautify;
		bool previousLineProbationTab;
		int  minConditionalIndent;
		int  parenDepth;
		int  indentLength;
		int  blockTabCount;
		int  leadingWhiteSpaces;
		int  maxInStatementIndent;
		int  templateDepth;
		int  prevFinalLineSpaceTabCount;
		int  prevFinalLineTabCount;
		int  defineTabCount;
		char quoteChar;
		char prevNonSpaceCh;
		char currentNonSpaceCh;
		char currentNonLegalCh;
		char prevNonLegalCh;

	protected:    // inline functions
		// check if a specific character can be used in a legal variable/method/class name
		inline bool isLegalNameChar(char ch) const {
			return (isalnum(ch) || ch == '.' || ch == '_'
			        || (isJavaStyle && ch == '$')
			        || (isCStyle && ch == '~')
			        || (isSharpStyle && ch == '@'));	// may be used as a prefix
		}

		// check if a specific character is a whitespace character
		inline bool isWhiteSpace(char ch) const {
			return (ch == ' ' || ch == '\t');
		}
};


class ASEnhancer
{
	public:
		// functions
		ASEnhancer();
		~ASEnhancer();
		void init(int, string, bool, bool, bool, bool, bool);
		void enhance(string &line);
		bool findKeyword(const string &line, int i, const char *header) const;
		char peekNextChar(const string &line, int i) const;

	private:
		// options from command line or options file
		// X ending prevents conflict with ASBeautifier variables
		int  indentLengthX;
		bool useTabsX;
		bool isCStyleX;
		bool isJavaStyleX;
		bool isSharpStyleX;
		bool caseIndentX;
		bool emptyLineFillX;

		// parsing variables
		int  lineNumber;
		bool isInQuote;
		bool isInComment;
		char quoteChar;

		// unindent variables
		int  bracketCount;
		int  switchDepth;
		bool lookingForCaseBracket;
		bool unindentNextLine;

		// struct used by ParseFormattedLine function
		// contains variables used to unindent the case blocks
		struct switchVariables {
			int  switchBracketCount;
			int  unindentDepth;
			bool unindentCase;
		};

		switchVariables sw;                      // switch variables struct
		vector<switchVariables>  swVector;       // stack vector of switch variables

		// event table variables
		bool nextLineIsEventTable;				// begin event table is reached
		bool isInEventTable;					// need to indent an event table

		// stringstream for trace
		stringstream *traceOut;

	private:    // private functions
		int  indentLine(string  &line, const int indent) const;
		int  unindentLine(string  &line, const int unindent) const;

	private:    // inline functions
		// check if a specific character can be used in a legal variable/method/class name
		inline bool isLegalNameCharX(char ch) const {
			return (isalnum(ch) || ch == '.' || ch == '_'
			        || (isJavaStyleX && ch == '$')
			        || (isCStyleX && ch == '~')
			        || (isSharpStyleX && ch == '@'));	// may be used as a prefix

		}

		// check if a specific character is a whitespace character
		inline bool isWhiteSpaceX(char ch) const {
			return (ch == ' ' || ch == '\t');
		}
};


class ASFormatter : public ASBeautifier, private ASEnhancer
{
	public:
		ASFormatter();
		virtual ~ASFormatter();
		virtual void init(ASSourceIterator* iter);
		virtual bool hasMoreLines() const;
		virtual string nextLine();
		void setBracketFormatMode(BracketMode mode);
		void setBreakClosingHeaderBracketsMode(bool state);
		void setOperatorPaddingMode(bool mode);
		void setParensOutsidePaddingMode(bool mode);
		void setParensInsidePaddingMode(bool mode);
		void setParensUnPaddingMode(bool state);
		void setBreakOneLineBlocksMode(bool state);
		void setSingleStatementsMode(bool state);
		void setTabSpaceConversionMode(bool state);
		void setBreakBlocksMode(bool state);
		void setBreakClosingHeaderBlocksMode(bool state);
		void setBreakElseIfsMode(bool state);
		string traceFileName;

	private:
		void ASformatter(ASFormatter &copy);            // not to be imlpemented
		void operator=(ASFormatter&);                  // not to be implemented
		void staticInit();
		void goForward(int i);
		void trimNewLine();
		char peekNextChar() const;
		BracketType getBracketType();
		bool getNextChar();
		bool isBeforeComment() const;
		bool isBeforeLineEndComment(int startPos) const;
		bool isNextWordSharpAccessor() const;
		bool isPointerOrReference() const;
		bool isUnaryMinus() const;
		bool isInExponent() const;
		bool isOneLineBlockReached() const;
		bool lineBeginsWith(char charToCheck) const;
		void appendChar(char ch, bool canBreakLine = true);
		void appendCharInsideComments();
		void appendSequence(const string &sequence, bool canBreakLine = true);
		void appendSpacePad();
		void appendSpaceAfter();
		void breakLine();
		void padOperators(const string *newOperator);
		void padParens();
		void formatBrackets(BracketType bracketType);
		void formatArrayBrackets(BracketType bracketType, bool isOpeningArrayBracket);
		void adjustComments();
		const string *findHeader(const vector<const string*> &headers, bool checkBoundry = true);

		static vector<const string*> headers;
		static vector<const string*> nonParenHeaders;
		static vector<const string*> preDefinitionHeaders;
		static vector<const string*> preCommandHeaders;
		static vector<const string*> operators;
		static vector<const string*> assignmentOperators;
		static vector<const string*> castOperators;

		ASSourceIterator *sourceIterator;
		vector<const string*> *preBracketHeaderStack;
		vector<BracketType> *bracketTypeStack;
		vector<int> *parenStack;
		string readyFormattedLine;
		string currentLine;
		string formattedLine;
		const string *currentHeader;
		const string *previousOperator;    // used ONLY by pad=oper
		char currentChar;
		char previousChar;
		char previousNonWSChar;
		char previousCommandChar;
		char quoteChar;
		int  charNum;
		int  spacePadNum;
		int  templateDepth;
		int  traceLineNumber;
		size_t formattedLineCommentNum;		// comment location on formattedLine
		size_t previousReadyFormattedLineLength;
		BracketMode bracketFormatMode;
		BracketType previousBracketType;
		bool isVirgin;
		bool shouldPadOperators;
		bool shouldPadParensOutside;
		bool shouldPadParensInside;
		bool shouldUnPadParens;
		bool shouldConvertTabs;
		bool isInLineComment;
		bool isInComment;
		bool isInPreprocessor;
		bool isInTemplate;   // true both in template definitions (e.g. template<class A>) and template usage (e.g. F<int>).
		bool doesLineStartComment;
		bool isInQuote;
		bool isInVerbatimQuote;
		bool haveLineContinuationChar;
		bool isInQuoteContinuation;
		bool isInBlParen;
		bool isSpecialChar;
		bool isNonParenHeader;
		bool foundQuestionMark;
		bool foundPreDefinitionHeader;
		bool foundNamespaceHeader;
		bool foundClassHeader;
		bool foundPreCommandHeader;
		bool foundCastOperator;
		bool isInLineBreak;
		bool endOfCodeReached;
		bool lineCommentNoIndent;
		bool isLineReady;
		bool isPreviousBracketBlockRelated;
		bool isInPotentialCalculation;
		bool isCharImmediatelyPostComment;
		bool isPreviousCharPostComment;
		bool isCharImmediatelyPostLineComment;
		bool isCharImmediatelyPostOpenBlock;
		bool isCharImmediatelyPostCloseBlock;
		bool isCharImmediatelyPostTemplate;
		bool shouldBreakOneLineBlocks;
		bool shouldReparseCurrentChar;
		bool shouldBreakOneLineStatements;
//		bool shouldBreakLineAfterComments;
		bool shouldBreakClosingHeaderBrackets;
		bool shouldBreakElseIfs;
		bool passedSemicolon;
		bool passedColon;
		bool isImmediatelyPostComment;
		bool isImmediatelyPostLineComment;
		bool isImmediatelyPostEmptyBlock;
		bool isImmediatelyPostPreprocessor;

		bool shouldBreakBlocks;
		bool shouldBreakClosingHeaderBlocks;
		bool isPrependPostBlockEmptyLineRequested;
		bool isAppendPostBlockEmptyLineRequested;

		bool prependEmptyLine;
		bool appendOpeningBracket;
		bool foundClosingHeader;

		bool isInHeader;
		bool isImmediatelyPostHeader;
		bool isInCase;
		bool isInSharpGenericDefinition;

	private:    // inline functions
		// append the CURRENT character (curentChar)to the current formatted line.
		inline void appendCurrentChar(bool canBreakLine = true) {
			appendChar(currentChar, canBreakLine);
		}

		// check if a specific sequence exists in the current placement of the current line
		inline bool isSequenceReached(const char *sequence) const {
			return currentLine.compare(charNum, strlen(sequence), sequence) == 0;
		}
};

}   // end of namespace astyle


#endif // closes ASTYLE_H
