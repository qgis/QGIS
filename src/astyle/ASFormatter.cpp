/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   ASFormatter.cpp
 *
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

#include "astyle.h"

#include <algorithm>
#include <fstream>


namespace astyle {
/**
 * Constructor of ASFormatter
 */
ASFormatter::ASFormatter()
{
	sourceIterator = NULL;
	enhancer = new ASEnhancer;
	preBracketHeaderStack = NULL;
	bracketTypeStack = NULL;
	parenStack = NULL;
	structStack = NULL;
	questionMarkStack = NULL;
	lineCommentNoIndent = false;
	formattingStyle = STYLE_NONE;
	bracketFormatMode = NONE_MODE;
	pointerAlignment = PTR_ALIGN_NONE;
	referenceAlignment = REF_SAME_AS_PTR;
	objCColonPadMode = COLON_PAD_NO_CHANGE;
	lineEnd = LINEEND_DEFAULT;
	maxCodeLength = string::npos;
	shouldPadOperators = false;
	shouldPadParensOutside = false;
	shouldPadFirstParen = false;
	shouldPadParensInside = false;
	shouldPadHeader = false;
	shouldStripCommentPrefix = false;
	shouldUnPadParens = false;
	attachClosingBracketMode = false;
	shouldBreakOneLineBlocks = true;
	shouldBreakOneLineStatements = true;
	shouldConvertTabs = false;
	shouldIndentCol1Comments = false;
	shouldIndentPreprocBlock = false;
	shouldCloseTemplates = false;
	shouldAttachExternC = false;
	shouldAttachNamespace = false;
	shouldAttachClass = false;
	shouldAttachInline = false;
	shouldBreakBlocks = false;
	shouldBreakClosingHeaderBlocks = false;
	shouldBreakClosingHeaderBrackets = false;
	shouldDeleteEmptyLines = false;
	shouldBreakElseIfs = false;
	shouldBreakLineAfterLogical = false;
	shouldAddBrackets = false;
	shouldAddOneLineBrackets = false;
	shouldRemoveBrackets = false;
	shouldPadMethodColon = false;
	shouldPadMethodPrefix = false;
	shouldUnPadMethodPrefix = false;

	// initialize ASFormatter member vectors
	formatterFileType = 9;		// reset to an invalid type
	headers = new vector<const string*>;
	nonParenHeaders = new vector<const string*>;
	preDefinitionHeaders = new vector<const string*>;
	preCommandHeaders = new vector<const string*>;
	operators = new vector<const string*>;
	assignmentOperators = new vector<const string*>;
	castOperators = new vector<const string*>;

	// initialize ASEnhancer member vectors
	indentableMacros = new vector<const pair<const string, const string>* >;
}

/**
 * Destructor of ASFormatter
 */
ASFormatter::~ASFormatter()
{
	// delete ASFormatter stack vectors
	deleteContainer(preBracketHeaderStack);
	deleteContainer(bracketTypeStack);
	deleteContainer(parenStack);
	deleteContainer(structStack);
	deleteContainer(questionMarkStack);

	// delete ASFormatter member vectors
	formatterFileType = 9;		// reset to an invalid type
	delete headers;
	delete nonParenHeaders;
	delete preDefinitionHeaders;
	delete preCommandHeaders;
	delete operators;
	delete assignmentOperators;
	delete castOperators;

	// delete ASEnhancer member vectors
	delete indentableMacros;

	// delete ASBeautifier member vectors
	// must be done when the ASFormatter object is deleted (not ASBeautifier)
	ASBeautifier::deleteBeautifierVectors();

	delete enhancer;
}

/**
 * initialize the ASFormatter.
 *
 * init() should be called every time a ASFormatter object is to start
 * formatting a NEW source file.
 * init() receives a pointer to a ASSourceIterator object that will be
 * used to iterate through the source code.
 *
 * @param si        a pointer to the ASSourceIterator or ASStreamIterator object.
 */
void ASFormatter::init(ASSourceIterator* si)
{
	buildLanguageVectors();
	fixOptionVariableConflicts();
	ASBeautifier::init(si);
	sourceIterator = si;

	enhancer->init(getFileType(),
	               getIndentLength(),
	               getTabLength(),
	               getIndentString() == "\t" ? true : false,
	               getForceTabIndentation(),
	               getNamespaceIndent(),
	               getCaseIndent(),
	               shouldIndentPreprocBlock,
	               getPreprocDefineIndent(),
	               getEmptyLineFill(),
	               indentableMacros);

	initContainer(preBracketHeaderStack, new vector<const string*>);
	initContainer(parenStack, new vector<int>);
	initContainer(structStack, new vector<bool>);
	initContainer(questionMarkStack, new vector<bool>);
	parenStack->push_back(0);               // parenStack must contain this default entry
	initContainer(bracketTypeStack, new vector<BracketType>);
	bracketTypeStack->push_back(NULL_TYPE); // bracketTypeStack must contain this default entry
	clearFormattedLineSplitPoints();

	currentHeader = NULL;
	currentLine = "";
	readyFormattedLine = "";
	formattedLine = "";
	verbatimDelimiter = "";
	currentChar = ' ';
	previousChar = ' ';
	previousCommandChar = ' ';
	previousNonWSChar = ' ';
	quoteChar = '"';
	preprocBlockEnd = 0;
	charNum = 0;
	checksumIn = 0;
	checksumOut = 0;
	currentLineFirstBracketNum = string::npos;
	formattedLineCommentNum = 0;
	leadingSpaces = 0;
	previousReadyFormattedLineLength = string::npos;
	preprocBracketTypeStackSize = 0;
	spacePadNum = 0;
	nextLineSpacePadNum = 0;
	templateDepth = 0;
	squareBracketCount = 0;
	horstmannIndentChars = 0;
	tabIncrementIn = 0;
	previousBracketType = NULL_TYPE;
	previousOperator = NULL;

	isVirgin = true;
	isInLineComment = false;
	isInComment = false;
	isInCommentStartLine = false;
	noTrimCommentContinuation = false;
	isInPreprocessor = false;
	isInPreprocessorBeautify = false;
	doesLineStartComment = false;
	lineEndsInCommentOnly = false;
	lineIsCommentOnly = false;
	lineIsLineCommentOnly = false;
	lineIsEmpty = false;
	isImmediatelyPostCommentOnly = false;
	isImmediatelyPostEmptyLine = false;
	isInClassInitializer = false;
	isInQuote = false;
	isInVerbatimQuote = false;
	haveLineContinuationChar = false;
	isInQuoteContinuation = false;
	isHeaderInMultiStatementLine = false;
	isSpecialChar = false;
	isNonParenHeader = false;
	foundNamespaceHeader = false;
	foundClassHeader = false;
	foundStructHeader = false;
	foundInterfaceHeader = false;
	foundPreDefinitionHeader = false;
	foundPreCommandHeader = false;
	foundPreCommandMacro = false;
	foundCastOperator = false;
	foundQuestionMark = false;
	isInLineBreak = false;
	endOfAsmReached = false;
	endOfCodeReached = false;
	isFormattingModeOff = false;
	isInEnum = false;
	isInExecSQL = false;
	isInAsm = false;
	isInAsmOneLine = false;
	isInAsmBlock = false;
	isLineReady = false;
	elseHeaderFollowsComments = false;
	caseHeaderFollowsComments = false;
	isPreviousBracketBlockRelated = false;
	isInPotentialCalculation = false;
	shouldReparseCurrentChar = false;
	needHeaderOpeningBracket = false;
	shouldBreakLineAtNextChar = false;
	shouldKeepLineUnbroken = false;
	passedSemicolon = false;
	passedColon = false;
	isImmediatelyPostNonInStmt = false;
	isCharImmediatelyPostNonInStmt = false;
	isInTemplate = false;
	isImmediatelyPostComment = false;
	isImmediatelyPostLineComment = false;
	isImmediatelyPostEmptyBlock = false;
	isImmediatelyPostPreprocessor = false;
	isImmediatelyPostReturn = false;
	isImmediatelyPostThrow = false;
	isImmediatelyPostOperator = false;
	isImmediatelyPostTemplate = false;
	isImmediatelyPostPointerOrReference = false;
	isCharImmediatelyPostReturn = false;
	isCharImmediatelyPostThrow = false;
	isCharImmediatelyPostOperator = false;
	isCharImmediatelyPostComment = false;
	isPreviousCharPostComment = false;
	isCharImmediatelyPostLineComment = false;
	isCharImmediatelyPostOpenBlock = false;
	isCharImmediatelyPostCloseBlock = false;
	isCharImmediatelyPostTemplate = false;
	isCharImmediatelyPostPointerOrReference = false;
	isInObjCMethodDefinition = false;
	isInObjCInterface = false;
	isInObjCSelector = false;
	breakCurrentOneLineBlock = false;
	shouldRemoveNextClosingBracket = false;
	isInHorstmannRunIn = false;
	currentLineBeginsWithBracket = false;
	isPrependPostBlockEmptyLineRequested = false;
	isAppendPostBlockEmptyLineRequested = false;
	isIndentableProprocessor = false;
	isIndentableProprocessorBlock = false;
	prependEmptyLine = false;
	appendOpeningBracket = false;
	foundClosingHeader = false;
	isImmediatelyPostHeader = false;
	isInHeader = false;
	isInCase = false;
	isFirstPreprocConditional = false;
	processedFirstConditional = false;
	isJavaStaticConstructor = false;
}

/**
 * build vectors for each programing language
 * depending on the file extension.
 */
void ASFormatter::buildLanguageVectors()
{
	if (getFileType() == formatterFileType)  // don't build unless necessary
		return;

	formatterFileType = getFileType();

	headers->clear();
	nonParenHeaders->clear();
	preDefinitionHeaders->clear();
	preCommandHeaders->clear();
	operators->clear();
	assignmentOperators->clear();
	castOperators->clear();
	indentableMacros->clear();	// ASEnhancer

	ASResource::buildHeaders(headers, getFileType());
	ASResource::buildNonParenHeaders(nonParenHeaders, getFileType());
	ASResource::buildPreDefinitionHeaders(preDefinitionHeaders, getFileType());
	ASResource::buildPreCommandHeaders(preCommandHeaders, getFileType());
	ASResource::buildOperators(operators, getFileType());
	ASResource::buildAssignmentOperators(assignmentOperators);
	ASResource::buildCastOperators(castOperators);
	ASResource::buildIndentableMacros(indentableMacros);	//ASEnhancer
}

/**
 * set the variables for each predefined style.
 * this will override any previous settings.
 */
void ASFormatter::fixOptionVariableConflicts()
{
	if (formattingStyle == STYLE_ALLMAN)
	{
		setBracketFormatMode(BREAK_MODE);
	}
	else if (formattingStyle == STYLE_JAVA)
	{
		setBracketFormatMode(ATTACH_MODE);
	}
	else if (formattingStyle == STYLE_KR)
	{
		setBracketFormatMode(LINUX_MODE);
	}
	else if (formattingStyle == STYLE_STROUSTRUP)
	{
		setBracketFormatMode(STROUSTRUP_MODE);
	}
	else if (formattingStyle == STYLE_WHITESMITH)
	{
		setBracketFormatMode(BREAK_MODE);
		setBracketIndent(true);
		setClassIndent(true);			// avoid hanging indent with access modifiers
		setSwitchIndent(true);			// avoid hanging indent with case statements
	}
	else if (formattingStyle == STYLE_VTK)
	{
		// the unindented class bracket does NOT cause a hanging indent like Whitesmith
		setBracketFormatMode(BREAK_MODE);
		setBracketIndentVtk(true);		// sets both bracketIndent and bracketIndentVtk
		setSwitchIndent(true);			// avoid hanging indent with case statements
	}
	else if (formattingStyle == STYLE_BANNER)
	{
		// attached brackets can have hanging indents with the closing bracket
		setBracketFormatMode(ATTACH_MODE);
		setBracketIndent(true);
		setClassIndent(true);			// avoid hanging indent with access modifiers
		setSwitchIndent(true);			// avoid hanging indent with case statements
	}
	else if (formattingStyle == STYLE_GNU)
	{
		setBracketFormatMode(BREAK_MODE);
		setBlockIndent(true);
	}
	else if (formattingStyle == STYLE_LINUX)
	{
		setBracketFormatMode(LINUX_MODE);
		// always for Linux style
		setMinConditionalIndentOption(MINCOND_ONEHALF);
	}
	else if (formattingStyle == STYLE_HORSTMANN)
	{
		setBracketFormatMode(RUN_IN_MODE);
		setSwitchIndent(true);
	}
	else if (formattingStyle == STYLE_1TBS)
	{
		setBracketFormatMode(LINUX_MODE);
		setAddBracketsMode(true);
		setRemoveBracketsMode(false);
	}
	else if (formattingStyle == STYLE_GOOGLE)
	{
		setBracketFormatMode(ATTACH_MODE);
		setModifierIndent(true);
		setClassIndent(false);
	}
	else if (formattingStyle == STYLE_PICO)
	{
		setBracketFormatMode(RUN_IN_MODE);
		setAttachClosingBracketMode(true);
		setSwitchIndent(true);
		setBreakOneLineBlocksMode(false);
		setSingleStatementsMode(false);
		// add-brackets won't work for pico, but it could be fixed if necessary
		// both options should be set to true
		if (shouldAddBrackets)
			shouldAddOneLineBrackets = true;
	}
	else if (formattingStyle == STYLE_LISP)
	{
		setBracketFormatMode(ATTACH_MODE);
		setAttachClosingBracketMode(true);
		setSingleStatementsMode(false);
		// add-one-line-brackets won't work for lisp
		// only shouldAddBrackets should be set to true
		if (shouldAddOneLineBrackets)
		{
			shouldAddBrackets = true;
			shouldAddOneLineBrackets = false;
		}
	}
	setMinConditionalIndentLength();
	// if not set by indent=force-tab-x set equal to indentLength
	if (!getTabLength())
		setDefaultTabLength();
	// add-one-line-brackets implies keep-one-line-blocks
	if (shouldAddOneLineBrackets)
		setBreakOneLineBlocksMode(false);
	// don't allow add-brackets and remove-brackets
	if (shouldAddBrackets || shouldAddOneLineBrackets)
		setRemoveBracketsMode(false);
	// don't allow indent-classes and indent-modifiers
	if (getClassIndent())
		setModifierIndent(false);
}

/**
 * get the next formatted line.
 *
 * @return    formatted line.
 */
string ASFormatter::nextLine()
{
	const string* newHeader;
	bool isInVirginLine = isVirgin;
	isCharImmediatelyPostComment = false;
	isPreviousCharPostComment = false;
	isCharImmediatelyPostLineComment = false;
	isCharImmediatelyPostOpenBlock = false;
	isCharImmediatelyPostCloseBlock = false;
	isCharImmediatelyPostTemplate = false;

	while (!isLineReady)
	{
		if (shouldReparseCurrentChar)
			shouldReparseCurrentChar = false;
		else if (!getNextChar())
		{
			breakLine();
			continue;
		}
		else // stuff to do when reading a new character...
		{
			// make sure that a virgin '{' at the beginning of the file will be treated as a block...
			if (isInVirginLine && currentChar == '{'
			        && currentLineBeginsWithBracket
			        && previousCommandChar == ' ')
				previousCommandChar = '{';
			if (isInClassInitializer
			        && isBracketType(bracketTypeStack->back(), COMMAND_TYPE))
				isInClassInitializer = false;
			if (isInHorstmannRunIn)
				isInLineBreak = false;
			if (!isWhiteSpace(currentChar))
				isInHorstmannRunIn = false;
			isPreviousCharPostComment = isCharImmediatelyPostComment;
			isCharImmediatelyPostComment = false;
			isCharImmediatelyPostTemplate = false;
			isCharImmediatelyPostReturn = false;
			isCharImmediatelyPostThrow = false;
			isCharImmediatelyPostOperator = false;
			isCharImmediatelyPostPointerOrReference = false;
			isCharImmediatelyPostOpenBlock = false;
			isCharImmediatelyPostCloseBlock = false;
		}

		if ((lineIsLineCommentOnly || lineIsCommentOnly)
		        && currentLine.find("*INDENT-ON*", charNum) != string::npos
		        && isFormattingModeOff)
		{
			isFormattingModeOff = false;
			breakLine();
			formattedLine = currentLine;
			charNum = (int) currentLine.length() - 1;
			continue;
		}
		if (isFormattingModeOff)
		{
			breakLine();
			formattedLine = currentLine;
			charNum = (int) currentLine.length() - 1;
			continue;
		}
		if ((lineIsLineCommentOnly || lineIsCommentOnly)
		        && currentLine.find("*INDENT-OFF*", charNum) != string::npos)
		{
			isFormattingModeOff = true;
			if (isInLineBreak)			// is true if not the first line
				breakLine();
			formattedLine = currentLine;
			charNum = (int)currentLine.length() - 1;
			continue;
		}

		if (shouldBreakLineAtNextChar)
		{
			if (isWhiteSpace(currentChar) && !lineIsEmpty)
				continue;
			isInLineBreak = true;
			shouldBreakLineAtNextChar = false;
		}

		if (isInExecSQL && !passedSemicolon)
		{
			if (currentChar == ';')
				passedSemicolon = true;
			appendCurrentChar();
			continue;
		}

		if (isInLineComment)
		{
			formatLineCommentBody();
			continue;
		}
		else if (isInComment)
		{
			formatCommentBody();
			continue;
		}

		else if (isInQuote)
		{
			formatQuoteBody();
			continue;
		}

		// not in quote or comment or line comment

		if (isSequenceReached("//"))
		{
			formatLineCommentOpener();
			testForTimeToSplitFormattedLine();
			continue;
		}
		else if (isSequenceReached("/*"))
		{
			formatCommentOpener();
			testForTimeToSplitFormattedLine();
			continue;
		}
		else if (currentChar == '"' || currentChar == '\'')
		{
			formatQuoteOpener();
			testForTimeToSplitFormattedLine();
			continue;
		}
		// treat these preprocessor statements as a line comment
		else if (currentChar == '#'
		         && currentLine.find_first_not_of(" \t") == (size_t) charNum)
		{
			string preproc = trim(currentLine.c_str() + charNum + 1);
			if (preproc.length() > 0
			        && isCharPotentialHeader(preproc, 0)
			        && (findKeyword(preproc, 0, "region")
			            || findKeyword(preproc, 0, "endregion")
			            || findKeyword(preproc, 0, "error")
			            || findKeyword(preproc, 0, "warning")
			            || findKeyword(preproc, 0, "line")))
			{
				currentLine = rtrim(currentLine);	// trim the end only
				// check for horstmann run-in
				if (formattedLine.length() > 0 && formattedLine[0] == '{')
				{
					isInLineBreak = true;
					isInHorstmannRunIn = false;
				}
				if (previousCommandChar == '}')
					currentHeader = NULL;
				isInLineComment = true;
				appendCurrentChar();
				continue;
			}
		}

		if (isInPreprocessor)
		{
			appendCurrentChar();
			continue;
		}

		if (isInTemplate && shouldCloseTemplates)
		{
			if (previousCommandChar == '<' && isWhiteSpace(currentChar))
				continue;
			if (isWhiteSpace(currentChar) && peekNextChar() == '>')
				continue;
		}

		if (shouldRemoveNextClosingBracket && currentChar == '}')
		{
			currentLine[charNum] = currentChar = ' ';
			shouldRemoveNextClosingBracket = false;
			assert(adjustChecksumIn(-'}'));
			// if the line is empty, delete it
			if (currentLine.find_first_not_of(" \t"))
				continue;
		}

		// handle white space - needed to simplify the rest.
		if (isWhiteSpace(currentChar))
		{
			appendCurrentChar();
			continue;
		}

		/* not in MIDDLE of quote or comment or SQL or white-space of any type ... */

		// check if in preprocessor
		// ** isInPreprocessor will be automatically reset at the beginning
		//    of a new line in getnextChar()
		if (currentChar == '#')
		{
			isInPreprocessor = true;
			// check for horstmann run-in
			if (formattedLine.length() > 0 && formattedLine[0] == '{')
			{
				isInLineBreak = true;
				isInHorstmannRunIn = false;
			}
			processPreprocessor();
			// if top level it is potentially indentable
			if (shouldIndentPreprocBlock
			        && (isBracketType(bracketTypeStack->back(), NULL_TYPE)
			            || isBracketType(bracketTypeStack->back(), NAMESPACE_TYPE))
			        && !foundClassHeader
			        && !isInClassInitializer
			        && sourceIterator->tellg() > preprocBlockEnd)
			{
				// indent the #if preprocessor blocks
				string preproc = ASBeautifier::extractPreprocessorStatement(currentLine);
				if (preproc.length() >= 2 && preproc.substr(0, 2) == "if") // #if, #ifdef, #ifndef
				{
					if (isImmediatelyPostPreprocessor)
						breakLine();
					isIndentableProprocessorBlock = isIndentablePreprocessorBlock(currentLine, charNum);
					isIndentableProprocessor = isIndentableProprocessorBlock;
				}
			}
			if (isIndentableProprocessorBlock
			        && charNum < (int) currentLine.length() - 1
			        && isWhiteSpace(currentLine[charNum + 1]))
			{
				size_t nextText = currentLine.find_first_not_of(" \t", charNum + 1);
				if (nextText != string::npos)
					currentLine.erase(charNum + 1, nextText - charNum - 1);
			}
			if (isIndentableProprocessorBlock
			        && sourceIterator->tellg() >= preprocBlockEnd)
				isIndentableProprocessorBlock = false;
			//  need to fall thru here to reset the variables
		}

		/* not in preprocessor ... */

		if (isImmediatelyPostComment)
		{
			caseHeaderFollowsComments = false;
			isImmediatelyPostComment = false;
			isCharImmediatelyPostComment = true;
		}

		if (isImmediatelyPostLineComment)
		{
			caseHeaderFollowsComments = false;
			isImmediatelyPostLineComment = false;
			isCharImmediatelyPostLineComment = true;
		}

		if (isImmediatelyPostReturn)
		{
			isImmediatelyPostReturn = false;
			isCharImmediatelyPostReturn = true;
		}

		if (isImmediatelyPostThrow)
		{
			isImmediatelyPostThrow = false;
			isCharImmediatelyPostThrow = true;
		}

		if (isImmediatelyPostOperator)
		{
			isImmediatelyPostOperator = false;
			isCharImmediatelyPostOperator = true;
		}
		if (isImmediatelyPostTemplate)
		{
			isImmediatelyPostTemplate = false;
			isCharImmediatelyPostTemplate = true;
		}
		if (isImmediatelyPostPointerOrReference)
		{
			isImmediatelyPostPointerOrReference = false;
			isCharImmediatelyPostPointerOrReference = true;
		}

		// reset isImmediatelyPostHeader information
		if (isImmediatelyPostHeader)
		{
			// should brackets be added
			if (currentChar != '{' && shouldAddBrackets)
			{
				bool bracketsAdded = addBracketsToStatement();
				if (bracketsAdded && !shouldAddOneLineBrackets)
				{
					size_t firstText = currentLine.find_first_not_of(" \t");
					assert(firstText != string::npos);
					if ((int) firstText == charNum)
						breakCurrentOneLineBlock = true;
				}
			}
			// should brackets be removed
			else if (currentChar == '{' && shouldRemoveBrackets)
			{
				bool bracketsRemoved = removeBracketsFromStatement();
				if (bracketsRemoved)
				{
					shouldRemoveNextClosingBracket = true;
					if (isBeforeAnyLineEndComment(charNum))
						spacePadNum--;
					else if (shouldBreakOneLineBlocks
					         || (currentLineBeginsWithBracket
					             && currentLine.find_first_not_of(" \t") != string::npos))
						shouldBreakLineAtNextChar = true;
					continue;
				}
			}

			// break 'else-if' if shouldBreakElseIfs is requested
			if (shouldBreakElseIfs
			        && currentHeader == &AS_ELSE
			        && isOkToBreakBlock(bracketTypeStack->back())
			        && !isBeforeAnyComment()
			        && (shouldBreakOneLineStatements || !isHeaderInMultiStatementLine))
			{
				string nextText = peekNextText(currentLine.substr(charNum));
				if (nextText.length() > 0
				        && isCharPotentialHeader(nextText, 0)
				        && ASBeautifier::findHeader(nextText, 0, headers) == &AS_IF)
				{
					isInLineBreak = true;
				}
			}

			isImmediatelyPostHeader = false;
		}

		if (passedSemicolon)    // need to break the formattedLine
		{
			passedSemicolon = false;
			if (parenStack->back() == 0 && !isCharImmediatelyPostComment && currentChar != ';') // allow ;;
			{
				// does a one-line block have ending comments?
				if (isBracketType(bracketTypeStack->back(), SINGLE_LINE_TYPE))
				{
					size_t blockEnd = currentLine.rfind(AS_CLOSE_BRACKET);
					assert(blockEnd != string::npos);
					// move ending comments to this formattedLine
					if (isBeforeAnyLineEndComment(blockEnd))
					{
						size_t commentStart = currentLine.find_first_not_of(" \t", blockEnd + 1);
						assert(commentStart != string::npos);
						assert((currentLine.compare(commentStart, 2, "//") == 0)
						       || (currentLine.compare(commentStart, 2, "/*") == 0));
						size_t commentLength = currentLine.length() - commentStart;
						formattedLine.append(getIndentLength() - 1, ' ');
						formattedLine.append(currentLine, commentStart, commentLength);
						currentLine.erase(commentStart, commentLength);
						testForTimeToSplitFormattedLine();
					}
				}
				isInExecSQL = false;
				shouldReparseCurrentChar = true;
				if (formattedLine.find_first_not_of(" \t") != string::npos)
					isInLineBreak = true;
				if (needHeaderOpeningBracket)
				{
					isCharImmediatelyPostCloseBlock = true;
					needHeaderOpeningBracket = false;
				}
				continue;
			}
		}

		if (passedColon)
		{
			passedColon = false;
			if (parenStack->back() == 0
			        && !isBeforeAnyComment()
			        && (formattedLine.find_first_not_of(" \t") != string::npos))
			{
				shouldReparseCurrentChar = true;
				isInLineBreak = true;
				continue;
			}
		}

		// Check if in template declaration, e.g. foo<bar> or foo<bar,fig>
		if (!isInTemplate && currentChar == '<')
		{
			checkIfTemplateOpener();
		}

		// handle parens
		if (currentChar == '(' || currentChar == '[' || (isInTemplate && currentChar == '<'))
		{
			questionMarkStack->push_back(foundQuestionMark);
			foundQuestionMark = false;
			parenStack->back()++;
			if (currentChar == '[')
				++squareBracketCount;
		}
		else if (currentChar == ')' || currentChar == ']' || (isInTemplate && currentChar == '>'))
		{
			foundPreCommandHeader = false;
			parenStack->back()--;
			// this can happen in preprocessor directives
			if (parenStack->back() < 0)
				parenStack->back() = 0;
			if (!questionMarkStack->empty())
			{
				foundQuestionMark = questionMarkStack->back();
				questionMarkStack->pop_back();
			}
			if (isInTemplate && currentChar == '>')
			{
				templateDepth--;
				if (templateDepth == 0)
				{
					isInTemplate = false;
					isImmediatelyPostTemplate = true;
				}
			}

			// check if this parenthesis closes a header, e.g. if (...), while (...)
			if (isInHeader && parenStack->back() == 0)
			{
				isInHeader = false;
				isImmediatelyPostHeader = true;
				foundQuestionMark = false;
			}
			if (currentChar == ']')
			{
				--squareBracketCount;
				if (squareBracketCount < 0)
					squareBracketCount = 0;
			}
			if (currentChar == ')')
			{
				foundCastOperator = false;
				if (parenStack->back() == 0)
					endOfAsmReached = true;
			}
		}

		// handle brackets
		if (currentChar == '{' || currentChar == '}')
		{
			// if appendOpeningBracket this was already done for the original bracket
			if (currentChar == '{' && !appendOpeningBracket)
			{
				BracketType newBracketType = getBracketType();
				foundNamespaceHeader = false;
				foundClassHeader = false;
				foundStructHeader = false;
				foundInterfaceHeader = false;
				foundPreDefinitionHeader = false;
				foundPreCommandHeader = false;
				foundPreCommandMacro = false;
				isInPotentialCalculation = false;
				isInObjCMethodDefinition = false;
				isInObjCInterface = false;
				isInEnum = false;
				isJavaStaticConstructor = false;
				isCharImmediatelyPostNonInStmt = false;
				needHeaderOpeningBracket = false;
				shouldKeepLineUnbroken = false;

				isPreviousBracketBlockRelated = !isBracketType(newBracketType, ARRAY_TYPE);
				bracketTypeStack->push_back(newBracketType);
				preBracketHeaderStack->push_back(currentHeader);
				currentHeader = NULL;
				structStack->push_back(isInIndentableStruct);
				if (isBracketType(newBracketType, STRUCT_TYPE) && isCStyle())
					isInIndentableStruct = isStructAccessModified(currentLine, charNum);
				else
					isInIndentableStruct = false;
			}

			// this must be done before the bracketTypeStack is popped
			BracketType bracketType = bracketTypeStack->back();
			bool isOpeningArrayBracket = (isBracketType(bracketType, ARRAY_TYPE)
			                              && bracketTypeStack->size() >= 2
			                              && !isBracketType((*bracketTypeStack)[bracketTypeStack->size() - 2], ARRAY_TYPE)
			                             );

			if (currentChar == '}')
			{
				// if a request has been made to append a post block empty line,
				// but the block exists immediately before a closing bracket,
				// then there is no need for the post block empty line.
				isAppendPostBlockEmptyLineRequested = false;
				breakCurrentOneLineBlock = false;
				if (isInAsm)
					endOfAsmReached = true;
				isInAsmOneLine = isInQuote = false;
				shouldKeepLineUnbroken = false;
				squareBracketCount = 0;

				if (bracketTypeStack->size() > 1)
				{
					previousBracketType = bracketTypeStack->back();
					bracketTypeStack->pop_back();
					isPreviousBracketBlockRelated = !isBracketType(bracketType, ARRAY_TYPE);
				}
				else
				{
					previousBracketType = NULL_TYPE;
					isPreviousBracketBlockRelated = false;
				}

				if (!preBracketHeaderStack->empty())
				{
					currentHeader = preBracketHeaderStack->back();
					preBracketHeaderStack->pop_back();
				}
				else
					currentHeader = NULL;

				if (!structStack->empty())
				{
					isInIndentableStruct = structStack->back();
					structStack->pop_back();
				}
				else
					isInIndentableStruct = false;

				if (isNonInStatementArray
				        && (!isBracketType(bracketTypeStack->back(), ARRAY_TYPE)	// check previous bracket
				            || peekNextChar() == ';'))								// check for "};" added V2.01
					isImmediatelyPostNonInStmt = true;
			}

			// format brackets
			appendOpeningBracket = false;
			if (isBracketType(bracketType, ARRAY_TYPE))
			{
				formatArrayBrackets(bracketType, isOpeningArrayBracket);
			}
			else
			{
				if (currentChar == '{')
					formatOpeningBracket(bracketType);
				else
					formatClosingBracket(bracketType);
			}
			continue;
		}

		if ((((previousCommandChar == '{' && isPreviousBracketBlockRelated)
		        || ((previousCommandChar == '}'
		             && !isImmediatelyPostEmptyBlock
		             && isPreviousBracketBlockRelated
		             && !isPreviousCharPostComment       // Fixes wrongly appended newlines after '}' immediately after comments
		             && peekNextChar() != ' '
		             && !isBracketType(previousBracketType, DEFINITION_TYPE))
		            && !isBracketType(bracketTypeStack->back(), DEFINITION_TYPE)))
		        && isOkToBreakBlock(bracketTypeStack->back()))
		        // check for array
		        || (previousCommandChar == '{'			// added 9/30/2010
		            && isBracketType(bracketTypeStack->back(), ARRAY_TYPE)
		            && !isBracketType(bracketTypeStack->back(), SINGLE_LINE_TYPE)
		            && isNonInStatementArray))
		{
			isCharImmediatelyPostOpenBlock = (previousCommandChar == '{');
			isCharImmediatelyPostCloseBlock = (previousCommandChar == '}');

			if (isCharImmediatelyPostOpenBlock
			        && !isCharImmediatelyPostComment
			        && !isCharImmediatelyPostLineComment)
			{
				previousCommandChar = ' ';

				if (bracketFormatMode == NONE_MODE)
				{
					if (shouldBreakOneLineBlocks
					        && isBracketType(bracketTypeStack->back(), SINGLE_LINE_TYPE))
						isInLineBreak = true;
					else if (currentLineBeginsWithBracket)
						formatRunIn();
					else
						breakLine();
				}
				else if (bracketFormatMode == RUN_IN_MODE
				         && currentChar != '#')
					formatRunIn();
				else
					isInLineBreak = true;
			}
			else if (isCharImmediatelyPostCloseBlock
			         && shouldBreakOneLineStatements
			         && (isLegalNameChar(currentChar) && currentChar != '.')
			         && !isCharImmediatelyPostComment)
			{
				previousCommandChar = ' ';
				isInLineBreak = true;
			}
		}

		// reset block handling flags
		isImmediatelyPostEmptyBlock = false;

		// look for headers
		bool isPotentialHeader = isCharPotentialHeader(currentLine, charNum);

		if (isPotentialHeader && !isInTemplate && !squareBracketCount)
		{
			isNonParenHeader = false;
			foundClosingHeader = false;
			newHeader = findHeader(headers);

			// Qt headers may be variables in C++
			if (newHeader == &AS_FOREVER || newHeader == &AS_FOREACH)
			{
				if (currentLine.find_first_of("=;", charNum) != string::npos)
					newHeader = NULL;
			}

			if (newHeader != NULL)
			{
				const string* previousHeader;

				// recognize closing headers of do..while, if..else, try..catch..finally
				if ((newHeader == &AS_ELSE && currentHeader == &AS_IF)
				        || (newHeader == &AS_WHILE && currentHeader == &AS_DO)
				        || (newHeader == &AS_CATCH && currentHeader == &AS_TRY)
				        || (newHeader == &AS_CATCH && currentHeader == &AS_CATCH)
				        || (newHeader == &AS_FINALLY && currentHeader == &AS_TRY)
				        || (newHeader == &AS_FINALLY && currentHeader == &AS_CATCH)
				        || (newHeader == &_AS_FINALLY && currentHeader == &_AS_TRY)
				        || (newHeader == &_AS_EXCEPT && currentHeader == &_AS_TRY)
				        || (newHeader == &AS_SET && currentHeader == &AS_GET)
				        || (newHeader == &AS_REMOVE && currentHeader == &AS_ADD))
					foundClosingHeader = true;

				previousHeader = currentHeader;
				currentHeader = newHeader;
				needHeaderOpeningBracket = true;

				// is the previous statement on the same line?
				if ((previousNonWSChar == ';' || previousNonWSChar == ':')
				        && !isInLineBreak
				        && isOkToBreakBlock(bracketTypeStack->back()))
				{
					// if breaking lines, break the line at the header
					// except for multiple 'case' statements on a line
					if (maxCodeLength != string::npos
					        && previousHeader != &AS_CASE)
						isInLineBreak = true;
					else
						isHeaderInMultiStatementLine = true;
				}

				if (foundClosingHeader && previousNonWSChar == '}')
				{
					if (isOkToBreakBlock(bracketTypeStack->back()))
						isLineBreakBeforeClosingHeader();

					// get the adjustment for a comment following the closing header
					if (isInLineBreak)
						nextLineSpacePadNum = getNextLineCommentAdjustment();
					else
						spacePadNum = getCurrentLineCommentAdjustment();
				}

				// check if the found header is non-paren header
				isNonParenHeader = findHeader(nonParenHeaders) != NULL;

				// join 'else if' statements
				if (currentHeader == &AS_IF && previousHeader == &AS_ELSE && isInLineBreak
				        && !shouldBreakElseIfs && !isCharImmediatelyPostLineComment)
				{
					// 'else' must be last thing on the line
					size_t start = formattedLine.length() >= 6 ? formattedLine.length() - 6 : 0;
					if (formattedLine.find(AS_ELSE, start) != string::npos)
					{
						appendSpacePad();
						isInLineBreak = false;
					}
				}

				appendSequence(*currentHeader);
				goForward(currentHeader->length() - 1);
				// if a paren-header is found add a space after it, if needed
				// this checks currentLine, appendSpacePad() checks formattedLine
				// in 'case' and C# 'catch' can be either a paren or non-paren header
				if (shouldPadHeader
				        && (!isNonParenHeader
				            || (currentHeader == &AS_CASE && peekNextChar() == '(')
				            || (currentHeader == &AS_CATCH && peekNextChar() == '('))
				        && charNum < (int) currentLine.length() - 1 && !isWhiteSpace(currentLine[charNum + 1]))
					appendSpacePad();

				// Signal that a header has been reached
				// *** But treat a closing while() (as in do...while)
				//     as if it were NOT a header since a closing while()
				//     should never have a block after it!
				if (currentHeader != &AS_CASE && currentHeader != &AS_DEFAULT
				        && !(foundClosingHeader && currentHeader == &AS_WHILE))
				{
					isInHeader = true;

					// in C# 'catch' and 'delegate' can be a paren or non-paren header
					if (isNonParenHeader && !isSharpStyleWithParen(currentHeader))
					{
						isImmediatelyPostHeader = true;
						isInHeader = false;
					}
				}

				if (shouldBreakBlocks
				        && isOkToBreakBlock(bracketTypeStack->back())
				        && !isHeaderInMultiStatementLine)
				{
					if (previousHeader == NULL
					        && !foundClosingHeader
					        && !isCharImmediatelyPostOpenBlock
					        && !isImmediatelyPostCommentOnly)
					{
						isPrependPostBlockEmptyLineRequested = true;
					}

					if (currentHeader == &AS_ELSE
					        || currentHeader == &AS_CATCH
					        || currentHeader == &AS_FINALLY
					        || foundClosingHeader)
					{
						isPrependPostBlockEmptyLineRequested = false;
					}

					if (shouldBreakClosingHeaderBlocks
					        && isCharImmediatelyPostCloseBlock
					        && !isImmediatelyPostCommentOnly
					        && currentHeader != &AS_WHILE)    // closing do-while block
					{
						isPrependPostBlockEmptyLineRequested = true;
					}
				}

				if (currentHeader == &AS_CASE
				        || currentHeader == &AS_DEFAULT)
					isInCase = true;

				continue;
			}
			else if ((newHeader = findHeader(preDefinitionHeaders)) != NULL
			         && parenStack->back() == 0
			         && !isInEnum)		// not C++11 enum class
			{
				if (newHeader == &AS_NAMESPACE)
					foundNamespaceHeader = true;
				if (newHeader == &AS_CLASS)
					foundClassHeader = true;
				if (newHeader == &AS_STRUCT)
					foundStructHeader = true;
				if (newHeader == &AS_INTERFACE)
					foundInterfaceHeader = true;
				foundPreDefinitionHeader = true;
				appendSequence(*newHeader);
				goForward(newHeader->length() - 1);

				continue;
			}
			else if ((newHeader = findHeader(preCommandHeaders)) != NULL)
			{
				// a 'const' variable is not a preCommandHeader
				if (previousNonWSChar != ';'
				        && previousNonWSChar != '{'
				        && getPreviousWord(currentLine, charNum) != AS_STATIC)
					foundPreCommandHeader = true;
			}
			else if ((newHeader = findHeader(castOperators)) != NULL)
			{
				foundCastOperator = true;
				appendSequence(*newHeader);
				goForward(newHeader->length() - 1);
				continue;
			}
		}   // (isPotentialHeader && !isInTemplate)

		if (isInLineBreak)          // OK to break line here
		{
			breakLine();
			if (isInVirginLine)		// adjust for the first line
			{
				lineCommentNoBeautify = lineCommentNoIndent;
				lineCommentNoIndent = false;
				if (isImmediatelyPostPreprocessor)
				{
					isInIndentablePreproc = isIndentableProprocessor;
					isIndentableProprocessor = false;
				}
			}
		}

		if (previousNonWSChar == '}' || currentChar == ';')
		{
			if (currentChar == ';')
			{
				squareBracketCount = 0;

				if (((shouldBreakOneLineStatements
				        || isBracketType(bracketTypeStack->back(), SINGLE_LINE_TYPE))
				        && isOkToBreakBlock(bracketTypeStack->back()))
				        && !(attachClosingBracketMode && peekNextChar() == '}'))
				{
					passedSemicolon = true;
				}

				// append post block empty line for unbracketed header
				if (shouldBreakBlocks
				        && currentHeader != NULL
				        && currentHeader != &AS_CASE
				        && currentHeader != &AS_DEFAULT
				        && !isHeaderInMultiStatementLine
				        && parenStack->back() == 0)
				{
					isAppendPostBlockEmptyLineRequested = true;
				}
			}
			if (currentChar != ';'
			        || (needHeaderOpeningBracket && parenStack->back() == 0))
				currentHeader = NULL;
			resetEndOfStatement();
		}

		if (currentChar == ':'
		        && previousChar != ':'         // not part of '::'
		        && peekNextChar() != ':')      // not part of '::'
		{
			if (isInCase)
			{
				isInCase = false;
				if (shouldBreakOneLineStatements)
					passedColon = true;
			}
			else if (isCStyle()                     // for C/C++ only
			         && isOkToBreakBlock(bracketTypeStack->back())
			         && shouldBreakOneLineStatements
			         && !foundQuestionMark          // not in a ?: sequence
			         && !foundPreDefinitionHeader   // not in a definition block (e.g. class foo : public bar
			         && previousCommandChar != ')'  // not immediately after closing paren of a method header, e.g. ASFormatter::ASFormatter(...) : ASBeautifier(...)
			         && !foundPreCommandHeader      // not after a 'noexcept'
			         && !squareBracketCount         // not in objC method call
			         && !isInObjCMethodDefinition   // not objC '-' or '+' method
			         && !isInObjCInterface          // not objC @interface
			         && !isInObjCSelector           // not objC @selector
			         && !isDigit(peekNextChar())    // not a bit field
			         && !isInEnum                   // not an enum with a base type
			         && !isInAsm                    // not in extended assembler
			         && !isInAsmOneLine             // not in extended assembler
			         && !isInAsmBlock)              // not in extended assembler
			{
				passedColon = true;
			}

			if (isCStyle()
			        && shouldPadMethodColon
			        && (squareBracketCount > 0 || isInObjCMethodDefinition || isInObjCSelector)
			        && !foundQuestionMark)			// not in a ?: sequence
				padObjCMethodColon();

			if (isInObjCInterface)
			{
				appendSpacePad();
				if ((int) currentLine.length() > charNum + 1 && !isWhiteSpace(currentLine[charNum + 1]))
					currentLine.insert(charNum + 1, " ");
			}

			if (isClassInitializer())
				isInClassInitializer = true;
		}

		if (currentChar == '?')
			foundQuestionMark = true;

		if (isPotentialHeader && !isInTemplate)
		{
			if (findKeyword(currentLine, charNum, AS_NEW))
				isInPotentialCalculation = false;

			if (findKeyword(currentLine, charNum, AS_RETURN))
			{
				isInPotentialCalculation = true;	// return is the same as an = sign
				isImmediatelyPostReturn = true;
			}

			if (findKeyword(currentLine, charNum, AS_OPERATOR))
				isImmediatelyPostOperator = true;

			if (findKeyword(currentLine, charNum, AS_ENUM))
			{
				size_t firstNum = currentLine.find_first_of("(){},/");
				if (firstNum == string::npos
				        || currentLine[firstNum] == '{'
				        || currentLine[firstNum] == '/')
					isInEnum = true;
			}

			if (isCStyle()
			        && findKeyword(currentLine, charNum, AS_THROW)
			        && previousCommandChar != ')'
			        && !foundPreCommandHeader)      // 'const' throw()
				isImmediatelyPostThrow = true;

			if (isCStyle() && findKeyword(currentLine, charNum, AS_EXTERN) && isExternC())
				isInExternC = true;

			// Objective-C NSException macros are preCommandHeaders
			if (isCStyle() && findKeyword(currentLine, charNum, AS_NS_DURING))
				foundPreCommandMacro = true;
			if (isCStyle() && findKeyword(currentLine, charNum, AS_NS_HANDLER))
				foundPreCommandMacro = true;

			if (isCStyle() && isExecSQL(currentLine, charNum))
				isInExecSQL = true;

			if (isCStyle())
			{
				if (findKeyword(currentLine, charNum, AS_ASM)
				        || findKeyword(currentLine, charNum, AS__ASM__))
				{
					isInAsm = true;
				}
				else if (findKeyword(currentLine, charNum, AS_MS_ASM)		// microsoft specific
				         || findKeyword(currentLine, charNum, AS_MS__ASM))
				{
					int index = 4;
					if (peekNextChar() == '_')	// check for __asm
						index = 5;

					char peekedChar = ASBase::peekNextChar(currentLine, charNum + index);
					if (peekedChar == '{' || peekedChar == ' ')
						isInAsmBlock = true;
					else
						isInAsmOneLine = true;
				}
			}

			if (isJavaStyle()
			        && (findKeyword(currentLine, charNum, AS_STATIC)
			            && isNextCharOpeningBracket(charNum + 6)))
				isJavaStaticConstructor = true;

			if (isSharpStyle()
			        && (findKeyword(currentLine, charNum, AS_DELEGATE)
			            || findKeyword(currentLine, charNum, AS_UNCHECKED)))
				isSharpDelegate = true;

			// append the entire name
			string name = getCurrentWord(currentLine, charNum);
			// must pad the 'and' and 'or' operators if required
			if (name == "and" || name == "or")
			{
				if (shouldPadOperators && previousNonWSChar != ':')
				{
					appendSpacePad();
					appendOperator(name);
					goForward(name.length() - 1);
					if (!isBeforeAnyComment()
					        && !(currentLine.compare(charNum + 1, 1, AS_SEMICOLON) == 0)
					        && !(currentLine.compare(charNum + 1, 2, AS_SCOPE_RESOLUTION) == 0))
						appendSpaceAfter();
				}
				else
				{
					appendOperator(name);
					goForward(name.length() - 1);
				}
			}
			else
			{
				appendSequence(name);
				goForward(name.length() - 1);
			}

			continue;

		}   // (isPotentialHeader &&  !isInTemplate)

		// determine if this is an Objective-C statement

		if (currentChar == '@'
		        && isCharPotentialHeader(currentLine, charNum + 1)
		        && findKeyword(currentLine, charNum + 1, AS_INTERFACE)
		        && isBracketType(bracketTypeStack->back(), NULL_TYPE))
		{
			isInObjCInterface = true;
			string name = '@' + AS_INTERFACE;
			appendSequence(name);
			goForward(name.length() - 1);
			continue;
		}
		else if (currentChar == '@'
		         && isCharPotentialHeader(currentLine, charNum + 1)
		         && findKeyword(currentLine, charNum + 1, AS_SELECTOR))
		{
			isInObjCSelector = true;
			string name = '@' + AS_SELECTOR;
			appendSequence(name);
			goForward(name.length() - 1);
			continue;
		}
		else if ((currentChar == '-' || currentChar == '+')
		         && peekNextChar() == '('
		         && isBracketType(bracketTypeStack->back(), NULL_TYPE)
		         && !isInPotentialCalculation)
		{
			isInObjCMethodDefinition = true;
			isInObjCInterface = false;
			appendCurrentChar();
			if (shouldPadMethodPrefix || shouldUnPadMethodPrefix)
			{
				size_t i = currentLine.find_first_not_of(" \t", charNum + 1);
				if (i != string::npos)
					goForward(i - charNum - 1);
				if (shouldPadMethodPrefix)
					appendSpaceAfter();
			}
			continue;
		}

		// determine if this is a potential calculation

		bool isPotentialOperator = isCharPotentialOperator(currentChar);
		newHeader = NULL;

		if (isPotentialOperator)
		{
			newHeader = findOperator(operators);

			// check for Java ? wildcard
			if (newHeader == &AS_GCC_MIN_ASSIGN && isJavaStyle() && isInTemplate)
				newHeader = NULL;

			if (newHeader != NULL)
			{
				if (newHeader == &AS_LAMBDA)
					foundPreCommandHeader = true;

				// correct mistake of two >> closing a template
				if (isInTemplate && (newHeader == &AS_GR_GR || newHeader == &AS_GR_GR_GR))
					newHeader = &AS_GR;

				if (!isInPotentialCalculation)
				{
					// must determine if newHeader is an assignment operator
					// do NOT use findOperator!!!
					if (find(assignmentOperators->begin(), assignmentOperators->end(), newHeader)
					        != assignmentOperators->end())
					{
						foundPreCommandHeader = false;
						char peekedChar = peekNextChar();
						isInPotentialCalculation = !(newHeader == &AS_EQUAL && peekedChar == '*')
						                           && !(newHeader == &AS_EQUAL && peekedChar == '&')
						                           && !isCharImmediatelyPostOperator;
					}
				}
			}
		}

		// process pointers and references
		// check newHeader to eliminate things like '&&' sequence
		if (!isJavaStyle()
		        && (newHeader == &AS_MULT
		            || newHeader == &AS_BIT_AND
		            || newHeader == &AS_BIT_XOR
		            || newHeader == &AS_AND)
		        && isPointerOrReference())
		{
			if (!isDereferenceOrAddressOf() && !isOperatorPaddingDisabled())
				formatPointerOrReference();
			else
			{
				appendOperator(*newHeader);
				goForward(newHeader->length() - 1);
			}
			isImmediatelyPostPointerOrReference = true;
			continue;
		}

		if (shouldPadOperators && newHeader != NULL && !isOperatorPaddingDisabled())
		{
			padOperators(newHeader);
			continue;
		}

		// pad commas and semi-colons
		if (currentChar == ';'
		        || (currentChar == ',' && shouldPadOperators))
		{
			char nextChar = ' ';
			if (charNum + 1 < (int) currentLine.length())
				nextChar = currentLine[charNum + 1];
			if (!isWhiteSpace(nextChar)
			        && nextChar != '}'
			        && nextChar != ')'
			        && nextChar != ']'
			        && nextChar != '>'
			        && nextChar != ';'
			        && !isBeforeAnyComment()
			        /* && !(isBracketType(bracketTypeStack->back(), ARRAY_TYPE)) */
			   )
			{
				appendCurrentChar();
				appendSpaceAfter();
				continue;
			}
		}

		// do NOT use 'continue' after this, it must do padParens if necessary
		if (currentChar == '('
		        && shouldPadHeader
		        && (isCharImmediatelyPostReturn || isCharImmediatelyPostThrow))
			appendSpacePad();

		if ((currentChar == '(' || currentChar == ')')
		        && (shouldPadParensOutside || shouldPadParensInside || shouldUnPadParens || shouldPadFirstParen))
		{
			padParens();
			continue;
		}

		// bypass the entire operator
		if (newHeader != NULL)
		{
			appendOperator(*newHeader);
			goForward(newHeader->length() - 1);
			continue;
		}

		appendCurrentChar();

	}   // end of while loop  *  end of while loop  *  end of while loop  *  end of while loop

	// return a beautified (i.e. correctly indented) line.

	string beautifiedLine;
	size_t readyFormattedLineLength = trim(readyFormattedLine).length();
	bool isInNamespace = isBracketType(bracketTypeStack->back(), NAMESPACE_TYPE);

	if (prependEmptyLine		// prepend a blank line before this formatted line
	        && readyFormattedLineLength > 0
	        && previousReadyFormattedLineLength > 0)
	{
		isLineReady = true;		// signal a waiting readyFormattedLine
		beautifiedLine = beautify("");
		previousReadyFormattedLineLength = 0;
		// call the enhancer for new empty lines
		enhancer->enhance(beautifiedLine, isInNamespace, isInPreprocessorBeautify, isInBeautifySQL);
	}
	else		// format the current formatted line
	{
		isLineReady = false;
		horstmannIndentInStatement = horstmannIndentChars;
		beautifiedLine = beautify(readyFormattedLine);
		previousReadyFormattedLineLength = readyFormattedLineLength;
		// the enhancer is not called for no-indent line comments
		if (!lineCommentNoBeautify && !isFormattingModeOff)
			enhancer->enhance(beautifiedLine, isInNamespace, isInPreprocessorBeautify, isInBeautifySQL);
		horstmannIndentChars = 0;
		lineCommentNoBeautify = lineCommentNoIndent;
		lineCommentNoIndent = false;
		isInIndentablePreproc = isIndentableProprocessor;
		isIndentableProprocessor = false;
		isElseHeaderIndent = elseHeaderFollowsComments;
		isCaseHeaderCommentIndent = caseHeaderFollowsComments;
		if (isCharImmediatelyPostNonInStmt)
		{
			isNonInStatementArray = false;
			isCharImmediatelyPostNonInStmt = false;
		}
		isInPreprocessorBeautify = isInPreprocessor;	// used by ASEnhancer
		isInBeautifySQL = isInExecSQL;					// used by ASEnhancer
	}

	prependEmptyLine = false;
	assert(computeChecksumOut(beautifiedLine));
	return beautifiedLine;
}

/**
 * check if there are any indented lines ready to be read by nextLine()
 *
 * @return    are there any indented lines ready?
 */
bool ASFormatter::hasMoreLines() const
{
	return !endOfCodeReached;
}

/**
 * comparison function for BracketType enum
 */
bool ASFormatter::isBracketType(BracketType a, BracketType b) const
{
	if (a == NULL_TYPE || b == NULL_TYPE)
		return (a == b);
	return ((a & b) == b);
}

/**
 * set the formatting style.
 *
 * @param style         the formatting style.
 */
void ASFormatter::setFormattingStyle(FormatStyle style)
{
	formattingStyle = style;
}

/**
 * set the add brackets mode.
 * options:
 *    true     brackets added to headers for single line statements.
 *    false    brackets NOT added to headers for single line statements.
 *
 * @param state         the add brackets state.
 */
void ASFormatter::setAddBracketsMode(bool state)
{
	shouldAddBrackets = state;
}

/**
 * set the add one line brackets mode.
 * options:
 *    true     one line brackets added to headers for single line statements.
 *    false    one line brackets NOT added to headers for single line statements.
 *
 * @param state         the add one line brackets state.
 */
void ASFormatter::setAddOneLineBracketsMode(bool state)
{
	shouldAddBrackets = state;
	shouldAddOneLineBrackets = state;
}

/**
 * set the remove brackets mode.
 * options:
 *    true     brackets removed from headers for single line statements.
 *    false    brackets NOT removed from headers for single line statements.
 *
 * @param state         the remove brackets state.
 */
void ASFormatter::setRemoveBracketsMode(bool state)
{
	shouldRemoveBrackets = state;
}

/**
 * set the bracket formatting mode.
 * options:
 *
 * @param mode         the bracket formatting mode.
 */
void ASFormatter::setBracketFormatMode(BracketMode mode)
{
	bracketFormatMode = mode;
}

/**
 * set 'break after' mode for maximum code length
 *
 * @param state         the 'break after' mode.
 */
void ASFormatter::setBreakAfterMode(bool state)
{
	shouldBreakLineAfterLogical = state;
}

/**
 * set closing header bracket breaking mode
 * options:
 *    true     brackets just before closing headers (e.g. 'else', 'catch')
 *             will be broken, even if standard brackets are attached.
 *    false    closing header brackets will be treated as standard brackets.
 *
 * @param state         the closing header bracket breaking mode.
 */
void ASFormatter::setBreakClosingHeaderBracketsMode(bool state)
{
	shouldBreakClosingHeaderBrackets = state;
}

/**
 * set 'else if()' breaking mode
 * options:
 *    true     'else' headers will be broken from their succeeding 'if' headers.
 *    false    'else' headers will be attached to their succeeding 'if' headers.
 *
 * @param state         the 'else if()' breaking mode.
 */
void ASFormatter::setBreakElseIfsMode(bool state)
{
	shouldBreakElseIfs = state;
}

/**
 * set maximum code length
 *
 * @param max         the maximum code length.
 */
void ASFormatter::setMaxCodeLength(int max)
{
	maxCodeLength = max;
}

/**
 * set operator padding mode.
 * options:
 *    true     statement operators will be padded with spaces around them.
 *    false    statement operators will not be padded.
 *
 * @param state         the padding mode.
 */
void ASFormatter::setOperatorPaddingMode(bool state)
{
	shouldPadOperators = state;
}

/**
 * set parenthesis outside padding mode.
 * options:
 *    true     statement parentheses will be padded with spaces around them.
 *    false    statement parentheses will not be padded.
 *
 * @param state         the padding mode.
 */
void ASFormatter::setParensOutsidePaddingMode(bool state)
{
	shouldPadParensOutside = state;
}

/**
 * set parenthesis inside padding mode.
 * options:
 *    true     statement parenthesis will be padded with spaces around them.
 *    false    statement parenthesis will not be padded.
 *
 * @param state         the padding mode.
 */
void ASFormatter::setParensInsidePaddingMode(bool state)
{
	shouldPadParensInside = state;
}

/**
 * set padding mode before one or more open parentheses.
 * options:
 *    true     first open parenthesis will be padded with a space before.
 *    false    first open parenthesis will not be padded.
 *
 * @param state         the padding mode.
 */
void ASFormatter::setParensFirstPaddingMode(bool state)
{
	shouldPadFirstParen = state;
}

/**
 * set header padding mode.
 * options:
 *    true     headers will be padded with spaces around them.
 *    false    headers will not be padded.
 *
 * @param state         the padding mode.
 */
void ASFormatter::setParensHeaderPaddingMode(bool state)
{
	shouldPadHeader = state;
}

/**
 * set parenthesis unpadding mode.
 * options:
 *    true     statement parenthesis will be unpadded with spaces removed around them.
 *    false    statement parenthesis will not be unpadded.
 *
 * @param state         the padding mode.
 */
void ASFormatter::setParensUnPaddingMode(bool state)
{
	shouldUnPadParens = state;
}

/**
* set the state of the preprocessor indentation option.
* If true, #ifdef blocks at level 0 will be indented.
*
* @param   state             state of option.
*/
void ASFormatter::setPreprocBlockIndent(bool state)
{
	shouldIndentPreprocBlock = state;
}

/**
 * Set strip comment prefix mode.
 * options:
 *    true     strip leading '*' in a comment.
 *    false    leading '*' in a comment will be left unchanged.
 *
 * @param state         the strip comment prefix mode.
 */
void ASFormatter::setStripCommentPrefix(bool state)
{
	shouldStripCommentPrefix = state;
}

/**
 * set objective-c '-' or '+' class prefix padding mode.
 * options:
 *    true     class prefix will be padded a spaces after them.
 *    false    class prefix will be left unchanged.
 *
 * @param state         the padding mode.
 */
void ASFormatter::setMethodPrefixPaddingMode(bool state)
{
	shouldPadMethodPrefix = state;
}

/**
 * set objective-c '-' or '+' class prefix unpadding mode.
 * options:
 *    true     class prefix will be unpadded with spaces after them removed.
 *    false    class prefix will left unchanged.
 *
 * @param state         the unpadding mode.
 */
void ASFormatter::setMethodPrefixUnPaddingMode(bool state)
{
	shouldUnPadMethodPrefix = state;
}

/**
 * set objective-c method colon padding mode.
 *
 * @param mode         objective-c colon padding mode.
 */
void ASFormatter::setObjCColonPaddingMode(ObjCColonPad mode)
{
	shouldPadMethodColon = true;
	objCColonPadMode = mode;
}

/**
 * set option to attach closing brackets
 *
 * @param state        true = attach, false = don't attach.
 */
void ASFormatter::setAttachClosingBracketMode(bool state)
{
	attachClosingBracketMode = state;
}

/**
 * set option to attach class brackets
 *
 * @param state        true = attach, false = use style default.
 */
void ASFormatter::setAttachClass(bool state)
{
	shouldAttachClass = state;
}

/**
 * set option to attach extern "C" brackets
 *
 * @param state        true = attach, false = use style default.
 */
void ASFormatter::setAttachExternC(bool state)
{
	shouldAttachExternC = state;
}

/**
 * set option to attach namespace brackets
 *
 * @param state        true = attach, false = use style default.
 */
void ASFormatter::setAttachNamespace(bool state)
{
	shouldAttachNamespace = state;
}

/**
 * set option to attach inline brackets
 *
 * @param state        true = attach, false = use style default.
 */
void ASFormatter::setAttachInline(bool state)
{
	shouldAttachInline = state;
}

/**
 * set option to break/not break one-line blocks
 *
 * @param state        true = break, false = don't break.
 */
void ASFormatter::setBreakOneLineBlocksMode(bool state)
{
	shouldBreakOneLineBlocks = state;
}

void ASFormatter::setCloseTemplatesMode(bool state)
{
	shouldCloseTemplates = state;
}

/**
 * set option to break/not break lines consisting of multiple statements.
 *
 * @param state        true = break, false = don't break.
 */
void ASFormatter::setSingleStatementsMode(bool state)
{
	shouldBreakOneLineStatements = state;
}

/**
 * set option to convert tabs to spaces.
 *
 * @param state        true = convert, false = don't convert.
 */
void ASFormatter::setTabSpaceConversionMode(bool state)
{
	shouldConvertTabs = state;
}

/**
 * set option to indent comments in column 1.
 *
 * @param state        true = indent, false = don't indent.
 */
void ASFormatter::setIndentCol1CommentsMode(bool state)
{
	shouldIndentCol1Comments = state;
}

/**
 * set option to force all line ends to a particular style.
 *
 * @param fmt           format enum value
 */
void ASFormatter::setLineEndFormat(LineEndFormat fmt)
{
	lineEnd = fmt;
}

/**
 * set option to break unrelated blocks of code with empty lines.
 *
 * @param state        true = convert, false = don't convert.
 */
void ASFormatter::setBreakBlocksMode(bool state)
{
	shouldBreakBlocks = state;
}

/**
 * set option to break closing header blocks of code (such as 'else', 'catch', ...) with empty lines.
 *
 * @param state        true = convert, false = don't convert.
 */
void ASFormatter::setBreakClosingHeaderBlocksMode(bool state)
{
	shouldBreakClosingHeaderBlocks = state;
}

/**
 * set option to delete empty lines.
 *
 * @param state        true = delete, false = don't delete.
 */
void ASFormatter::setDeleteEmptyLinesMode(bool state)
{
	shouldDeleteEmptyLines = state;
}

/**
 * set the pointer alignment.
 *
 * @param alignment    the pointer alignment.
 */
void ASFormatter::setPointerAlignment(PointerAlign alignment)
{
	pointerAlignment = alignment;
}

void ASFormatter::setReferenceAlignment(ReferenceAlign alignment)
{
	referenceAlignment = alignment;
}

/**
 * jump over several characters.
 *
 * @param i       the number of characters to jump over.
 */
void ASFormatter::goForward(int i)
{
	while (--i >= 0)
		getNextChar();
}

/**
 * peek at the next unread character.
 *
 * @return     the next unread character.
 */
char ASFormatter::peekNextChar() const
{
	char ch = ' ';
	size_t peekNum = currentLine.find_first_not_of(" \t", charNum + 1);

	if (peekNum == string::npos)
		return ch;

	ch = currentLine[peekNum];

	return ch;
}

/**
 * check if current placement is before a comment
 *
 * @return     is before a comment.
 */
bool ASFormatter::isBeforeComment() const
{
	bool foundComment = false;
	size_t peekNum = currentLine.find_first_not_of(" \t", charNum + 1);

	if (peekNum == string::npos)
		return foundComment;

	foundComment = (currentLine.compare(peekNum, 2, "/*") == 0);

	return foundComment;
}

/**
 * check if current placement is before a comment or line-comment
 *
 * @return     is before a comment or line-comment.
 */
bool ASFormatter::isBeforeAnyComment() const
{
	bool foundComment = false;
	size_t peekNum = currentLine.find_first_not_of(" \t", charNum + 1);

	if (peekNum == string::npos)
		return foundComment;

	foundComment = (currentLine.compare(peekNum, 2, "/*") == 0
	                || currentLine.compare(peekNum, 2, "//") == 0);

	return foundComment;
}

/**
 * check if current placement is before a comment or line-comment
 * if a block comment it must be at the end of the line
 *
 * @return     is before a comment or line-comment.
 */
bool ASFormatter::isBeforeAnyLineEndComment(int startPos) const
{
	bool foundLineEndComment = false;
	size_t peekNum = currentLine.find_first_not_of(" \t", startPos + 1);

	if (peekNum != string::npos)
	{
		if (currentLine.compare(peekNum, 2, "//") == 0)
			foundLineEndComment = true;
		else if (currentLine.compare(peekNum, 2, "/*") == 0)
		{
			// comment must be closed on this line with nothing after it
			size_t endNum = currentLine.find("*/", peekNum + 2);
			if (endNum != string::npos)
			{
				size_t nextChar = currentLine.find_first_not_of(" \t", endNum + 2);
				if (nextChar == string::npos)
					foundLineEndComment = true;
			}
		}
	}
	return foundLineEndComment;
}

/**
 * check if current placement is before a comment followed by a line-comment
 *
 * @return     is before a multiple line-end comment.
 */
bool ASFormatter::isBeforeMultipleLineEndComments(int startPos) const
{
	bool foundMultipleLineEndComment = false;
	size_t peekNum = currentLine.find_first_not_of(" \t", startPos + 1);

	if (peekNum != string::npos)
	{
		if (currentLine.compare(peekNum, 2, "/*") == 0)
		{
			// comment must be closed on this line with nothing after it
			size_t endNum = currentLine.find("*/", peekNum + 2);
			if (endNum != string::npos)
			{
				size_t nextChar = currentLine.find_first_not_of(" \t", endNum + 2);
				if (nextChar != string::npos
				        && currentLine.compare(nextChar, 2, "//") == 0)
					foundMultipleLineEndComment = true;
			}
		}
	}
	return foundMultipleLineEndComment;
}

/**
 * get the next character, increasing the current placement in the process.
 * the new character is inserted into the variable currentChar.
 *
 * @return   whether succeeded to receive the new character.
 */
bool ASFormatter::getNextChar()
{
	isInLineBreak = false;
	previousChar = currentChar;

	if (!isWhiteSpace(currentChar))
	{
		previousNonWSChar = currentChar;
		if (!isInComment && !isInLineComment && !isInQuote
		        && !isImmediatelyPostComment
		        && !isImmediatelyPostLineComment
		        && !isInPreprocessor
		        && !isSequenceReached("/*")
		        && !isSequenceReached("//"))
			previousCommandChar = currentChar;
	}

	if (charNum + 1 < (int) currentLine.length()
	        && (!isWhiteSpace(peekNextChar()) || isInComment || isInLineComment))
	{
		currentChar = currentLine[++charNum];

		if (currentChar == '\t' && shouldConvertTabs)
			convertTabToSpaces();

		return true;
	}

	// end of line has been reached
	return getNextLine();
}

/**
 * get the next line of input, increasing the current placement in the process.
 *
 * @param emptyLineWasDeleted         an empty line was deleted.
 * @return   whether succeeded in reading the next line.
 */
bool ASFormatter::getNextLine(bool emptyLineWasDeleted /*false*/)
{
	if (sourceIterator->hasMoreLines())
	{
		if (appendOpeningBracket)
			currentLine = "{";		// append bracket that was removed from the previous line
		else
		{
			currentLine = sourceIterator->nextLine(emptyLineWasDeleted);
			assert(computeChecksumIn(currentLine));
		}
		// reset variables for new line
		inLineNumber++;
		if (endOfAsmReached)
			endOfAsmReached = isInAsmBlock = isInAsm = false;
		shouldKeepLineUnbroken = false;
		isInCommentStartLine = false;
		isInCase = false;
		isInAsmOneLine = false;
		isHeaderInMultiStatementLine = false;
		isInQuoteContinuation = isInVerbatimQuote | haveLineContinuationChar;
		haveLineContinuationChar = false;
		isImmediatelyPostEmptyLine = lineIsEmpty;
		previousChar = ' ';

		if (currentLine.length() == 0)
			currentLine = string(" ");        // a null is inserted if this is not done

		// unless reading in the first line of the file, break a new line.
		if (!isVirgin)
			isInLineBreak = true;
		else
			isVirgin = false;

		if (isImmediatelyPostNonInStmt)
		{
			isCharImmediatelyPostNonInStmt = true;
			isImmediatelyPostNonInStmt = false;
		}

		// check if is in preprocessor before line trimming
		// a blank line after a \ will remove the flag
		isImmediatelyPostPreprocessor = isInPreprocessor;
		if (!isInComment
		        && (previousNonWSChar != '\\'
		            || isEmptyLine(currentLine)))
			isInPreprocessor = false;

		if (passedSemicolon)
			isInExecSQL = false;
		initNewLine();

		currentChar = currentLine[charNum];
		if (isInHorstmannRunIn && previousNonWSChar == '{' && !isInComment)
			isInLineBreak = false;
		isInHorstmannRunIn = false;

		if (currentChar == '\t' && shouldConvertTabs)
			convertTabToSpaces();

		// check for an empty line inside a command bracket.
		// if yes then read the next line (calls getNextLine recursively).
		// must be after initNewLine.
		if (shouldDeleteEmptyLines
		        && lineIsEmpty
		        && isBracketType((*bracketTypeStack)[bracketTypeStack->size() - 1], COMMAND_TYPE))
		{
			if (!shouldBreakBlocks || previousNonWSChar == '{' || !commentAndHeaderFollows())
			{
				isInPreprocessor = isImmediatelyPostPreprocessor;		// restore
				lineIsEmpty = false;
				return getNextLine(true);
			}
		}
		return true;
	}
	else
	{
		endOfCodeReached = true;
		return false;
	}
}

/**
 * jump over the leading white space in the current line,
 * IF the line does not begin a comment or is in a preprocessor definition.
 */
void ASFormatter::initNewLine()
{
	size_t len = currentLine.length();
	size_t tabSize = getTabLength();
	charNum = 0;

	// don't trim these
	if (isInQuoteContinuation
	        || (isInPreprocessor && !getPreprocDefineIndent()))
		return;

	// SQL continuation lines must be adjusted so the leading spaces
	// is equivalent to the opening EXEC SQL
	if (isInExecSQL)
	{
		// replace leading tabs with spaces
		// so that continuation indent will be spaces
		size_t tabCount_ = 0;
		size_t i;
		for (i = 0; i < currentLine.length(); i++)
		{
			if (!isWhiteSpace(currentLine[i]))		// stop at first text
				break;
			if (currentLine[i] == '\t')
			{
				size_t numSpaces = tabSize - ((tabCount_ + i) % tabSize);
				currentLine.replace(i, 1, numSpaces, ' ');
				tabCount_++;
				i += tabSize - 1;
			}
		}
		// this will correct the format if EXEC SQL is not a hanging indent
		trimContinuationLine();
		return;
	}

	// comment continuation lines must be adjusted so the leading spaces
	// is equivalent to the opening comment
	if (isInComment)
	{
		if (noTrimCommentContinuation)
			leadingSpaces = tabIncrementIn = 0;
		trimContinuationLine();
		return;
	}

	// compute leading spaces
	isImmediatelyPostCommentOnly = lineIsLineCommentOnly || lineEndsInCommentOnly;
	lineIsCommentOnly = false;
	lineIsLineCommentOnly = false;
	lineEndsInCommentOnly = false;
	doesLineStartComment = false;
	currentLineBeginsWithBracket = false;
	lineIsEmpty = false;
	currentLineFirstBracketNum = string::npos;
	tabIncrementIn = 0;

	// bypass whitespace at the start of a line
	// preprocessor tabs are replaced later in the program
	for (charNum = 0; isWhiteSpace(currentLine[charNum]) && charNum + 1 < (int) len; charNum++)
	{
		if (currentLine[charNum] == '\t' && !isInPreprocessor)
			tabIncrementIn += tabSize - 1 - ((tabIncrementIn + charNum) % tabSize);
	}
	leadingSpaces = charNum + tabIncrementIn;

	if (isSequenceReached("/*"))
	{
		doesLineStartComment = true;
		if ((int) currentLine.length() > charNum + 2
		        && currentLine.find("*/", charNum + 2) != string::npos)
			lineIsCommentOnly = true;
	}
	else if (isSequenceReached("//"))
	{
		lineIsLineCommentOnly = true;
	}
	else if (isSequenceReached("{"))
	{
		currentLineBeginsWithBracket = true;
		currentLineFirstBracketNum = charNum;
		size_t firstText = currentLine.find_first_not_of(" \t", charNum + 1);
		if (firstText != string::npos)
		{
			if (currentLine.compare(firstText, 2, "//") == 0)
				lineIsLineCommentOnly = true;
			else if (currentLine.compare(firstText, 2, "/*") == 0
			         || isExecSQL(currentLine, firstText))
			{
				// get the extra adjustment
				size_t j;
				for (j = charNum + 1; j < firstText && isWhiteSpace(currentLine[j]); j++)
				{
					if (currentLine[j] == '\t')
						tabIncrementIn += tabSize - 1 - ((tabIncrementIn + j) % tabSize);
				}
				leadingSpaces = j + tabIncrementIn;
				if (currentLine.compare(firstText, 2, "/*") == 0)
					doesLineStartComment = true;
			}
		}
	}
	else if (isWhiteSpace(currentLine[charNum]) && !(charNum + 1 < (int) currentLine.length()))
	{
		lineIsEmpty = true;
	}

	// do not trim indented preprocessor define (except for comment continuation lines)
	if (isInPreprocessor)
	{
		if (!doesLineStartComment)
			leadingSpaces = 0;
		charNum = 0;
	}
}

/**
 * Append a character to the current formatted line.
 * The formattedLine split points are updated.
 *
 * @param ch               the character to append.
 * @param canBreakLine     if true, a registered line-break
 */
void ASFormatter::appendChar(char ch, bool canBreakLine)
{
	if (canBreakLine && isInLineBreak)
		breakLine();

	formattedLine.append(1, ch);
	isImmediatelyPostCommentOnly = false;
	if (maxCodeLength != string::npos)
	{
		// These compares reduce the frequency of function calls.
		if (isOkToSplitFormattedLine())
			updateFormattedLineSplitPoints(ch);
		if (formattedLine.length() > maxCodeLength)
			testForTimeToSplitFormattedLine();
	}
}

/**
 * Append a string sequence to the current formatted line.
 * The formattedLine split points are NOT updated.
 * But the formattedLine is checked for time to split.
 *
 * @param sequence         the sequence to append.
 * @param canBreakLine     if true, a registered line-break
 */
void ASFormatter::appendSequence(const string &sequence, bool canBreakLine)
{
	if (canBreakLine && isInLineBreak)
		breakLine();
	formattedLine.append(sequence);
	if (formattedLine.length() > maxCodeLength)
		testForTimeToSplitFormattedLine();
}

/**
 * Append an operator sequence to the current formatted line.
 * The formattedLine split points are updated.
 *
 * @param sequence         the sequence to append.
 * @param canBreakLine     if true, a registered line-break
 */
void ASFormatter::appendOperator(const string &sequence, bool canBreakLine)
{
	if (canBreakLine && isInLineBreak)
		breakLine();
	formattedLine.append(sequence);
	if (maxCodeLength != string::npos)
	{
		// These compares reduce the frequency of function calls.
		if (isOkToSplitFormattedLine())
			updateFormattedLineSplitPointsOperator(sequence);
		if (formattedLine.length() > maxCodeLength)
			testForTimeToSplitFormattedLine();
	}
}

/**
 * append a space to the current formattedline, UNLESS the
 * last character is already a white-space character.
 */
void ASFormatter::appendSpacePad()
{
	int len = formattedLine.length();
	if (len > 0 && !isWhiteSpace(formattedLine[len - 1]))
	{
		formattedLine.append(1, ' ');
		spacePadNum++;
		if (maxCodeLength != string::npos)
		{
			// These compares reduce the frequency of function calls.
			if (isOkToSplitFormattedLine())
				updateFormattedLineSplitPoints(' ');
			if (formattedLine.length() > maxCodeLength)
				testForTimeToSplitFormattedLine();
		}
	}
}

/**
 * append a space to the current formattedline, UNLESS the
 * next character is already a white-space character.
 */
void ASFormatter::appendSpaceAfter()
{
	int len = currentLine.length();
	if (charNum + 1 < len && !isWhiteSpace(currentLine[charNum + 1]))
	{
		formattedLine.append(1, ' ');
		spacePadNum++;
		if (maxCodeLength != string::npos)
		{
			// These compares reduce the frequency of function calls.
			if (isOkToSplitFormattedLine())
				updateFormattedLineSplitPoints(' ');
			if (formattedLine.length() > maxCodeLength)
				testForTimeToSplitFormattedLine();
		}
	}
}

/**
 * register a line break for the formatted line.
 */
void ASFormatter::breakLine(bool isSplitLine /*false*/)
{
	isLineReady = true;
	isInLineBreak = false;
	spacePadNum = nextLineSpacePadNum;
	nextLineSpacePadNum = 0;
	readyFormattedLine = formattedLine;
	formattedLine.erase();
	// queue an empty line prepend request if one exists
	prependEmptyLine = isPrependPostBlockEmptyLineRequested;

	if (!isSplitLine)
	{
		formattedLineCommentNum = string::npos;
		clearFormattedLineSplitPoints();

		if (isAppendPostBlockEmptyLineRequested)
		{
			isAppendPostBlockEmptyLineRequested = false;
			isPrependPostBlockEmptyLineRequested = true;
		}
		else
			isPrependPostBlockEmptyLineRequested = false;
	}
}

/**
 * check if the currently reached open-bracket (i.e. '{')
 * opens a:
 * - a definition type block (such as a class or namespace),
 * - a command block (such as a method block)
 * - a static array
 * this method takes for granted that the current character
 * is an opening bracket.
 *
 * @return    the type of the opened block.
 */
BracketType ASFormatter::getBracketType()
{
	assert(currentChar == '{');

	BracketType returnVal;

	if ((previousNonWSChar == '='
	        || isBracketType(bracketTypeStack->back(), ARRAY_TYPE))
	        && previousCommandChar != ')')
		returnVal = ARRAY_TYPE;
	else if (foundPreDefinitionHeader && previousCommandChar != ')')
	{
		returnVal = DEFINITION_TYPE;
		if (foundNamespaceHeader)
			returnVal = (BracketType)(returnVal | NAMESPACE_TYPE);
		else if (foundClassHeader)
			returnVal = (BracketType)(returnVal | CLASS_TYPE);
		else if (foundStructHeader)
			returnVal = (BracketType)(returnVal | STRUCT_TYPE);
		else if (foundInterfaceHeader)
			returnVal = (BracketType)(returnVal | INTERFACE_TYPE);
	}
	else if (isInEnum)
	{
		returnVal = (BracketType)(ARRAY_TYPE | ENUM_TYPE);
	}
	else
	{
		bool isCommandType = (foundPreCommandHeader
		                      || foundPreCommandMacro
		                      || (currentHeader != NULL && isNonParenHeader)
		                      || (previousCommandChar == ')')
		                      || (previousCommandChar == ':' && !foundQuestionMark)
		                      || (previousCommandChar == ';')
		                      || ((previousCommandChar == '{' || previousCommandChar == '}')
		                          && isPreviousBracketBlockRelated)
		                      || (isInClassInitializer
		                          && (!isLegalNameChar(previousNonWSChar) || foundPreCommandHeader))
		                      || isInObjCMethodDefinition
		                      || isInObjCInterface
		                      || isJavaStaticConstructor
		                      || isSharpDelegate);

		// C# methods containing 'get', 'set', 'add', and 'remove' do NOT end with parens
		if (!isCommandType && isSharpStyle() && isNextWordSharpNonParenHeader(charNum + 1))
		{
			isCommandType = true;
			isSharpAccessor = true;
		}

		if (isInExternC)
			returnVal = (isCommandType ? COMMAND_TYPE : EXTERN_TYPE);
		else
			returnVal = (isCommandType ? COMMAND_TYPE : ARRAY_TYPE);
	}

	int foundOneLineBlock = isOneLineBlockReached(currentLine, charNum);
	// this assumes each array definition is on a single line
	// (foundOneLineBlock == 2) is a one line block followed by a comma
	if (foundOneLineBlock == 2 && returnVal == COMMAND_TYPE)
		returnVal = ARRAY_TYPE;

	if (foundOneLineBlock > 0)		// found one line block
		returnVal = (BracketType)(returnVal | SINGLE_LINE_TYPE);

	if (isBracketType(returnVal, ARRAY_TYPE))
	{
		if (isNonInStatementArrayBracket())
		{
			returnVal = (BracketType)(returnVal | ARRAY_NIS_TYPE);
			isNonInStatementArray = true;
			isImmediatelyPostNonInStmt = false;		// in case of "},{"
			nonInStatementBracket = formattedLine.length() - 1;
		}
		if (isUniformInitializerBracket())
			returnVal = (BracketType)(returnVal | INIT_TYPE);
	}

	return returnVal;
}

/**
* check if a colon is a class initializer separator
*
* @return        whether it is a class initializer separator
*/
bool ASFormatter::isClassInitializer() const
{
	assert(currentLine[charNum] == ':');
	assert(previousChar != ':' && peekNextChar() != ':');	// not part of '::'

	// this should be similar to ASBeautifier::parseCurrentLine()
	bool foundClassInitializer = false;

	if (foundQuestionMark)
	{
		// do nothing special
	}
	else if (parenStack->back() > 0)
	{
		// found a 'for' loop or an objective-C statement
		// so do nothing special
	}
	else if (isInEnum)
	{
		// found an enum with a base-type
	}
	else if (isCStyle()
	         && !isInCase
	         && (previousCommandChar == ')' || foundPreCommandHeader))
	{
		// found a 'class' c'tor initializer
		foundClassInitializer = true;
	}
	return foundClassInitializer;
}

/**
 * check if a line is empty
 *
 * @return        whether line is empty
 */
bool ASFormatter::isEmptyLine(const string &line) const
{
	return line.find_first_not_of(" \t") == string::npos;
}

/**
 * Check if the following text is "C" as in extern "C".
 *
 * @return        whether the statement is extern "C"
 */
bool ASFormatter::isExternC() const
{
	// charNum should be at 'extern'
	assert(!isWhiteSpace(currentLine[charNum]));
	size_t startQuote = currentLine.find_first_of(" \t\"", charNum);
	if (startQuote == string::npos)
		return false;
	startQuote = currentLine.find_first_not_of(" \t", startQuote);
	if (startQuote == string::npos)
		return false;
	if (currentLine.compare(startQuote, 3, "\"C\"") != 0)
		return false;
	return true;
}

/**
 * Check if the currently reached '*', '&' or '^' character is
 * a pointer-or-reference symbol, or another operator.
 * A pointer dereference (*) or an "address of" character (&)
 * counts as a pointer or reference because it is not an
 * arithmetic operator.
 *
 * @return        whether current character is a reference-or-pointer
 */
bool ASFormatter::isPointerOrReference() const
{
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');

	if (isJavaStyle())
		return false;

	if (isCharImmediatelyPostOperator)
		return false;

	// get the last legal word (may be a number)
	string lastWord = getPreviousWord(currentLine, charNum);
	if (lastWord.empty())
		lastWord = " ";

	// check for preceding or following numeric values
	string nextText = peekNextText(currentLine.substr(charNum + 1));
	if (nextText.length() == 0)
		nextText = " ";
	char nextChar = nextText[0];
	if (isDigit(lastWord[0])
	        || isDigit(nextChar)
	        || nextChar == '!'
	        || nextChar == '~')
		return false;

	// check for multiply then a dereference (a * *b)
	if (currentChar == '*'
	        && charNum < (int) currentLine.length() - 1
	        && isWhiteSpace(currentLine[charNum + 1])
	        && nextChar == '*')
		return false;

	if ((foundCastOperator && nextChar == '>')
	        || isPointerOrReferenceVariable(lastWord))
		return true;

	if (isInClassInitializer
	        && previousNonWSChar != '('
	        && previousNonWSChar != '{'
	        && previousCommandChar != ','
	        && nextChar != ')'
	        && nextChar != '}')
		return false;

	//check for rvalue reference
	if (currentChar == '&' && nextChar == '&')
	{
		string followingText = peekNextText(currentLine.substr(charNum + 2));
		if (followingText.length() > 0 && followingText[0] == ')')
			return true;
		if (currentHeader != NULL || isInPotentialCalculation)
			return false;
		if (parenStack->back() > 0 && isBracketType(bracketTypeStack->back(), COMMAND_TYPE))
			return false;
		return true;
	}
	if (nextChar == '*'
	        || previousNonWSChar == '='
	        || previousNonWSChar == '('
	        || previousNonWSChar == '['
	        || isCharImmediatelyPostReturn
	        || isInTemplate
	        || isCharImmediatelyPostTemplate
	        || currentHeader == &AS_CATCH
	        || currentHeader == &AS_FOREACH
	        || currentHeader == &AS_QFOREACH)
		return true;

	if (isBracketType(bracketTypeStack->back(), ARRAY_TYPE)
	        && isLegalNameChar(lastWord[0])
	        && isLegalNameChar(nextChar)
	        && previousNonWSChar != ')')
	{
		if (isArrayOperator())
			return false;
	}

	// checks on operators in parens
	if (parenStack->back() > 0
	        && isLegalNameChar(lastWord[0])
	        && isLegalNameChar(nextChar))
	{
		// if followed by an assignment it is a pointer or reference
		// if followed by semicolon it is a pointer or reference in range-based for
		const string* followingOperator = getFollowingOperator();
		if (followingOperator
		        && followingOperator != &AS_MULT
		        && followingOperator != &AS_BIT_AND)
		{
			if (followingOperator == &AS_ASSIGN || followingOperator == &AS_COLON)
				return true;
			else
				return false;
		}

		if (isBracketType(bracketTypeStack->back(), COMMAND_TYPE)
		        || squareBracketCount > 0)
			return false;
		else
			return true;
	}

	// checks on operators in parens with following '('
	if (parenStack->back() > 0
	        && nextChar == '('
	        && previousNonWSChar != ','
	        && previousNonWSChar != '('
	        && previousNonWSChar != '!'
	        && previousNonWSChar != '&'
	        && previousNonWSChar != '*'
	        && previousNonWSChar != '|')
		return false;

	if (nextChar == '-'
	        || nextChar == '+')
	{
		size_t nextNum = currentLine.find_first_not_of(" \t", charNum + 1);
		if (nextNum != string::npos)
		{
			if (currentLine.compare(nextNum, 2, "++") != 0
			        && currentLine.compare(nextNum, 2, "--") != 0)
				return false;
		}
	}

	bool isPR = (!isInPotentialCalculation
	             || (!isLegalNameChar(previousNonWSChar)
	                 && !(previousNonWSChar == ')' && nextChar == '(')
	                 && !(previousNonWSChar == ')' && currentChar == '*' && !isImmediatelyPostCast())
	                 && previousNonWSChar != ']')
	             || (!isWhiteSpace(nextChar)
	                 && nextChar != '-'
	                 && nextChar != '('
	                 && nextChar != '['
	                 && !isLegalNameChar(nextChar))
	            );

	return isPR;
}

/**
 * Check if the currently reached  '*' or '&' character is
 * a dereferenced pointer or "address of" symbol.
 * NOTE: this MUST be a pointer or reference as determined by
 * the function isPointerOrReference().
 *
 * @return        whether current character is a dereference or address of
 */
bool ASFormatter::isDereferenceOrAddressOf() const
{
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');

	if (isCharImmediatelyPostTemplate)
		return false;

	if (previousNonWSChar == '='
	        || previousNonWSChar == ','
	        || previousNonWSChar == '.'
	        || previousNonWSChar == '{'
	        || previousNonWSChar == '>'
	        || previousNonWSChar == '<'
	        || previousNonWSChar == '?'
	        || isCharImmediatelyPostLineComment
	        || isCharImmediatelyPostComment
	        || isCharImmediatelyPostReturn)
		return true;

	char nextChar = peekNextChar();
	if (currentChar == '*' && nextChar == '*')
	{
		if (previousNonWSChar == '(')
			return true;
		if ((int) currentLine.length() < charNum + 2)
			return true;
		return false;
	}
	if (currentChar == '&' && nextChar == '&')
	{
		if (previousNonWSChar == '(' || isInTemplate)
			return true;
		if ((int) currentLine.length() < charNum + 2)
			return true;
		return false;
	}

	// check first char on the line
	if (charNum == (int) currentLine.find_first_not_of(" \t")
	        && (isBracketType(bracketTypeStack->back(), COMMAND_TYPE)
	            || parenStack->back() != 0))
		return true;

	string nextText = peekNextText(currentLine.substr(charNum + 1));
	if (nextText.length() > 0)
	{
		if (nextText[0] == ')' || nextText[0] == '>'
		        || nextText[0] == ',' || nextText[0] == '=')
			return false;
		if (nextText[0] == ';')
			return true;
	}

	// check for reference to a pointer *& (cannot have &*)
	if ((currentChar == '*' && nextChar == '&')
	        || (previousNonWSChar == '*' && currentChar == '&'))
		return false;

	if (!isBracketType(bracketTypeStack->back(), COMMAND_TYPE)
	        && parenStack->back() == 0)
		return false;

	string lastWord = getPreviousWord(currentLine, charNum);
	if (lastWord == "else" || lastWord == "delete")
		return true;

	if (isPointerOrReferenceVariable(lastWord))
		return false;

	bool isDA = (!(isLegalNameChar(previousNonWSChar) || previousNonWSChar == '>')
	             || (nextText.length() > 0 && !isLegalNameChar(nextText[0]) && nextText[0] != '/')
	             || (ispunct((unsigned char)previousNonWSChar) && previousNonWSChar != '.')
	             || isCharImmediatelyPostReturn);

	return isDA;
}

/**
 * Check if the currently reached  '*' or '&' character is
 * centered with one space on each side.
 * Only spaces are checked, not tabs.
 * If true then a space will be deleted on the output.
 *
 * @return        whether current character is centered.
 */
bool ASFormatter::isPointerOrReferenceCentered() const
{
	assert(currentLine[charNum] == '*' || currentLine[charNum] == '&' || currentLine[charNum] == '^');

	int prNum = charNum;
	int lineLength = (int) currentLine.length();

	// check for end of line
	if (peekNextChar() == ' ')
		return false;

	// check space before
	if (prNum < 1
	        || currentLine[prNum - 1] != ' ')
		return false;

	// check no space before that
	if (prNum < 2
	        || currentLine[prNum - 2] == ' ')
		return false;

	// check for ** or &&
	if (prNum + 1 < lineLength
	        && (currentLine[prNum + 1] == '*' || currentLine[prNum + 1] == '&'))
		prNum++;

	// check space after
	if (prNum + 1 <= lineLength
	        && currentLine[prNum + 1] != ' ')
		return false;

	// check no space after that
	if (prNum + 2 < lineLength
	        && currentLine[prNum + 2] == ' ')
		return false;

	return true;
}

/**
 * Check if a word is a pointer or reference variable type.
 *
 * @return        whether word is a pointer or reference variable.
 */
bool ASFormatter::isPointerOrReferenceVariable(string &word) const
{
	if (word == "char"
	        || word == "int"
	        || word == "void"
	        || (word.length() >= 6     // check end of word for _t
	            && word.compare(word.length() - 2, 2, "_t") == 0)
	        || word == "INT"
	        || word == "VOID")
		return true;
	return false;
}

/**
 * check if the currently reached '+' or '-' character is a unary operator
 * this method takes for granted that the current character
 * is a '+' or '-'.
 *
 * @return        whether the current '+' or '-' is a unary operator.
 */
bool ASFormatter::isUnaryOperator() const
{
	assert(currentChar == '+' || currentChar == '-');

	return ((isCharImmediatelyPostReturn || !isLegalNameChar(previousCommandChar))
	        && previousCommandChar != '.'
	        && previousCommandChar != '\"'
	        && previousCommandChar != '\''
	        && previousCommandChar != ')'
	        && previousCommandChar != ']');
}

/**
 * check if the currently reached comment is in a 'switch' statement
 *
 * @return        whether the current '+' or '-' is in an exponent.
 */
bool ASFormatter::isInSwitchStatement() const
{
	assert(isInLineComment || isInComment);
	if (preBracketHeaderStack->size() > 0)
		for (size_t i = 1; i < preBracketHeaderStack->size(); i++)
			if (preBracketHeaderStack->at(i) == &AS_SWITCH)
				return true;
	return false;
}

/**
 * check if the currently reached '+' or '-' character is
 * part of an exponent, i.e. 0.2E-5.
 *
 * @return        whether the current '+' or '-' is in an exponent.
 */
bool ASFormatter::isInExponent() const
{
	assert(currentChar == '+' || currentChar == '-');

	int formattedLineLength = formattedLine.length();
	if (formattedLineLength >= 2)
	{
		char prevPrevFormattedChar = formattedLine[formattedLineLength - 2];
		char prevFormattedChar = formattedLine[formattedLineLength - 1];

		return ((prevFormattedChar == 'e' || prevFormattedChar == 'E')
		        && (prevPrevFormattedChar == '.' || isDigit(prevPrevFormattedChar)));
	}
	else
		return false;
}

/**
 * check if an array bracket should NOT have an in-statement indent
 *
 * @return        the array is non in-statement
 */
bool ASFormatter::isNonInStatementArrayBracket() const
{
	bool returnVal = false;
	char nextChar = peekNextChar();
	// if this opening bracket begins the line there will be no inStatement indent
	if (currentLineBeginsWithBracket
	        && charNum == (int) currentLineFirstBracketNum
	        && nextChar != '}')
		returnVal = true;
	// if an opening bracket ends the line there will be no inStatement indent
	if (isWhiteSpace(nextChar)
	        || isBeforeAnyLineEndComment(charNum)
	        || nextChar == '{')
		returnVal = true;

	// Java "new Type [] {...}" IS an inStatement indent
	if (isJavaStyle() && previousNonWSChar == ']')
		returnVal = false;

	return returnVal;
}

/**
 * check if a one-line bracket has been reached,
 * i.e. if the currently reached '{' character is closed
 * with a complimentary '}' elsewhere on the current line,
 *.
 * @return     0 = one-line bracket has not been reached.
 *             1 = one-line bracket has been reached.
 *             2 = one-line bracket has been reached and is followed by a comma.
 */
int ASFormatter::isOneLineBlockReached(string &line, int startChar) const
{
	assert(line[startChar] == '{');

	bool isInComment_ = false;
	bool isInQuote_ = false;
	int bracketCount = 1;
	int lineLength = line.length();
	char quoteChar_ = ' ';
	char ch = ' ';
	char prevCh = ' ';

	for (int i = startChar + 1; i < lineLength; ++i)
	{
		ch = line[i];

		if (isInComment_)
		{
			if (line.compare(i, 2, "*/") == 0)
			{
				isInComment_ = false;
				++i;
			}
			continue;
		}

		if (ch == '\\')
		{
			++i;
			continue;
		}

		if (isInQuote_)
		{
			if (ch == quoteChar_)
				isInQuote_ = false;
			continue;
		}

		if (ch == '"' || ch == '\'')
		{
			isInQuote_ = true;
			quoteChar_ = ch;
			continue;
		}

		if (line.compare(i, 2, "//") == 0)
			break;

		if (line.compare(i, 2, "/*") == 0)
		{
			isInComment_ = true;
			++i;
			continue;
		}

		if (ch == '{')
			++bracketCount;
		else if (ch == '}')
			--bracketCount;

		if (bracketCount == 0)
		{
			// is this an array?
			if (parenStack->back() == 0 && prevCh != '}')
			{
				size_t peekNum = line.find_first_not_of(" \t", i + 1);
				if (peekNum != string::npos && line[peekNum] == ',')
					return 2;
			}
			return 1;
		}
		if (!isWhiteSpace(ch))
			prevCh = ch;
	}

	return 0;
}

/**
 * peek at the next word to determine if it is a C# non-paren header.
 * will look ahead in the input file if necessary.
 *
 * @param  startChar      position on currentLine to start the search
 * @return                true if the next word is get or set.
 */
bool ASFormatter::isNextWordSharpNonParenHeader(int startChar) const
{
	// look ahead to find the next non-comment text
	string nextText = peekNextText(currentLine.substr(startChar));
	if (nextText.length() == 0)
		return false;
	if (nextText[0] == '[')
		return true;
	if (!isCharPotentialHeader(nextText, 0))
		return false;
	if (findKeyword(nextText, 0, AS_GET) || findKeyword(nextText, 0, AS_SET)
	        || findKeyword(nextText, 0, AS_ADD) || findKeyword(nextText, 0, AS_REMOVE))
		return true;
	return false;
}

/**
 * peek at the next char to determine if it is an opening bracket.
 * will look ahead in the input file if necessary.
 * this determines a java static constructor.
 *
 * @param startChar     position on currentLine to start the search
 * @return              true if the next word is an opening bracket.
 */
bool ASFormatter::isNextCharOpeningBracket(int startChar) const
{
	bool retVal = false;
	string nextText = peekNextText(currentLine.substr(startChar));
	if (nextText.length() > 0
	        && nextText.compare(0, 1, "{") == 0)
		retVal = true;
	return retVal;
}

/**
* Check if operator and, pointer, and reference padding is disabled.
* Disabling is done thru a NOPAD tag in an ending comment.
*
* @return              true if the formatting on this line is disabled.
*/
bool ASFormatter::isOperatorPaddingDisabled() const
{
	size_t commentStart = currentLine.find("//", charNum);
	if (commentStart == string::npos)
	{
		commentStart = currentLine.find("/*", charNum);
		// comment must end on this line
		if (commentStart != string::npos)
		{
			size_t commentEnd = currentLine.find("*/", commentStart + 2);
			if (commentEnd == string::npos)
				commentStart = string::npos;
		}
	}
	if (commentStart == string::npos)
		return false;
	size_t noPadStart = currentLine.find("*NOPAD*", commentStart);
	if (noPadStart == string::npos)
		return false;
	return true;
}

/**
* Determine if an opening array-type bracket should have a leading space pad.
* This is to identify C++11 uniform initializers.
*/
bool ASFormatter::isUniformInitializerBracket() const
{
	if (isCStyle() && !isInEnum && !isImmediatelyPostPreprocessor)
	{
		if (isInClassInitializer
		        || isLegalNameChar(previousNonWSChar))
			return true;
	}
	return false;
}

/**
 * get the next non-whitespace substring on following lines, bypassing all comments.
 *
 * @param   firstLine   the first line to check
 * @return  the next non-whitespace substring.
 */
string ASFormatter::peekNextText(const string &firstLine, bool endOnEmptyLine /*false*/, bool shouldReset /*false*/) const
{
	bool isFirstLine = true;
	bool needReset = shouldReset;
	string nextLine_ = firstLine;
	size_t firstChar = string::npos;

	// find the first non-blank text, bypassing all comments.
	bool isInComment_ = false;
	while (sourceIterator->hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
		{
			nextLine_ = sourceIterator->peekNextLine();
			needReset = true;
		}

		firstChar = nextLine_.find_first_not_of(" \t");
		if (firstChar == string::npos)
		{
			if (endOnEmptyLine && !isInComment_)
				break;
			continue;
		}

		if (nextLine_.compare(firstChar, 2, "/*") == 0)
		{
			firstChar += 2;
			isInComment_ = true;
		}

		if (isInComment_)
		{
			firstChar = nextLine_.find("*/", firstChar);
			if (firstChar == string::npos)
				continue;
			firstChar += 2;
			isInComment_ = false;
			firstChar = nextLine_.find_first_not_of(" \t", firstChar);
			if (firstChar == string::npos)
				continue;
		}

		if (nextLine_.compare(firstChar, 2, "//") == 0)
			continue;

		// found the next text
		break;
	}

	if (firstChar == string::npos)
		nextLine_ = "";
	else
		nextLine_ = nextLine_.substr(firstChar);
	if (needReset)
		sourceIterator->peekReset();
	return nextLine_;
}

/**
 * adjust comment position because of adding or deleting spaces
 * the spaces are added or deleted to formattedLine
 * spacePadNum contains the adjustment
 */
void ASFormatter::adjustComments(void)
{
	assert(spacePadNum != 0);
	assert(currentLine.compare(charNum, 2, "//") == 0
	       || currentLine.compare(charNum, 2, "/*") == 0);

	// block comment must be closed on this line with nothing after it
	if (currentLine.compare(charNum, 2, "/*") == 0)
	{
		size_t endNum = currentLine.find("*/", charNum + 2);
		if (endNum == string::npos)
			return;
		if (currentLine.find_first_not_of(" \t", endNum + 2) != string::npos)
			return;
	}

	size_t len = formattedLine.length();
	// don't adjust a tab
	if (formattedLine[len - 1] == '\t')
		return;
	// if spaces were removed, need to add spaces before the comment
	if (spacePadNum < 0)
	{
		int adjust = -spacePadNum;          // make the number positive
		formattedLine.append(adjust, ' ');
	}
	// if spaces were added, need to delete extra spaces before the comment
	// if cannot be done put the comment one space after the last text
	else if (spacePadNum > 0)
	{
		int adjust = spacePadNum;
		size_t lastText = formattedLine.find_last_not_of(' ');
		if (lastText != string::npos
		        && lastText < len - adjust - 1)
			formattedLine.resize(len - adjust);
		else if (len > lastText + 2)
			formattedLine.resize(lastText + 2);
		else if (len < lastText + 2)
			formattedLine.append(len - lastText, ' ');
	}
}

/**
 * append the current bracket inside the end of line comments
 * currentChar contains the bracket, it will be appended to formattedLine
 * formattedLineCommentNum is the comment location on formattedLine
 */
void ASFormatter::appendCharInsideComments(void)
{
	if (formattedLineCommentNum == string::npos)    // does the comment start on the previous line?
	{
		appendCurrentChar();                        // don't attach
		return;
	}
	assert(formattedLine.compare(formattedLineCommentNum, 2, "//") == 0
	       || formattedLine.compare(formattedLineCommentNum, 2, "/*") == 0);

	// find the previous non space char
	size_t end = formattedLineCommentNum;
	size_t beg = formattedLine.find_last_not_of(" \t", end - 1);
	if (beg == string::npos)
	{
		appendCurrentChar();                // don't attach
		return;
	}
	beg++;

	// insert the bracket
	if (end - beg < 3)                      // is there room to insert?
		formattedLine.insert(beg, 3 - end + beg, ' ');
	if (formattedLine[beg] == '\t')         // don't pad with a tab
		formattedLine.insert(beg, 1, ' ');
	formattedLine[beg + 1] = currentChar;
	testForTimeToSplitFormattedLine();

	if (isBeforeComment())
		breakLine();
	else if (isCharImmediatelyPostLineComment)
		shouldBreakLineAtNextChar = true;
	return;
}

/**
 * add or remove space padding to operators
 * the operators and necessary padding will be appended to formattedLine
 * the calling function should have a continue statement after calling this method
 *
 * @param newOperator     the operator to be padded
 */
void ASFormatter::padOperators(const string* newOperator)
{
	assert(shouldPadOperators);
	assert(newOperator != NULL);

	bool shouldPad = (newOperator != &AS_SCOPE_RESOLUTION
	                  && newOperator != &AS_PLUS_PLUS
	                  && newOperator != &AS_MINUS_MINUS
	                  && newOperator != &AS_NOT
	                  && newOperator != &AS_BIT_NOT
	                  && newOperator != &AS_ARROW
	                  && !(newOperator == &AS_COLON && !foundQuestionMark			// objC methods
	                       && (isInObjCMethodDefinition || isInObjCInterface
	                           || isInObjCSelector || squareBracketCount))
	                  && !(newOperator == &AS_MINUS && isInExponent())
	                  && !((newOperator == &AS_PLUS || newOperator == &AS_MINUS)	// check for unary plus or minus
	                       && (previousNonWSChar == '('
	                           || previousNonWSChar == '['
	                           || previousNonWSChar == '='
	                           || previousNonWSChar == ','))
	                  && !(newOperator == &AS_PLUS && isInExponent())
	                  && !isCharImmediatelyPostOperator
//?                   // commented out in release 2.05.1 - doesn't seem to do anything???
//x                   && !((newOperator == &AS_MULT || newOperator == &AS_BIT_AND || newOperator == &AS_AND)
//x                        && isPointerOrReference())
	                  && !(newOperator == &AS_MULT
	                       && (previousNonWSChar == '.'
	                           || previousNonWSChar == '>'))    // check for ->
	                  && !((isInTemplate || isImmediatelyPostTemplate)
	                       && (newOperator == &AS_LS || newOperator == &AS_GR))
	                  && !(newOperator == &AS_GCC_MIN_ASSIGN
	                       && ASBase::peekNextChar(currentLine, charNum + 1) == '>')
	                  && !(newOperator == &AS_GR && previousNonWSChar == '?')
	                  && !(newOperator == &AS_QUESTION			// check for Java wildcard
	                       && (previousNonWSChar == '<'
	                           || ASBase::peekNextChar(currentLine, charNum) == '>'
	                           || ASBase::peekNextChar(currentLine, charNum) == '.'))
	                  && !isInCase
	                  && !isInAsm
	                  && !isInAsmOneLine
	                  && !isInAsmBlock
	                 );

	// pad before operator
	if (shouldPad
	        && !(newOperator == &AS_COLON
	             && (!foundQuestionMark && !isInEnum) && currentHeader != &AS_FOR)
	        && !(newOperator == &AS_QUESTION && isSharpStyle() // check for C# nullable type (e.g. int?)
	             && currentLine.find(':', charNum + 1) == string::npos)
	   )
		appendSpacePad();
	appendOperator(*newOperator);
	goForward(newOperator->length() - 1);

	currentChar = (*newOperator)[newOperator->length() - 1];
	// pad after operator
	// but do not pad after a '-' that is a unary-minus.
	if (shouldPad
	        && !isBeforeAnyComment()
	        && !(newOperator == &AS_PLUS && isUnaryOperator())
	        && !(newOperator == &AS_MINUS && isUnaryOperator())
	        && !(currentLine.compare(charNum + 1, 1, AS_SEMICOLON) == 0)
	        && !(currentLine.compare(charNum + 1, 2, AS_SCOPE_RESOLUTION) == 0)
	        && !(peekNextChar() == ',')
	        && !(newOperator == &AS_QUESTION && isSharpStyle() // check for C# nullable type (e.g. int?)
	             && peekNextChar() == '[')
	   )
		appendSpaceAfter();

	previousOperator = newOperator;
	return;
}

/**
 * format pointer or reference
 * currentChar contains the pointer or reference
 * the symbol and necessary padding will be appended to formattedLine
 * the calling function should have a continue statement after calling this method
 *
 * NOTE: Do NOT use appendCurrentChar() in this method. The line should not be
 *       broken once the calculation starts.
 */
void ASFormatter::formatPointerOrReference(void)
{
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');
	assert(!isJavaStyle());

	int pa = pointerAlignment;
	int ra = referenceAlignment;
	int itemAlignment = (currentChar == '*' || currentChar == '^') ? pa : ((ra == REF_SAME_AS_PTR) ? pa : ra);

	// check for ** and &&
	char peekedChar = peekNextChar();
	if ((currentChar == '*' && peekedChar == '*')
	        || (currentChar == '&' && peekedChar == '&'))
	{
		size_t nextChar = currentLine.find_first_not_of(" \t", charNum + 2);
		if (nextChar == string::npos)
			peekedChar = ' ';
		else
			peekedChar = currentLine[nextChar];
	}
	// check for cast
	if (peekedChar == ')' || peekedChar == '>' || peekedChar == ',')
	{
		formatPointerOrReferenceCast();
		return;
	}

	// check for a padded space and remove it
	if (charNum > 0
	        && !isWhiteSpace(currentLine[charNum - 1])
	        && formattedLine.length() > 0
	        && isWhiteSpace(formattedLine[formattedLine.length() - 1]))
	{
		formattedLine.erase(formattedLine.length() - 1);
		spacePadNum--;
	}

	if (itemAlignment == PTR_ALIGN_TYPE)
	{
		formatPointerOrReferenceToType();
	}
	else if (itemAlignment == PTR_ALIGN_MIDDLE)
	{
		formatPointerOrReferenceToMiddle();
	}
	else if (itemAlignment == PTR_ALIGN_NAME)
	{
		formatPointerOrReferenceToName();
	}
	else	// pointerAlignment == PTR_ALIGN_NONE
	{
		formattedLine.append(1, currentChar);
	}
}

/**
 * format pointer or reference with align to type
 */
void ASFormatter::formatPointerOrReferenceToType()
{
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');
	assert(!isJavaStyle());

	// do this before bumping charNum
	bool isOldPRCentered = isPointerOrReferenceCentered();

	size_t prevCh = formattedLine.find_last_not_of(" \t");
	if (prevCh == string::npos)
		prevCh = 0;
	if (formattedLine.length() == 0 || prevCh == formattedLine.length() - 1)
		formattedLine.append(1, currentChar);
	else
	{
		// exchange * or & with character following the type
		// this may not work every time with a tab character
		string charSave = formattedLine.substr(prevCh + 1, 1);
		formattedLine[prevCh + 1] = currentChar;
		formattedLine.append(charSave);
	}
	if (isSequenceReached("**") || isSequenceReached("&&"))
	{
		if (formattedLine.length() == 1)
			formattedLine.append(1, currentChar);
		else
			formattedLine.insert(prevCh + 2, 1, currentChar);
		goForward(1);
	}
	// if no space after then add one
	if (charNum < (int) currentLine.length() - 1
	        && !isWhiteSpace(currentLine[charNum + 1])
	        && currentLine[charNum + 1] != ')')
		appendSpacePad();
	// if old pointer or reference is centered, remove a space
	if (isOldPRCentered
	        && isWhiteSpace(formattedLine[formattedLine.length() - 1]))
	{
		formattedLine.erase(formattedLine.length() - 1, 1);
		spacePadNum--;
	}
	// update the formattedLine split point
	if (maxCodeLength != string::npos)
	{
		size_t index = formattedLine.length() - 1;
		if (isWhiteSpace(formattedLine[index]))
		{
			updateFormattedLineSplitPointsPointerOrReference(index);
			testForTimeToSplitFormattedLine();
		}
	}
}

/**
 * format pointer or reference with align in the middle
 */
void ASFormatter::formatPointerOrReferenceToMiddle()
{
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');
	assert(!isJavaStyle());

	// compute current whitespace before
	size_t wsBefore = currentLine.find_last_not_of(" \t", charNum - 1);
	if (wsBefore == string::npos)
		wsBefore = 0;
	else
		wsBefore = charNum - wsBefore - 1;
	string sequenceToInsert(1, currentChar);
	if (isSequenceReached("**"))
	{
		sequenceToInsert = "**";
		goForward(1);
	}
	else if (isSequenceReached("&&"))
	{
		sequenceToInsert = "&&";
		goForward(1);
	}
	// if reference to a pointer check for conflicting alignment
	else if (currentChar == '*' && peekNextChar() == '&'
	         && (referenceAlignment == REF_ALIGN_TYPE
	             || referenceAlignment == REF_ALIGN_MIDDLE
	             || referenceAlignment == REF_SAME_AS_PTR))
	{
		sequenceToInsert = "*&";
		goForward(1);
		for (size_t i = charNum; i < currentLine.length() - 1 && isWhiteSpace(currentLine[i]); i++)
			goForward(1);
	}
	// if a comment follows don't align, just space pad
	if (isBeforeAnyComment())
	{
		appendSpacePad();
		formattedLine.append(sequenceToInsert);
		appendSpaceAfter();
		return;
	}
	// do this before goForward()
	bool isAfterScopeResolution = previousNonWSChar == ':';
	size_t charNumSave = charNum;
	// if this is the last thing on the line
	if (currentLine.find_first_not_of(" \t", charNum + 1) == string::npos)
	{
		if (wsBefore == 0 && !isAfterScopeResolution)
			formattedLine.append(1, ' ');
		formattedLine.append(sequenceToInsert);
		return;
	}
	// goForward() to convert tabs to spaces, if necessary,
	// and move following characters to preceding characters
	// this may not work every time with tab characters
	for (size_t i = charNum + 1; i < currentLine.length() && isWhiteSpace(currentLine[i]); i++)
	{
		goForward(1);
		if (formattedLine.length() > 0)
			formattedLine.append(1, currentLine[i]);
		else
			spacePadNum--;
	}
	// find space padding after
	size_t wsAfter = currentLine.find_first_not_of(" \t", charNumSave + 1);
	if (wsAfter == string::npos || isBeforeAnyComment())
		wsAfter = 0;
	else
		wsAfter = wsAfter - charNumSave - 1;
	// don't pad before scope resolution operator, but pad after
	if (isAfterScopeResolution)
	{
		size_t lastText = formattedLine.find_last_not_of(" \t");
		formattedLine.insert(lastText + 1, sequenceToInsert);
		appendSpacePad();
	}
	else if (formattedLine.length() > 0)
	{
		// whitespace should be at least 2 chars to center
		if (wsBefore + wsAfter < 2)
		{
			size_t charsToAppend = (2 - (wsBefore + wsAfter));
			formattedLine.append(charsToAppend, ' ');
			spacePadNum += charsToAppend;
			if (wsBefore == 0) wsBefore++;
			if (wsAfter == 0) wsAfter++;
		}
		// insert the pointer or reference char
		size_t padAfter = (wsBefore + wsAfter) / 2;
		size_t index = formattedLine.length() - padAfter;
		formattedLine.insert(index, sequenceToInsert);
	}
	else	// formattedLine.length() == 0
	{
		formattedLine.append(sequenceToInsert);
		if (wsAfter == 0)
			wsAfter++;
		formattedLine.append(wsAfter, ' ');
		spacePadNum += wsAfter;
	}
	// update the formattedLine split point after the pointer
	if (maxCodeLength != string::npos && formattedLine.length() > 0)
	{
		size_t index = formattedLine.find_last_not_of(" \t");
		if (index != string::npos && (index < formattedLine.length() - 1))
		{
			index++;
			updateFormattedLineSplitPointsPointerOrReference(index);
			testForTimeToSplitFormattedLine();
		}
	}
}

/**
 * format pointer or reference with align to name
 */
void ASFormatter::formatPointerOrReferenceToName()
{
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');
	assert(!isJavaStyle());

	// do this before bumping charNum
	bool isOldPRCentered = isPointerOrReferenceCentered();

	size_t startNum = formattedLine.find_last_not_of(" \t");
	if (startNum == string::npos)
		startNum = 0;
	string sequenceToInsert(1, currentChar);
	if (isSequenceReached("**"))
	{
		sequenceToInsert = "**";
		goForward(1);
	}
	else if (isSequenceReached("&&"))
	{
		sequenceToInsert = "&&";
		goForward(1);
	}
	// if reference to a pointer align both to name
	else if (currentChar == '*' && peekNextChar() == '&')
	{
		sequenceToInsert = "*&";
		goForward(1);
		for (size_t i = charNum; i < currentLine.length() - 1 && isWhiteSpace(currentLine[i]); i++)
			goForward(1);
	}
	char peekedChar = peekNextChar();
	bool isAfterScopeResolution = previousNonWSChar == ':';		// check for ::
	// if this is not the last thing on the line
	if (!isBeforeAnyComment()
	        && (int) currentLine.find_first_not_of(" \t", charNum + 1) > charNum)
	{
		// goForward() to convert tabs to spaces, if necessary,
		// and move following characters to preceding characters
		// this may not work every time with tab characters
		for (size_t i = charNum + 1; i < currentLine.length() && isWhiteSpace(currentLine[i]); i++)
		{
			// if a padded paren follows don't move
			if (shouldPadParensOutside && peekedChar == '(' && !isOldPRCentered)
			{
				// empty parens don't count
				size_t start = currentLine.find_first_not_of("( \t", charNum + 1);
				if (start != string::npos && currentLine[start] != ')')
					break;
			}
			goForward(1);
			if (formattedLine.length() > 0)
				formattedLine.append(1, currentLine[i]);
			else
				spacePadNum--;
		}
	}
	// don't pad before scope resolution operator
	if (isAfterScopeResolution)
	{
		size_t lastText = formattedLine.find_last_not_of(" \t");
		if (lastText != string::npos && lastText + 1 < formattedLine.length())
			formattedLine.erase(lastText + 1);
	}
	// if no space before * then add one
	else if (formattedLine.length() > 0
	         && (formattedLine.length() <= startNum + 1
	             || !isWhiteSpace(formattedLine[startNum + 1])))
	{
		formattedLine.insert(startNum + 1, 1, ' ');
		spacePadNum++;
	}
	appendSequence(sequenceToInsert, false);
	// if old pointer or reference is centered, remove a space
	if (isOldPRCentered
	        && formattedLine.length() > startNum + 1
	        && isWhiteSpace(formattedLine[startNum + 1])
	        && !isBeforeAnyComment())
	{
		formattedLine.erase(startNum + 1, 1);
		spacePadNum--;
	}
	// don't convert to *= or &=
	if (peekedChar == '=')
	{
		appendSpaceAfter();
		// if more than one space before, delete one
		if (formattedLine.length() > startNum
		        && isWhiteSpace(formattedLine[startNum + 1])
		        && isWhiteSpace(formattedLine[startNum + 2]))
		{
			formattedLine.erase(startNum + 1, 1);
			spacePadNum--;
		}
	}
	// update the formattedLine split point
	if (maxCodeLength != string::npos)
	{
		size_t index = formattedLine.find_last_of(" \t");
		if (index != string::npos
		        && index < formattedLine.length() - 1
		        && (formattedLine[index + 1] == '*'
		            || formattedLine[index + 1] == '&'
		            || formattedLine[index + 1] == '^'))
		{
			updateFormattedLineSplitPointsPointerOrReference(index);
			testForTimeToSplitFormattedLine();
		}
	}
}

/**
 * format pointer or reference cast
 * currentChar contains the pointer or reference
 * NOTE: the pointers and references in function definitions
 *       are processed as a cast (e.g. void foo(void*, void*))
 *       is processed here.
 */
void ASFormatter::formatPointerOrReferenceCast(void)
{
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');
	assert(!isJavaStyle());

	int pa = pointerAlignment;
	int ra = referenceAlignment;
	int itemAlignment = (currentChar == '*' || currentChar == '^') ? pa : ((ra == REF_SAME_AS_PTR) ? pa : ra);

	string sequenceToInsert(1, currentChar);
	if (isSequenceReached("**") || isSequenceReached("&&"))
	{
		goForward(1);
		sequenceToInsert.append(1, currentLine[charNum]);
	}
	if (itemAlignment == PTR_ALIGN_NONE)
	{
		appendSequence(sequenceToInsert, false);
		return;
	}
	// remove preceding whitespace
	char prevCh = ' ';
	size_t prevNum = formattedLine.find_last_not_of(" \t");
	if (prevNum != string::npos)
	{
		prevCh = formattedLine[prevNum];
		if (prevNum + 1 < formattedLine.length()
		        && isWhiteSpace(formattedLine[prevNum + 1])
		        && prevCh != '(')
		{
			spacePadNum -= (formattedLine.length() - 1 - prevNum);
			formattedLine.erase(prevNum + 1);
		}
	}
	bool isAfterScopeResolution = previousNonWSChar == ':';
	if ((itemAlignment == PTR_ALIGN_MIDDLE || itemAlignment == PTR_ALIGN_NAME)
	        && !isAfterScopeResolution && prevCh != '(')
	{
		appendSpacePad();
		// in this case appendSpacePad may or may not update the split point
		if (maxCodeLength != string::npos && formattedLine.length() > 0)
			updateFormattedLineSplitPointsPointerOrReference(formattedLine.length() - 1);
		appendSequence(sequenceToInsert, false);
	}
	else
		appendSequence(sequenceToInsert, false);
	// remove trailing whitespace if comma follows
	char nextChar = peekNextChar();
	if (nextChar == ',')
	{
		while (isWhiteSpace(currentLine[charNum + 1]))
		{
			goForward(1);
			spacePadNum--;
		}
	}
}

/**
 * add or remove space padding to parens
 * currentChar contains the paren
 * the parens and necessary padding will be appended to formattedLine
 * the calling function should have a continue statement after calling this method
 */
void ASFormatter::padParens(void)
{
	assert(shouldPadParensOutside || shouldPadParensInside || shouldUnPadParens || shouldPadFirstParen);
	assert(currentChar == '(' || currentChar == ')');

	int spacesOutsideToDelete = 0;
	int spacesInsideToDelete = 0;

	if (currentChar == '(')
	{
		spacesOutsideToDelete = formattedLine.length() - 1;
		spacesInsideToDelete = 0;

		// compute spaces outside the opening paren to delete
		if (shouldUnPadParens)
		{
			char lastChar = ' ';
			bool prevIsParenHeader = false;
			size_t i = formattedLine.find_last_not_of(" \t");
			if (i != string::npos)
			{
				// if last char is a bracket the previous whitespace is an indent
				if (formattedLine[i] == '{')
					spacesOutsideToDelete = 0;
				else if (isCharImmediatelyPostPointerOrReference)
					spacesOutsideToDelete = 0;
				else
				{
					spacesOutsideToDelete -= i;
					lastChar = formattedLine[i];
					// if previous word is a header, it will be a paren header
					string prevWord = getPreviousWord(formattedLine, formattedLine.length());
					const string* prevWordH = NULL;
					if (shouldPadHeader
					        && prevWord.length() > 0
					        && isCharPotentialHeader(prevWord, 0))
						prevWordH = ASBeautifier::findHeader(prevWord, 0, headers);
					if (prevWordH != NULL)
						prevIsParenHeader = true;
					else if (prevWord == "return")  // don't unpad
						prevIsParenHeader = true;
					else if (isCStyle() && prevWord == "throw" && shouldPadHeader) // don't unpad
						prevIsParenHeader = true;
					else if (prevWord == "and" || prevWord == "or")  // don't unpad
						prevIsParenHeader = true;
					// don't unpad variables
					else if (prevWord == "bool"
					         || prevWord == "int"
					         || prevWord == "void"
					         || prevWord == "void*"
					         || prevWord == "char"
					         || prevWord == "long"
					         || prevWord == "double"
					         || prevWord == "float"
					         || (prevWord.length() >= 4     // check end of word for _t
					             && prevWord.compare(prevWord.length() - 2, 2, "_t") == 0)
					         || prevWord == "Int32"
					         || prevWord == "UInt32"
					         || prevWord == "Int64"
					         || prevWord == "UInt64"
					         || prevWord == "BOOL"
					         || prevWord == "DWORD"
					         || prevWord == "HWND"
					         || prevWord == "INT"
					         || prevWord == "LPSTR"
					         || prevWord == "VOID"
					         || prevWord == "LPVOID"
					        )
					{
						prevIsParenHeader = true;
					}
				}
			}
			// do not unpad operators, but leave them if already padded
			if (shouldPadParensOutside || prevIsParenHeader)
				spacesOutsideToDelete--;
			else if (lastChar == '|'          // check for ||
			         || lastChar == '&'       // check for &&
			         || lastChar == ','
			         || (lastChar == '(' && shouldPadParensInside)
			         || (lastChar == '>' && !foundCastOperator)
			         || lastChar == '<'
			         || lastChar == '?'
			         || lastChar == ':'
			         || lastChar == ';'
			         || lastChar == '='
			         || lastChar == '+'
			         || lastChar == '-'
			         || lastChar == '*'
			         || lastChar == '/'
			         || lastChar == '%'
			         || lastChar == '^'
			        )
				spacesOutsideToDelete--;

			if (spacesOutsideToDelete > 0)
			{
				formattedLine.erase(i + 1, spacesOutsideToDelete);
				spacePadNum -= spacesOutsideToDelete;
			}
		}

		// pad open paren outside
		char peekedCharOutside = peekNextChar();
		if (shouldPadFirstParen && previousChar != '(' && peekedCharOutside != ')')
			appendSpacePad();
		else if (shouldPadParensOutside)
		{
			if (!(currentChar == '(' && peekedCharOutside == ')'))
				appendSpacePad();
		}

		appendCurrentChar();

		// unpad open paren inside
		if (shouldUnPadParens)
		{
			size_t j = currentLine.find_first_not_of(" \t", charNum + 1);
			if (j != string::npos)
				spacesInsideToDelete = j - charNum - 1;
			if (shouldPadParensInside)
				spacesInsideToDelete--;
			if (spacesInsideToDelete > 0)
			{
				currentLine.erase(charNum + 1, spacesInsideToDelete);
				spacePadNum -= spacesInsideToDelete;
			}
			// convert tab to space if requested
			if (shouldConvertTabs
			        && (int) currentLine.length() > charNum + 1
			        && currentLine[charNum + 1] == '\t')
				currentLine[charNum + 1] = ' ';
		}

		// pad open paren inside
		char peekedCharInside = peekNextChar();
		if (shouldPadParensInside)
			if (!(currentChar == '(' && peekedCharInside == ')'))
				appendSpaceAfter();
	}
	else if (currentChar == ')')
	{
		// unpad close paren inside
		if (shouldUnPadParens)
		{
			spacesInsideToDelete = formattedLine.length();
			size_t i = formattedLine.find_last_not_of(" \t");
			if (i != string::npos)
				spacesInsideToDelete = formattedLine.length() - 1 - i;
			if (shouldPadParensInside)
				spacesInsideToDelete--;
			if (spacesInsideToDelete > 0)
			{
				formattedLine.erase(i + 1, spacesInsideToDelete);
				spacePadNum -= spacesInsideToDelete;
			}
		}

		// pad close paren inside
		if (shouldPadParensInside)
			if (!(previousChar == '(' && currentChar == ')'))
				appendSpacePad();

		appendCurrentChar();

		// unpad close paren outside
		// close parens outside are left unchanged
		if (shouldUnPadParens)
		{
			//spacesOutsideToDelete = 0;
			//size_t j = currentLine.find_first_not_of(" \t", charNum + 1);
			//if (j != string::npos)
			//	spacesOutsideToDelete = j - charNum - 1;
			//if (shouldPadParensOutside)
			//	spacesOutsideToDelete--;

			//if (spacesOutsideToDelete > 0)
			//{
			//	currentLine.erase(charNum + 1, spacesOutsideToDelete);
			//	spacePadNum -= spacesOutsideToDelete;
			//}
		}

		// pad close paren outside
		char peekedCharOutside = peekNextChar();
		if (shouldPadParensOutside)
			if (peekedCharOutside != ';'
			        && peekedCharOutside != ','
			        && peekedCharOutside != '.'
			        && peekedCharOutside != '+'    // check for ++
			        && peekedCharOutside != '-'    // check for --
			        && peekedCharOutside != ']')
				appendSpaceAfter();
	}
	return;
}

/**
 * format opening bracket as attached or broken
 * currentChar contains the bracket
 * the brackets will be appended to the current formattedLine or a new formattedLine as necessary
 * the calling function should have a continue statement after calling this method
 *
 * @param bracketType    the type of bracket to be formatted.
 */
void ASFormatter::formatOpeningBracket(BracketType bracketType)
{
	assert(!isBracketType(bracketType, ARRAY_TYPE));
	assert(currentChar == '{');

	parenStack->push_back(0);

	bool breakBracket = isCurrentBracketBroken();

	if (breakBracket)
	{
		if (isBeforeAnyComment() && isOkToBreakBlock(bracketType))
		{
			// if comment is at line end leave the comment on this line
			if (isBeforeAnyLineEndComment(charNum) && !currentLineBeginsWithBracket)
			{
				currentChar = ' ';              // remove bracket from current line
				if (parenStack->size() > 1)
					parenStack->pop_back();
				currentLine[charNum] = currentChar;
				appendOpeningBracket = true;    // append bracket to following line
			}
			// else put comment after the bracket
			else if (!isBeforeMultipleLineEndComments(charNum))
				breakLine();
		}
		else if (!isBracketType(bracketType, SINGLE_LINE_TYPE))
			breakLine();
		else if (shouldBreakOneLineBlocks && peekNextChar() != '}')
			breakLine();
		else if (!isInLineBreak)
			appendSpacePad();

		appendCurrentChar();

		// should a following comment break from the bracket?
		// must break the line AFTER the bracket
		if (isBeforeComment()
		        && formattedLine.length() > 0
		        && formattedLine[0] == '{'
		        && isOkToBreakBlock(bracketType)
		        && (bracketFormatMode == BREAK_MODE
		            || bracketFormatMode == LINUX_MODE
		            || bracketFormatMode == STROUSTRUP_MODE))
		{
			shouldBreakLineAtNextChar = true;
		}
	}
	else    // attach bracket
	{
		// are there comments before the bracket?
		if (isCharImmediatelyPostComment || isCharImmediatelyPostLineComment)
		{
			if (isOkToBreakBlock(bracketType)
			        && !(isCharImmediatelyPostComment && isCharImmediatelyPostLineComment)	// don't attach if two comments on the line
			        && !isImmediatelyPostPreprocessor
//			        && peekNextChar() != '}'		// don't attach { }		// removed release 2.03
			        && previousCommandChar != '{'	// don't attach { {
			        && previousCommandChar != '}'	// don't attach } {
			        && previousCommandChar != ';')	// don't attach ; {
			{
				appendCharInsideComments();
			}
			else
			{
				appendCurrentChar();				// don't attach
			}
		}
		else if (previousCommandChar == '{'
		         || (previousCommandChar == '}' && !isInClassInitializer)
		         || previousCommandChar == ';')		// '}' , ';' chars added for proper handling of '{' immediately after a '}' or ';'
		{
			appendCurrentChar();					// don't attach
		}
		else
		{
			// if a blank line precedes this don't attach
			if (isEmptyLine(formattedLine))
				appendCurrentChar();				// don't attach
			else if (isOkToBreakBlock(bracketType)
			         && !(isImmediatelyPostPreprocessor
			              && currentLineBeginsWithBracket))
			{
				if (peekNextChar() != '}')
				{
					appendSpacePad();
					appendCurrentChar(false);				// OK to attach
					testForTimeToSplitFormattedLine();		// line length will have changed
					// should a following comment attach with the bracket?
					// insert spaces to reposition the comment
					if (isBeforeComment()
					        && !isBeforeMultipleLineEndComments(charNum)
					        && (!isBeforeAnyLineEndComment(charNum)	|| currentLineBeginsWithBracket))
					{
						shouldBreakLineAtNextChar = true;
						currentLine.insert(charNum + 1, charNum + 1, ' ');
					}
					else if (!isBeforeAnyComment())		// added in release 2.03
					{
						shouldBreakLineAtNextChar = true;
					}
				}
				else
				{
					if (currentLineBeginsWithBracket && charNum == (int) currentLineFirstBracketNum)
					{
						appendSpacePad();
						appendCurrentChar(false);		// attach
						shouldBreakLineAtNextChar = true;
					}
					else
					{
						appendSpacePad();
						appendCurrentChar();		// don't attach
					}
				}
			}
			else
			{
				if (!isInLineBreak)
					appendSpacePad();
				appendCurrentChar();				// don't attach
			}
		}
	}
}

/**
 * format closing bracket
 * currentChar contains the bracket
 * the calling function should have a continue statement after calling this method
 *
 * @param bracketType    the type of the opening bracket for this closing bracket.
 */
void ASFormatter::formatClosingBracket(BracketType bracketType)
{
	assert(!isBracketType(bracketType, ARRAY_TYPE));
	assert(currentChar == '}');

	// parenStack must contain one entry
	if (parenStack->size() > 1)
		parenStack->pop_back();

	// mark state of immediately after empty block
	// this state will be used for locating brackets that appear immediately AFTER an empty block (e.g. '{} \n}').
	if (previousCommandChar == '{')
		isImmediatelyPostEmptyBlock = true;

	if (attachClosingBracketMode)
	{
		// for now, namespaces and classes will be attached. Uncomment the lines below to break.
		if ((isEmptyLine(formattedLine)			// if a blank line precedes this
		        || isCharImmediatelyPostLineComment
		        || isCharImmediatelyPostComment
		        || (isImmediatelyPostPreprocessor && (int) currentLine.find_first_not_of(" \t") == charNum)
//		        || (isBracketType(bracketType, CLASS_TYPE) && isOkToBreakBlock(bracketType) && previousNonWSChar != '{')
//		        || (isBracketType(bracketType, NAMESPACE_TYPE) && isOkToBreakBlock(bracketType) && previousNonWSChar != '{')
		    )
		        && (!isBracketType(bracketType, SINGLE_LINE_TYPE) || isOkToBreakBlock(bracketType)))
		{
			breakLine();
			appendCurrentChar();				// don't attach
		}
		else
		{
			if (previousNonWSChar != '{'
			        && (!isBracketType(bracketType, SINGLE_LINE_TYPE) || isOkToBreakBlock(bracketType)))
				appendSpacePad();
			appendCurrentChar(false);			// attach
		}
	}
	else if ((!(previousCommandChar == '{' && isPreviousBracketBlockRelated))	// this '}' does not close an empty block
	         && isOkToBreakBlock(bracketType))									// astyle is allowed to break one line blocks
	{
		breakLine();
		appendCurrentChar();
	}
	else
	{
		appendCurrentChar();
	}

	// if a declaration follows a definition, space pad
	if (isLegalNameChar(peekNextChar()))
		appendSpaceAfter();

	if (shouldBreakBlocks
	        && currentHeader != NULL
	        && !isHeaderInMultiStatementLine
	        && parenStack->back() == 0)
	{
		if (currentHeader == &AS_CASE || currentHeader == &AS_DEFAULT)
		{
			// do not yet insert a line if "break" statement is outside the brackets
			string nextText = peekNextText(currentLine.substr(charNum + 1));
			if (nextText.length() > 0
			        && nextText.substr(0, 5) != "break")
				isAppendPostBlockEmptyLineRequested = true;
		}
		else
			isAppendPostBlockEmptyLineRequested = true;
	}
}

/**
 * format array brackets as attached or broken
 * determine if the brackets can have an inStatement indent
 * currentChar contains the bracket
 * the brackets will be appended to the current formattedLine or a new formattedLine as necessary
 * the calling function should have a continue statement after calling this method
 *
 * @param bracketType            the type of bracket to be formatted, must be an ARRAY_TYPE.
 * @param isOpeningArrayBracket  indicates if this is the opening bracket for the array block.
 */
void ASFormatter::formatArrayBrackets(BracketType bracketType, bool isOpeningArrayBracket)
{
	assert(isBracketType(bracketType, ARRAY_TYPE));
	assert(currentChar == '{' || currentChar == '}');

	if (currentChar == '{')
	{
		// is this the first opening bracket in the array?
		if (isOpeningArrayBracket)
		{
			if (bracketFormatMode == ATTACH_MODE
			        || bracketFormatMode == LINUX_MODE
			        || bracketFormatMode == STROUSTRUP_MODE)
			{
				// don't attach to a preprocessor directive or '\' line
				if ((isImmediatelyPostPreprocessor
				        || (formattedLine.length() > 0
				            && formattedLine[formattedLine.length() - 1] == '\\'))
				        && currentLineBeginsWithBracket)
				{
					isInLineBreak = true;
					appendCurrentChar();                // don't attach
				}
				else if (isCharImmediatelyPostComment)
				{
					// TODO: attach bracket to line-end comment
					appendCurrentChar();                // don't attach
				}
				else if (isCharImmediatelyPostLineComment && !isBracketType(bracketType, SINGLE_LINE_TYPE))
				{
					appendCharInsideComments();
				}
				else
				{
					// if a blank line precedes this don't attach
					if (isEmptyLine(formattedLine))
						appendCurrentChar();            // don't attach
					else
					{
						// if bracket is broken or not an assignment
						if (currentLineBeginsWithBracket
						        && !isBracketType(bracketType, SINGLE_LINE_TYPE))
						{
							appendSpacePad();
							appendCurrentChar(false);				// OK to attach
							// TODO: debug the following line
							testForTimeToSplitFormattedLine();		// line length will have changed

							if (currentLineBeginsWithBracket
							        && (int) currentLineFirstBracketNum == charNum)
								shouldBreakLineAtNextChar = true;
						}
						else
						{
							if (previousNonWSChar != '(')
							{
								// don't space pad C++11 uniform initialization
								if (!isBracketType(bracketType, INIT_TYPE))
									appendSpacePad();
							}
							appendCurrentChar();
						}
					}
				}
			}
			else if (bracketFormatMode == BREAK_MODE)
			{
				if (isWhiteSpace(peekNextChar()))
					breakLine();
				else if (isBeforeAnyComment())
				{
					// do not break unless comment is at line end
					if (isBeforeAnyLineEndComment(charNum) && !currentLineBeginsWithBracket)
					{
						currentChar = ' ';              // remove bracket from current line
						appendOpeningBracket = true;    // append bracket to following line
					}
				}
				if (!isInLineBreak && previousNonWSChar != '(')
				{
					// don't space pad C++11 uniform initialization
					if (!isBracketType(bracketType, INIT_TYPE))
						appendSpacePad();
				}
				appendCurrentChar();

				if (currentLineBeginsWithBracket
				        && (int) currentLineFirstBracketNum == charNum
				        && !isBracketType(bracketType, SINGLE_LINE_TYPE))
					shouldBreakLineAtNextChar = true;
			}
			else if (bracketFormatMode == RUN_IN_MODE)
			{
				if (isWhiteSpace(peekNextChar()))
					breakLine();
				else if (isBeforeAnyComment())
				{
					// do not break unless comment is at line end
					if (isBeforeAnyLineEndComment(charNum) && !currentLineBeginsWithBracket)
					{
						currentChar = ' ';              // remove bracket from current line
						appendOpeningBracket = true;    // append bracket to following line
					}
				}
				if (!isInLineBreak && previousNonWSChar != '(')
				{
					// don't space pad C++11 uniform initialization
					if (!isBracketType(bracketType, INIT_TYPE))
						appendSpacePad();
				}
				appendCurrentChar();
			}
			else if (bracketFormatMode == NONE_MODE)
			{
				if (currentLineBeginsWithBracket
				        && charNum == (int) currentLineFirstBracketNum)
				{
					appendCurrentChar();                // don't attach
				}
				else
				{
					if (previousNonWSChar != '(')
					{
						// don't space pad C++11 uniform initialization
						if (!isBracketType(bracketType, INIT_TYPE))
							appendSpacePad();
					}
					appendCurrentChar(false);           // OK to attach
				}
			}
		}
		else	     // not the first opening bracket
		{
			if (bracketFormatMode == RUN_IN_MODE)
			{
				if (previousNonWSChar == '{'
				        && bracketTypeStack->size() > 2
				        && !isBracketType((*bracketTypeStack)[bracketTypeStack->size() - 2], SINGLE_LINE_TYPE))
					formatArrayRunIn();
			}
			else if (!isInLineBreak
			         && !isWhiteSpace(peekNextChar())
			         && previousNonWSChar == '{'
			         && bracketTypeStack->size() > 2
			         && !isBracketType((*bracketTypeStack)[bracketTypeStack->size() - 2], SINGLE_LINE_TYPE))
				formatArrayRunIn();

			appendCurrentChar();
		}
	}
	else if (currentChar == '}')
	{
		if (attachClosingBracketMode)
		{
			if (isEmptyLine(formattedLine)			// if a blank line precedes this
			        || isImmediatelyPostPreprocessor
			        || isCharImmediatelyPostLineComment
			        || isCharImmediatelyPostComment)
				appendCurrentChar();				// don't attach
			else
			{
				appendSpacePad();
				appendCurrentChar(false);			// attach
			}
		}
		else
		{
			// does this close the first opening bracket in the array?
			// must check if the block is still a single line because of anonymous statements
			if (!isBracketType(bracketType, INIT_TYPE)
			        && (!isBracketType(bracketType, SINGLE_LINE_TYPE)
			            || formattedLine.find('{') == string::npos))
				breakLine();
			appendCurrentChar();
		}

		// if a declaration follows an enum definition, space pad
		char peekedChar = peekNextChar();
		if (isLegalNameChar(peekedChar)
		        || peekedChar == '[')
			appendSpaceAfter();
	}
}

/**
 * determine if a run-in can be attached.
 * if it can insert the indents in formattedLine and reset the current line break.
 */
void ASFormatter::formatRunIn()
{
	assert(bracketFormatMode == RUN_IN_MODE || bracketFormatMode == NONE_MODE);

	// keep one line blocks returns true without indenting the run-in
	if (!isOkToBreakBlock(bracketTypeStack->back()))
		return; // true;

	// make sure the line begins with a bracket
	size_t lastText = formattedLine.find_last_not_of(" \t");
	if (lastText == string::npos || formattedLine[lastText] != '{')
		return; // false;

	// make sure the bracket is broken
	if (formattedLine.find_first_not_of(" \t{") != string::npos)
		return; // false;

	if (isBracketType(bracketTypeStack->back(), NAMESPACE_TYPE))
		return; // false;

	bool extraIndent = false;
	bool extraHalfIndent = false;
	isInLineBreak = true;

	// cannot attach a class modifier without indent-classes
	if (isCStyle()
	        && isCharPotentialHeader(currentLine, charNum)
	        && (isBracketType(bracketTypeStack->back(), CLASS_TYPE)
	            || (isBracketType(bracketTypeStack->back(), STRUCT_TYPE)
	                && isInIndentableStruct)))
	{
		if (findKeyword(currentLine, charNum, AS_PUBLIC)
		        || findKeyword(currentLine, charNum, AS_PRIVATE)
		        || findKeyword(currentLine, charNum, AS_PROTECTED))
		{
			if (getModifierIndent())
				extraHalfIndent = true;
			else if (!getClassIndent())
				return; // false;
		}
		else if (getClassIndent())
			extraIndent = true;
	}

	// cannot attach a 'case' statement without indent-switches
	if (!getSwitchIndent()
	        && isCharPotentialHeader(currentLine, charNum)
	        && (findKeyword(currentLine, charNum, AS_CASE)
	            || findKeyword(currentLine, charNum, AS_DEFAULT)))
		return; // false;

	// extra indent for switch statements
	if (getSwitchIndent()
	        && !preBracketHeaderStack->empty()
	        && preBracketHeaderStack->back() == &AS_SWITCH
	        && ((isLegalNameChar(currentChar)
	             && !findKeyword(currentLine, charNum, AS_CASE))))
		extraIndent = true;

	isInLineBreak = false;
	// remove for extra whitespace
	if (formattedLine.length() > lastText + 1
	        && formattedLine.find_first_not_of(" \t", lastText + 1) == string::npos)
		formattedLine.erase(lastText + 1);

	if (extraHalfIndent)
	{
		int indentLength_ = getIndentLength();
		horstmannIndentChars = indentLength_ / 2;
		formattedLine.append(horstmannIndentChars - 1, ' ');
	}
	else if (getForceTabIndentation() && getIndentLength() != getTabLength())
	{
		// insert the space indents
		string indent;
		int indentLength_ = getIndentLength();
		int tabLength_ = getTabLength();
		indent.append(indentLength_, ' ');
		if (extraIndent)
			indent.append(indentLength_, ' ');
		// replace spaces indents with tab indents
		size_t tabCount = indent.length() / tabLength_;		// truncate extra spaces
		indent.erase(0U, tabCount * tabLength_);
		indent.insert(0U, tabCount, '\t');
		horstmannIndentChars = indentLength_;
		if (indent[0] == ' ')			// allow for bracket
			indent.erase(0, 1);
		formattedLine.append(indent);
	}
	else if (getIndentString() == "\t")
	{
		appendChar('\t', false);
		horstmannIndentChars = 2;	// one for { and one for tab
		if (extraIndent)
		{
			appendChar('\t', false);
			horstmannIndentChars++;
		}
	}
	else // spaces
	{
		int indentLength_ = getIndentLength();
		formattedLine.append(indentLength_ - 1, ' ');
		horstmannIndentChars = indentLength_;
		if (extraIndent)
		{
			formattedLine.append(indentLength_, ' ');
			horstmannIndentChars += indentLength_;
		}
	}
	isInHorstmannRunIn = true;
}

/**
 * remove whitespace and add indentation for an array run-in.
 */
void ASFormatter::formatArrayRunIn()
{
	assert(isBracketType(bracketTypeStack->back(), ARRAY_TYPE));

	// make sure the bracket is broken
	if (formattedLine.find_first_not_of(" \t{") != string::npos)
		return;

	size_t lastText = formattedLine.find_last_not_of(" \t");
	if (lastText == string::npos || formattedLine[lastText] != '{')
		return;

	// check for extra whitespace
	if (formattedLine.length() > lastText + 1
	        && formattedLine.find_first_not_of(" \t", lastText + 1) == string::npos)
		formattedLine.erase(lastText + 1);

	if (getIndentString() == "\t")
	{
		appendChar('\t', false);
		horstmannIndentChars = 2;	// one for { and one for tab
	}
	else
	{
		int indent = getIndentLength();
		formattedLine.append(indent - 1, ' ');
		horstmannIndentChars = indent;
	}
	isInHorstmannRunIn = true;
	isInLineBreak = false;
}

/**
 * delete a bracketTypeStack vector object
 * BracketTypeStack did not work with the DeleteContainer template
 */
void ASFormatter::deleteContainer(vector<BracketType>* &container)
{
	if (container != NULL)
	{
		container->clear();
		delete (container);
		container = NULL;
	}
}

/**
 * delete a vector object
 * T is the type of vector
 * used for all vectors except bracketTypeStack
 */
template<typename T>
void ASFormatter::deleteContainer(T &container)
{
	if (container != NULL)
	{
		container->clear();
		delete (container);
		container = NULL;
	}
}

/**
 * initialize a BracketType vector object
 * BracketType did not work with the DeleteContainer template
 */
void ASFormatter::initContainer(vector<BracketType>* &container, vector<BracketType>* value)
{
	if (container != NULL)
		deleteContainer(container);
	container = value;
}

/**
 * initialize a vector object
 * T is the type of vector
 * used for all vectors except bracketTypeStack
 */
template<typename T>
void ASFormatter::initContainer(T &container, T value)
{
	// since the ASFormatter object is never deleted,
	// the existing vectors must be deleted before creating new ones
	if (container != NULL)
		deleteContainer(container);
	container = value;
}

/**
 * convert a tab to spaces.
 * charNum points to the current character to convert to spaces.
 * tabIncrementIn is the increment that must be added for tab indent characters
 *     to get the correct column for the current tab.
 * replaces the tab in currentLine with the required number of spaces.
 * replaces the value of currentChar.
 */
void ASFormatter::convertTabToSpaces()
{
	assert(currentLine[charNum] == '\t');

	// do NOT replace if in quotes
	if (isInQuote || isInQuoteContinuation)
		return;

	size_t tabSize = getTabLength();
	size_t numSpaces = tabSize - ((tabIncrementIn + charNum) % tabSize);
	currentLine.replace(charNum, 1, numSpaces, ' ');
	currentChar = currentLine[charNum];
}

/**
* is it ok to break this block?
*/
bool ASFormatter::isOkToBreakBlock(BracketType bracketType) const
{
	// Actually, there should not be an ARRAY_TYPE bracket here.
	// But this will avoid breaking a one line block when there is.
	// Otherwise they will be formatted differently on consecutive runs.
	if (isBracketType(bracketType, ARRAY_TYPE)
	        && isBracketType(bracketType, SINGLE_LINE_TYPE))
		return false;
	if (!isBracketType(bracketType, SINGLE_LINE_TYPE)
	        || shouldBreakOneLineBlocks
	        || breakCurrentOneLineBlock)
		return true;
	return false;
}

/**
* check if a sharp header is a paren or non-paren header
*/
bool ASFormatter::isSharpStyleWithParen(const string* header) const
{
	if (isSharpStyle() && peekNextChar() == '('
	        && (header == &AS_CATCH
	            || header == &AS_DELEGATE))
		return true;
	return false;
}

/**
 * Check for a following header when a comment is reached.
 * firstLine must contain the start of the comment.
 * return value is a pointer to the header or NULL.
 */
const string* ASFormatter::checkForHeaderFollowingComment(const string &firstLine) const
{
	assert(isInComment || isInLineComment);
	assert(shouldBreakElseIfs || shouldBreakBlocks || isInSwitchStatement());
	// look ahead to find the next non-comment text
	bool endOnEmptyLine = (currentHeader == NULL);
	if (isInSwitchStatement())
		endOnEmptyLine = false;
	string nextText = peekNextText(firstLine, endOnEmptyLine);

	if (nextText.length() == 0 || !isCharPotentialHeader(nextText, 0))
		return NULL;

	return ASBeautifier::findHeader(nextText, 0, headers);
}

/**
 * process preprocessor statements.
 * charNum should be the index of the #.
 *
 * delete bracketTypeStack entries added by #if if a #else is found.
 * prevents double entries in the bracketTypeStack.
 */
void ASFormatter::processPreprocessor()
{
	assert(currentChar == '#');

	const size_t preproc = currentLine.find_first_not_of(" \t", charNum + 1);

	if (preproc == string::npos)
		return;

	if (currentLine.compare(preproc, 2, "if") == 0)
	{
		preprocBracketTypeStackSize = bracketTypeStack->size();
	}
	else if (currentLine.compare(preproc, 4, "else") == 0)
	{
		// delete stack entries added in #if
		// should be replaced by #else
		if (preprocBracketTypeStackSize > 0)
		{
			int addedPreproc = bracketTypeStack->size() - preprocBracketTypeStackSize;
			for (int i = 0; i < addedPreproc; i++)
				bracketTypeStack->pop_back();
		}
	}
}

/**
 * determine if the next line starts a comment
 * and a header follows the comment or comments.
 */
bool ASFormatter::commentAndHeaderFollows()
{
	// called ONLY IF shouldDeleteEmptyLines and shouldBreakBlocks are TRUE.
	assert(shouldDeleteEmptyLines && shouldBreakBlocks);

	// is the next line a comment
	if (!sourceIterator->hasMoreLines())
		return false;
	string nextLine_ = sourceIterator->peekNextLine();
	size_t firstChar = nextLine_.find_first_not_of(" \t");
	if (firstChar == string::npos
	        || !(nextLine_.compare(firstChar, 2, "//") == 0
	             || nextLine_.compare(firstChar, 2, "/*") == 0))
	{
		sourceIterator->peekReset();
		return false;
	}

	// find the next non-comment text, and reset
	string nextText = peekNextText(nextLine_, false, true);
	if (nextText.length() == 0 || !isCharPotentialHeader(nextText, 0))
		return false;

	const string* newHeader = ASBeautifier::findHeader(nextText, 0, headers);

	if (newHeader == NULL)
		return false;

	// if a closing header, reset break unless break is requested
	if (isClosingHeader(newHeader) && !shouldBreakClosingHeaderBlocks)
	{
		isAppendPostBlockEmptyLineRequested = false;
		return false;
	}

	return true;
}

/**
 * determine if a bracket should be attached or broken
 * uses brackets in the bracketTypeStack
 * the last bracket in the bracketTypeStack is the one being formatted
 * returns true if the bracket should be broken
 */
bool ASFormatter::isCurrentBracketBroken() const
{
	assert(bracketTypeStack->size() > 1);

	bool breakBracket = false;
	size_t stackEnd = bracketTypeStack->size() - 1;

	// check bracket modifiers
	if (shouldAttachExternC
	        && isBracketType((*bracketTypeStack)[stackEnd], EXTERN_TYPE))
	{
		return false;
	}
	if (shouldAttachNamespace
	        && isBracketType((*bracketTypeStack)[stackEnd], NAMESPACE_TYPE))
	{
		return false;
	}
	else if (shouldAttachClass
	         && (isBracketType((*bracketTypeStack)[stackEnd], CLASS_TYPE)
	             || isBracketType((*bracketTypeStack)[stackEnd], INTERFACE_TYPE)))
	{
		return false;
	}
	else if (shouldAttachInline
	         && isCStyle()			// for C++ only
	         && bracketFormatMode != RUN_IN_MODE
	         && isBracketType((*bracketTypeStack)[stackEnd], COMMAND_TYPE))
	{
		size_t i;
		for (i = 1; i < bracketTypeStack->size(); i++)
			if (isBracketType((*bracketTypeStack)[i], CLASS_TYPE)
			        || isBracketType((*bracketTypeStack)[i], STRUCT_TYPE))
				return false;
	}

	// check brackets
	if (isBracketType((*bracketTypeStack)[stackEnd], EXTERN_TYPE))
	{
		if (currentLineBeginsWithBracket
		        || bracketFormatMode == RUN_IN_MODE)
			breakBracket = true;
	}
	else if (bracketFormatMode == NONE_MODE)
	{
		if (currentLineBeginsWithBracket
		        && (int) currentLineFirstBracketNum == charNum)
			breakBracket = true;
	}
	else if (bracketFormatMode == BREAK_MODE || bracketFormatMode == RUN_IN_MODE)
	{
		breakBracket = true;
	}
	else if (bracketFormatMode == LINUX_MODE || bracketFormatMode == STROUSTRUP_MODE)
	{
		// break a namespace, class, or interface if Linux
		if (isBracketType((*bracketTypeStack)[stackEnd], NAMESPACE_TYPE)
		        || isBracketType((*bracketTypeStack)[stackEnd], CLASS_TYPE)
		        || isBracketType((*bracketTypeStack)[stackEnd], INTERFACE_TYPE))
		{
			if (bracketFormatMode == LINUX_MODE)
				breakBracket = true;
		}
		// break the first bracket if a function
		else if (isBracketType((*bracketTypeStack)[stackEnd], COMMAND_TYPE))
		{
			if (stackEnd == 1)
			{
				breakBracket = true;
			}
			else if (stackEnd > 1)
			{
				// break the first bracket after these if a function
				if (isBracketType((*bracketTypeStack)[stackEnd - 1], NAMESPACE_TYPE)
				        || isBracketType((*bracketTypeStack)[stackEnd - 1], CLASS_TYPE)
				        || isBracketType((*bracketTypeStack)[stackEnd - 1], ARRAY_TYPE)
				        || isBracketType((*bracketTypeStack)[stackEnd - 1], STRUCT_TYPE)
				        || isBracketType((*bracketTypeStack)[stackEnd - 1], EXTERN_TYPE))
				{
					breakBracket = true;
				}
			}
		}
	}
	return breakBracket;
}

/**
 * format comment body
 * the calling function should have a continue statement after calling this method
 */
void ASFormatter::formatCommentBody()
{
	assert(isInComment);

	// append the comment
	while (charNum < (int) currentLine.length())
	{
		currentChar = currentLine[charNum];
		if (currentLine.compare(charNum, 2, "*/") == 0)
		{
			formatCommentCloser();
			break;
		}
		if (currentChar == '\t' && shouldConvertTabs)
			convertTabToSpaces();
		appendCurrentChar();
		++charNum;
	}
	if (shouldStripCommentPrefix)
		stripCommentPrefix();
}

/**
 * format a comment opener
 * the comment opener will be appended to the current formattedLine or a new formattedLine as necessary
 * the calling function should have a continue statement after calling this method
 */
void ASFormatter::formatCommentOpener()
{
	assert(isSequenceReached("/*"));

	isInComment = isInCommentStartLine = true;
	isImmediatelyPostLineComment = false;
	if (previousNonWSChar == '}')
		resetEndOfStatement();

	// Check for a following header.
	// For speed do not check multiple comment lines more than once.
	// For speed do not check shouldBreakBlocks if previous line is empty, a comment, or a '{'.
	const string* followingHeader = NULL;
	if ((doesLineStartComment
	        && !isImmediatelyPostCommentOnly
	        && isBracketType(bracketTypeStack->back(), COMMAND_TYPE))
	        && (shouldBreakElseIfs
	            || isInSwitchStatement()
	            || (shouldBreakBlocks
	                && !isImmediatelyPostEmptyLine
	                && previousCommandChar != '{')))
		followingHeader = checkForHeaderFollowingComment(currentLine.substr(charNum));

	if (spacePadNum != 0 && !isInLineBreak)
		adjustComments();
	formattedLineCommentNum = formattedLine.length();

	// must be done BEFORE appendSequence
	if (previousCommandChar == '{'
	        && !isImmediatelyPostComment
	        && !isImmediatelyPostLineComment)
	{
		if (bracketFormatMode == NONE_MODE)
		{
			// should a run-in statement be attached?
			if (currentLineBeginsWithBracket)
				formatRunIn();
		}
		else if (bracketFormatMode == ATTACH_MODE)
		{
			// if the bracket was not attached?
			if (formattedLine.length() > 0 && formattedLine[0] == '{'
			        && !isBracketType(bracketTypeStack->back(), SINGLE_LINE_TYPE))
				isInLineBreak = true;
		}
		else if (bracketFormatMode == RUN_IN_MODE)
		{
			// should a run-in statement be attached?
			if (formattedLine.length() > 0 && formattedLine[0] == '{')
				formatRunIn();
		}
	}
	else if (!doesLineStartComment)
		noTrimCommentContinuation = true;

	// ASBeautifier needs to know the following statements
	if (shouldBreakElseIfs && followingHeader == &AS_ELSE)
		elseHeaderFollowsComments = true;
	if (followingHeader == &AS_CASE || followingHeader == &AS_DEFAULT)
		caseHeaderFollowsComments = true;

	// appendSequence will write the previous line
	appendSequence(AS_OPEN_COMMENT);
	goForward(1);

	// must be done AFTER appendSequence

	// Break before the comment if a header follows the line comment.
	// But not break if previous line is empty, a comment, or a '{'.
	if (shouldBreakBlocks
	        && followingHeader != NULL
	        && !isImmediatelyPostEmptyLine
	        && previousCommandChar != '{')
	{
		if (isClosingHeader(followingHeader))
		{
			if (!shouldBreakClosingHeaderBlocks)
				isPrependPostBlockEmptyLineRequested = false;
		}
		// if an opening header, break before the comment
		else
			isPrependPostBlockEmptyLineRequested = true;
	}

	if (previousCommandChar == '}')
		currentHeader = NULL;
}

/**
 * format a comment closer
 * the comment closer will be appended to the current formattedLine
 */
void ASFormatter::formatCommentCloser()
{
	isInComment = false;
	noTrimCommentContinuation = false;
	isImmediatelyPostComment = true;
	appendSequence(AS_CLOSE_COMMENT);
	goForward(1);
	if (doesLineStartComment
	        && (currentLine.find_first_not_of(" \t", charNum + 1) == string::npos))
		lineEndsInCommentOnly = true;
	if (peekNextChar() == '}'
	        && previousCommandChar != ';'
	        && !isBracketType(bracketTypeStack->back(),  ARRAY_TYPE)
	        && !isInPreprocessor
	        && isOkToBreakBlock(bracketTypeStack->back()))
	{
		isInLineBreak = true;
		shouldBreakLineAtNextChar = true;
	}
}

/**
 * format a line comment body
 * the calling function should have a continue statement after calling this method
 */
void ASFormatter::formatLineCommentBody()
{
	assert(isInLineComment);

	// append the comment
	while (charNum < (int) currentLine.length())
//	        && !isLineReady	// commented out in release 2.04, unnecessary
	{
		currentChar = currentLine[charNum];
		if (currentChar == '\t' && shouldConvertTabs)
			convertTabToSpaces();
		appendCurrentChar();
		++charNum;
	}

	// explicitly break a line when a line comment's end is found.
	if (charNum == (int) currentLine.length())
	{
		isInLineBreak = true;
		isInLineComment = false;
		isImmediatelyPostLineComment = true;
		currentChar = 0;  //make sure it is a neutral char.
	}
}

/**
 * format a line comment opener
 * the line comment opener will be appended to the current formattedLine or a new formattedLine as necessary
 * the calling function should have a continue statement after calling this method
 */
void ASFormatter::formatLineCommentOpener()
{
	assert(isSequenceReached("//"));

	if ((int) currentLine.length() > charNum + 2
	        && currentLine[charNum + 2] == '\xf2')     // check for windows line marker
		isAppendPostBlockEmptyLineRequested = false;

	isInLineComment = true;
	isCharImmediatelyPostComment = false;
	if (previousNonWSChar == '}')
		resetEndOfStatement();

	// Check for a following header.
	// For speed do not check multiple comment lines more than once.
	// For speed do not check shouldBreakBlocks if previous line is empty, a comment, or a '{'.
	const string* followingHeader = NULL;
	if ((lineIsLineCommentOnly
	        && !isImmediatelyPostCommentOnly
	        && isBracketType(bracketTypeStack->back(), COMMAND_TYPE))
	        && (shouldBreakElseIfs
	            || isInSwitchStatement()
	            || (shouldBreakBlocks
	                && !isImmediatelyPostEmptyLine
	                && previousCommandChar != '{')))
		followingHeader = checkForHeaderFollowingComment(currentLine.substr(charNum));

	// do not indent if in column 1 or 2
	// or in a namespace before the opening bracket
	if ((!shouldIndentCol1Comments && !lineCommentNoIndent)
	        || foundNamespaceHeader)
	{
		if (charNum == 0)
			lineCommentNoIndent = true;
		else if (charNum == 1 && currentLine[0] == ' ')
			lineCommentNoIndent = true;
	}
	// move comment if spaces were added or deleted
	if (lineCommentNoIndent == false && spacePadNum != 0 && !isInLineBreak)
		adjustComments();
	formattedLineCommentNum = formattedLine.length();

	// must be done BEFORE appendSequence
	// check for run-in statement
	if (previousCommandChar == '{'
	        && !isImmediatelyPostComment
	        && !isImmediatelyPostLineComment)
	{
		if (bracketFormatMode == NONE_MODE)
		{
			if (currentLineBeginsWithBracket)
				formatRunIn();
		}
		else if (bracketFormatMode == RUN_IN_MODE)
		{
			if (!lineCommentNoIndent)
				formatRunIn();
			else
				isInLineBreak = true;
		}
		else if (bracketFormatMode == BREAK_MODE)
		{
			if (formattedLine.length() > 0 && formattedLine[0] == '{')
				isInLineBreak = true;
		}
		else
		{
			if (currentLineBeginsWithBracket)
				isInLineBreak = true;
		}
	}

	// ASBeautifier needs to know the following statements
	if (shouldBreakElseIfs && followingHeader == &AS_ELSE)
		elseHeaderFollowsComments = true;
	if (followingHeader == &AS_CASE || followingHeader == &AS_DEFAULT)
		caseHeaderFollowsComments = true;

	// appendSequence will write the previous line
	appendSequence(AS_OPEN_LINE_COMMENT);
	goForward(1);

	// must be done AFTER appendSequence

	// Break before the comment if a header follows the line comment.
	// But do not break if previous line is empty, a comment, or a '{'.
	if (shouldBreakBlocks
	        && followingHeader != NULL
	        && !isImmediatelyPostEmptyLine
	        && previousCommandChar != '{')
	{
		if (isClosingHeader(followingHeader))
		{
			if (!shouldBreakClosingHeaderBlocks)
				isPrependPostBlockEmptyLineRequested = false;
		}
		// if an opening header, break before the comment
		else
			isPrependPostBlockEmptyLineRequested = true;
	}

	if (previousCommandChar == '}')
		currentHeader = NULL;

	// if tabbed input don't convert the immediately following tabs to spaces
	if (getIndentString() == "\t" && lineCommentNoIndent)
	{
		while (charNum + 1 < (int) currentLine.length()
		        && currentLine[charNum + 1] == '\t')
		{
			currentChar = currentLine[++charNum];
			appendCurrentChar();
		}
	}

	// explicitly break a line when a line comment's end is found.
	if (charNum + 1 == (int) currentLine.length())
	{
		isInLineBreak = true;
		isInLineComment = false;
		isImmediatelyPostLineComment = true;
		currentChar = 0;  //make sure it is a neutral char.
	}
}

/**
 * format quote body
 * the calling function should have a continue statement after calling this method
 */
void ASFormatter::formatQuoteBody()
{
	assert(isInQuote);

	if (isSpecialChar)
	{
		isSpecialChar = false;
	}
	else if (currentChar == '\\' && !isInVerbatimQuote)
	{
		if (peekNextChar() == ' ')              // is this '\' at end of line
			haveLineContinuationChar = true;
		else
			isSpecialChar = true;
	}
	else if (isInVerbatimQuote && currentChar == '"')
	{
		if (isCStyle())
		{
			string delim = ')' + verbatimDelimiter;
			int delimStart = charNum - delim.length();
			if (delimStart > 0 && currentLine.substr(delimStart, delim.length()) == delim)
			{
				isInQuote = false;
				isInVerbatimQuote = false;
			}
		}
		else if (isSharpStyle())
		{
			if (peekNextChar() == '"')              // check consecutive quotes
			{
				appendSequence("\"\"");
				goForward(1);
				return;
			}
			else
			{
				isInQuote = false;
				isInVerbatimQuote = false;
			}
		}
	}
	else if (quoteChar == currentChar)
	{
		isInQuote = false;
	}

	appendCurrentChar();

	// append the text to the ending quoteChar or an escape sequence
	// tabs in quotes are NOT changed by convert-tabs
	if (isInQuote && currentChar != '\\')
	{
		while (charNum + 1 < (int) currentLine.length()
		        && currentLine[charNum + 1] != quoteChar
		        && currentLine[charNum + 1] != '\\')
		{
			currentChar = currentLine[++charNum];
			appendCurrentChar();
		}
	}
}

/**
 * format a quote opener
 * the quote opener will be appended to the current formattedLine or a new formattedLine as necessary
 * the calling function should have a continue statement after calling this method
 */
void ASFormatter::formatQuoteOpener()
{
	assert(currentChar == '"' || currentChar == '\'');

	isInQuote = true;
	quoteChar = currentChar;
	if (isCStyle() && previousChar == 'R')
	{
		int parenPos = currentLine.find('(', charNum);
		if (parenPos != -1)
		{
			isInVerbatimQuote = true;
			verbatimDelimiter = currentLine.substr(charNum + 1, parenPos - charNum - 1);
		}
	}
	else if (isSharpStyle() && previousChar == '@')
		isInVerbatimQuote = true;

	// a quote following a bracket is an array
	if (previousCommandChar == '{'
	        && !isImmediatelyPostComment
	        && !isImmediatelyPostLineComment
	        && isNonInStatementArray
	        && !isBracketType(bracketTypeStack->back(), SINGLE_LINE_TYPE)
	        && !isWhiteSpace(peekNextChar()))
	{
		if (bracketFormatMode == NONE_MODE)
		{
			if (currentLineBeginsWithBracket)
				formatRunIn();
		}
		else if (bracketFormatMode == RUN_IN_MODE)
		{
			formatRunIn();
		}
		else if (bracketFormatMode == BREAK_MODE)
		{
			if (formattedLine.length() > 0 && formattedLine[0] == '{')
				isInLineBreak = true;
		}
		else
		{
			if (currentLineBeginsWithBracket)
				isInLineBreak = true;
		}
	}
	previousCommandChar = ' ';
	appendCurrentChar();
}

/**
 * get the next line comment adjustment that results from breaking a closing bracket.
 * the bracket must be on the same line as the closing header.
 * i.e "} else" changed to "} <NL> else".
 */
int ASFormatter::getNextLineCommentAdjustment()
{
	assert(foundClosingHeader && previousNonWSChar == '}');
	if (charNum < 1)			// "else" is in column 1
		return 0;
	size_t lastBracket = currentLine.rfind('}', charNum - 1);
	if (lastBracket != string::npos)
		return (lastBracket - charNum);	// return a negative number
	return 0;
}

// for console build only
LineEndFormat ASFormatter::getLineEndFormat() const
{
	return lineEnd;
}

/**
 * get the current line comment adjustment that results from attaching
 * a closing header to a closing bracket.
 * the bracket must be on the line previous to the closing header.
 * the adjustment is 2 chars, one for the bracket and one for the space.
 * i.e "} <NL> else" changed to "} else".
 */
int ASFormatter::getCurrentLineCommentAdjustment()
{
	assert(foundClosingHeader && previousNonWSChar == '}');
	if (charNum < 1)
		return 2;
	size_t lastBracket = currentLine.rfind('}', charNum - 1);
	if (lastBracket == string::npos)
		return 2;
	return 0;
}

/**
 * get the previous word on a line
 * the argument 'currPos' must point to the current position.
 *
 * @return is the previous word or an empty string if none found.
 */
string ASFormatter::getPreviousWord(const string &line, int currPos) const
{
	// get the last legal word (may be a number)
	if (currPos == 0)
		return string();

	size_t end = line.find_last_not_of(" \t", currPos - 1);
	if (end == string::npos || !isLegalNameChar(line[end]))
		return string();

	int start;          // start of the previous word
	for (start = end; start > -1; start--)
	{
		if (!isLegalNameChar(line[start]) || line[start] == '.')
			break;
	}
	start++;

	return (line.substr(start, end - start + 1));
}

/**
 * check if a line break is needed when a closing bracket
 * is followed by a closing header.
 * the break depends on the bracketFormatMode and other factors.
 */
void ASFormatter::isLineBreakBeforeClosingHeader()
{
	assert(foundClosingHeader && previousNonWSChar == '}');
	if (bracketFormatMode == BREAK_MODE
	        || bracketFormatMode == RUN_IN_MODE
	        || attachClosingBracketMode)
	{
		isInLineBreak = true;
	}
	else if (bracketFormatMode == NONE_MODE)
	{
		if (shouldBreakClosingHeaderBrackets
		        || getBracketIndent() || getBlockIndent())
		{
			isInLineBreak = true;
		}
		else
		{
			appendSpacePad();
			// is closing bracket broken?
			size_t i = currentLine.find_first_not_of(" \t");
			if (i != string::npos && currentLine[i] == '}')
				isInLineBreak = false;

			if (shouldBreakBlocks)
				isAppendPostBlockEmptyLineRequested = false;
		}
	}
	// bracketFormatMode == ATTACH_MODE, LINUX_MODE, STROUSTRUP_MODE
	else
	{
		if (shouldBreakClosingHeaderBrackets
		        || getBracketIndent() || getBlockIndent())
		{
			isInLineBreak = true;
		}
		else
		{
			// if a blank line does not precede this
			// or last line is not a one line block, attach header
			bool previousLineIsEmpty = isEmptyLine(formattedLine);
			int previousLineIsOneLineBlock = 0;
			size_t firstBracket = findNextChar(formattedLine, '{');
			if (firstBracket != string::npos)
				previousLineIsOneLineBlock = isOneLineBlockReached(formattedLine, firstBracket);
			if (!previousLineIsEmpty
			        && previousLineIsOneLineBlock == 0)
			{
				isInLineBreak = false;
				appendSpacePad();
				spacePadNum = 0;	// don't count as comment padding
			}

			if (shouldBreakBlocks)
				isAppendPostBlockEmptyLineRequested = false;
		}
	}
}

/**
 * Add brackets to a single line statement following a header.
 * Brackets are not added if the proper conditions are not met.
 * Brackets are added to the currentLine.
 */
bool ASFormatter::addBracketsToStatement()
{
	assert(isImmediatelyPostHeader);

	if (currentHeader != &AS_IF
	        && currentHeader != &AS_ELSE
	        && currentHeader != &AS_FOR
	        && currentHeader != &AS_WHILE
	        && currentHeader != &AS_DO
	        && currentHeader != &AS_FOREACH
	        && currentHeader != &AS_QFOREACH
	        && currentHeader != &AS_QFOREVER
	        && currentHeader != &AS_FOREVER)
		return false;

	if (currentHeader == &AS_WHILE && foundClosingHeader)	// do-while
		return false;

	// do not bracket an empty statement
	if (currentChar == ';')
		return false;

	// do not add if a header follows
	if (isCharPotentialHeader(currentLine, charNum))
		if (findHeader(headers) != NULL)
			return false;

	// find the next semi-colon
	size_t nextSemiColon = charNum;
	if (currentChar != ';')
		nextSemiColon = findNextChar(currentLine, ';', charNum + 1);
	if (nextSemiColon == string::npos)
		return false;

	// add closing bracket before changing the line length
	if (nextSemiColon == currentLine.length() - 1)
		currentLine.append(" }");
	else
		currentLine.insert(nextSemiColon + 1, " }");
	// add opening bracket
	currentLine.insert(charNum, "{ ");
	assert(computeChecksumIn("{}"));
	currentChar = '{';
	// remove extra spaces
	if (!shouldAddOneLineBrackets)
	{
		size_t lastText = formattedLine.find_last_not_of(" \t");
		if ((formattedLine.length() - 1) - lastText > 1)
			formattedLine.erase(lastText + 1);
	}
	return true;
}

/**
 * Remove brackets from a single line statement following a header.
 * Brackets are not removed if the proper conditions are not met.
 * The first bracket is replaced by a space.
 */
bool ASFormatter::removeBracketsFromStatement()
{
	assert(isImmediatelyPostHeader);
	assert(currentChar == '{');

	if (currentHeader != &AS_IF
	        && currentHeader != &AS_ELSE
	        && currentHeader != &AS_FOR
	        && currentHeader != &AS_WHILE
	        && currentHeader != &AS_FOREACH)
		return false;

	if (currentHeader == &AS_WHILE && foundClosingHeader)	// do-while
		return false;

	bool isFirstLine = true;
	bool needReset = false;
	string nextLine_;
	// leave nextLine_ empty if end of line comment follows
	if (!isBeforeAnyLineEndComment(charNum) || currentLineBeginsWithBracket)
		nextLine_ = currentLine.substr(charNum + 1);
	size_t nextChar = 0;

	// find the first non-blank text
	while (sourceIterator->hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
		{
			nextLine_ = sourceIterator->peekNextLine();
			nextChar = 0;
			needReset = true;
		}

		nextChar = nextLine_.find_first_not_of(" \t", nextChar);
		if (nextChar != string::npos)
			break;
	}

	// don't remove if comments or a header follow the bracket
	if ((nextLine_.compare(nextChar, 2, "/*") == 0)
	        || (nextLine_.compare(nextChar, 2, "//") == 0)
	        || (isCharPotentialHeader(nextLine_, nextChar)
	            && ASBeautifier::findHeader(nextLine_, nextChar, headers) != NULL))
	{
		if (needReset)
			sourceIterator->peekReset();
		return false;
	}

	// find the next semi-colon
	size_t nextSemiColon = nextChar;
	if (nextLine_[nextChar] != ';')
		nextSemiColon = findNextChar(nextLine_, ';', nextChar + 1);
	if (nextSemiColon == string::npos)
	{
		if (needReset)
			sourceIterator->peekReset();
		return false;
	}

	// find the closing bracket
	isFirstLine = true;
	nextChar = nextSemiColon + 1;
	while (sourceIterator->hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
		{
			nextLine_ = sourceIterator->peekNextLine();
			nextChar = 0;
			needReset = true;
		}
		nextChar = nextLine_.find_first_not_of(" \t", nextChar);
		if (nextChar != string::npos)
			break;
	}
	if (nextLine_.length() == 0 || nextLine_[nextChar] != '}')
	{
		if (needReset)
			sourceIterator->peekReset();
		return false;
	}

	// remove opening bracket
	currentLine[charNum] = currentChar = ' ';
	assert(adjustChecksumIn(-'{'));
	if (needReset)
		sourceIterator->peekReset();
	return true;
}

/**
 * Find the next character that is not in quotes or a comment.
 *
 * @param line         the line to be searched.
 * @param searchChar   the char to find.
 * @param searchStart  the start position on the line (default is 0).
 * @return the position on the line or string::npos if not found.
 */
size_t ASFormatter::findNextChar(string &line, char searchChar, int searchStart /*0*/)
{
	// find the next searchChar
	size_t i;
	for (i = searchStart; i < line.length(); i++)
	{
		if (line.compare(i, 2, "//") == 0)
			return string::npos;
		if (line.compare(i, 2, "/*") == 0)
		{
			size_t endComment = line.find("*/", i + 2);
			if (endComment == string::npos)
				return string::npos;
			i = endComment + 2;
			if (i >= line.length())
				return string::npos;
		}
		if (line[i] == '\'' || line[i] == '\"')
		{
			char quote = line[i];
			while (i < line.length())
			{
				size_t endQuote = line.find(quote, i + 1);
				if (endQuote == string::npos)
					return string::npos;
				i = endQuote;
				if (line[endQuote - 1] != '\\')	// check for '\"'
					break;
				if (line[endQuote - 2] == '\\')	// check for '\\'
					break;
			}
		}

		if (line[i] == searchChar)
			break;

		// for now don't process C# 'delegate' brackets
		// do this last in case the search char is a '{'
		if (line[i] == '{')
			return string::npos;
	}
	if (i >= line.length())	// didn't find searchChar
		return string::npos;

	return i;
}

/**
 * Look ahead in the file to see if a struct has access modifiers.
 *
 * @param firstLine     a reference to the line to indent.
 * @param index         the current line index.
 * @return              true if the struct has access modifiers.
 */
bool ASFormatter::isStructAccessModified(string &firstLine, size_t index) const
{
	assert(firstLine[index] == '{');
	assert(isCStyle());

	bool isFirstLine = true;
	bool needReset = false;
	size_t bracketCount = 1;
	string nextLine_ = firstLine.substr(index + 1);

	// find the first non-blank text, bypassing all comments and quotes.
	bool isInComment_ = false;
	bool isInQuote_ = false;
	char quoteChar_ = ' ';
	while (sourceIterator->hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
		{
			nextLine_ = sourceIterator->peekNextLine();
			needReset = true;
		}
		// parse the line
		for (size_t i = 0; i < nextLine_.length(); i++)
		{
			if (isWhiteSpace(nextLine_[i]))
				continue;
			if (nextLine_.compare(i, 2, "/*") == 0)
				isInComment_ = true;
			if (isInComment_)
			{
				if (nextLine_.compare(i, 2, "*/") == 0)
				{
					isInComment_ = false;
					++i;
				}
				continue;
			}
			if (nextLine_[i] == '\\')
			{
				++i;
				continue;
			}

			if (isInQuote_)
			{
				if (nextLine_[i] == quoteChar_)
					isInQuote_ = false;
				continue;
			}

			if (nextLine_[i] == '"' || nextLine_[i] == '\'')
			{
				isInQuote_ = true;
				quoteChar_ = nextLine_[i];
				continue;
			}
			if (nextLine_.compare(i, 2, "//") == 0)
			{
				i = nextLine_.length();
				continue;
			}
			// handle brackets
			if (nextLine_[i] == '{')
				++bracketCount;
			if (nextLine_[i] == '}')
				--bracketCount;
			if (bracketCount == 0)
			{
				if (needReset)
					sourceIterator->peekReset();
				return false;
			}
			// check for access modifiers
			if (isCharPotentialHeader(nextLine_, i))
			{
				if (findKeyword(nextLine_, i, AS_PUBLIC)
				        || findKeyword(nextLine_, i, AS_PRIVATE)
				        || findKeyword(nextLine_, i, AS_PROTECTED))
				{
					if (needReset)
						sourceIterator->peekReset();
					return true;
				}
				string name = getCurrentWord(nextLine_, i);
				i += name.length() - 1;
			}
		}	// end of for loop
	}	// end of while loop

	if (needReset)
		sourceIterator->peekReset();
	return false;
}

/**
* Look ahead in the file to see if a preprocessor block is indentable.
*
* @param firstLine     a reference to the line to indent.
* @param index         the current line index.
* @return              true if the block is indentable.
*/
bool ASFormatter::isIndentablePreprocessorBlock(string &firstLine, size_t index)
{
	assert(firstLine[index] == '#');

	bool isFirstLine = true;
	bool needReset = false;
	bool isInIndentableBlock = false;
	bool blockContainsBrackets = false;
	bool blockContainsDefineContinuation = false;
	bool isInClassConstructor = false;
	int  numBlockIndents = 0;
	int  lineParenCount = 0;
	string nextLine_ = firstLine.substr(index);

	// find end of the block, bypassing all comments and quotes.
	bool isInComment_ = false;
	bool isInQuote_ = false;
	char quoteChar_ = ' ';
	while (sourceIterator->hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
		{
			nextLine_ = sourceIterator->peekNextLine();
			needReset = true;
		}
		// parse the line
		for (size_t i = 0; i < nextLine_.length(); i++)
		{
			if (isWhiteSpace(nextLine_[i]))
				continue;
			if (nextLine_.compare(i, 2, "/*") == 0)
				isInComment_ = true;
			if (isInComment_)
			{
				if (nextLine_.compare(i, 2, "*/") == 0)
				{
					isInComment_ = false;
					++i;
				}
				continue;
			}
			if (nextLine_[i] == '\\')
			{
				++i;
				continue;
			}
			if (isInQuote_)
			{
				if (nextLine_[i] == quoteChar_)
					isInQuote_ = false;
				continue;
			}

			if (nextLine_[i] == '"' || nextLine_[i] == '\'')
			{
				isInQuote_ = true;
				quoteChar_ = nextLine_[i];
				continue;
			}
			if (nextLine_.compare(i, 2, "//") == 0)
			{
				i = nextLine_.length();
				continue;
			}
			// handle preprocessor statement
			if (nextLine_[i] == '#')
			{
				string preproc = ASBeautifier::extractPreprocessorStatement(nextLine_);
				if (preproc.length() >= 2 && preproc.substr(0, 2) == "if") // #if, #ifdef, #ifndef
				{
					numBlockIndents += 1;
					isInIndentableBlock = true;
					// flag first preprocessor conditional for header include guard check
					if (!processedFirstConditional)
					{
						processedFirstConditional = true;
						isFirstPreprocConditional = true;
					}
				}
				else if (preproc == "endif")
				{
					if (numBlockIndents > 0)
						numBlockIndents -= 1;
					// must exit BOTH loops
					if (numBlockIndents == 0)
						goto EndOfWhileLoop;
				}
				else if (preproc == "define" && nextLine_[nextLine_.length() - 1] == '\\')
				{
					blockContainsDefineContinuation = true;
				}
				i = nextLine_.length();
				continue;
			}
			// handle exceptions
			if (nextLine_[i] == '{' || nextLine_[i] == '}')
				blockContainsBrackets = true;
			else if (nextLine_[i] == '(')
				++lineParenCount;
			else if (nextLine_[i] == ')')
				--lineParenCount;
			else if (nextLine_[i] == ':')
			{
				// check for '::'
				if (nextLine_.length() > i && nextLine_[i + 1] == ':')
					++i;
				else
					isInClassConstructor = true;
			}
			// bypass unnecessary parsing - must exit BOTH loops
			if (blockContainsBrackets || isInClassConstructor || blockContainsDefineContinuation)
				goto EndOfWhileLoop;
		}	// end of for loop, end of line
		if (lineParenCount != 0)
			break;
	}	// end of while loop
EndOfWhileLoop:
	preprocBlockEnd = sourceIterator->tellg();
	if (preprocBlockEnd < 0)
		preprocBlockEnd = sourceIterator->getStreamLength();
	if (blockContainsBrackets
	        || isInClassConstructor
	        || blockContainsDefineContinuation
	        || lineParenCount != 0
	        || numBlockIndents != 0)
		isInIndentableBlock = false;
	// find next executable instruction
	// this WILL RESET the get pointer
	string nextText = peekNextText("", false, needReset);
	// bypass header include guards, with an exception for small test files
	if (isFirstPreprocConditional)
	{
		isFirstPreprocConditional = false;
		if (nextText.empty() && sourceIterator->getStreamLength() > 250)
		{
			isInIndentableBlock = false;
			preprocBlockEnd = 0;
		}
	}
	// this allows preprocessor blocks within this block to be indented
	if (!isInIndentableBlock)
		preprocBlockEnd = 0;
	// peekReset() is done by previous peekNextText()
	return isInIndentableBlock;
}

/**
 * Check to see if this is an EXEC SQL statement.
 *
 * @param line          a reference to the line to indent.
 * @param index         the current line index.
 * @return              true if the statement is EXEC SQL.
 */
bool ASFormatter::isExecSQL(string  &line, size_t index) const
{
	if (line[index] != 'e' && line[index] != 'E')	// quick check to reject most
		return false;
	string word;
	if (isCharPotentialHeader(line, index))
		word = getCurrentWord(line, index);
	for (size_t i = 0; i < word.length(); i++)
		word[i] = (char) toupper(word[i]);
	if (word != "EXEC")
		return false;
	size_t index2 = index + word.length();
	index2 = line.find_first_not_of(" \t", index2);
	if (index2 == string::npos)
		return false;
	word.erase();
	if (isCharPotentialHeader(line, index2))
		word = getCurrentWord(line, index2);
	for (size_t i = 0; i < word.length(); i++)
		word[i] = (char) toupper(word[i]);
	if (word != "SQL")
		return false;
	return true;
}

/**
 * The continuation lines must be adjusted so the leading spaces
 *     is equivalent to the text on the opening line.
 *
 * Updates currentLine and charNum.
 */
void ASFormatter::trimContinuationLine()
{
	size_t len = currentLine.length();
	size_t tabSize = getTabLength();
	charNum = 0;

	if (leadingSpaces > 0 && len > 0)
	{
		size_t i;
		size_t continuationIncrementIn = 0;
		for (i = 0; (i < len) && (i + continuationIncrementIn < leadingSpaces); i++)
		{
			if (!isWhiteSpace(currentLine[i]))		// don't delete any text
			{
				if (i < continuationIncrementIn)
					leadingSpaces = i + tabIncrementIn;
				continuationIncrementIn = tabIncrementIn;
				break;
			}
			if (currentLine[i] == '\t')
				continuationIncrementIn += tabSize - 1 - ((continuationIncrementIn + i) % tabSize);
		}

		if ((int) continuationIncrementIn == tabIncrementIn)
			charNum = i;
		else
		{
			// build a new line with the equivalent leading chars
			string newLine;
			int leadingChars = 0;
			if ((int) leadingSpaces > tabIncrementIn)
				leadingChars = leadingSpaces - tabIncrementIn;
			newLine.append(leadingChars, ' ');
			newLine.append(currentLine, i, len - i);
			currentLine = newLine;
			charNum = leadingChars;
			if (currentLine.length() == 0)
				currentLine = string(" ");        // a null is inserted if this is not done
		}
		if (i >= len)
			charNum = 0;
	}
	return;
}

/**
 * Determine if a header is a closing header
 *
 * @return      true if the header is a closing header.
 */
bool ASFormatter::isClosingHeader(const string* header) const
{
	return (header == &AS_ELSE
	        || header == &AS_CATCH
	        || header == &AS_FINALLY);
}

/**
 * Determine if a * following a closing paren is immediately.
 * after a cast. If so it is a deference and not a multiply.
 * e.g. "(int*) *ptr" is a deference.
 */
bool ASFormatter::isImmediatelyPostCast() const
{
	assert(previousNonWSChar == ')' && currentChar == '*');
	// find preceding closing paren on currentLine or readyFormattedLine
	string line;		// currentLine or readyFormattedLine
	size_t paren = currentLine.rfind(")", charNum);
	if (paren != string::npos)
		line = currentLine;
	// if not on currentLine it must be on the previous line
	else
	{
		line = readyFormattedLine;
		paren = line.rfind(")");
		if (paren == string::npos)
			return false;
	}
	if (paren == 0)
		return false;

	// find character preceding the closing paren
	size_t lastChar = line.find_last_not_of(" \t", paren - 1);
	if (lastChar == string::npos)
		return false;
	// check for pointer cast
	if (line[lastChar] == '*')
		return true;
	return false;
}

/**
 * Determine if a < is a template definition or instantiation.
 * Sets the class variables isInTemplate and templateDepth.
 */
void ASFormatter::checkIfTemplateOpener()
{
	assert(!isInTemplate && currentChar == '<');

	// find first char after the '<' operators
	size_t firstChar = currentLine.find_first_not_of("< \t", charNum);
	if (firstChar == string::npos
	        || currentLine[firstChar] == '=')
	{
		// this is not a template -> leave...
		isInTemplate = false;
		return;
	}

	bool isFirstLine = true;
	bool needReset = false;
	int parenDepth_ = 0;
	int maxTemplateDepth = 0;
	templateDepth = 0;
	string nextLine_ = currentLine.substr(charNum);

	// find the angle brackets, bypassing all comments and quotes.
	bool isInComment_ = false;
	bool isInQuote_ = false;
	char quoteChar_ = ' ';
	while (sourceIterator->hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
		{
			nextLine_ = sourceIterator->peekNextLine();
			needReset = true;
		}
		// parse the line
		for (size_t i = 0; i < nextLine_.length(); i++)
		{
			char currentChar_ = nextLine_[i];
			if (isWhiteSpace(currentChar_))
				continue;
			if (nextLine_.compare(i, 2, "/*") == 0)
				isInComment_ = true;
			if (isInComment_)
			{
				if (nextLine_.compare(i, 2, "*/") == 0)
				{
					isInComment_ = false;
					++i;
				}
				continue;
			}
			if (currentChar_ == '\\')
			{
				++i;
				continue;
			}

			if (isInQuote_)
			{
				if (currentChar_ == quoteChar_)
					isInQuote_ = false;
				continue;
			}

			if (currentChar_ == '"' || currentChar_ == '\'')
			{
				isInQuote_ = true;
				quoteChar_ = currentChar_;
				continue;
			}
			if (nextLine_.compare(i, 2, "//") == 0)
			{
				i = nextLine_.length();
				continue;
			}

			// not in a comment or quote
			if (currentChar_ == '<')
			{
				++templateDepth;
				++maxTemplateDepth;
				continue;
			}
			else if (currentChar_ == '>')
			{
				--templateDepth;
				if (templateDepth == 0)
				{
					if (parenDepth_ == 0)
					{
						// this is a template!
						isInTemplate = true;
						templateDepth = maxTemplateDepth;
					}
					goto exitFromSearch;
				}
				continue;
			}
			else if (currentChar_ == '(' || currentChar_ == ')')
			{
				if (currentChar_ == '(')
					++parenDepth_;
				else
					--parenDepth_;
				if (parenDepth_ >= 0)
					continue;
				// this is not a template -> leave...
				isInTemplate = false;
				goto exitFromSearch;
			}
			else if (nextLine_.compare(i, 2, AS_AND) == 0
			         || nextLine_.compare(i, 2, AS_OR) == 0)
			{
				// this is not a template -> leave...
				isInTemplate = false;
				goto exitFromSearch;
			}
			else if (currentChar_ == ','  // comma,     e.g. A<int, char>
			         || currentChar_ == '&'    // reference, e.g. A<int&>
			         || currentChar_ == '*'    // pointer,   e.g. A<int*>
			         || currentChar_ == '^'    // C++/CLI managed pointer, e.g. A<int^>
			         || currentChar_ == ':'    // ::,        e.g. std::string
			         || currentChar_ == '='    // assign     e.g. default parameter
			         || currentChar_ == '['    // []         e.g. string[]
			         || currentChar_ == ']'    // []         e.g. string[]
			         || currentChar_ == '('    // (...)      e.g. function definition
			         || currentChar_ == ')'    // (...)      e.g. function definition
			         || (isJavaStyle() && currentChar_ == '?')   // Java wildcard
			        )
			{
				continue;
			}
			else if (!isLegalNameChar(currentChar_))
			{
				// this is not a template -> leave...
				isInTemplate = false;
				templateDepth = 0;
				goto exitFromSearch;
			}
			string name = getCurrentWord(nextLine_, i);
			i += name.length() - 1;
		}	// end of for loop
	}	// end of while loop

	// goto needed to exit from two loops
exitFromSearch:
	if (needReset)
		sourceIterator->peekReset();
}

void ASFormatter::updateFormattedLineSplitPoints(char appendedChar)
{
	assert(maxCodeLength != string::npos);
	assert(formattedLine.length() > 0);

	if (!isOkToSplitFormattedLine())
		return;

	char nextChar = peekNextChar();

	// don't split before an end of line comment
	if (nextChar == '/')
		return;

	// don't split before or after a bracket
	if (appendedChar == '{' || appendedChar == '}'
	        || previousNonWSChar == '{' || previousNonWSChar == '}'
	        || nextChar == '{' || nextChar == '}'
	        || currentChar == '{' || currentChar == '}')	// currentChar tests for an appended bracket
		return;

	// don't split before or after a block paren
	if (appendedChar == '[' || appendedChar == ']'
	        || previousNonWSChar == '['
	        || nextChar == '[' || nextChar == ']')
		return;

	if (isWhiteSpace(appendedChar))
	{
		if (nextChar != ')'						// space before a closing paren
		        && nextChar != '('				// space before an opening paren
		        && nextChar != '/'				// space before a comment
		        && nextChar != ':'				// space before a colon
		        && currentChar != ')'			// appended space before and after a closing paren
		        && currentChar != '('			// appended space before and after a opening paren
		        && previousNonWSChar != '('		// decided at the '('
		        // don't break before a pointer or reference aligned to type
		        && !(nextChar == '*'
		             && !isCharPotentialOperator(previousNonWSChar)
		             && pointerAlignment == PTR_ALIGN_TYPE)
		        && !(nextChar == '&'
		             && !isCharPotentialOperator(previousNonWSChar)
		             && (referenceAlignment == REF_ALIGN_TYPE
		                 || (referenceAlignment == REF_SAME_AS_PTR && pointerAlignment == PTR_ALIGN_TYPE)))
		   )
		{
			if (formattedLine.length() - 1 <= maxCodeLength)
				maxWhiteSpace = formattedLine.length() - 1;
			else
				maxWhiteSpacePending = formattedLine.length() - 1;
		}
	}
	// unpadded closing parens may split after the paren (counts as whitespace)
	else if (appendedChar == ')')
	{
		if (nextChar != ')'
		        && nextChar != ' '
		        && nextChar != ';'
		        && nextChar != ','
		        && nextChar != '.'
		        && !(nextChar == '-' && pointerSymbolFollows()))	// check for ->
		{
			if (formattedLine.length() <= maxCodeLength)
				maxWhiteSpace = formattedLine.length();
			else
				maxWhiteSpacePending = formattedLine.length();
		}
	}
	// unpadded commas may split after the comma
	else if (appendedChar == ',')
	{
		if (formattedLine.length() <= maxCodeLength)
			maxComma = formattedLine.length();
		else
			maxCommaPending = formattedLine.length();
	}
	else if (appendedChar == '(')
	{
		if (nextChar != ')' && nextChar != '(' && nextChar != '"' && nextChar != '\'')
		{
			// if follows an operator break before
			size_t parenNum;
			if (isCharPotentialOperator(previousNonWSChar))
				parenNum = formattedLine.length() - 1 ;
			else
				parenNum = formattedLine.length();
			if (formattedLine.length() <= maxCodeLength)
				maxParen = parenNum;
			else
				maxParenPending = parenNum;
		}
	}
	else if (appendedChar == ';')
	{
		if (nextChar != ' '  && nextChar != '}' && nextChar != '/')	// check for following comment
		{
			if (formattedLine.length() <= maxCodeLength)
				maxSemi = formattedLine.length();
			else
				maxSemiPending = formattedLine.length();
		}
	}
}

void ASFormatter::updateFormattedLineSplitPointsOperator(const string &sequence)
{
	assert(maxCodeLength != string::npos);
	assert(formattedLine.length() > 0);

	if (!isOkToSplitFormattedLine())
		return;

	char nextChar = peekNextChar();

	// don't split before an end of line comment
	if (nextChar == '/')
		return;

	// check for logical conditional
	if (sequence == "||" || sequence ==  "&&" || sequence ==  "or" || sequence ==  "and")
	{
		if (shouldBreakLineAfterLogical)
		{
			if (formattedLine.length() <= maxCodeLength)
				maxAndOr = formattedLine.length();
			else
				maxAndOrPending = formattedLine.length();
		}
		else
		{
			// adjust for leading space in the sequence
			size_t sequenceLength = sequence.length();
			if (formattedLine.length() > sequenceLength
			        && isWhiteSpace(formattedLine[formattedLine.length() - sequenceLength - 1]))
				sequenceLength++;
			if (formattedLine.length() - sequenceLength <= maxCodeLength)
				maxAndOr = formattedLine.length() - sequenceLength;
			else
				maxAndOrPending = formattedLine.length() - sequenceLength;
		}
	}
	// comparison operators will split after the operator (counts as whitespace)
	else if (sequence == "==" || sequence ==  "!=" || sequence ==  ">=" || sequence ==  "<=")
	{
		if (formattedLine.length() <= maxCodeLength)
			maxWhiteSpace = formattedLine.length();
		else
			maxWhiteSpacePending = formattedLine.length();
	}
	// unpadded operators that will split BEFORE the operator (counts as whitespace)
	else if (sequence == "+" || sequence == "-" || sequence == "?")
	{
		if (charNum > 0
		        && (isLegalNameChar(currentLine[charNum - 1])
		            || currentLine[charNum - 1] == ')'
		            || currentLine[charNum - 1] == ']'
		            || currentLine[charNum - 1] == '\"'))
		{
			if (formattedLine.length() - 1 <=  maxCodeLength)
				maxWhiteSpace = formattedLine.length() - 1;
			else
				maxWhiteSpacePending = formattedLine.length() - 1;
		}
	}
	// unpadded operators that will USUALLY split AFTER the operator (counts as whitespace)
	else if (sequence == "=" || sequence == ":")
	{
		// split BEFORE if the line is too long
		// do NOT use <= here, must allow for a bracket attached to an array
		size_t splitPoint = 0;
		if (formattedLine.length() <  maxCodeLength)
			splitPoint = formattedLine.length();
		else
			splitPoint = formattedLine.length() - 1;
		// padded or unpadded arrays
		if (previousNonWSChar == ']')
		{
			if (formattedLine.length() - 1 <=  maxCodeLength)
				maxWhiteSpace = splitPoint;
			else
				maxWhiteSpacePending = splitPoint;
		}
		else if (charNum > 0
		         && (isLegalNameChar(currentLine[charNum - 1])
		             || currentLine[charNum - 1] == ')'
		             || currentLine[charNum - 1] == ']'))
		{
			if (formattedLine.length() <=  maxCodeLength)
				maxWhiteSpace = splitPoint;
			else
				maxWhiteSpacePending = splitPoint;
		}
	}
}

/**
 * Update the split point when a pointer or reference is formatted.
 * The argument is the maximum index of the last whitespace character.
 */
void ASFormatter::updateFormattedLineSplitPointsPointerOrReference(size_t index)
{
	assert(maxCodeLength != string::npos);
	assert(formattedLine.length() > 0);
	assert(index < formattedLine.length());

	if (!isOkToSplitFormattedLine())
		return;

	if (index < maxWhiteSpace)		// just in case
		return;

	if (index <= maxCodeLength)
		maxWhiteSpace = index;
	else
		maxWhiteSpacePending = index;
}

bool ASFormatter::isOkToSplitFormattedLine()
{
	assert(maxCodeLength != string::npos);
	// Is it OK to split the line?
	if (shouldKeepLineUnbroken
	        || isInLineComment
	        || isInComment
	        || isInQuote
	        || isInCase
	        || isInPreprocessor
	        || isInExecSQL
	        || isInAsm || isInAsmOneLine || isInAsmBlock
	        || isInTemplate)
		return false;

	if (!isOkToBreakBlock(bracketTypeStack->back()) && currentChar != '{')
	{
		shouldKeepLineUnbroken = true;
		clearFormattedLineSplitPoints();
		return false;
	}
	else if (isBracketType(bracketTypeStack->back(), ARRAY_TYPE))
	{
		shouldKeepLineUnbroken = true;
		if (!isBracketType(bracketTypeStack->back(), ARRAY_NIS_TYPE))
			clearFormattedLineSplitPoints();
		return false;
	}
	return true;
}

/* This is called if the option maxCodeLength is set.
 */
void ASFormatter::testForTimeToSplitFormattedLine()
{
	//	DO NOT ASSERT maxCodeLength HERE
	// should the line be split
	if (formattedLine.length() > maxCodeLength && !isLineReady)
	{
		size_t splitPoint = findFormattedLineSplitPoint();
		if (splitPoint > 0 && splitPoint < formattedLine.length())
		{
			string splitLine = formattedLine.substr(splitPoint);
			formattedLine = formattedLine.substr(0, splitPoint);
			breakLine(true);
			formattedLine = splitLine;
			// if break-blocks is requested and this is a one-line statement
			string nextWord = ASBeautifier::getNextWord(currentLine, charNum - 1);
			if (isAppendPostBlockEmptyLineRequested
			        && (nextWord == "break" || nextWord == "continue"))
			{
				isAppendPostBlockEmptyLineRequested = false;
				isPrependPostBlockEmptyLineRequested = true;
			}
			else
				isPrependPostBlockEmptyLineRequested = false;
			// adjust max split points
			maxAndOr = (maxAndOr > splitPoint) ? (maxAndOr - splitPoint) : 0;
			maxSemi = (maxSemi > splitPoint) ? (maxSemi - splitPoint) : 0;
			maxComma = (maxComma > splitPoint) ? (maxComma - splitPoint) : 0;
			maxParen = (maxParen > splitPoint) ? (maxParen - splitPoint) : 0;
			maxWhiteSpace = (maxWhiteSpace > splitPoint) ? (maxWhiteSpace - splitPoint) : 0;
			if (maxSemiPending > 0)
			{
				maxSemi = (maxSemiPending > splitPoint) ? (maxSemiPending - splitPoint) : 0;
				maxSemiPending = 0;
			}
			if (maxAndOrPending > 0)
			{
				maxAndOr = (maxAndOrPending > splitPoint) ? (maxAndOrPending - splitPoint) : 0;
				maxAndOrPending = 0;
			}
			if (maxCommaPending > 0)
			{
				maxComma = (maxCommaPending > splitPoint) ? (maxCommaPending - splitPoint) : 0;
				maxCommaPending = 0;
			}
			if (maxParenPending > 0)
			{
				maxParen = (maxParenPending > splitPoint) ? (maxParenPending - splitPoint) : 0;
				maxParenPending = 0;
			}
			if (maxWhiteSpacePending > 0)
			{
				maxWhiteSpace = (maxWhiteSpacePending > splitPoint) ? (maxWhiteSpacePending - splitPoint) : 0;
				maxWhiteSpacePending = 0;
			}
			// don't allow an empty formatted line
			size_t firstText = formattedLine.find_first_not_of(" \t");
			if (firstText == string::npos && formattedLine.length() > 0)
			{
				formattedLine.erase();
				clearFormattedLineSplitPoints();
				if (isWhiteSpace(currentChar))
					for (size_t i = charNum + 1; i < currentLine.length() && isWhiteSpace(currentLine[i]); i++)
						goForward(1);
			}
			else if (firstText > 0)
			{
				formattedLine.erase(0, firstText);
				maxSemi = (maxSemi > firstText) ? (maxSemi - firstText) : 0;
				maxAndOr = (maxAndOr > firstText) ? (maxAndOr - firstText) : 0;
				maxComma = (maxComma > firstText) ? (maxComma - firstText) : 0;
				maxParen = (maxParen > firstText) ? (maxParen - firstText) : 0;
				maxWhiteSpace = (maxWhiteSpace > firstText) ? (maxWhiteSpace - firstText) : 0;
			}
			// reset formattedLineCommentNum
			if (formattedLineCommentNum != string::npos)
			{
				formattedLineCommentNum = formattedLine.find("//");
				if (formattedLineCommentNum == string::npos)
					formattedLineCommentNum = formattedLine.find("/*");
			}
		}
	}
}

size_t ASFormatter::findFormattedLineSplitPoint() const
{
	assert(maxCodeLength != string::npos);
	// determine where to split
	size_t minCodeLength = 10;
	size_t splitPoint = 0;
	splitPoint = maxSemi;
	if (maxAndOr >= minCodeLength)
		splitPoint = maxAndOr;
	if (splitPoint < minCodeLength)
	{
		splitPoint = maxWhiteSpace;
		// use maxParen instead if it is long enough
		if (maxParen > splitPoint
		        || maxParen >= maxCodeLength * .7)
			splitPoint = maxParen;
		// use maxComma instead if it is long enough
		// increasing the multiplier causes more splits at whitespace
		if (maxComma > splitPoint
		        || maxComma >= maxCodeLength * .3)
			splitPoint = maxComma;
	}
	// replace split point with first available break point
	if (splitPoint < minCodeLength)
	{
		splitPoint = string::npos;
		if (maxSemiPending > 0 && maxSemiPending < splitPoint)
			splitPoint = maxSemiPending;
		if (maxAndOrPending > 0 && maxAndOrPending < splitPoint)
			splitPoint = maxAndOrPending;
		if (maxCommaPending > 0 && maxCommaPending < splitPoint)
			splitPoint = maxCommaPending;
		if (maxParenPending > 0 && maxParenPending < splitPoint)
			splitPoint = maxParenPending;
		if (maxWhiteSpacePending > 0 && maxWhiteSpacePending < splitPoint)
			splitPoint = maxWhiteSpacePending;
		if (splitPoint == string::npos)
			splitPoint = 0;
	}
	// if remaining line after split is too long
	else if (formattedLine.length() - splitPoint > maxCodeLength)
	{
		// if end of the currentLine, find a new split point
		size_t newCharNum;
		if (isCharPotentialHeader(currentLine, charNum))
			newCharNum = getCurrentWord(currentLine, charNum).length() + charNum;
		else
			newCharNum = charNum + 2;
		if (newCharNum + 1 > currentLine.length())
		{
			// don't move splitPoint from before a conditional to after
			if (maxWhiteSpace > splitPoint + 3)
				splitPoint = maxWhiteSpace;
			if (maxParen > splitPoint)
				splitPoint = maxParen;
		}
	}

	return splitPoint;
}

void ASFormatter::clearFormattedLineSplitPoints()
{
	maxSemi = 0;
	maxAndOr = 0;
	maxComma = 0;
	maxParen = 0;
	maxWhiteSpace = 0;
	maxSemiPending = 0;
	maxAndOrPending = 0;
	maxCommaPending = 0;
	maxParenPending = 0;
	maxWhiteSpacePending = 0;
}

/**
 * Check if a pointer symbol (->) follows on the currentLine.
 */
bool ASFormatter::pointerSymbolFollows() const
{
	size_t peekNum = currentLine.find_first_not_of(" \t", charNum + 1);
	if (peekNum == string::npos || currentLine.compare(peekNum, 2, "->") != 0)
		return false;
	return true;
}

/**
 * Compute the input checksum.
 * This is called as an assert so it for is debug config only
 */
bool ASFormatter::computeChecksumIn(const string &currentLine_)
{
	for (size_t i = 0; i < currentLine_.length(); i++)
		if (!isWhiteSpace(currentLine_[i]))
			checksumIn += currentLine_[i];
	return true;
}

/**
 * Adjust the input checksum for deleted chars.
 * This is called as an assert so it for is debug config only
 */
bool ASFormatter::adjustChecksumIn(int adjustment)
{
	checksumIn += adjustment;
	return true;
}

/**
 * get the value of checksumIn for unit testing
 *
 * @return   checksumIn.
 */
size_t ASFormatter::getChecksumIn() const
{
	return checksumIn;
}

/**
 * Compute the output checksum.
 * This is called as an assert so it is for debug config only
 */
bool ASFormatter::computeChecksumOut(const string &beautifiedLine)
{
	for (size_t i = 0; i < beautifiedLine.length(); i++)
		if (!isWhiteSpace(beautifiedLine[i]))
			checksumOut += beautifiedLine[i];
	return true;
}

/**
 * Return isLineReady for the final check at end of file.
 */
bool ASFormatter::getIsLineReady() const
{
	return isLineReady;
}

/**
 * get the value of checksumOut for unit testing
 *
 * @return   checksumOut.
 */
size_t ASFormatter::getChecksumOut() const
{
	return checksumOut;
}

/**
 * Return the difference in checksums.
 * If zero all is okay.
 */
int ASFormatter::getChecksumDiff() const
{
	return checksumOut - checksumIn;
}

// for unit testing
int ASFormatter::getFormatterFileType() const
{
	return formatterFileType;
}

// Check if an operator follows the next word.
// The next word must be a legal name.
const string* ASFormatter::getFollowingOperator() const
{
	// find next word
	size_t nextNum = currentLine.find_first_not_of(" \t", charNum + 1);
	if (nextNum == string::npos)
		return NULL;

	if (!isLegalNameChar(currentLine[nextNum]))
		return NULL;

	// bypass next word and following spaces
	while (nextNum < currentLine.length())
	{
		if (!isLegalNameChar(currentLine[nextNum])
		        && !isWhiteSpace(currentLine[nextNum]))
			break;
		nextNum++;
	}

	if (nextNum >= currentLine.length()
	        || !isCharPotentialOperator(currentLine[nextNum])
	        || currentLine[nextNum] == '/')		// comment
		return NULL;

	const string* newOperator = ASBeautifier::findOperator(currentLine, nextNum, operators);
	return newOperator;
}

// Check following data to determine if the current character is an array operator.
bool ASFormatter::isArrayOperator() const
{
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');
	assert(isBracketType(bracketTypeStack->back(), ARRAY_TYPE));

	// find next word
	size_t nextNum = currentLine.find_first_not_of(" \t", charNum + 1);
	if (nextNum == string::npos)
		return false;

	if (!isLegalNameChar(currentLine[nextNum]))
		return false;

	// bypass next word and following spaces
	while (nextNum < currentLine.length())
	{
		if (!isLegalNameChar(currentLine[nextNum])
		        && !isWhiteSpace(currentLine[nextNum]))
			break;
		nextNum++;
	}

	// check for characters that indicate an operator
	if (currentLine[nextNum] == ','
	        || currentLine[nextNum] == '}'
	        || currentLine[nextNum] == ')'
	        || currentLine[nextNum] == '(')
		return true;
	return false;
}

// Reset the flags that indicate various statement information.
void ASFormatter::resetEndOfStatement()
{
	foundQuestionMark = false;
	foundNamespaceHeader = false;
	foundClassHeader = false;
	foundStructHeader = false;
	foundInterfaceHeader = false;
	foundPreDefinitionHeader = false;
	foundPreCommandHeader = false;
	foundPreCommandMacro = false;
	foundCastOperator = false;
	isInPotentialCalculation = false;
	isSharpAccessor = false;
	isSharpDelegate = false;
	isInObjCMethodDefinition = false;
	isInObjCInterface = false;
	isInObjCSelector = false;
	isInEnum = false;
	isInExternC = false;
	elseHeaderFollowsComments = false;
	nonInStatementBracket = 0;
	while (!questionMarkStack->empty())
		questionMarkStack->pop_back();
}

// pad an Objective-C method colon
void ASFormatter::padObjCMethodColon()
{
	assert(currentChar == ':');
	char nextChar = peekNextChar();
	if (objCColonPadMode == COLON_PAD_NONE
	        || objCColonPadMode == COLON_PAD_AFTER
	        || nextChar == ')')
	{
		// remove spaces before
		for (int i = formattedLine.length() - 1; (i > -1) && isWhiteSpace(formattedLine[i]); i--)
			formattedLine.erase(i);
	}
	else
	{
		// pad space before
		for (int i = formattedLine.length() - 1; (i > 0) && isWhiteSpace(formattedLine[i]); i--)
			if (isWhiteSpace(formattedLine[i - 1]))
				formattedLine.erase(i);
		appendSpacePad();
	}
	if (objCColonPadMode == COLON_PAD_NONE
	        || objCColonPadMode == COLON_PAD_BEFORE
	        || nextChar == ')')
	{
		// remove spaces after
		// do not need to bump i since a char is erased
		size_t i = charNum + 1;
		while ((i < currentLine.length()) && isWhiteSpace(currentLine[i]))
			currentLine.erase(i, 1);
	}
	else
	{
		// pad space after
		// do not need to bump i since a char is erased
		size_t i = charNum + 1;
		while ((i + 1 < currentLine.length()) && isWhiteSpace(currentLine[i]))
			currentLine.erase(i, 1);
		if (((int) currentLine.length() > charNum + 1) && !isWhiteSpace(currentLine[charNum + 1]))
			currentLine.insert(charNum + 1, " ");
	}
}

// Remove the leading '*' from a comment line and indent to the next tab.
void ASFormatter::stripCommentPrefix()
{
	int firstChar = formattedLine.find_first_not_of(" \t");
	if (firstChar < 0)
		return;

	if (isInCommentStartLine)
	{
		// comment opener must begin the line
		if (formattedLine.compare(firstChar, 2, "/*") != 0)
			return;
		int commentOpener = firstChar;
		// ignore single line comments
		int commentEnd = formattedLine.find("*/", firstChar + 2);
		if (commentEnd != -1)
			return;
		// first char after the comment opener must be at least one indent
		int followingText = formattedLine.find_first_not_of(" \t", commentOpener + 2);
		if (followingText < 0)
			return;
		if (formattedLine[followingText] == '*' || formattedLine[followingText] == '!')
			followingText = formattedLine.find_first_not_of(" \t", followingText + 1);
		if (followingText < 0)
			return;
		if (formattedLine[followingText] == '*')
			return;
		int indentLen = getIndentLength();
		int followingTextIndent = followingText - commentOpener;
		if (followingTextIndent < indentLen)
		{
			string stringToInsert(indentLen - followingTextIndent, ' ');
			formattedLine.insert(followingText, stringToInsert);
		}
		return;
	}
	// comment body including the closer
	else if (formattedLine[firstChar] == '*')
	{
		if (formattedLine.compare(firstChar, 2, "*/") == 0)
		{
			// line starts with an end comment
			formattedLine = "*/";
		}
		else
		{
			// build a new line with one indent
			int secondChar = formattedLine.find_first_not_of(" \t", firstChar + 1);
			if (secondChar < 0)
			{
				adjustChecksumIn(-'*');
				formattedLine.erase();
				return;
			}
			if (formattedLine[secondChar] == '*')
				return;
			// replace the leading '*'
			int indentLen = getIndentLength();
			adjustChecksumIn(-'*');
			// second char must be at least one indent
			if (formattedLine.substr(0, secondChar).find('\t') != string::npos)
			{
				formattedLine.erase(firstChar, 1);
			}
			else
			{
				int spacesToInsert = 0;
				if (secondChar >= indentLen)
					spacesToInsert = secondChar;
				else
					spacesToInsert = indentLen;
				formattedLine = string(spacesToInsert, ' ') + formattedLine.substr(secondChar);
			}
			// remove a trailing '*'
			int lastChar = formattedLine.find_last_not_of(" \t");
			if (lastChar > -1 && formattedLine[lastChar] == '*')
			{
				adjustChecksumIn(-'*');
				formattedLine[lastChar] = ' ';
			}
		}
	}
	else
	{
		// first char not a '*'
		// first char must be at least one indent
		if (formattedLine.substr(0, firstChar).find('\t') == string::npos)
		{
			int indentLen = getIndentLength();
			if (firstChar < indentLen)
			{
				string stringToInsert(indentLen, ' ');
				formattedLine = stringToInsert + formattedLine.substr(firstChar);
			}
		}
	}
}

}   // end namespace astyle
