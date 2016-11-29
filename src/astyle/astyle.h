/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   astyle.h

 *   Copyright (C) 2014 by Jim Pattee
 *   <http://www.gnu.org/licenses/lgpl-3.0.html>
 *
 *   This file is a part of Artistic Style - an indentation and
 *   reformatting tool for C, C++, C# and Java source files.
 *   <http://astyle.sourceforge.net>
 *
 *   Artistic Style is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Artistic Style is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with Artistic Style.  If not, see <http://www.gnu.org/licenses/>.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#ifndef ASTYLE_H
#define ASTYLE_H

#ifdef __VMS
	#define __USE_STD_IOSTREAM 1
	#include <assert>
#else
	#include <cassert>
#endif

#include <cctype>
#include <iostream>		// for cout
#include <string>
#include <vector>

#ifdef __GNUC__
	#include <string.h>		// need both string and string.h for GCC
#endif

#ifdef _MSC_VER
	#pragma warning(disable: 4996)  // secure version deprecation warnings
	#pragma warning(disable: 4267)  // 64 bit signed/unsigned loss of data
#endif

#ifdef __BORLANDC__
	#pragma warn -8004	            // variable is assigned a value that is never used
#endif

#ifdef __INTEL_COMPILER
	#pragma warning(disable:  383)  // value copied to temporary, reference to temporary used
	#pragma warning(disable:  981)  // operands are evaluated in unspecified order
#endif

#ifdef __clang__
	#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif

namespace astyle {

using namespace std;

enum FileType { C_TYPE = 0, JAVA_TYPE = 1, SHARP_TYPE = 2 };

/* The enums below are not recognized by 'vectors' in Microsoft Visual C++
   V5 when they are part of a namespace!!!  Use Visual C++ V6 or higher.
*/
enum FormatStyle
{
	STYLE_NONE,
	STYLE_ALLMAN,
	STYLE_JAVA,
	STYLE_KR,
	STYLE_STROUSTRUP,
	STYLE_WHITESMITH,
	STYLE_VTK,
	STYLE_BANNER,
	STYLE_GNU,
	STYLE_LINUX,
	STYLE_HORSTMANN,
	STYLE_1TBS,
	STYLE_GOOGLE,
	STYLE_PICO,
	STYLE_LISP
};

enum BracketMode
{
	NONE_MODE,
	ATTACH_MODE,
	BREAK_MODE,
	LINUX_MODE,
	STROUSTRUP_MODE,
	RUN_IN_MODE
};

enum BracketType
{
	NULL_TYPE = 0,
	NAMESPACE_TYPE = 1,			// also a DEFINITION_TYPE
	CLASS_TYPE = 2,				// also a DEFINITION_TYPE
	STRUCT_TYPE = 4,			// also a DEFINITION_TYPE
	INTERFACE_TYPE = 8,			// also a DEFINITION_TYPE
	DEFINITION_TYPE = 16,
	COMMAND_TYPE = 32,
	ARRAY_NIS_TYPE = 64,		// also an ARRAY_TYPE
	ENUM_TYPE = 128,			// also an ARRAY_TYPE
	INIT_TYPE = 256,			// also an ARRAY_TYPE
	ARRAY_TYPE = 512,
	EXTERN_TYPE = 1024,			// extern "C", not a command type extern
	SINGLE_LINE_TYPE = 2048
};

enum MinConditional
{
	MINCOND_ZERO,
	MINCOND_ONE,
	MINCOND_TWO,
	MINCOND_ONEHALF,
	MINCOND_END
};

enum ObjCColonPad
{
	COLON_PAD_NO_CHANGE,
	COLON_PAD_NONE,
	COLON_PAD_ALL,
	COLON_PAD_AFTER,
	COLON_PAD_BEFORE
};

enum PointerAlign
{
	PTR_ALIGN_NONE,
	PTR_ALIGN_TYPE,
	PTR_ALIGN_MIDDLE,
	PTR_ALIGN_NAME
};

enum ReferenceAlign
{
	REF_ALIGN_NONE = PTR_ALIGN_NONE,
	REF_ALIGN_TYPE = PTR_ALIGN_TYPE,
	REF_ALIGN_MIDDLE = PTR_ALIGN_MIDDLE,
	REF_ALIGN_NAME = PTR_ALIGN_NAME,
	REF_SAME_AS_PTR
};

enum FileEncoding
{
	ENCODING_8BIT,
	UTF_16BE,
	UTF_16LE,     // Windows default
	UTF_32BE,
	UTF_32LE
};

enum LineEndFormat
{
	LINEEND_DEFAULT,	// Use line break that matches most of the file
	LINEEND_WINDOWS,
	LINEEND_LINUX,
	LINEEND_MACOLD,
	LINEEND_CRLF = LINEEND_WINDOWS,
	LINEEND_LF   = LINEEND_LINUX,
	LINEEND_CR   = LINEEND_MACOLD
};

//-----------------------------------------------------------------------------
// Class ASSourceIterator
// A pure virtual class is used by ASFormatter and ASBeautifier instead of
// ASStreamIterator. This allows programs using AStyle as a plug-in to define
// their own ASStreamIterator. The ASStreamIterator class must inherit
// this class.
//-----------------------------------------------------------------------------

class ASSourceIterator
{
	public:
		ASSourceIterator() {}
		virtual ~ASSourceIterator() {}
		virtual int getStreamLength() const = 0;
		virtual bool hasMoreLines() const = 0;
		virtual string nextLine(bool emptyLineWasDeleted = false) = 0;
		virtual string peekNextLine() = 0;
		virtual void peekReset() = 0;
		virtual streamoff tellg() = 0;
};

//-----------------------------------------------------------------------------
// Class ASResource
//-----------------------------------------------------------------------------

class ASResource
{
	public:
		ASResource() {}
		virtual ~ASResource() {}
		void buildAssignmentOperators(vector<const string*>* assignmentOperators);
		void buildCastOperators(vector<const string*>* castOperators);
		void buildHeaders(vector<const string*>* headers, int fileType, bool beautifier = false);
		void buildIndentableMacros(vector<const pair<const string, const string>* >* indentableMacros);
		void buildIndentableHeaders(vector<const string*>* indentableHeaders);
		void buildNonAssignmentOperators(vector<const string*>* nonAssignmentOperators);
		void buildNonParenHeaders(vector<const string*>* nonParenHeaders, int fileType, bool beautifier = false);
		void buildOperators(vector<const string*>* operators, int fileType);
		void buildPreBlockStatements(vector<const string*>* preBlockStatements, int fileType);
		void buildPreCommandHeaders(vector<const string*>* preCommandHeaders, int fileType);
		void buildPreDefinitionHeaders(vector<const string*>* preDefinitionHeaders, int fileType);

	public:
		static const string AS_IF, AS_ELSE;
		static const string AS_DO, AS_WHILE;
		static const string AS_FOR;
		static const string AS_SWITCH, AS_CASE, AS_DEFAULT;
		static const string AS_TRY, AS_CATCH, AS_THROW, AS_THROWS, AS_FINALLY;
		static const string _AS_TRY, _AS_FINALLY, _AS_EXCEPT;
		static const string AS_PUBLIC, AS_PROTECTED, AS_PRIVATE;
		static const string AS_CLASS, AS_STRUCT, AS_UNION, AS_INTERFACE, AS_NAMESPACE;
		static const string AS_END;
		static const string AS_SELECTOR;
		static const string AS_EXTERN, AS_ENUM;
		static const string AS_STATIC, AS_CONST, AS_SEALED, AS_OVERRIDE, AS_VOLATILE, AS_NEW;
		static const string AS_NOEXCEPT, AS_INTERRUPT, AS_AUTORELEASEPOOL;
		static const string AS_WHERE, AS_LET, AS_SYNCHRONIZED;
		static const string AS_OPERATOR, AS_TEMPLATE;
		static const string AS_OPEN_BRACKET, AS_CLOSE_BRACKET;
		static const string AS_OPEN_LINE_COMMENT, AS_OPEN_COMMENT, AS_CLOSE_COMMENT;
		static const string AS_BAR_DEFINE, AS_BAR_INCLUDE, AS_BAR_IF, AS_BAR_EL, AS_BAR_ENDIF;
		static const string AS_RETURN;
		static const string AS_CIN, AS_COUT, AS_CERR;
		static const string AS_ASSIGN, AS_PLUS_ASSIGN, AS_MINUS_ASSIGN, AS_MULT_ASSIGN;
		static const string AS_DIV_ASSIGN, AS_MOD_ASSIGN, AS_XOR_ASSIGN, AS_OR_ASSIGN, AS_AND_ASSIGN;
		static const string AS_GR_GR_ASSIGN, AS_LS_LS_ASSIGN, AS_GR_GR_GR_ASSIGN, AS_LS_LS_LS_ASSIGN;
		static const string AS_GCC_MIN_ASSIGN, AS_GCC_MAX_ASSIGN;
		static const string AS_EQUAL, AS_PLUS_PLUS, AS_MINUS_MINUS, AS_NOT_EQUAL, AS_GR_EQUAL, AS_GR_GR_GR, AS_GR_GR;
		static const string AS_LS_EQUAL, AS_LS_LS_LS, AS_LS_LS;
		static const string AS_QUESTION_QUESTION, AS_LAMBDA;
		static const string AS_ARROW, AS_AND, AS_OR;
		static const string AS_SCOPE_RESOLUTION;
		static const string AS_PLUS, AS_MINUS, AS_MULT, AS_DIV, AS_MOD, AS_GR, AS_LS;
		static const string AS_NOT, AS_BIT_XOR, AS_BIT_OR, AS_BIT_AND, AS_BIT_NOT;
		static const string AS_QUESTION, AS_COLON, AS_SEMICOLON, AS_COMMA;
		static const string AS_ASM, AS__ASM__, AS_MS_ASM, AS_MS__ASM;
		static const string AS_QFOREACH, AS_QFOREVER, AS_FOREVER;
		static const string AS_FOREACH, AS_LOCK, AS_UNSAFE, AS_FIXED;
		static const string AS_GET, AS_SET, AS_ADD, AS_REMOVE;
		static const string AS_DELEGATE, AS_UNCHECKED;
		static const string AS_CONST_CAST, AS_DYNAMIC_CAST, AS_REINTERPRET_CAST, AS_STATIC_CAST;
		static const string AS_NS_DURING, AS_NS_HANDLER;
};  // Class ASResource

//-----------------------------------------------------------------------------
// Class ASBase
//-----------------------------------------------------------------------------

class ASBase
{
	private:
		// all variables should be set by the "init" function
		int baseFileType;      // a value from enum FileType

	protected:
		ASBase() : baseFileType(C_TYPE) { }
		virtual ~ASBase() {}

		// functions definitions are at the end of ASResource.cpp
		bool findKeyword(const string &line, int i, const string &keyword) const;
		string getCurrentWord(const string &line, size_t index) const;

	protected:
		void init(int fileTypeArg) { baseFileType = fileTypeArg; }
		bool isCStyle() const { return (baseFileType == C_TYPE); }
		bool isJavaStyle() const { return (baseFileType == JAVA_TYPE); }
		bool isSharpStyle() const { return (baseFileType == SHARP_TYPE); }

		// check if a specific character is a digit
		// NOTE: Visual C isdigit() gives assert error if char > 256
		bool isDigit(char ch) const {
			return (ch >= '0' && ch <= '9');
		}

		// check if a specific character can be used in a legal variable/method/class name
		bool isLegalNameChar(char ch) const {
			if (isWhiteSpace(ch)) return false;
			if ((unsigned) ch > 127) return false;
			return (isalnum((unsigned char)ch)
			        || ch == '.' || ch == '_'
			        || (isJavaStyle() && ch == '$')
			        || (isSharpStyle() && ch == '@'));  // may be used as a prefix
		}

		// check if a specific character can be part of a header
		bool isCharPotentialHeader(const string &line, size_t i) const {
			assert(!isWhiteSpace(line[i]));
			char prevCh = ' ';
			if (i > 0) prevCh = line[i - 1];
			if (!isLegalNameChar(prevCh) && isLegalNameChar(line[i]))
				return true;
			return false;
		}

		// check if a specific character can be part of an operator
		bool isCharPotentialOperator(char ch) const {
			assert(!isWhiteSpace(ch));
			if ((unsigned) ch > 127) return false;
			return (ispunct((unsigned char)ch)
			        && ch != '{' && ch != '}'
			        && ch != '(' && ch != ')'
			        && ch != '[' && ch != ']'
			        && ch != ';' && ch != ','
			        && ch != '#' && ch != '\\'
			        && ch != '\'' && ch != '\"');
		}

		// check if a specific character is a whitespace character
		bool isWhiteSpace(char ch) const { return (ch == ' ' || ch == '\t'); }

		// peek at the next unread character.
		char peekNextChar(const string &line, int i) const {
			char ch = ' ';
			size_t peekNum = line.find_first_not_of(" \t", i + 1);
			if (peekNum == string::npos)
				return ch;
			ch = line[peekNum];
			return ch;
		}
};  // Class ASBase

//-----------------------------------------------------------------------------
// Class ASBeautifier
//-----------------------------------------------------------------------------

class ASBeautifier : protected ASResource, protected ASBase
{
	public:
		ASBeautifier();
		virtual ~ASBeautifier();
		virtual void init(ASSourceIterator* iter);
		virtual string beautify(const string &line);
		void setCaseIndent(bool state);
		void setClassIndent(bool state);
		void setCStyle();
		void setDefaultTabLength();
		void setEmptyLineFill(bool state);
		void setForceTabXIndentation(int length);
		void setJavaStyle();
		void setLabelIndent(bool state);
		void setMaxInStatementIndentLength(int max);
		void setMinConditionalIndentOption(int min);
		void setMinConditionalIndentLength();
		void setModeManuallySet(bool state);
		void setModifierIndent(bool state);
		void setNamespaceIndent(bool state);
		void setAlignMethodColon(bool state);
		void setSharpStyle();
		void setSpaceIndentation(int length = 4);
		void setSwitchIndent(bool state);
		void setTabIndentation(int length = 4, bool forceTabs = false);
		void setPreprocDefineIndent(bool state);
		void setPreprocConditionalIndent(bool state);
		int  getBeautifierFileType() const;
		int  getFileType() const;
		int  getIndentLength(void) const;
		int  getTabLength(void) const;
		string getIndentString(void) const;
		string getNextWord(const string &line, size_t currPos) const;
		bool getBracketIndent(void) const;
		bool getBlockIndent(void) const;
		bool getCaseIndent(void) const;
		bool getClassIndent(void) const;
		bool getEmptyLineFill(void) const;
		bool getForceTabIndentation(void) const;
		bool getModeManuallySet(void) const;
		bool getModifierIndent(void) const;
		bool getNamespaceIndent(void) const;
		bool getPreprocDefineIndent(void) const;
		bool getSwitchIndent(void) const;

	protected:
		void deleteBeautifierVectors();
		const string* findHeader(const string &line, int i,
		                         const vector<const string*>* possibleHeaders) const;
		const string* findOperator(const string &line, int i,
		                           const vector<const string*>* possibleOperators) const;
		int  getNextProgramCharDistance(const string &line, int i) const;
		int  indexOf(vector<const string*> &container, const string* element) const;
		void setBlockIndent(bool state);
		void setBracketIndent(bool state);
		void setBracketIndentVtk(bool state);
		string extractPreprocessorStatement(const string &line) const;
		string trim(const string &str) const;
		string rtrim(const string &str) const;

		// variables set by ASFormatter - must be updated in activeBeautifierStack
		int  inLineNumber;
		int  horstmannIndentInStatement;
		int  nonInStatementBracket;
		bool lineCommentNoBeautify;
		bool isElseHeaderIndent;
		bool isCaseHeaderCommentIndent;
		bool isNonInStatementArray;
		bool isSharpAccessor;
		bool isSharpDelegate;
		bool isInExternC;
		bool isInBeautifySQL;
		bool isInIndentableStruct;
		bool isInIndentablePreproc;

	private:  // functions
		ASBeautifier(const ASBeautifier &copy);
		ASBeautifier &operator=(ASBeautifier &);       // not to be implemented

		void adjustParsedLineIndentation(size_t iPrelim, bool isInExtraHeaderIndent);
		void computePreliminaryIndentation();
		void parseCurrentLine(const string &line);
		void popLastInStatementIndent();
		void processPreprocessor(const string &preproc, const string &line);
		void registerInStatementIndent(const string &line, int i, int spaceIndentCount,
		                               int tabIncrementIn, int minIndent, bool updateParenStack);
		void registerInStatementIndentColon(const string &line, int i, int tabIncrementIn);
		void initVectors();
		void initTempStacksContainer(vector<vector<const string*>*>* &container,
		                             vector<vector<const string*>*>* value);
		void clearObjCMethodDefinitionAlignment();
		void deleteBeautifierContainer(vector<ASBeautifier*>* &container);
		void deleteTempStacksContainer(vector<vector<const string*>*>* &container);
		int  adjustIndentCountForBreakElseIfComments() const;
		int  computeObjCColonAlignment(string &line, int colonAlignPosition) const;
		int  convertTabToSpaces(int i, int tabIncrementIn) const;
		int  getInStatementIndentAssign(const string &line, size_t currPos) const;
		int  getInStatementIndentComma(const string &line, size_t currPos) const;
		bool isIndentedPreprocessor(const string &line, size_t currPos) const;
		bool isLineEndComment(const string &line, int startPos) const;
		bool isPreprocessorConditionalCplusplus(const string &line) const;
		bool isInPreprocessorUnterminatedComment(const string &line);
		bool statementEndsWithComma(const string &line, int index) const;
		string &getIndentedLineReturn(string &newLine, const string &originalLine) const;
		string preLineWS(int lineIndentCount, int lineSpaceIndentCount) const;
		template<typename T> void deleteContainer(T &container);
		template<typename T> void initContainer(T &container, T value);
		vector<vector<const string*>*>* copyTempStacks(const ASBeautifier &other) const;
		pair<int, int> computePreprocessorIndent();

	private:  // variables
		int beautifierFileType;
		vector<const string*>* headers;
		vector<const string*>* nonParenHeaders;
		vector<const string*>* preBlockStatements;
		vector<const string*>* preCommandHeaders;
		vector<const string*>* assignmentOperators;
		vector<const string*>* nonAssignmentOperators;
		vector<const string*>* indentableHeaders;

		vector<ASBeautifier*>* waitingBeautifierStack;
		vector<ASBeautifier*>* activeBeautifierStack;
		vector<int>* waitingBeautifierStackLengthStack;
		vector<int>* activeBeautifierStackLengthStack;
		vector<const string*>* headerStack;
		vector<vector<const string*>* >* tempStacks;
		vector<int>* blockParenDepthStack;
		vector<bool>* blockStatementStack;
		vector<bool>* parenStatementStack;
		vector<bool>* bracketBlockStateStack;
		vector<int>* inStatementIndentStack;
		vector<int>* inStatementIndentStackSizeStack;
		vector<int>* parenIndentStack;
		vector<pair<int, int> >* preprocIndentStack;

		ASSourceIterator* sourceIterator;
		const string* currentHeader;
		const string* previousLastLineHeader;
		const string* probationHeader;
		const string* lastLineHeader;
		string indentString;
		string verbatimDelimiter;
		bool isInQuote;
		bool isInVerbatimQuote;
		bool haveLineContinuationChar;
		bool isInAsm;
		bool isInAsmOneLine;
		bool isInAsmBlock;
		bool isInComment;
		bool isInPreprocessorComment;
		bool isInHorstmannComment;
		bool isInCase;
		bool isInQuestion;
		bool isInStatement;
		bool isInHeader;
		bool isInTemplate;
		bool isInDefine;
		bool isInDefineDefinition;
		bool classIndent;
		bool isIndentModeOff;
		bool isInClassHeader;			// is in a class before the opening bracket
		bool isInClassHeaderTab;		// is in an indentable class header line
		bool isInClassInitializer;		// is in a class after the ':' initializer
		bool isInClass;					// is in a class after the opening bracket
		bool isInObjCMethodDefinition;
		bool isImmediatelyPostObjCMethodDefinition;
		bool isInIndentablePreprocBlock;
		bool isInObjCInterface;
		bool isInEnum;
		bool isInEnumTypeID;
		bool isInLet;
		bool modifierIndent;
		bool switchIndent;
		bool caseIndent;
		bool namespaceIndent;
		bool bracketIndent;
		bool bracketIndentVtk;
		bool blockIndent;
		bool labelIndent;
		bool shouldIndentPreprocDefine;
		bool isInConditional;
		bool isModeManuallySet;
		bool shouldForceTabIndentation;
		bool emptyLineFill;
		bool backslashEndsPrevLine;
		bool lineOpensWithLineComment;
		bool lineOpensWithComment;
		bool lineStartsInComment;
		bool blockCommentNoIndent;
		bool blockCommentNoBeautify;
		bool previousLineProbationTab;
		bool lineBeginsWithOpenBracket;
		bool lineBeginsWithCloseBracket;
		bool lineBeginsWithComma;
		bool lineIsCommentOnly;
		bool lineIsLineCommentOnly;
		bool shouldIndentBrackettedLine;
		bool isInSwitch;
		bool foundPreCommandHeader;
		bool foundPreCommandMacro;
		bool shouldAlignMethodColon;
		bool shouldIndentPreprocConditional;
		int  indentCount;
		int  spaceIndentCount;
		int  spaceIndentObjCMethodDefinition;
		int  colonIndentObjCMethodDefinition;
		int  lineOpeningBlocksNum;
		int  lineClosingBlocksNum;
		int  fileType;
		int  minConditionalOption;
		int  minConditionalIndent;
		int  parenDepth;
		int  indentLength;
		int  tabLength;
		int  blockTabCount;
		int  maxInStatementIndent;
		int  classInitializerIndents;
		int  templateDepth;
		int  squareBracketCount;
		int  prevFinalLineSpaceIndentCount;
		int  prevFinalLineIndentCount;
		int  defineIndentCount;
		int  preprocBlockIndent;
		char quoteChar;
		char prevNonSpaceCh;
		char currentNonSpaceCh;
		char currentNonLegalCh;
		char prevNonLegalCh;
};  // Class ASBeautifier

//-----------------------------------------------------------------------------
// Class ASEnhancer
//-----------------------------------------------------------------------------

class ASEnhancer : protected ASBase
{
	public:  // functions
		ASEnhancer();
		virtual ~ASEnhancer();
		void init(int, int, int, bool, bool, bool, bool, bool, bool, bool,
		          vector<const pair<const string, const string>* >*);
		void enhance(string &line, bool isInNamespace, bool isInPreprocessor, bool isInSQL);

	private:  // functions
		void    convertForceTabIndentToSpaces(string  &line) const;
		void    convertSpaceIndentToForceTab(string &line) const;
		size_t  findCaseColon(string  &line, size_t caseIndex) const;
		int     indentLine(string  &line, int indent) const;
		bool    isBeginDeclareSectionSQL(string  &line, size_t index) const;
		bool    isEndDeclareSectionSQL(string  &line, size_t index) const;
		bool    isOneLineBlockReached(string &line, int startChar) const;
		void    parseCurrentLine(string &line, bool isInPreprocessor, bool isInSQL);
		size_t  processSwitchBlock(string  &line, size_t index);
		int     unindentLine(string  &line, int unindent) const;

	private:
		// options from command line or options file
		int  indentLength;
		int  tabLength;
		bool useTabs;
		bool forceTab;
		bool namespaceIndent;
		bool caseIndent;
		bool preprocBlockIndent;
		bool preprocDefineIndent;
		bool emptyLineFill;

		// parsing variables
		int  lineNumber;
		bool isInQuote;
		bool isInComment;
		char quoteChar;

		// unindent variables
		int  bracketCount;
		int  switchDepth;
		int  eventPreprocDepth;
		bool lookingForCaseBracket;
		bool unindentNextLine;
		bool shouldUnindentLine;
		bool shouldUnindentComment;

		// struct used by ParseFormattedLine function
		// contains variables used to unindent the case blocks
		struct switchVariables
		{
			int  switchBracketCount;
			int  unindentDepth;
			bool unindentCase;
		};

		switchVariables sw;                      // switch variables struct
		vector<switchVariables> switchStack;     // stack vector of switch variables

		// event table variables
		bool nextLineIsEventIndent;             // begin event table indent is reached
		bool isInEventTable;                    // need to indent an event table
		vector<const pair<const string, const string>* >* indentableMacros;

		// SQL variables
		bool nextLineIsDeclareIndent;           // begin declare section indent is reached
		bool isInDeclareSection;                // need to indent a declare section

};  // Class ASEnhancer

//-----------------------------------------------------------------------------
// Class ASFormatter
//-----------------------------------------------------------------------------

class ASFormatter : public ASBeautifier
{
	public:	// functions
		ASFormatter();
		virtual ~ASFormatter();
		virtual void init(ASSourceIterator* iter);
		virtual bool hasMoreLines() const;
		virtual string nextLine();
		LineEndFormat getLineEndFormat() const;
		bool getIsLineReady() const;
		void setFormattingStyle(FormatStyle style);
		void setAddBracketsMode(bool state);
		void setAddOneLineBracketsMode(bool state);
		void setRemoveBracketsMode(bool state);
		void setAttachClass(bool state);
		void setAttachExternC(bool state);
		void setAttachNamespace(bool state);
		void setAttachInline(bool state);
		void setBracketFormatMode(BracketMode mode);
		void setBreakAfterMode(bool state);
		void setBreakClosingHeaderBracketsMode(bool state);
		void setBreakBlocksMode(bool state);
		void setBreakClosingHeaderBlocksMode(bool state);
		void setBreakElseIfsMode(bool state);
		void setBreakOneLineBlocksMode(bool state);
		void setMethodPrefixPaddingMode(bool state);
		void setMethodPrefixUnPaddingMode(bool state);
		void setCloseTemplatesMode(bool state);
		void setDeleteEmptyLinesMode(bool state);
		void setIndentCol1CommentsMode(bool state);
		void setLineEndFormat(LineEndFormat fmt);
		void setMaxCodeLength(int max);
		void setObjCColonPaddingMode(ObjCColonPad mode);
		void setOperatorPaddingMode(bool mode);
		void setParensOutsidePaddingMode(bool mode);
		void setParensFirstPaddingMode(bool mode);
		void setParensInsidePaddingMode(bool mode);
		void setParensHeaderPaddingMode(bool mode);
		void setParensUnPaddingMode(bool state);
		void setPointerAlignment(PointerAlign alignment);
		void setPreprocBlockIndent(bool state);
		void setReferenceAlignment(ReferenceAlign alignment);
		void setSingleStatementsMode(bool state);
		void setStripCommentPrefix(bool state);
		void setTabSpaceConversionMode(bool state);
		size_t getChecksumIn() const;
		size_t getChecksumOut() const;
		int  getChecksumDiff() const;
		int  getFormatterFileType() const;

	private:  // functions
		ASFormatter(const ASFormatter &copy);       // copy constructor not to be implemented
		ASFormatter &operator=(ASFormatter &);      // assignment operator not to be implemented
		template<typename T> void deleteContainer(T &container);
		template<typename T> void initContainer(T &container, T value);
		char peekNextChar() const;
		BracketType getBracketType();
		bool adjustChecksumIn(int adjustment);
		bool computeChecksumIn(const string &currentLine_);
		bool computeChecksumOut(const string &beautifiedLine);
		bool addBracketsToStatement();
		bool removeBracketsFromStatement();
		bool commentAndHeaderFollows();
		bool getNextChar();
		bool getNextLine(bool emptyLineWasDeleted = false);
		bool isArrayOperator() const;
		bool isBeforeComment() const;
		bool isBeforeAnyComment() const;
		bool isBeforeAnyLineEndComment(int startPos) const;
		bool isBeforeMultipleLineEndComments(int startPos) const;
		bool isBracketType(BracketType a, BracketType b) const;
		bool isClassInitializer() const;
		bool isClosingHeader(const string* header) const;
		bool isCurrentBracketBroken() const;
		bool isDereferenceOrAddressOf() const;
		bool isExecSQL(string &line, size_t index) const;
		bool isEmptyLine(const string &line) const;
		bool isExternC() const;
		bool isNextWordSharpNonParenHeader(int startChar) const;
		bool isNonInStatementArrayBracket() const;
		bool isOkToSplitFormattedLine();
		bool isPointerOrReference() const;
		bool isPointerOrReferenceCentered() const;
		bool isPointerOrReferenceVariable(string &word) const;
		bool isSharpStyleWithParen(const string* header) const;
		bool isStructAccessModified(string &firstLine, size_t index) const;
		bool isIndentablePreprocessorBlock(string &firstLine, size_t index);
		bool isUnaryOperator() const;
		bool isUniformInitializerBracket() const;
		bool isImmediatelyPostCast() const;
		bool isInExponent() const;
		bool isInSwitchStatement() const;
		bool isNextCharOpeningBracket(int startChar) const;
		bool isOkToBreakBlock(BracketType bracketType) const;
		bool isOperatorPaddingDisabled() const;
		bool pointerSymbolFollows() const;
		int  getCurrentLineCommentAdjustment();
		int  getNextLineCommentAdjustment();
		int  isOneLineBlockReached(string &line, int startChar) const;
		void adjustComments();
		void appendChar(char ch, bool canBreakLine);
		void appendCharInsideComments();
		void appendOperator(const string &sequence, bool canBreakLine = true);
		void appendSequence(const string &sequence, bool canBreakLine = true);
		void appendSpacePad();
		void appendSpaceAfter();
		void breakLine(bool isSplitLine = false);
		void buildLanguageVectors();
		void updateFormattedLineSplitPoints(char appendedChar);
		void updateFormattedLineSplitPointsOperator(const string &sequence);
		void checkIfTemplateOpener();
		void clearFormattedLineSplitPoints();
		void convertTabToSpaces();
		void deleteContainer(vector<BracketType>* &container);
		void formatArrayRunIn();
		void formatRunIn();
		void formatArrayBrackets(BracketType bracketType, bool isOpeningArrayBracket);
		void formatClosingBracket(BracketType bracketType);
		void formatCommentBody();
		void formatCommentOpener();
		void formatCommentCloser();
		void formatLineCommentBody();
		void formatLineCommentOpener();
		void formatOpeningBracket(BracketType bracketType);
		void formatQuoteBody();
		void formatQuoteOpener();
		void formatPointerOrReference();
		void formatPointerOrReferenceCast();
		void formatPointerOrReferenceToMiddle();
		void formatPointerOrReferenceToName();
		void formatPointerOrReferenceToType();
		void fixOptionVariableConflicts();
		void goForward(int i);
		void isLineBreakBeforeClosingHeader();
		void initContainer(vector<BracketType>* &container, vector<BracketType>* value);
		void initNewLine();
		void padObjCMethodColon();
		void padOperators(const string* newOperator);
		void padParens();
		void processPreprocessor();
		void resetEndOfStatement();
		void setAttachClosingBracketMode(bool state);
		void setBreakBlocksVariables();
		void stripCommentPrefix();
		void testForTimeToSplitFormattedLine();
		void trimContinuationLine();
		void updateFormattedLineSplitPointsPointerOrReference(size_t index);
		size_t findFormattedLineSplitPoint() const;
		size_t findNextChar(string &line, char searchChar, int searchStart = 0);
		const string* checkForHeaderFollowingComment(const string &firstLine) const;
		const string* getFollowingOperator() const;
		string getPreviousWord(const string &line, int currPos) const;
		string peekNextText(const string &firstLine, bool endOnEmptyLine = false, bool shouldReset = false) const;

	private:  // variables
		int formatterFileType;
		vector<const string*>* headers;
		vector<const string*>* nonParenHeaders;
		vector<const string*>* preDefinitionHeaders;
		vector<const string*>* preCommandHeaders;
		vector<const string*>* operators;
		vector<const string*>* assignmentOperators;
		vector<const string*>* castOperators;
		vector<const pair<const string, const string>* >* indentableMacros;	// for ASEnhancer

		ASSourceIterator* sourceIterator;
		ASEnhancer* enhancer;

		vector<const string*>* preBracketHeaderStack;
		vector<BracketType>* bracketTypeStack;
		vector<int>* parenStack;
		vector<bool>* structStack;
		vector<bool>* questionMarkStack;

		string currentLine;
		string formattedLine;
		string readyFormattedLine;
		string verbatimDelimiter;
		const string* currentHeader;
		const string* previousOperator;    // used ONLY by pad-oper
		char currentChar;
		char previousChar;
		char previousNonWSChar;
		char previousCommandChar;
		char quoteChar;
		streamoff preprocBlockEnd;
		int  charNum;
		int  horstmannIndentChars;
		int  nextLineSpacePadNum;
		int  preprocBracketTypeStackSize;
		int  spacePadNum;
		int  tabIncrementIn;
		int  templateDepth;
		int  squareBracketCount;
		size_t checksumIn;
		size_t checksumOut;
		size_t currentLineFirstBracketNum;	// first bracket location on currentLine
		size_t formattedLineCommentNum;     // comment location on formattedLine
		size_t leadingSpaces;
		size_t maxCodeLength;

		// possible split points
		size_t maxSemi;			// probably a 'for' statement
		size_t maxAndOr;		// probably an 'if' statement
		size_t maxComma;
		size_t maxParen;
		size_t maxWhiteSpace;
		size_t maxSemiPending;
		size_t maxAndOrPending;
		size_t maxCommaPending;
		size_t maxParenPending;
		size_t maxWhiteSpacePending;

		size_t previousReadyFormattedLineLength;
		FormatStyle formattingStyle;
		BracketMode bracketFormatMode;
		BracketType previousBracketType;
		PointerAlign pointerAlignment;
		ReferenceAlign referenceAlignment;
		ObjCColonPad objCColonPadMode;
		LineEndFormat lineEnd;
		bool isVirgin;
		bool shouldPadOperators;
		bool shouldPadParensOutside;
		bool shouldPadFirstParen;
		bool shouldPadParensInside;
		bool shouldPadHeader;
		bool shouldStripCommentPrefix;
		bool shouldUnPadParens;
		bool shouldConvertTabs;
		bool shouldIndentCol1Comments;
		bool shouldIndentPreprocBlock;
		bool shouldCloseTemplates;
		bool shouldAttachExternC;
		bool shouldAttachNamespace;
		bool shouldAttachClass;
		bool shouldAttachInline;
		bool isInLineComment;
		bool isInComment;
		bool isInCommentStartLine;
		bool noTrimCommentContinuation;
		bool isInPreprocessor;
		bool isInPreprocessorBeautify;
		bool isInTemplate;
		bool doesLineStartComment;
		bool lineEndsInCommentOnly;
		bool lineIsCommentOnly;
		bool lineIsLineCommentOnly;
		bool lineIsEmpty;
		bool isImmediatelyPostCommentOnly;
		bool isImmediatelyPostEmptyLine;
		bool isInClassInitializer;
		bool isInQuote;
		bool isInVerbatimQuote;
		bool haveLineContinuationChar;
		bool isInQuoteContinuation;
		bool isHeaderInMultiStatementLine;
		bool isSpecialChar;
		bool isNonParenHeader;
		bool foundQuestionMark;
		bool foundPreDefinitionHeader;
		bool foundNamespaceHeader;
		bool foundClassHeader;
		bool foundStructHeader;
		bool foundInterfaceHeader;
		bool foundPreCommandHeader;
		bool foundPreCommandMacro;
		bool foundCastOperator;
		bool isInLineBreak;
		bool endOfAsmReached;
		bool endOfCodeReached;
		bool lineCommentNoIndent;
		bool isFormattingModeOff;
		bool isInEnum;
		bool isInExecSQL;
		bool isInAsm;
		bool isInAsmOneLine;
		bool isInAsmBlock;
		bool isLineReady;
		bool elseHeaderFollowsComments;
		bool caseHeaderFollowsComments;
		bool isPreviousBracketBlockRelated;
		bool isInPotentialCalculation;
		bool isCharImmediatelyPostComment;
		bool isPreviousCharPostComment;
		bool isCharImmediatelyPostLineComment;
		bool isCharImmediatelyPostOpenBlock;
		bool isCharImmediatelyPostCloseBlock;
		bool isCharImmediatelyPostTemplate;
		bool isCharImmediatelyPostReturn;
		bool isCharImmediatelyPostThrow;
		bool isCharImmediatelyPostOperator;
		bool isCharImmediatelyPostPointerOrReference;
		bool isInObjCMethodDefinition;
		bool isInObjCInterface;
		bool isInObjCSelector;
		bool breakCurrentOneLineBlock;
		bool shouldRemoveNextClosingBracket;
		bool isInHorstmannRunIn;
		bool currentLineBeginsWithBracket;
		bool attachClosingBracketMode;
		bool shouldBreakOneLineBlocks;
		bool shouldReparseCurrentChar;
		bool shouldBreakOneLineStatements;
		bool shouldBreakClosingHeaderBrackets;
		bool shouldBreakElseIfs;
		bool shouldBreakLineAfterLogical;
		bool shouldAddBrackets;
		bool shouldAddOneLineBrackets;
		bool shouldRemoveBrackets;
		bool shouldPadMethodColon;
		bool shouldPadMethodPrefix;
		bool shouldUnPadMethodPrefix;
		bool shouldDeleteEmptyLines;
		bool needHeaderOpeningBracket;
		bool shouldBreakLineAtNextChar;
		bool shouldKeepLineUnbroken;
		bool passedSemicolon;
		bool passedColon;
		bool isImmediatelyPostNonInStmt;
		bool isCharImmediatelyPostNonInStmt;
		bool isImmediatelyPostComment;
		bool isImmediatelyPostLineComment;
		bool isImmediatelyPostEmptyBlock;
		bool isImmediatelyPostPreprocessor;
		bool isImmediatelyPostReturn;
		bool isImmediatelyPostThrow;
		bool isImmediatelyPostOperator;
		bool isImmediatelyPostTemplate;
		bool isImmediatelyPostPointerOrReference;
		bool shouldBreakBlocks;
		bool shouldBreakClosingHeaderBlocks;
		bool isPrependPostBlockEmptyLineRequested;
		bool isAppendPostBlockEmptyLineRequested;
		bool isIndentableProprocessor;
		bool isIndentableProprocessorBlock;
		bool prependEmptyLine;
		bool appendOpeningBracket;
		bool foundClosingHeader;
		bool isInHeader;
		bool isImmediatelyPostHeader;
		bool isInCase;
		bool isFirstPreprocConditional;
		bool processedFirstConditional;
		bool isJavaStaticConstructor;

	private:  // inline functions
		// append the CURRENT character (curentChar) to the current formatted line.
		void appendCurrentChar(bool canBreakLine = true) {
			appendChar(currentChar, canBreakLine);
		}

		// check if a specific sequence exists in the current placement of the current line
		bool isSequenceReached(const char* sequence) const {
			return currentLine.compare(charNum, strlen(sequence), sequence) == 0;
		}

		// call ASBase::findHeader for the current character
		const string* findHeader(const vector<const string*>* headers_) {
			return ASBeautifier::findHeader(currentLine, charNum, headers_);
		}

		// call ASBase::findOperator for the current character
		const string* findOperator(const vector<const string*>* headers_) {
			return ASBeautifier::findOperator(currentLine, charNum, headers_);
		}
};  // Class ASFormatter


//-----------------------------------------------------------------------------
// astyle namespace global declarations
//-----------------------------------------------------------------------------
// sort comparison functions for ASResource
bool sortOnLength(const string* a, const string* b);
bool sortOnName(const string* a, const string* b);

}   // end of astyle namespace

// end of astyle namespace  --------------------------------------------------

#endif // closes ASTYLE_H
