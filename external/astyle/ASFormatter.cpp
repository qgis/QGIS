// ASFormatter.cpp
// Copyright (c) 2017 by Jim Pattee <jimp03@email.com>.
// This code is licensed under the MIT License.
// License.md describes the conditions under which this software may be distributed.

//-----------------------------------------------------------------------------
// headers
//-----------------------------------------------------------------------------

#include "astyle.h"

#include <algorithm>
#include <fstream>

//-----------------------------------------------------------------------------
// astyle namespace
//-----------------------------------------------------------------------------

namespace astyle {
//
//-----------------------------------------------------------------------------
// ASFormatter class
//-----------------------------------------------------------------------------

/**
 * Constructor of ASFormatter
 */
ASFormatter::ASFormatter()
{
	sourceIterator = nullptr;
	enhancer = new ASEnhancer;
	preBraceHeaderStack = nullptr;
	braceTypeStack = nullptr;
	parenStack = nullptr;
	structStack = nullptr;
	questionMarkStack = nullptr;
	lineCommentNoIndent = false;
	formattingStyle = STYLE_NONE;
	braceFormatMode = NONE_MODE;
	pointerAlignment = PTR_ALIGN_NONE;
	referenceAlignment = REF_SAME_AS_PTR;
	objCColonPadMode = COLON_PAD_NO_CHANGE;
	lineEnd = LINEEND_DEFAULT;
	maxCodeLength = string::npos;
	shouldPadCommas = false;
	shouldPadOperators = false;
	shouldPadParensOutside = false;
	shouldPadFirstParen = false;
	shouldPadParensInside = false;
	shouldPadHeader = false;
	shouldStripCommentPrefix = false;
	shouldUnPadParens = false;
	attachClosingBraceMode = false;
	shouldBreakOneLineBlocks = true;
	shouldBreakOneLineHeaders = false;
	shouldBreakOneLineStatements = true;
	shouldConvertTabs = false;
	shouldIndentCol1Comments = false;
	shouldIndentPreprocBlock = false;
	shouldCloseTemplates = false;
	shouldAttachExternC = false;
	shouldAttachNamespace = false;
	shouldAttachClass = false;
	shouldAttachClosingWhile = false;
	shouldAttachInline = false;
	shouldBreakBlocks = false;
	shouldBreakClosingHeaderBlocks = false;
	shouldBreakClosingHeaderBraces = false;
	shouldDeleteEmptyLines = false;
	shouldBreakElseIfs = false;
	shouldBreakLineAfterLogical = false;
	shouldAddBraces = false;
	shouldAddOneLineBraces = false;
	shouldRemoveBraces = false;
	shouldPadMethodColon = false;
	shouldPadMethodPrefix = false;
	shouldUnPadMethodPrefix = false;
	shouldPadReturnType = false;
	shouldUnPadReturnType = false;
	shouldPadParamType = false;
	shouldUnPadParamType = false;

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
	deleteContainer(preBraceHeaderStack);
	deleteContainer(braceTypeStack);
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

	// must be done when the ASFormatter object is deleted (not ASBeautifier)
	// delete ASBeautifier member vectors
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
	               getIndentString() == "\t",
	               getForceTabIndentation(),
	               getNamespaceIndent(),
	               getCaseIndent(),
	               shouldIndentPreprocBlock,
	               getPreprocDefineIndent(),
	               getEmptyLineFill(),
	               indentableMacros);

	initContainer(preBraceHeaderStack, new vector<const string*>);
	initContainer(parenStack, new vector<int>);
	initContainer(structStack, new vector<bool>);
	initContainer(questionMarkStack, new vector<bool>);
	parenStack->emplace_back(0);               // parenStack must contain this default entry
	initContainer(braceTypeStack, new vector<BraceType>);
	braceTypeStack->emplace_back(NULL_TYPE);   // braceTypeStack must contain this default entry
	clearFormattedLineSplitPoints();

	currentHeader = nullptr;
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
	currentLineFirstBraceNum = string::npos;
	formattedLineCommentNum = 0;
	leadingSpaces = 0;
	previousReadyFormattedLineLength = string::npos;
	preprocBraceTypeStackSize = 0;
	spacePadNum = 0;
	nextLineSpacePadNum = 0;
	objCColonAlign = 0;
	templateDepth = 0;
	squareBracketCount = 0;
	runInIndentChars = 0;
	tabIncrementIn = 0;
	previousBraceType = NULL_TYPE;

	isVirgin = true;
	isInVirginLine = true;
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
	foundTrailingReturnType = false;
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
	isPreviousBraceBlockRelated = false;
	isInPotentialCalculation = false;
	needHeaderOpeningBrace = false;
	shouldBreakLineAtNextChar = false;
	shouldKeepLineUnbroken = false;
	shouldReparseCurrentChar = false;
	passedSemicolon = false;
	passedColon = false;
	isImmediatelyPostNonInStmt = false;
	isCharImmediatelyPostNonInStmt = false;
	isInTemplate = false;
	isImmediatelyPostComment = false;
	isImmediatelyPostLineComment = false;
	isImmediatelyPostEmptyBlock = false;
	isImmediatelyPostObjCMethodPrefix = false;
	isImmediatelyPostPreprocessor = false;
	isImmediatelyPostReturn = false;
	isImmediatelyPostThrow = false;
	isImmediatelyPostNewDelete = false;
	isImmediatelyPostOperator = false;
	isImmediatelyPostTemplate = false;
	isImmediatelyPostPointerOrReference = false;
	isCharImmediatelyPostReturn = false;
	isCharImmediatelyPostThrow = false;
	isCharImmediatelyPostNewDelete = false;
	isCharImmediatelyPostOperator = false;
	isCharImmediatelyPostComment = false;
	isPreviousCharPostComment = false;
	isCharImmediatelyPostLineComment = false;
	isCharImmediatelyPostOpenBlock = false;
	isCharImmediatelyPostCloseBlock = false;
	isCharImmediatelyPostTemplate = false;
	isCharImmediatelyPostPointerOrReference = false;
	isInObjCInterface = false;
	isInObjCMethodDefinition = false;
	isInObjCReturnType = false;
	isInObjCSelector = false;
	breakCurrentOneLineBlock = false;
	shouldRemoveNextClosingBrace = false;
	isInBraceRunIn = false;
	currentLineBeginsWithBrace = false;
	isPrependPostBlockEmptyLineRequested = false;
	isAppendPostBlockEmptyLineRequested = false;
	isIndentableProprocessor = false;
	isIndentableProprocessorBlock = false;
	prependEmptyLine = false;
	appendOpeningBrace = false;
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
		setBraceFormatMode(BREAK_MODE);
	}
	else if (formattingStyle == STYLE_JAVA)
	{
		setBraceFormatMode(ATTACH_MODE);
	}
	else if (formattingStyle == STYLE_KR)
	{
		setBraceFormatMode(LINUX_MODE);
	}
	else if (formattingStyle == STYLE_STROUSTRUP)
	{
		setBraceFormatMode(LINUX_MODE);
		setBreakClosingHeaderBracesMode(true);
	}
	else if (formattingStyle == STYLE_WHITESMITH)
	{
		setBraceFormatMode(BREAK_MODE);
		setBraceIndent(true);
		setClassIndent(true);			// avoid hanging indent with access modifiers
		setSwitchIndent(true);			// avoid hanging indent with case statements
	}
	else if (formattingStyle == STYLE_VTK)
	{
		// the unindented class brace does NOT cause a hanging indent like Whitesmith
		setBraceFormatMode(BREAK_MODE);
		setBraceIndentVtk(true);		// sets both braceIndent and braceIndentVtk
		setSwitchIndent(true);			// avoid hanging indent with case statements
	}
	else if (formattingStyle == STYLE_BANNER)
	{
		// attached braces can have hanging indents with the closing brace
		setBraceFormatMode(ATTACH_MODE);
		setBraceIndent(true);
		setClassIndent(true);			// avoid hanging indent with access modifiers
		setSwitchIndent(true);			// avoid hanging indent with case statements
	}
	else if (formattingStyle == STYLE_GNU)
	{
		setBraceFormatMode(BREAK_MODE);
		setBlockIndent(true);
	}
	else if (formattingStyle == STYLE_LINUX)
	{
		setBraceFormatMode(LINUX_MODE);
		// always for Linux style
		setMinConditionalIndentOption(MINCOND_ONEHALF);
	}
	else if (formattingStyle == STYLE_HORSTMANN)
	{
		setBraceFormatMode(RUN_IN_MODE);
		setSwitchIndent(true);
	}
	else if (formattingStyle == STYLE_1TBS)
	{
		setBraceFormatMode(LINUX_MODE);
		setAddBracesMode(true);
		setRemoveBracesMode(false);
	}
	else if (formattingStyle == STYLE_GOOGLE)
	{
		setBraceFormatMode(ATTACH_MODE);
		setModifierIndent(true);
		setClassIndent(false);
	}
	else if (formattingStyle == STYLE_MOZILLA)
	{
		setBraceFormatMode(LINUX_MODE);
	}
	else if (formattingStyle == STYLE_PICO)
	{
		setBraceFormatMode(RUN_IN_MODE);
		setAttachClosingBraceMode(true);
		setSwitchIndent(true);
		setBreakOneLineBlocksMode(false);
		setBreakOneLineStatementsMode(false);
		// add-braces won't work for pico, but it could be fixed if necessary
		// both options should be set to true
		if (shouldAddBraces)
			shouldAddOneLineBraces = true;
	}
	else if (formattingStyle == STYLE_LISP)
	{
		setBraceFormatMode(ATTACH_MODE);
		setAttachClosingBraceMode(true);
		setBreakOneLineStatementsMode(false);
		// add-one-line-braces won't work for lisp
		// only shouldAddBraces should be set to true
		if (shouldAddOneLineBraces)
		{
			shouldAddBraces = true;
			shouldAddOneLineBraces = false;
		}
	}
	setMinConditionalIndentLength();
	// if not set by indent=force-tab-x set equal to indentLength
	if (getTabLength() == 0)
		setDefaultTabLength();
	// add-one-line-braces implies keep-one-line-blocks
	if (shouldAddOneLineBraces)
		setBreakOneLineBlocksMode(false);
	// don't allow add-braces and remove-braces
	if (shouldAddBraces || shouldAddOneLineBraces)
		setRemoveBracesMode(false);
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
	const string* newHeader = nullptr;
	isInVirginLine = isVirgin;
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
			        && currentLineBeginsWithBrace
			        && previousCommandChar == ' ')
				previousCommandChar = '{';
			if (isInClassInitializer
			        && isBraceType(braceTypeStack->back(), COMMAND_TYPE))
				isInClassInitializer = false;
			if (isInBraceRunIn)
				isInLineBreak = false;
			if (!isWhiteSpace(currentChar))
				isInBraceRunIn = false;
			isPreviousCharPostComment = isCharImmediatelyPostComment;
			isCharImmediatelyPostComment = false;
			isCharImmediatelyPostTemplate = false;
			isCharImmediatelyPostReturn = false;
			isCharImmediatelyPostThrow = false;
			isCharImmediatelyPostNewDelete = false;
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
		else if (currentChar == '"'
		         || (currentChar == '\'' && !isDigitSeparator(currentLine, charNum)))
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
				// check for run-in
				if (formattedLine.length() > 0 && formattedLine[0] == '{')
				{
					isInLineBreak = true;
					isInBraceRunIn = false;
				}
				if (previousCommandChar == '}')
					currentHeader = nullptr;
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
			if (previousNonWSChar == '>' && isWhiteSpace(currentChar) && peekNextChar() == '>')
				continue;
		}

		if (shouldRemoveNextClosingBrace && currentChar == '}')
		{
			currentLine[charNum] = currentChar = ' ';
			shouldRemoveNextClosingBrace = false;
			assert(adjustChecksumIn(-'}'));
			if (isEmptyLine(currentLine))
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
			// check for run-in
			if (formattedLine.length() > 0 && formattedLine[0] == '{')
			{
				isInLineBreak = true;
				isInBraceRunIn = false;
			}
			processPreprocessor();
			// if top level it is potentially indentable
			if (shouldIndentPreprocBlock
			        && (isBraceType(braceTypeStack->back(), NULL_TYPE)
			            || isBraceType(braceTypeStack->back(), NAMESPACE_TYPE))
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

		if (isImmediatelyPostNewDelete)
		{
			isImmediatelyPostNewDelete = false;
			isCharImmediatelyPostNewDelete = true;
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
			// should braces be added
			if (currentChar != '{'
			        && shouldAddBraces
			        && (shouldBreakOneLineStatements || !isHeaderInMultiStatementLine)
			        && isOkToBreakBlock(braceTypeStack->back()))
			{
				bool bracesAdded = addBracesToStatement();
				if (bracesAdded && !shouldAddOneLineBraces)
				{
					size_t firstText = currentLine.find_first_not_of(" \t");
					assert(firstText != string::npos);
					if ((int) firstText == charNum || shouldBreakOneLineHeaders)
						breakCurrentOneLineBlock = true;
				}
			}
			// should braces be removed
			else if (currentChar == '{' && shouldRemoveBraces)
			{
				bool bracesRemoved = removeBracesFromStatement();
				if (bracesRemoved)
				{
					shouldRemoveNextClosingBrace = true;
					if (isBeforeAnyLineEndComment(charNum))
						spacePadNum--;
					else if (shouldBreakOneLineBlocks
					         || (currentLineBeginsWithBrace
					             && currentLine.find_first_not_of(" \t") != string::npos))
						shouldBreakLineAtNextChar = true;
					continue;
				}
			}

			// break 'else-if' if shouldBreakElseIfs is requested
			if (shouldBreakElseIfs
			        && currentHeader == &AS_ELSE
			        && isOkToBreakBlock(braceTypeStack->back())
			        && !isBeforeAnyComment()
			        && (shouldBreakOneLineStatements || !isHeaderInMultiStatementLine))
			{
				string nextText = peekNextText(currentLine.substr(charNum));
				if (nextText.length() > 0
				        && isCharPotentialHeader(nextText, 0)
				        && ASBase::findHeader(nextText, 0, headers) == &AS_IF)
				{
					isInLineBreak = true;
				}
			}

			// break a header (e.g. if, while, else) from the following statement
			if (shouldBreakOneLineHeaders
			        && peekNextChar() != ' '
			        && (shouldBreakOneLineStatements
			            || (!isHeaderInMultiStatementLine
			                && !isMultiStatementLine()))
			        && isOkToBreakBlock(braceTypeStack->back())
			        && !isBeforeAnyComment())
			{
				if (currentChar == '{')
				{
					if (!currentLineBeginsWithBrace)
					{
						if (isOneLineBlockReached(currentLine, charNum) == 3)
							isInLineBreak = false;
						else
							breakCurrentOneLineBlock = true;
					}
				}
				else if (currentHeader == &AS_ELSE)
				{
					string nextText = peekNextText(currentLine.substr(charNum), true);
					if (nextText.length() > 0
					        && ((isCharPotentialHeader(nextText, 0)
					             && ASBase::findHeader(nextText, 0, headers) != &AS_IF)
					            || nextText[0] == '{'))
						isInLineBreak = true;
				}
				else
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
				if (isBraceType(braceTypeStack->back(), SINGLE_LINE_TYPE))
				{
					size_t blockEnd = currentLine.rfind(AS_CLOSE_BRACE);
					assert(blockEnd != string::npos);
					// move ending comments to this formattedLine
					if (isBeforeAnyLineEndComment(blockEnd))
					{
						size_t commentStart = currentLine.find_first_not_of(" \t", blockEnd + 1);
						assert(commentStart != string::npos);
						assert((currentLine.compare(commentStart, 2, "//") == 0)
						       || (currentLine.compare(commentStart, 2, "/*") == 0));
						formattedLine.append(getIndentLength() - 1, ' ');
						// append comment
						int charNumSave = charNum;
						charNum = commentStart;
						while (charNum < (int) currentLine.length())
						{
							currentChar = currentLine[charNum];
							if (currentChar == '\t' && shouldConvertTabs)
								convertTabToSpaces();
							formattedLine.append(1, currentChar);
							++charNum;
						}
						size_t commentLength = currentLine.length() - commentStart;
						currentLine.erase(commentStart, commentLength);
						charNum = charNumSave;
						currentChar = currentLine[charNum];
						testForTimeToSplitFormattedLine();
					}
				}
				isInExecSQL = false;
				shouldReparseCurrentChar = true;
				if (formattedLine.find_first_not_of(" \t") != string::npos)
					isInLineBreak = true;
				if (needHeaderOpeningBrace)
				{
					isCharImmediatelyPostCloseBlock = true;
					needHeaderOpeningBrace = false;
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
			{
				++squareBracketCount;
				if (getAlignMethodColon() && squareBracketCount == 1 && isCStyle())
					objCColonAlign = findObjCColonAlignment();
			}
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
				if (squareBracketCount <= 0)
				{
					squareBracketCount = 0;
					objCColonAlign = 0;
				}
			}
			if (currentChar == ')')
			{
				foundCastOperator = false;
				if (parenStack->back() == 0)
					endOfAsmReached = true;
			}
		}

		// handle braces
		if (currentChar == '{' || currentChar == '}')
		{
			// if appendOpeningBrace this was already done for the original brace
			if (currentChar == '{' && !appendOpeningBrace)
			{
				BraceType newBraceType = getBraceType();
				breakCurrentOneLineBlock = false;
				foundNamespaceHeader = false;
				foundClassHeader = false;
				foundStructHeader = false;
				foundInterfaceHeader = false;
				foundPreDefinitionHeader = false;
				foundPreCommandHeader = false;
				foundPreCommandMacro = false;
				foundTrailingReturnType = false;
				isInPotentialCalculation = false;
				isInObjCMethodDefinition = false;
				isInObjCInterface = false;
				isInEnum = false;
				isJavaStaticConstructor = false;
				isCharImmediatelyPostNonInStmt = false;
				needHeaderOpeningBrace = false;
				shouldKeepLineUnbroken = false;
				objCColonAlign = 0;

				isPreviousBraceBlockRelated = !isBraceType(newBraceType, ARRAY_TYPE);
				braceTypeStack->emplace_back(newBraceType);
				preBraceHeaderStack->emplace_back(currentHeader);
				currentHeader = nullptr;
				structStack->push_back(isInIndentableStruct);
				if (isBraceType(newBraceType, STRUCT_TYPE) && isCStyle())
					isInIndentableStruct = isStructAccessModified(currentLine, charNum);
				else
					isInIndentableStruct = false;
			}

			// this must be done before the braceTypeStack is popped
			BraceType braceType = braceTypeStack->back();
			bool isOpeningArrayBrace = (isBraceType(braceType, ARRAY_TYPE)
			                            && braceTypeStack->size() >= 2
			                            && !isBraceType((*braceTypeStack)[braceTypeStack->size() - 2], ARRAY_TYPE)
			                           );

			if (currentChar == '}')
			{
				// if a request has been made to append a post block empty line,
				// but the block exists immediately before a closing brace,
				// then there is no need for the post block empty line.
				isAppendPostBlockEmptyLineRequested = false;
				if (isInAsm)
					endOfAsmReached = true;
				isInAsmOneLine = isInQuote = false;
				shouldKeepLineUnbroken = false;
				squareBracketCount = 0;

				if (braceTypeStack->size() > 1)
				{
					previousBraceType = braceTypeStack->back();
					braceTypeStack->pop_back();
					isPreviousBraceBlockRelated = !isBraceType(braceType, ARRAY_TYPE);
				}
				else
				{
					previousBraceType = NULL_TYPE;
					isPreviousBraceBlockRelated = false;
				}

				if (!preBraceHeaderStack->empty())
				{
					currentHeader = preBraceHeaderStack->back();
					preBraceHeaderStack->pop_back();
				}
				else
					currentHeader = nullptr;

				if (!structStack->empty())
				{
					isInIndentableStruct = structStack->back();
					structStack->pop_back();
				}
				else
					isInIndentableStruct = false;

				if (isNonInStatementArray
				        && (!isBraceType(braceTypeStack->back(), ARRAY_TYPE)	// check previous brace
				            || peekNextChar() == ';'))							// check for "};" added V2.01
					isImmediatelyPostNonInStmt = true;

				if (!shouldBreakOneLineStatements
				        && ASBeautifier::getNextWord(currentLine, charNum) == AS_ELSE)
				{
					// handle special case of "else" at the end of line
					size_t nextText = currentLine.find_first_not_of(" \t", charNum + 1);
					if (ASBeautifier::peekNextChar(currentLine, nextText + 3) == ' ')
						shouldBreakLineAtNextChar = true;
				}
			}

			// format braces
			appendOpeningBrace = false;
			if (isBraceType(braceType, ARRAY_TYPE))
			{
				formatArrayBraces(braceType, isOpeningArrayBrace);
			}
			else
			{
				if (currentChar == '{')
					formatOpeningBrace(braceType);
				else
					formatClosingBrace(braceType);
			}
			continue;
		}

		if ((((previousCommandChar == '{' && isPreviousBraceBlockRelated)
		        || ((previousCommandChar == '}'
		             && !isImmediatelyPostEmptyBlock
		             && isPreviousBraceBlockRelated
		             && !isPreviousCharPostComment       // Fixes wrongly appended newlines after '}' immediately after comments
		             && peekNextChar() != ' '
		             && !isBraceType(previousBraceType, DEFINITION_TYPE))
		            && !isBraceType(braceTypeStack->back(), DEFINITION_TYPE)))
		        && isOkToBreakBlock(braceTypeStack->back()))
		        // check for array
		        || (previousCommandChar == '{'			// added 9/30/2010
		            && isBraceType(braceTypeStack->back(), ARRAY_TYPE)
		            && !isBraceType(braceTypeStack->back(), SINGLE_LINE_TYPE)
		            && isNonInStatementArray)
		        // check for pico one line braces
		        || (formattingStyle == STYLE_PICO
		            && (previousCommandChar == '{' && isPreviousBraceBlockRelated)
		            && isBraceType(braceTypeStack->back(), COMMAND_TYPE)
		            && isBraceType(braceTypeStack->back(), SINGLE_LINE_TYPE)
		            && braceFormatMode == RUN_IN_MODE)
		   )
		{
			isCharImmediatelyPostOpenBlock = (previousCommandChar == '{');
			isCharImmediatelyPostCloseBlock = (previousCommandChar == '}');

			if (isCharImmediatelyPostOpenBlock
			        && !isCharImmediatelyPostComment
			        && !isCharImmediatelyPostLineComment)
			{
				previousCommandChar = ' ';

				if (braceFormatMode == NONE_MODE)
				{
					if (isBraceType(braceTypeStack->back(), SINGLE_LINE_TYPE)
					        && (isBraceType(braceTypeStack->back(), BREAK_BLOCK_TYPE)
					            || shouldBreakOneLineBlocks))
						isInLineBreak = true;
					else if (currentLineBeginsWithBrace)
						formatRunIn();
					else
						breakLine();
				}
				else if (braceFormatMode == RUN_IN_MODE
				         && currentChar != '#')
					formatRunIn();
				else
					isInLineBreak = true;
			}
			else if (isCharImmediatelyPostCloseBlock
			         && shouldBreakOneLineStatements
			         && !isCharImmediatelyPostComment
			         && ((isLegalNameChar(currentChar) && currentChar != '.')
			             || currentChar == '+'
			             || currentChar == '-'
			             || currentChar == '*'
			             || currentChar == '&'
			             || currentChar == '('))
			{
				previousCommandChar = ' ';
				isInLineBreak = true;
			}
		}

		// reset block handling flags
		isImmediatelyPostEmptyBlock = false;

		// look for headers
		bool isPotentialHeader = isCharPotentialHeader(currentLine, charNum);

		if (isPotentialHeader && !isInTemplate && squareBracketCount == 0)
		{
			isNonParenHeader = false;
			foundClosingHeader = false;

			newHeader = findHeader(headers);

			// Qt headers may be variables in C++
			if (isCStyle()
			        && (newHeader == &AS_FOREVER || newHeader == &AS_FOREACH))
			{
				if (currentLine.find_first_of("=;", charNum) != string::npos)
					newHeader = nullptr;
			}
			if (isJavaStyle()
			        && (newHeader == &AS_SYNCHRONIZED))
			{
				// want synchronized statements not synchronized methods
				if (!isBraceType(braceTypeStack->back(), COMMAND_TYPE))
					newHeader = nullptr;
			}
			else if (newHeader == &AS_USING
			         && ASBeautifier::peekNextChar(
			             currentLine, charNum + (*newHeader).length() - 1) != '(')
				newHeader = nullptr;

			if (newHeader != nullptr)
			{
				foundClosingHeader = isClosingHeader(newHeader);

				if (!foundClosingHeader)
				{
					// these are closing headers
					if ((newHeader == &AS_WHILE && currentHeader == &AS_DO)
					        || (newHeader == &_AS_FINALLY && currentHeader == &_AS_TRY)
					        || (newHeader == &_AS_EXCEPT && currentHeader == &_AS_TRY))
						foundClosingHeader = true;
					// don't append empty block for these related headers
					else if (isSharpStyle()
					         && previousNonWSChar == '}'
					         && ((newHeader == &AS_SET && currentHeader == &AS_GET)
					             || (newHeader == &AS_REMOVE && currentHeader == &AS_ADD))
					         && isOkToBreakBlock(braceTypeStack->back()))
						isAppendPostBlockEmptyLineRequested = false;
				}

				// TODO: this can be removed in a future release
				// version 3.0 - break erroneous attached header from previous versions
				if (isSharpStyle()
				        && ((newHeader == &AS_SET && currentHeader == &AS_GET)
				            || (newHeader == &AS_REMOVE && currentHeader == &AS_ADD))
				        && !isBraceType(braceTypeStack->back(), SINGLE_LINE_TYPE)
				        && currentLine[currentLine.find_first_not_of(" \t")] == '}')
					isInLineBreak = true;
				// END TODO

				const string* previousHeader = currentHeader;
				currentHeader = newHeader;
				needHeaderOpeningBrace = true;

				// is the previous statement on the same line?
				if ((previousNonWSChar == ';' || previousNonWSChar == ':')
				        && !isInLineBreak
				        && isOkToBreakBlock(braceTypeStack->back()))
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
					if (isOkToBreakBlock(braceTypeStack->back()))
						isLineBreakBeforeClosingHeader();

					// get the adjustment for a comment following the closing header
					if (isInLineBreak)
						nextLineSpacePadNum = getNextLineCommentAdjustment();
					else
						spacePadNum = getCurrentLineCommentAdjustment();
				}

				// check if the found header is non-paren header
				isNonParenHeader = findHeader(nonParenHeaders) != nullptr;

				if (isNonParenHeader
				        && (currentHeader == &AS_CATCH
				            || currentHeader == &AS_CASE))
				{
					int startChar = charNum + currentHeader->length() - 1;
					if (ASBeautifier::peekNextChar(currentLine, startChar) == '(')
						isNonParenHeader = false;
				}

				// join 'else if' statements
				if (currentHeader == &AS_IF
				        && previousHeader == &AS_ELSE
				        && isInLineBreak
				        && !shouldBreakElseIfs
				        && !isCharImmediatelyPostLineComment
				        && !isImmediatelyPostPreprocessor)
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
				        && !isNonParenHeader
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
				        && isOkToBreakBlock(braceTypeStack->back())
				        && !isHeaderInMultiStatementLine)
				{
					if (previousHeader == nullptr
					        && !foundClosingHeader
					        && !isCharImmediatelyPostOpenBlock
					        && !isImmediatelyPostCommentOnly)
					{
						isPrependPostBlockEmptyLineRequested = true;
					}

					if (isClosingHeader(currentHeader)
					        || foundClosingHeader)
					{
						isPrependPostBlockEmptyLineRequested = false;
					}

					if (shouldBreakClosingHeaderBlocks
					        && isCharImmediatelyPostCloseBlock
					        && !isImmediatelyPostCommentOnly
					        && !(currentHeader == &AS_WHILE			// do-while
					             && foundClosingHeader))
					{
						isPrependPostBlockEmptyLineRequested = true;
					}
				}

				if (currentHeader == &AS_CASE
				        || currentHeader == &AS_DEFAULT)
					isInCase = true;

				continue;
			}
			else if ((newHeader = findHeader(preDefinitionHeaders)) != nullptr
			         && parenStack->back() == 0
			         && !isInEnum)		// not C++11 enum class
			{
				if (newHeader == &AS_NAMESPACE || newHeader == &AS_MODULE)
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
			else if ((newHeader = findHeader(preCommandHeaders)) != nullptr)
			{
				// a 'const' variable is not a preCommandHeader
				if (previousNonWSChar == ')')
					foundPreCommandHeader = true;
			}
			else if ((newHeader = findHeader(castOperators)) != nullptr)
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
				        || isBraceType(braceTypeStack->back(), SINGLE_LINE_TYPE))
				        && isOkToBreakBlock(braceTypeStack->back()))
				        && !(attachClosingBraceMode && peekNextChar() == '}'))
				{
					passedSemicolon = true;
				}
				else if (!shouldBreakOneLineStatements
				         && ASBeautifier::getNextWord(currentLine, charNum) == AS_ELSE)
				{
					// handle special case of "else" at the end of line
					size_t nextText = currentLine.find_first_not_of(" \t", charNum + 1);
					if (ASBeautifier::peekNextChar(currentLine, nextText + 3) == ' ')
						passedSemicolon = true;
				}

				if (shouldBreakBlocks
				        && currentHeader != nullptr
				        && currentHeader != &AS_CASE
				        && currentHeader != &AS_DEFAULT
				        && !isHeaderInMultiStatementLine
				        && parenStack->back() == 0)
				{
					isAppendPostBlockEmptyLineRequested = true;
				}
			}
			if (currentChar != ';'
			        || (needHeaderOpeningBrace && parenStack->back() == 0))
				currentHeader = nullptr;
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
			         && isOkToBreakBlock(braceTypeStack->back())
			         && shouldBreakOneLineStatements
			         && !foundQuestionMark          // not in a ?: sequence
			         && !foundPreDefinitionHeader   // not in a definition block
			         && previousCommandChar != ')'  // not after closing paren of a method header
			         && !foundPreCommandHeader      // not after a 'noexcept'
			         && squareBracketCount == 0        // not in objC method call
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
				if ((int) currentLine.length() > charNum + 1
				        && !isWhiteSpace(currentLine[charNum + 1]))
					currentLine.insert(charNum + 1, " ");
			}

			if (isClassInitializer())
				isInClassInitializer = true;
		}

		if (currentChar == '?')
			foundQuestionMark = true;

		if (isPotentialHeader && !isInTemplate)
		{
			if (findKeyword(currentLine, charNum, AS_NEW)
			        || findKeyword(currentLine, charNum, AS_DELETE))
			{
				isInPotentialCalculation = false;
				isImmediatelyPostNewDelete = true;
			}

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

			if (isCStyle() && findKeyword(currentLine, charNum, AS_AUTO)
			        && (isBraceType(braceTypeStack->back(), NULL_TYPE)
			            || isBraceType(braceTypeStack->back(), NAMESPACE_TYPE)
			            || isBraceType(braceTypeStack->back(), CLASS_TYPE)))
				foundTrailingReturnType = true;

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
			            && isNextCharOpeningBrace(charNum + 6)))
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
		        && isBraceType(braceTypeStack->back(), NULL_TYPE))
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
		         && (int) currentLine.find_first_not_of(" \t") == charNum
		         && peekNextChar() == '('
		         && isBraceType(braceTypeStack->back(), NULL_TYPE)
		         && !isInPotentialCalculation)
		{
			isInObjCMethodDefinition = true;
			isImmediatelyPostObjCMethodPrefix = true;
			isInObjCInterface = false;
			if (getAlignMethodColon())
				objCColonAlign = findObjCColonAlignment();
			appendCurrentChar();
			continue;
		}

		// determine if this is a potential calculation

		bool isPotentialOperator = isCharPotentialOperator(currentChar);
		newHeader = nullptr;

		if (isPotentialOperator)
		{
			newHeader = findOperator(operators);

			// check for Java ? wildcard
			if (newHeader != nullptr
			        && newHeader == &AS_GCC_MIN_ASSIGN
			        && isJavaStyle()
			        && isInTemplate)
				newHeader = nullptr;

			if (newHeader != nullptr)
			{
				if (newHeader == &AS_LAMBDA)
					foundPreCommandHeader = true;

				// correct mistake of two >> closing a template
				if (isInTemplate && (newHeader == &AS_GR_GR || newHeader == &AS_GR_GR_GR))
					newHeader = &AS_GR;

				if (!isInPotentialCalculation)
				{
					// must determine if newHeader is an assignment operator
					// do NOT use findOperator - the length must be exact!!!
					if (find(begin(*assignmentOperators), end(*assignmentOperators), newHeader)
					        != end(*assignmentOperators))
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
		if (newHeader != nullptr && !isJavaStyle()
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

		if (shouldPadOperators && newHeader != nullptr && !isOperatorPaddingDisabled())
		{
			padOperators(newHeader);
			continue;
		}

		// remove spaces before commas
		if (currentChar == ',')
		{
			const size_t len = formattedLine.length();
			size_t lastText = formattedLine.find_last_not_of(' ');
			if (lastText != string::npos && lastText < len - 1)
			{
				formattedLine.resize(lastText + 1);
				int size_diff = len - (lastText + 1);
				spacePadNum -= size_diff;
			}
		}

		// pad commas and semi-colons
		if (currentChar == ';'
		        || (currentChar == ',' && (shouldPadOperators || shouldPadCommas)))
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
			        /* && !(isBraceType(braceTypeStack->back(), ARRAY_TYPE)) */
			   )
			{
				appendCurrentChar();
				appendSpaceAfter();
				continue;
			}
		}

		// pad parens
		if (currentChar == '(' || currentChar == ')')
		{
			if (currentChar == '(')
			{
				if (shouldPadHeader
				        && (isCharImmediatelyPostReturn
				            || isCharImmediatelyPostThrow
				            || isCharImmediatelyPostNewDelete))
					appendSpacePad();
			}

			if (shouldPadParensOutside || shouldPadParensInside || shouldUnPadParens || shouldPadFirstParen)
				padParens();
			else
				appendCurrentChar();

			if (isInObjCMethodDefinition)
			{
				if (currentChar == '(' && isImmediatelyPostObjCMethodPrefix)
				{
					if (shouldPadMethodPrefix || shouldUnPadMethodPrefix)
						padObjCMethodPrefix();
					isImmediatelyPostObjCMethodPrefix = false;
					isInObjCReturnType = true;
				}
				else if (currentChar == ')' && isInObjCReturnType)
				{
					if (shouldPadReturnType || shouldUnPadReturnType)
						padObjCReturnType();
					isInObjCReturnType = false;
				}
				else if (shouldPadParamType || shouldUnPadParamType)
					padObjCParamType();
			}
			continue;
		}

		// bypass the entire operator
		if (newHeader != nullptr)
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
	bool isInNamespace = isBraceType(braceTypeStack->back(), NAMESPACE_TYPE);

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
		runInIndentContinuation = runInIndentChars;
		beautifiedLine = beautify(readyFormattedLine);
		previousReadyFormattedLineLength = readyFormattedLineLength;
		// the enhancer is not called for no-indent line comments
		if (!lineCommentNoBeautify && !isFormattingModeOff)
			enhancer->enhance(beautifiedLine, isInNamespace, isInPreprocessorBeautify, isInBeautifySQL);
		runInIndentChars = 0;
		lineCommentNoBeautify = lineCommentNoIndent;
		lineCommentNoIndent = false;
		isInIndentablePreproc = isIndentableProprocessor;
		isIndentableProprocessor = false;
		isElseHeaderIndent = elseHeaderFollowsComments;
		isCaseHeaderCommentIndent = caseHeaderFollowsComments;
		objCColonAlignSubsequent = objCColonAlign;
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
 * comparison function for BraceType enum
 */
bool ASFormatter::isBraceType(BraceType a, BraceType b) const
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
 * set the add braces mode.
 * options:
 *    true     braces added to headers for single line statements.
 *    false    braces NOT added to headers for single line statements.
 *
 * @param state         the add braces state.
 */
void ASFormatter::setAddBracesMode(bool state)
{
	shouldAddBraces = state;
}

/**
 * set the add one line braces mode.
 * options:
 *    true     one line braces added to headers for single line statements.
 *    false    one line braces NOT added to headers for single line statements.
 *
 * @param state         the add one line braces state.
 */
void ASFormatter::setAddOneLineBracesMode(bool state)
{
	shouldAddBraces = state;
	shouldAddOneLineBraces = state;
}

/**
 * set the remove braces mode.
 * options:
 *    true     braces removed from headers for single line statements.
 *    false    braces NOT removed from headers for single line statements.
 *
 * @param state         the remove braces state.
 */
void ASFormatter::setRemoveBracesMode(bool state)
{
	shouldRemoveBraces = state;
}

// retained for compatability with release 2.06
// "Brackets" have been changed to "Braces" in 3.0
// it is referenced only by the old "bracket" options
void ASFormatter::setAddBracketsMode(bool state)
{
	setAddBracesMode(state);
}

// retained for compatability with release 2.06
// "Brackets" have been changed to "Braces" in 3.0
// it is referenced only by the old "bracket" options
void ASFormatter::setAddOneLineBracketsMode(bool state)
{
	setAddOneLineBracesMode(state);
}

// retained for compatability with release 2.06
// "Brackets" have been changed to "Braces" in 3.0
// it is referenced only by the old "bracket" options
void ASFormatter::setRemoveBracketsMode(bool state)
{
	setRemoveBracesMode(state);
}

// retained for compatability with release 2.06
// "Brackets" have been changed to "Braces" in 3.0
// it is referenced only by the old "bracket" options
void ASFormatter::setBreakClosingHeaderBracketsMode(bool state)
{
	setBreakClosingHeaderBracesMode(state);
}


/**
 * set the brace formatting mode.
 * options:
 *
 * @param mode         the brace formatting mode.
 */
void ASFormatter::setBraceFormatMode(BraceMode mode)
{
	braceFormatMode = mode;
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
 * set closing header brace breaking mode
 * options:
 *    true     braces just before closing headers (e.g. 'else', 'catch')
 *             will be broken, even if standard braces are attached.
 *    false    closing header braces will be treated as standard braces.
 *
 * @param state         the closing header brace breaking mode.
 */
void ASFormatter::setBreakClosingHeaderBracesMode(bool state)
{
	shouldBreakClosingHeaderBraces = state;
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
* set comma padding mode.
* options:
*    true     statement commas and semicolons will be padded with spaces around them.
*    false    statement commas and semicolons will not be padded.
*
* @param state         the padding mode.
*/
void ASFormatter::setCommaPaddingMode(bool state)
{
	shouldPadCommas = state;
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

// set objective-c '-' or '+' return type padding mode.
void ASFormatter::setReturnTypePaddingMode(bool state)
{
	shouldPadReturnType = state;
}

// set objective-c '-' or '+' return type unpadding mode.
void ASFormatter::setReturnTypeUnPaddingMode(bool state)
{
	shouldUnPadReturnType = state;
}

// set objective-c method parameter type padding mode.
void ASFormatter::setParamTypePaddingMode(bool state)
{
	shouldPadParamType = state;
}

// set objective-c method parameter type unpadding mode.
void ASFormatter::setParamTypeUnPaddingMode(bool state)
{
	shouldUnPadParamType = state;
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
 * set option to attach closing braces
 *
 * @param state        true = attach, false = don't attach.
 */
void ASFormatter::setAttachClosingBraceMode(bool state)
{
	attachClosingBraceMode = state;
}

/**
 * set option to attach class braces
 *
 * @param state        true = attach, false = use style default.
 */
void ASFormatter::setAttachClass(bool state)
{
	shouldAttachClass = state;
}

/**
 * set option to attach extern "C" braces
 *
 * @param state        true = attach, false = use style default.
 */
void ASFormatter::setAttachExternC(bool state)
{
	shouldAttachExternC = state;
}

/**
 * set option to attach namespace braces
 *
 * @param state        true = attach, false = use style default.
 */
void ASFormatter::setAttachNamespace(bool state)
{
	shouldAttachNamespace = state;
}

/**
 * set option to attach inline braces
 *
 * @param state        true = attach, false = use style default.
 */
void ASFormatter::setAttachInline(bool state)
{
	shouldAttachInline = state;
}

void ASFormatter::setAttachClosingWhile(bool state)
{
	shouldAttachClosingWhile = state;
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

/**
* set one line headers breaking mode
*/
void ASFormatter::setBreakOneLineHeadersMode(bool state)
{
	shouldBreakOneLineHeaders = state;
}

/**
* set option to break/not break lines consisting of multiple statements.
*
* @param state        true = break, false = don't break.
*/
void ASFormatter::setBreakOneLineStatementsMode(bool state)
{
	shouldBreakOneLineStatements = state;
}

void ASFormatter::setCloseTemplatesMode(bool state)
{
	shouldCloseTemplates = state;
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
	if (!sourceIterator->hasMoreLines())
	{
		endOfCodeReached = true;
		return false;
	}
	if (appendOpeningBrace)
		currentLine = "{";		// append brace that was removed from the previous line
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
	isInQuoteContinuation = isInVerbatimQuote || haveLineContinuationChar;
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
	if (isInBraceRunIn && previousNonWSChar == '{' && !isInComment)
		isInLineBreak = false;
	isInBraceRunIn = false;

	if (currentChar == '\t' && shouldConvertTabs)
		convertTabToSpaces();

	// check for an empty line inside a command brace.
	// if yes then read the next line (calls getNextLine recursively).
	// must be after initNewLine.
	if (shouldDeleteEmptyLines
	        && lineIsEmpty
	        && isBraceType((*braceTypeStack)[braceTypeStack->size() - 1], COMMAND_TYPE))
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
	currentLineBeginsWithBrace = false;
	lineIsEmpty = false;
	currentLineFirstBraceNum = string::npos;
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
		currentLineBeginsWithBrace = true;
		currentLineFirstBraceNum = charNum;
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
void ASFormatter::appendSequence(const string& sequence, bool canBreakLine)
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
void ASFormatter::appendOperator(const string& sequence, bool canBreakLine)
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
 * check if the currently reached open-brace (i.e. '{')
 * opens a:
 * - a definition type block (such as a class or namespace),
 * - a command block (such as a method block)
 * - a static array
 * this method takes for granted that the current character
 * is an opening brace.
 *
 * @return    the type of the opened block.
 */
BraceType ASFormatter::getBraceType()
{
	assert(currentChar == '{');

	BraceType returnVal = NULL_TYPE;

	if ((previousNonWSChar == '='
	        || isBraceType(braceTypeStack->back(), ARRAY_TYPE))
	        && previousCommandChar != ')'
	        && !isNonParenHeader)
		returnVal = ARRAY_TYPE;
	else if (foundPreDefinitionHeader && previousCommandChar != ')')
	{
		returnVal = DEFINITION_TYPE;
		if (foundNamespaceHeader)
			returnVal = (BraceType)(returnVal | NAMESPACE_TYPE);
		else if (foundClassHeader)
			returnVal = (BraceType)(returnVal | CLASS_TYPE);
		else if (foundStructHeader)
			returnVal = (BraceType)(returnVal | STRUCT_TYPE);
		else if (foundInterfaceHeader)
			returnVal = (BraceType)(returnVal | INTERFACE_TYPE);
	}
	else if (isInEnum)
	{
		returnVal = (BraceType)(ARRAY_TYPE | ENUM_TYPE);
	}
	else
	{
		bool isCommandType = (foundPreCommandHeader
		                      || foundPreCommandMacro
		                      || (currentHeader != nullptr && isNonParenHeader)
		                      || (previousCommandChar == ')')
		                      || (previousCommandChar == ':' && !foundQuestionMark)
		                      || (previousCommandChar == ';')
		                      || ((previousCommandChar == '{' || previousCommandChar == '}')
		                          && isPreviousBraceBlockRelated)
		                      || (isInClassInitializer
		                          && (!isLegalNameChar(previousNonWSChar) || foundPreCommandHeader))
		                      || foundTrailingReturnType
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

	if (foundOneLineBlock == 2 && returnVal == COMMAND_TYPE)
		returnVal = ARRAY_TYPE;

	if (foundOneLineBlock > 0)
	{
		returnVal = (BraceType) (returnVal | SINGLE_LINE_TYPE);
		if (breakCurrentOneLineBlock)
			returnVal = (BraceType) (returnVal | BREAK_BLOCK_TYPE);
		if (foundOneLineBlock == 3)
			returnVal = (BraceType)(returnVal | EMPTY_BLOCK_TYPE);
	}

	if (isBraceType(returnVal, ARRAY_TYPE))
	{
		if (isNonInStatementArrayBrace())
		{
			returnVal = (BraceType)(returnVal | ARRAY_NIS_TYPE);
			isNonInStatementArray = true;
			isImmediatelyPostNonInStmt = false;		// in case of "},{"
			nonInStatementBrace = formattedLine.length() - 1;
		}
		if (isUniformInitializerBrace())
			returnVal = (BraceType)(returnVal | INIT_TYPE);
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
	assert(currentChar == ':');
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
bool ASFormatter::isEmptyLine(const string& line) const
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
		if (previousNonWSChar == '>')
			return true;
		string followingText;
		if ((int) currentLine.length() > charNum + 2)
			followingText = peekNextText(currentLine.substr(charNum + 2));
		if (followingText.length() > 0 && followingText[0] == ')')
			return true;
		if (currentHeader != nullptr || isInPotentialCalculation)
			return false;
		if (parenStack->back() > 0 && isBraceType(braceTypeStack->back(), COMMAND_TYPE))
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

	if (isBraceType(braceTypeStack->back(), ARRAY_TYPE)
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
		if (followingOperator != nullptr
		        && followingOperator != &AS_MULT
		        && followingOperator != &AS_BIT_AND)
		{
			if (followingOperator == &AS_ASSIGN || followingOperator == &AS_COLON)
				return true;
			return false;
		}

		if (isBraceType(braceTypeStack->back(), COMMAND_TYPE)
		        || squareBracketCount > 0)
			return false;
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
	        && (isBraceType(braceTypeStack->back(), COMMAND_TYPE)
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

	if (!isBraceType(braceTypeStack->back(), COMMAND_TYPE)
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
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');

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
bool ASFormatter::isPointerOrReferenceVariable(const string& word) const
{
	return (word == "char"
	        || word == "int"
	        || word == "void"
	        || (word.length() >= 6     // check end of word for _t
	            && word.compare(word.length() - 2, 2, "_t") == 0)
	        || word == "INT"
	        || word == "VOID");
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
	if (!preBraceHeaderStack->empty())
		for (size_t i = 1; i < preBraceHeaderStack->size(); i++)
			if (preBraceHeaderStack->at(i) == &AS_SWITCH)
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

	if (charNum >= 2)
	{
		char prevPrevFormattedChar = currentLine[charNum - 2];
		char prevFormattedChar = currentLine[charNum - 1];
		return ((prevFormattedChar == 'e' || prevFormattedChar == 'E')
		        && (prevPrevFormattedChar == '.' || isDigit(prevPrevFormattedChar)));
	}
	return false;
}

/**
 * check if an array brace should NOT have an in-statement indent
 *
 * @return        the array is non in-statement
 */
bool ASFormatter::isNonInStatementArrayBrace() const
{
	bool returnVal = false;
	char nextChar = peekNextChar();
	// if this opening brace begins the line there will be no inStatement indent
	if (currentLineBeginsWithBrace
	        && charNum == (int) currentLineFirstBraceNum
	        && nextChar != '}')
		returnVal = true;
	// if an opening brace ends the line there will be no inStatement indent
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
 * check if a one-line block has been reached,
 * i.e. if the currently reached '{' character is closed
 * with a complimentary '}' elsewhere on the current line,
 *.
 * @return     0 = one-line block has not been reached.
 *             1 = one-line block has been reached.
 *             2 = one-line block has been reached and is followed by a comma.
 *             3 = one-line block has been reached and is an empty block.
 */
int ASFormatter::isOneLineBlockReached(const string& line, int startChar) const
{
	assert(line[startChar] == '{');

	bool isInComment_ = false;
	bool isInQuote_ = false;
	bool hasText = false;
	int braceCount = 0;
	int lineLength = line.length();
	char quoteChar_ = ' ';
	char ch = ' ';
	char prevCh = ' ';

	for (int i = startChar; i < lineLength; ++i)
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

		if (ch == '"'
		        || (ch == '\'' && !isDigitSeparator(line, i)))
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
		{
			++braceCount;
			continue;
		}
		if (ch == '}')
		{
			--braceCount;
			if (braceCount == 0)
			{
				// is this an array?
				if (parenStack->back() == 0 && prevCh != '}')
				{
					size_t peekNum = line.find_first_not_of(" \t", i + 1);
					if (peekNum != string::npos && line[peekNum] == ',')
						return 2;
				}
				if (!hasText)
					return 3;	// is an empty block
				return 1;
			}
		}
		if (ch == ';')
			continue;
		if (!isWhiteSpace(ch))
		{
			hasText = true;
			prevCh = ch;
		}
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
 * peek at the next char to determine if it is an opening brace.
 * will look ahead in the input file if necessary.
 * this determines a java static constructor.
 *
 * @param startChar     position on currentLine to start the search
 * @return              true if the next word is an opening brace.
 */
bool ASFormatter::isNextCharOpeningBrace(int startChar) const
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
* Determine if an opening array-type brace should have a leading space pad.
* This is to identify C++11 uniform initializers.
*/
bool ASFormatter::isUniformInitializerBrace() const
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
* Determine if there is a following statement on the current line.
*/
bool ASFormatter::isMultiStatementLine() const
{
	assert((isImmediatelyPostHeader || foundClosingHeader));
	bool isInComment_ = false;
	bool isInQuote_ = false;
	int  semiCount_ = 0;
	int  parenCount_ = 0;
	int  braceCount_ = 0;

	for (size_t i = 0; i < currentLine.length(); i++)
	{
		if (isInComment_)
		{
			if (currentLine.compare(i, 2, "*/") == 0)
			{
				isInComment_ = false;
				continue;
			}
		}
		if (currentLine.compare(i, 2, "/*") == 0)
		{
			isInComment_ = true;
			continue;
		}
		if (currentLine.compare(i, 2, "//") == 0)
			return false;
		if (isInQuote_)
		{
			if (currentLine[i] == '"' || currentLine[i] == '\'')
				isInQuote_ = false;
			continue;
		}
		if (currentLine[i] == '"' || currentLine[i] == '\'')
		{
			isInQuote_ = true;
			continue;
		}
		if (currentLine[i] == '(')
		{
			++parenCount_;
			continue;
		}
		if (currentLine[i] == ')')
		{
			--parenCount_;
			continue;
		}
		if (parenCount_ > 0)
			continue;
		if (currentLine[i] == '{')
		{
			++braceCount_;
		}
		if (currentLine[i] == '}')
		{
			--braceCount_;
		}
		if (braceCount_ > 0)
			continue;
		if (currentLine[i] == ';')
		{
			++semiCount_;
			if (semiCount_ > 1)
				return true;
			continue;
		}
	}
	return false;
}

/**
 * get the next non-whitespace substring on following lines, bypassing all comments.
 *
 * @param   firstLine   the first line to check
 * @return  the next non-whitespace substring.
 */
string ASFormatter::peekNextText(const string& firstLine,
                                 bool endOnEmptyLine /*false*/,
                                 shared_ptr<ASPeekStream> streamArg /*nullptr*/) const
{
	bool isFirstLine = true;
	string nextLine_ = firstLine;
	size_t firstChar = string::npos;
	shared_ptr<ASPeekStream> stream = streamArg;
	if (stream == nullptr)							// Borland may need == 0
		stream = make_shared<ASPeekStream>(sourceIterator);

	// find the first non-blank text, bypassing all comments.
	bool isInComment_ = false;
	while (stream->hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
			nextLine_ = stream->peekNextLine();

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
	return nextLine_;
}

/**
 * adjust comment position because of adding or deleting spaces
 * the spaces are added or deleted to formattedLine
 * spacePadNum contains the adjustment
 */
void ASFormatter::adjustComments()
{
	assert(spacePadNum != 0);
	assert(isSequenceReached("//") || isSequenceReached("/*"));

	// block comment must be closed on this line with nothing after it
	if (isSequenceReached("/*"))
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
 * append the current brace inside the end of line comments
 * currentChar contains the brace, it will be appended to formattedLine
 * formattedLineCommentNum is the comment location on formattedLine
 */
void ASFormatter::appendCharInsideComments()
{
	if (formattedLineCommentNum == string::npos     // does the comment start on the previous line?
	        || formattedLineCommentNum == 0)
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

	// insert the brace
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
	assert(newOperator != nullptr);

	char nextNonWSChar = ASBase::peekNextChar(currentLine, charNum);
	bool shouldPad = (newOperator != &AS_SCOPE_RESOLUTION
	                  && newOperator != &AS_PLUS_PLUS
	                  && newOperator != &AS_MINUS_MINUS
	                  && newOperator != &AS_NOT
	                  && newOperator != &AS_BIT_NOT
	                  && newOperator != &AS_ARROW
	                  && !(newOperator == &AS_COLON && !foundQuestionMark			// objC methods
	                       && (isInObjCMethodDefinition || isInObjCInterface
	                           || isInObjCSelector || squareBracketCount != 0))
	                  && !(newOperator == &AS_MINUS && isInExponent())
	                  && !(newOperator == &AS_PLUS && isInExponent())
	                  && !((newOperator == &AS_PLUS || newOperator == &AS_MINUS)	// check for unary plus or minus
	                       && (previousNonWSChar == '('
	                           || previousNonWSChar == '['
	                           || previousNonWSChar == '='
	                           || previousNonWSChar == ','
	                           || previousNonWSChar == ':'
	                           || previousNonWSChar == '{'))
//?                   // commented out in release 2.05.1 - doesn't seem to do anything???
//x                   && !((newOperator == &AS_MULT || newOperator == &AS_BIT_AND || newOperator == &AS_AND)
//x                        && isPointerOrReference())
	                  && !(newOperator == &AS_MULT
	                       && (previousNonWSChar == '.'
	                           || previousNonWSChar == '>'))    // check for ->
	                  && !(newOperator == &AS_MULT && peekNextChar() == '>')
	                  && !((isInTemplate || isImmediatelyPostTemplate)
	                       && (newOperator == &AS_LS || newOperator == &AS_GR))
	                  && !(newOperator == &AS_GCC_MIN_ASSIGN
	                       && ASBase::peekNextChar(currentLine, charNum + 1) == '>')
	                  && !(newOperator == &AS_GR && previousNonWSChar == '?')
	                  && !(newOperator == &AS_QUESTION			// check for Java wildcard
	                       && isJavaStyle()
	                       && (previousNonWSChar == '<'
	                           || nextNonWSChar == '>'
	                           || nextNonWSChar == '.'))
	                  && !(newOperator == &AS_QUESTION			// check for C# null conditional operator
	                       && isSharpStyle()
	                       && (nextNonWSChar == '.'
	                           || nextNonWSChar == '['))
	                  && !isCharImmediatelyPostOperator
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
void ASFormatter::formatPointerOrReference()
{
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');
	assert(!isJavaStyle());

	int pa = pointerAlignment;
	int ra = referenceAlignment;
	int itemAlignment = (currentChar == '*' || currentChar == '^') ? pa : ((ra == REF_SAME_AS_PTR) ? pa : ra);

	// check for ** and &&
	int ptrLength = 1;
	char peekedChar = peekNextChar();
	if ((currentChar == '*' && peekedChar == '*')
	        || (currentChar == '&' && peekedChar == '&'))
	{
		ptrLength = 2;
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
		formattedLine.append(ptrLength, currentChar);
		if (ptrLength > 1)
			goForward(ptrLength - 1);
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
			if (wsBefore == 0)
				wsBefore++;
			if (wsAfter == 0)
				wsAfter++;
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
void ASFormatter::formatPointerOrReferenceCast()
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
}

/**
 * add or remove space padding to parens
 * currentChar contains the paren
 * the parens and necessary padding will be appended to formattedLine
 * the calling function should have a continue statement after calling this method
 */
void ASFormatter::padParens()
{
	assert(currentChar == '(' || currentChar == ')');
	assert(shouldPadParensOutside || shouldPadParensInside || shouldUnPadParens || shouldPadFirstParen);

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
				// if last char is a brace the previous whitespace is an indent
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
					const string* prevWordH = nullptr;
					if (shouldPadHeader
					        && prevWord.length() > 0
					        && isCharPotentialHeader(prevWord, 0))
						prevWordH = ASBase::findHeader(prevWord, 0, headers);
					if (prevWordH != nullptr)
						prevIsParenHeader = true;
					else if (prevWord == AS_RETURN)  // don't unpad
						prevIsParenHeader = true;
					else if ((prevWord == AS_NEW || prevWord == AS_DELETE)
					         && shouldPadHeader)  // don't unpad
						prevIsParenHeader = true;
					else if (isCStyle() && prevWord == AS_THROW && shouldPadHeader) // don't unpad
						prevIsParenHeader = true;
					else if (prevWord == "and" || prevWord == "or" || prevWord == "in")  // don't unpad
						prevIsParenHeader = true;
					// don't unpad variables
					else if (prevWord == "bool"
					         || prevWord == "int"
					         || prevWord == "void"
					         || prevWord == "void*"
					         || prevWord == "char"
					         || prevWord == "char*"
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
}

/**
* add or remove space padding to objective-c parens
* these options have precedence over the padParens methods
* the padParens method has already been called, this method adjusts
*/
void ASFormatter::padObjCMethodPrefix()
{
	assert(currentChar == '(' && isImmediatelyPostObjCMethodPrefix);
	assert(shouldPadMethodPrefix || shouldUnPadMethodPrefix);

	size_t prefix = formattedLine.find_first_of("+-");
	if (prefix == string::npos)
		return;
	size_t paren = formattedLine.find_first_of('(');
	if (paren == string::npos)
		return;
	int spaces = paren - prefix - 1;

	if (shouldPadMethodPrefix)
	{
		if (spaces == 0)
		{
			formattedLine.insert(prefix + 1, 1, ' ');
			spacePadNum += 1;
		}
		else if (spaces > 1)
		{
			formattedLine.erase(prefix + 1, spaces - 1);
			spacePadNum -= spaces - 1;
		}
	}
	// this option will be ignored if used with pad-method-prefix
	else if (shouldUnPadMethodPrefix)
	{
		if (spaces > 0)
		{
			formattedLine.erase(prefix + 1, spaces);
			spacePadNum -= spaces;
		}
	}
}

/**
* add or remove space padding to objective-c parens
* these options have precedence over the padParens methods
* the padParens method has already been called, this method adjusts
*/
void ASFormatter::padObjCReturnType()
{
	assert(currentChar == ')' && isInObjCReturnType);
	assert(shouldPadReturnType || shouldUnPadReturnType);

	size_t nextText = currentLine.find_first_not_of(" \t", charNum + 1);
	if (nextText == string::npos)
		return;
	int spaces = nextText - charNum - 1;

	if (shouldPadReturnType)
	{
		if (spaces == 0)
		{
			// this will already be padded if pad-paren is used
			if (formattedLine[formattedLine.length() - 1] != ' ')
			{
				formattedLine.append(" ");
				spacePadNum += 1;
			}
		}
		else if (spaces > 1)
		{
			// do not use goForward here
			currentLine.erase(charNum + 1, spaces - 1);
			spacePadNum -= spaces - 1;
		}
	}
	// this option will be ignored if used with pad-return-type
	else if (shouldUnPadReturnType)
	{
		// this will already be padded if pad-paren is used
		if (formattedLine[formattedLine.length() - 1] == ' ')
		{
			spacePadNum -= formattedLine.length() - 1 - nextText;
			int lastText = formattedLine.find_last_not_of(" \t");
			formattedLine.resize(lastText + 1);
		}
		if (spaces > 0)
		{
			// do not use goForward here
			currentLine.erase(charNum + 1, spaces);
			spacePadNum -= spaces;
		}
	}
}

/**
* add or remove space padding to objective-c parens
* these options have precedence over the padParens methods
* the padParens method has already been called, this method adjusts
*/
void ASFormatter::padObjCParamType()
{
	assert((currentChar == '(' || currentChar == ')') && isInObjCMethodDefinition);
	assert(!isImmediatelyPostObjCMethodPrefix && !isInObjCReturnType);
	assert(shouldPadParamType || shouldUnPadParamType);

	if (currentChar == '(')
	{
		// open paren has already been attached to formattedLine by padParen
		size_t paramOpen = formattedLine.rfind('(');
		assert(paramOpen != string::npos);
		size_t prevText = formattedLine.find_last_not_of(" \t", paramOpen - 1);
		if (prevText == string::npos)
			return;
		int spaces = paramOpen - prevText - 1;

		if (shouldPadParamType
		        || objCColonPadMode == COLON_PAD_ALL
		        || objCColonPadMode == COLON_PAD_AFTER)
		{
			if (spaces == 0)
			{
				formattedLine.insert(paramOpen, 1, ' ');
				spacePadNum += 1;
			}
			if (spaces > 1)
			{
				formattedLine.erase(prevText + 1, spaces - 1);
				spacePadNum -= spaces - 1;
			}
		}
		// this option will be ignored if used with pad-param-type
		else if (shouldUnPadParamType
		         || objCColonPadMode == COLON_PAD_NONE
		         || objCColonPadMode == COLON_PAD_BEFORE)
		{
			if (spaces > 0)
			{
				formattedLine.erase(prevText + 1, spaces);
				spacePadNum -= spaces;
			}
		}
	}
	else if (currentChar == ')')
	{
		size_t nextText = currentLine.find_first_not_of(" \t", charNum + 1);
		if (nextText == string::npos)
			return;
		int spaces = nextText - charNum - 1;

		if (shouldPadParamType)
		{
			if (spaces == 0)
			{
				// this will already be padded if pad-paren is used
				if (formattedLine[formattedLine.length() - 1] != ' ')
				{
					formattedLine.append(" ");
					spacePadNum += 1;
				}
			}
			else if (spaces > 1)
			{
				// do not use goForward here
				currentLine.erase(charNum + 1, spaces - 1);
				spacePadNum -= spaces - 1;
			}
		}
		// this option will be ignored if used with pad-param-type
		else if (shouldUnPadParamType)
		{
			// this will already be padded if pad-paren is used
			if (formattedLine[formattedLine.length() - 1] == ' ')
			{
				spacePadNum -= 1;
				int lastText = formattedLine.find_last_not_of(" \t");
				formattedLine.resize(lastText + 1);
			}
			if (spaces > 0)
			{
				// do not use goForward here
				currentLine.erase(charNum + 1, spaces);
				spacePadNum -= spaces;
			}
		}
	}
}

/**
 * format opening brace as attached or broken
 * currentChar contains the brace
 * the braces will be appended to the current formattedLine or a new formattedLine as necessary
 * the calling function should have a continue statement after calling this method
 *
 * @param braceType    the type of brace to be formatted.
 */
void ASFormatter::formatOpeningBrace(BraceType braceType)
{
	assert(!isBraceType(braceType, ARRAY_TYPE));
	assert(currentChar == '{');

	parenStack->emplace_back(0);

	bool breakBrace = isCurrentBraceBroken();

	if (breakBrace)
	{
		if (isBeforeAnyComment() && isOkToBreakBlock(braceType))
		{
			// if comment is at line end leave the comment on this line
			if (isBeforeAnyLineEndComment(charNum) && !currentLineBeginsWithBrace)
			{
				currentChar = ' ';              // remove brace from current line
				if (parenStack->size() > 1)
					parenStack->pop_back();
				currentLine[charNum] = currentChar;
				appendOpeningBrace = true;      // append brace to following line
			}
			// else put comment after the brace
			else if (!isBeforeMultipleLineEndComments(charNum))
				breakLine();
		}
		else if (!isBraceType(braceType, SINGLE_LINE_TYPE))
		{
			formattedLine = rtrim(formattedLine);
			breakLine();
		}
		else if ((shouldBreakOneLineBlocks || isBraceType(braceType, BREAK_BLOCK_TYPE))
		         && !isBraceType(braceType, EMPTY_BLOCK_TYPE))
			breakLine();
		else if (!isInLineBreak)
			appendSpacePad();

		appendCurrentChar();

		// should a following comment break from the brace?
		// must break the line AFTER the brace
		if (isBeforeComment()
		        && formattedLine.length() > 0
		        && formattedLine[0] == '{'
		        && isOkToBreakBlock(braceType)
		        && (braceFormatMode == BREAK_MODE
		            || braceFormatMode == LINUX_MODE))
		{
			shouldBreakLineAtNextChar = true;
		}
	}
	else    // attach brace
	{
		// are there comments before the brace?
		if (isCharImmediatelyPostComment || isCharImmediatelyPostLineComment)
		{
			if (isOkToBreakBlock(braceType)
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
			else if (isOkToBreakBlock(braceType)
			         && !(isImmediatelyPostPreprocessor
			              && currentLineBeginsWithBrace))
			{
				if (!isBraceType(braceType, EMPTY_BLOCK_TYPE))
				{
					appendSpacePad();
					appendCurrentChar(false);				// OK to attach
					testForTimeToSplitFormattedLine();		// line length will have changed
					// should a following comment attach with the brace?
					// insert spaces to reposition the comment
					if (isBeforeComment()
					        && !isBeforeMultipleLineEndComments(charNum)
					        && (!isBeforeAnyLineEndComment(charNum)	|| currentLineBeginsWithBrace))
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
					if (currentLineBeginsWithBrace && charNum == (int) currentLineFirstBraceNum)
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
 * format closing brace
 * currentChar contains the brace
 * the calling function should have a continue statement after calling this method
 *
 * @param braceType    the type of the opening brace for this closing brace.
 */
void ASFormatter::formatClosingBrace(BraceType braceType)
{
	assert(!isBraceType(braceType, ARRAY_TYPE));
	assert(currentChar == '}');

	// parenStack must contain one entry
	if (parenStack->size() > 1)
		parenStack->pop_back();

	// mark state of immediately after empty block
	// this state will be used for locating braces that appear immediately AFTER an empty block (e.g. '{} \n}').
	if (previousCommandChar == '{')
		isImmediatelyPostEmptyBlock = true;

	if (attachClosingBraceMode)
	{
		// for now, namespaces and classes will be attached. Uncomment the lines below to break.
		if ((isEmptyLine(formattedLine)			// if a blank line precedes this
		        || isCharImmediatelyPostLineComment
		        || isCharImmediatelyPostComment
		        || (isImmediatelyPostPreprocessor && (int) currentLine.find_first_not_of(" \t") == charNum)
//		        || (isBraceType(braceType, CLASS_TYPE) && isOkToBreakBlock(braceType) && previousNonWSChar != '{')
//		        || (isBraceType(braceType, NAMESPACE_TYPE) && isOkToBreakBlock(braceType) && previousNonWSChar != '{')
		    )
		        && (!isBraceType(braceType, SINGLE_LINE_TYPE) || isOkToBreakBlock(braceType)))
		{
			breakLine();
			appendCurrentChar();				// don't attach
		}
		else
		{
			if (previousNonWSChar != '{'
			        && (!isBraceType(braceType, SINGLE_LINE_TYPE)
			            || isOkToBreakBlock(braceType)))
				appendSpacePad();
			appendCurrentChar(false);			// attach
		}
	}
	else if (!isBraceType(braceType, EMPTY_BLOCK_TYPE)
	         && (isBraceType(braceType, BREAK_BLOCK_TYPE)
	             || isOkToBreakBlock(braceType)))
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
	        && currentHeader != nullptr
	        && !isHeaderInMultiStatementLine
	        && parenStack->back() == 0)
	{
		if (currentHeader == &AS_CASE || currentHeader == &AS_DEFAULT)
		{
			// do not yet insert a line if "break" statement is outside the braces
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
 * format array braces as attached or broken
 * determine if the braces can have an inStatement indent
 * currentChar contains the brace
 * the braces will be appended to the current formattedLine or a new formattedLine as necessary
 * the calling function should have a continue statement after calling this method
 *
 * @param braceType            the type of brace to be formatted, must be an ARRAY_TYPE.
 * @param isOpeningArrayBrace  indicates if this is the opening brace for the array block.
 */
void ASFormatter::formatArrayBraces(BraceType braceType, bool isOpeningArrayBrace)
{
	assert(isBraceType(braceType, ARRAY_TYPE));
	assert(currentChar == '{' || currentChar == '}');

	if (currentChar == '{')
	{
		// is this the first opening brace in the array?
		if (isOpeningArrayBrace)
		{
			if (braceFormatMode == ATTACH_MODE
			        || braceFormatMode == LINUX_MODE)
			{
				// break an enum if mozilla
				if (isBraceType(braceType, ENUM_TYPE)
				        && formattingStyle == STYLE_MOZILLA)
				{
					isInLineBreak = true;
					appendCurrentChar();                // don't attach
				}
				// don't attach to a preprocessor directive or '\' line
				else if ((isImmediatelyPostPreprocessor
				          || (formattedLine.length() > 0
				              && formattedLine[formattedLine.length() - 1] == '\\'))
				         && currentLineBeginsWithBrace)
				{
					isInLineBreak = true;
					appendCurrentChar();                // don't attach
				}
				else if (isCharImmediatelyPostComment)
				{
					// TODO: attach brace to line-end comment
					appendCurrentChar();                // don't attach
				}
				else if (isCharImmediatelyPostLineComment && !isBraceType(braceType, SINGLE_LINE_TYPE))
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
						// if brace is broken or not an assignment
						if (currentLineBeginsWithBrace
						        && !isBraceType(braceType, SINGLE_LINE_TYPE))
						{
							appendSpacePad();
							appendCurrentChar(false);				// OK to attach
							// TODO: debug the following line
							testForTimeToSplitFormattedLine();		// line length will have changed

							if (currentLineBeginsWithBrace
							        && (int) currentLineFirstBraceNum == charNum)
								shouldBreakLineAtNextChar = true;
						}
						else
						{
							if (previousNonWSChar != '(')
							{
								// don't space pad C++11 uniform initialization
								if (!isBraceType(braceType, INIT_TYPE))
									appendSpacePad();
							}
							appendCurrentChar();
						}
					}
				}
			}
			else if (braceFormatMode == BREAK_MODE)
			{
				if (isWhiteSpace(peekNextChar()) && !isInVirginLine)
					breakLine();
				else if (isBeforeAnyComment())
				{
					// do not break unless comment is at line end
					if (isBeforeAnyLineEndComment(charNum) && !currentLineBeginsWithBrace)
					{
						currentChar = ' ';            // remove brace from current line
						appendOpeningBrace = true;    // append brace to following line
					}
				}
				if (!isInLineBreak && previousNonWSChar != '(')
				{
					// don't space pad C++11 uniform initialization
					if (!isBraceType(braceType, INIT_TYPE))
						appendSpacePad();
				}
				appendCurrentChar();

				if (currentLineBeginsWithBrace
				        && (int) currentLineFirstBraceNum == charNum
				        && !isBraceType(braceType, SINGLE_LINE_TYPE))
					shouldBreakLineAtNextChar = true;
			}
			else if (braceFormatMode == RUN_IN_MODE)
			{
				if (isWhiteSpace(peekNextChar()) && !isInVirginLine)
					breakLine();
				else if (isBeforeAnyComment())
				{
					// do not break unless comment is at line end
					if (isBeforeAnyLineEndComment(charNum) && !currentLineBeginsWithBrace)
					{
						currentChar = ' ';            // remove brace from current line
						appendOpeningBrace = true;    // append brace to following line
					}
				}
				if (!isInLineBreak && previousNonWSChar != '(')
				{
					// don't space pad C++11 uniform initialization
					if (!isBraceType(braceType, INIT_TYPE))
						appendSpacePad();
				}
				appendCurrentChar();
			}
			else if (braceFormatMode == NONE_MODE)
			{
				if (currentLineBeginsWithBrace
				        && charNum == (int) currentLineFirstBraceNum)
				{
					appendCurrentChar();                // don't attach
				}
				else
				{
					if (previousNonWSChar != '(')
					{
						// don't space pad C++11 uniform initialization
						if (!isBraceType(braceType, INIT_TYPE))
							appendSpacePad();
					}
					appendCurrentChar(false);           // OK to attach
				}
			}
		}
		else	     // not the first opening brace
		{
			if (braceFormatMode == RUN_IN_MODE)
			{
				if (previousNonWSChar == '{'
				        && braceTypeStack->size() > 2
				        && !isBraceType((*braceTypeStack)[braceTypeStack->size() - 2],
				                        SINGLE_LINE_TYPE))
					formatArrayRunIn();
			}
			else if (!isInLineBreak
			         && !isWhiteSpace(peekNextChar())
			         && previousNonWSChar == '{'
			         && braceTypeStack->size() > 2
			         && !isBraceType((*braceTypeStack)[braceTypeStack->size() - 2],
			                         SINGLE_LINE_TYPE))
				formatArrayRunIn();

			appendCurrentChar();
		}
	}
	else if (currentChar == '}')
	{
		if (attachClosingBraceMode)
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
			// does this close the first opening brace in the array?
			// must check if the block is still a single line because of anonymous statements
			if (!isBraceType(braceType, INIT_TYPE)
			        && (!isBraceType(braceType, SINGLE_LINE_TYPE)
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
	assert(braceFormatMode == RUN_IN_MODE || braceFormatMode == NONE_MODE);

	// keep one line blocks returns true without indenting the run-in
	if (formattingStyle != STYLE_PICO
	        && !isOkToBreakBlock(braceTypeStack->back()))
		return; // true;

	// make sure the line begins with a brace
	size_t lastText = formattedLine.find_last_not_of(" \t");
	if (lastText == string::npos || formattedLine[lastText] != '{')
		return; // false;

	// make sure the brace is broken
	if (formattedLine.find_first_not_of(" \t{") != string::npos)
		return; // false;

	if (isBraceType(braceTypeStack->back(), NAMESPACE_TYPE))
		return; // false;

	bool extraIndent = false;
	bool extraHalfIndent = false;
	isInLineBreak = true;

	// cannot attach a class modifier without indent-classes
	if (isCStyle()
	        && isCharPotentialHeader(currentLine, charNum)
	        && (isBraceType(braceTypeStack->back(), CLASS_TYPE)
	            || (isBraceType(braceTypeStack->back(), STRUCT_TYPE)
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
	        && !preBraceHeaderStack->empty()
	        && preBraceHeaderStack->back() == &AS_SWITCH
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
		runInIndentChars = indentLength_ / 2;
		formattedLine.append(runInIndentChars - 1, ' ');
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
		indent.replace(0U, tabCount * tabLength_, tabCount, '\t');
		runInIndentChars = indentLength_;
		if (indent[0] == ' ')			// allow for brace
			indent.erase(0, 1);
		formattedLine.append(indent);
	}
	else if (getIndentString() == "\t")
	{
		appendChar('\t', false);
		runInIndentChars = 2;	// one for { and one for tab
		if (extraIndent)
		{
			appendChar('\t', false);
			runInIndentChars++;
		}
	}
	else // spaces
	{
		int indentLength_ = getIndentLength();
		formattedLine.append(indentLength_ - 1, ' ');
		runInIndentChars = indentLength_;
		if (extraIndent)
		{
			formattedLine.append(indentLength_, ' ');
			runInIndentChars += indentLength_;
		}
	}
	isInBraceRunIn = true;
}

/**
 * remove whitespace and add indentation for an array run-in.
 */
void ASFormatter::formatArrayRunIn()
{
	assert(isBraceType(braceTypeStack->back(), ARRAY_TYPE));

	// make sure the brace is broken
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
		runInIndentChars = 2;	// one for { and one for tab
	}
	else
	{
		int indent = getIndentLength();
		formattedLine.append(indent - 1, ' ');
		runInIndentChars = indent;
	}
	isInBraceRunIn = true;
	isInLineBreak = false;
}

/**
 * delete a braceTypeStack vector object
 * BraceTypeStack did not work with the DeleteContainer template
 */
void ASFormatter::deleteContainer(vector<BraceType>*& container)
{
	if (container != nullptr)
	{
		container->clear();
		delete (container);
		container = nullptr;
	}
}

/**
 * delete a vector object
 * T is the type of vector
 * used for all vectors except braceTypeStack
 */
template<typename T>
void ASFormatter::deleteContainer(T& container)
{
	if (container != nullptr)
	{
		container->clear();
		delete (container);
		container = nullptr;
	}
}

/**
 * initialize a braceType vector object
 * braceType did not work with the DeleteContainer template
 */
void ASFormatter::initContainer(vector<BraceType>*& container, vector<BraceType>* value)
{
	if (container != nullptr)
		deleteContainer(container);
	container = value;
}

/**
 * initialize a vector object
 * T is the type of vector
 * used for all vectors except braceTypeStack
 */
template<typename T>
void ASFormatter::initContainer(T& container, T value)
{
	// since the ASFormatter object is never deleted,
	// the existing vectors must be deleted before creating new ones
	if (container != nullptr)
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
	assert(currentChar == '\t');

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
bool ASFormatter::isOkToBreakBlock(BraceType braceType) const
{
	// Actually, there should not be an ARRAY_TYPE brace here.
	// But this will avoid breaking a one line block when there is.
	// Otherwise they will be formatted differently on consecutive runs.
	if (isBraceType(braceType, ARRAY_TYPE)
	        && isBraceType(braceType, SINGLE_LINE_TYPE))
		return false;
	if (isBraceType(braceType, COMMAND_TYPE)
	        && isBraceType(braceType, EMPTY_BLOCK_TYPE))
		return false;
	if (!isBraceType(braceType, SINGLE_LINE_TYPE)
	        || isBraceType(braceType, BREAK_BLOCK_TYPE)
	        || shouldBreakOneLineBlocks)
		return true;
	return false;
}

/**
* check if a sharp header is a paren or non-paren header
*/
bool ASFormatter::isSharpStyleWithParen(const string* header) const
{
	return (isSharpStyle() && peekNextChar() == '('
	        && (header == &AS_CATCH
	            || header == &AS_DELEGATE));
}

/**
 * Check for a following header when a comment is reached.
 * firstLine must contain the start of the comment.
 * return value is a pointer to the header or nullptr.
 */
const string* ASFormatter::checkForHeaderFollowingComment(const string& firstLine) const
{
	assert(isInComment || isInLineComment);
	assert(shouldBreakElseIfs || shouldBreakBlocks || isInSwitchStatement());
	// look ahead to find the next non-comment text
	bool endOnEmptyLine = (currentHeader == nullptr);
	if (isInSwitchStatement())
		endOnEmptyLine = false;
	string nextText = peekNextText(firstLine, endOnEmptyLine);

	if (nextText.length() == 0 || !isCharPotentialHeader(nextText, 0))
		return nullptr;

	return ASBase::findHeader(nextText, 0, headers);
}

/**
 * process preprocessor statements.
 * charNum should be the index of the #.
 *
 * delete braceTypeStack entries added by #if if a #else is found.
 * prevents double entries in the braceTypeStack.
 */
void ASFormatter::processPreprocessor()
{
	assert(currentChar == '#');

	const size_t preproc = currentLine.find_first_not_of(" \t", charNum + 1);

	if (preproc == string::npos)
		return;

	if (currentLine.compare(preproc, 2, "if") == 0)
	{
		preprocBraceTypeStackSize = braceTypeStack->size();
	}
	else if (currentLine.compare(preproc, 4, "else") == 0)
	{
		// delete stack entries added in #if
		// should be replaced by #else
		if (preprocBraceTypeStackSize > 0)
		{
			int addedPreproc = braceTypeStack->size() - preprocBraceTypeStackSize;
			for (int i = 0; i < addedPreproc; i++)
				braceTypeStack->pop_back();
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
	auto stream = make_shared<ASPeekStream>(sourceIterator);
	if (!stream->hasMoreLines())
		return false;
	string nextLine_ = stream->peekNextLine();
	size_t firstChar = nextLine_.find_first_not_of(" \t");
	if (firstChar == string::npos
	        || !(nextLine_.compare(firstChar, 2, "//") == 0
	             || nextLine_.compare(firstChar, 2, "/*") == 0))
		return false;

	// find the next non-comment text, and reset
	string nextText = peekNextText(nextLine_, false, stream);
	if (nextText.length() == 0 || !isCharPotentialHeader(nextText, 0))
		return false;

	const string* newHeader = ASBase::findHeader(nextText, 0, headers);

	if (newHeader == nullptr)
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
 * determine if a brace should be attached or broken
 * uses braces in the braceTypeStack
 * the last brace in the braceTypeStack is the one being formatted
 * returns true if the brace should be broken
 */
bool ASFormatter::isCurrentBraceBroken() const
{
	assert(braceTypeStack->size() > 1);

	bool breakBrace = false;
	size_t stackEnd = braceTypeStack->size() - 1;

	// check brace modifiers
	if (shouldAttachExternC
	        && isBraceType((*braceTypeStack)[stackEnd], EXTERN_TYPE))
	{
		return false;
	}
	if (shouldAttachNamespace
	        && isBraceType((*braceTypeStack)[stackEnd], NAMESPACE_TYPE))
	{
		return false;
	}
	if (shouldAttachClass
	        && (isBraceType((*braceTypeStack)[stackEnd], CLASS_TYPE)
	            || isBraceType((*braceTypeStack)[stackEnd], INTERFACE_TYPE)))
	{
		return false;
	}
	if (shouldAttachInline
	        && isCStyle()			// for C++ only
	        && braceFormatMode != RUN_IN_MODE
	        && !(currentLineBeginsWithBrace && peekNextChar() == '/')
	        && isBraceType((*braceTypeStack)[stackEnd], COMMAND_TYPE))
	{
		size_t i;
		for (i = 1; i < braceTypeStack->size(); i++)
			if (isBraceType((*braceTypeStack)[i], CLASS_TYPE)
			        || isBraceType((*braceTypeStack)[i], STRUCT_TYPE))
				return false;
	}

	// check braces
	if (isBraceType((*braceTypeStack)[stackEnd], EXTERN_TYPE))
	{
		if (currentLineBeginsWithBrace
		        || braceFormatMode == RUN_IN_MODE)
			breakBrace = true;
	}
	else if (braceFormatMode == NONE_MODE)
	{
		if (currentLineBeginsWithBrace
		        && (int) currentLineFirstBraceNum == charNum)
			breakBrace = true;
	}
	else if (braceFormatMode == BREAK_MODE || braceFormatMode == RUN_IN_MODE)
	{
		breakBrace = true;
	}
	else if (braceFormatMode == LINUX_MODE)
	{
		// break a namespace if NOT stroustrup or mozilla
		if (isBraceType((*braceTypeStack)[stackEnd], NAMESPACE_TYPE))
		{
			if (formattingStyle != STYLE_STROUSTRUP
			        && formattingStyle != STYLE_MOZILLA)
				breakBrace = true;
		}
		// break a class or interface if NOT stroustrup
		else if (isBraceType((*braceTypeStack)[stackEnd], CLASS_TYPE)
		         || isBraceType((*braceTypeStack)[stackEnd], INTERFACE_TYPE))
		{
			if (formattingStyle != STYLE_STROUSTRUP)
				breakBrace = true;
		}
		// break a struct if mozilla - an enum is processed as an array brace
		else if (isBraceType((*braceTypeStack)[stackEnd], STRUCT_TYPE))
		{
			if (formattingStyle == STYLE_MOZILLA)
				breakBrace = true;
		}
		// break the first brace if a function
		else if (isBraceType((*braceTypeStack)[stackEnd], COMMAND_TYPE))
		{
			if (stackEnd == 1)
			{
				breakBrace = true;
			}
			else if (stackEnd > 1)
			{
				// break the first brace after these if a function
				if (isBraceType((*braceTypeStack)[stackEnd - 1], NAMESPACE_TYPE)
				        || isBraceType((*braceTypeStack)[stackEnd - 1], CLASS_TYPE)
				        || isBraceType((*braceTypeStack)[stackEnd - 1], ARRAY_TYPE)
				        || isBraceType((*braceTypeStack)[stackEnd - 1], STRUCT_TYPE)
				        || isBraceType((*braceTypeStack)[stackEnd - 1], EXTERN_TYPE))
				{
					breakBrace = true;
				}
			}
		}
	}
	return breakBrace;
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
		if (isSequenceReached("*/"))
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
	const string* followingHeader = nullptr;
	if ((doesLineStartComment
	        && !isImmediatelyPostCommentOnly
	        && isBraceType(braceTypeStack->back(), COMMAND_TYPE))
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
		if (isBraceType(braceTypeStack->back(), NAMESPACE_TYPE))
		{
			// namespace run-in is always broken.
			isInLineBreak = true;
		}
		else if (braceFormatMode == NONE_MODE)
		{
			// should a run-in statement be attached?
			if (currentLineBeginsWithBrace)
				formatRunIn();
		}
		else if (braceFormatMode == ATTACH_MODE)
		{
			// if the brace was not attached?
			if (formattedLine.length() > 0 && formattedLine[0] == '{'
			        && !isBraceType(braceTypeStack->back(), SINGLE_LINE_TYPE))
				isInLineBreak = true;
		}
		else if (braceFormatMode == RUN_IN_MODE)
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
	        && followingHeader != nullptr
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
		currentHeader = nullptr;
}

/**
 * format a comment closer
 * the comment closer will be appended to the current formattedLine
 */
void ASFormatter::formatCommentCloser()
{
	assert(isSequenceReached("*/"));
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
	        && !isBraceType(braceTypeStack->back(), ARRAY_TYPE)
	        && !isInPreprocessor
	        && isOkToBreakBlock(braceTypeStack->back()))
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
	const string* followingHeader = nullptr;
	if ((lineIsLineCommentOnly
	        && !isImmediatelyPostCommentOnly
	        && isBraceType(braceTypeStack->back(), COMMAND_TYPE))
	        && (shouldBreakElseIfs
	            || isInSwitchStatement()
	            || (shouldBreakBlocks
	                && !isImmediatelyPostEmptyLine
	                && previousCommandChar != '{')))
		followingHeader = checkForHeaderFollowingComment(currentLine.substr(charNum));

	// do not indent if in column 1 or 2
	// or in a namespace before the opening brace
	if ((!shouldIndentCol1Comments && !lineCommentNoIndent)
	        || foundNamespaceHeader)
	{
		if (charNum == 0)
			lineCommentNoIndent = true;
		else if (charNum == 1 && currentLine[0] == ' ')
			lineCommentNoIndent = true;
	}
	// move comment if spaces were added or deleted
	if (!lineCommentNoIndent && spacePadNum != 0 && !isInLineBreak)
		adjustComments();
	formattedLineCommentNum = formattedLine.length();

	// must be done BEFORE appendSequence
	// check for run-in statement
	if (previousCommandChar == '{'
	        && !isImmediatelyPostComment
	        && !isImmediatelyPostLineComment)
	{
		if (braceFormatMode == NONE_MODE)
		{
			if (currentLineBeginsWithBrace)
				formatRunIn();
		}
		else if (braceFormatMode == RUN_IN_MODE)
		{
			if (!lineCommentNoIndent)
				formatRunIn();
			else
				isInLineBreak = true;
		}
		else if (braceFormatMode == BREAK_MODE)
		{
			if (formattedLine.length() > 0 && formattedLine[0] == '{')
				isInLineBreak = true;
		}
		else
		{
			if (currentLineBeginsWithBrace)
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
	        && followingHeader != nullptr
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
		currentHeader = nullptr;

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
			if ((int) currentLine.length() > charNum + 1
			        && currentLine[charNum + 1] == '"')			// check consecutive quotes
			{
				appendSequence("\"\"");
				goForward(1);
				return;
			}
			isInQuote = false;
			isInVerbatimQuote = false;
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
	if (charNum + 1 >= (int) currentLine.length()
	        && currentChar != '\\'
	        && !isInVerbatimQuote)
		isInQuote = false;				// missing closing quote
}

/**
 * format a quote opener
 * the quote opener will be appended to the current formattedLine or a new formattedLine as necessary
 * the calling function should have a continue statement after calling this method
 */
void ASFormatter::formatQuoteOpener()
{
	assert(currentChar == '"'
	       || (currentChar == '\'' && !isDigitSeparator(currentLine, charNum)));

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

	// a quote following a brace is an array
	if (previousCommandChar == '{'
	        && !isImmediatelyPostComment
	        && !isImmediatelyPostLineComment
	        && isNonInStatementArray
	        && !isBraceType(braceTypeStack->back(), SINGLE_LINE_TYPE)
	        && !isWhiteSpace(peekNextChar()))
	{
		if (braceFormatMode == NONE_MODE)
		{
			if (currentLineBeginsWithBrace)
				formatRunIn();
		}
		else if (braceFormatMode == RUN_IN_MODE)
		{
			formatRunIn();
		}
		else if (braceFormatMode == BREAK_MODE)
		{
			if (formattedLine.length() > 0 && formattedLine[0] == '{')
				isInLineBreak = true;
		}
		else
		{
			if (currentLineBeginsWithBrace)
				isInLineBreak = true;
		}
	}
	previousCommandChar = ' ';
	appendCurrentChar();
}

/**
 * get the next line comment adjustment that results from breaking a closing brace.
 * the brace must be on the same line as the closing header.
 * i.e "} else" changed to "} <NL> else".
 */
int ASFormatter::getNextLineCommentAdjustment()
{
	assert(foundClosingHeader && previousNonWSChar == '}');
	if (charNum < 1)			// "else" is in column 1
		return 0;
	size_t lastBrace = currentLine.rfind('}', charNum - 1);
	if (lastBrace != string::npos)
		return (lastBrace - charNum);	// return a negative number
	return 0;
}

// for console build only
LineEndFormat ASFormatter::getLineEndFormat() const
{
	return lineEnd;
}

/**
 * get the current line comment adjustment that results from attaching
 * a closing header to a closing brace.
 * the brace must be on the line previous to the closing header.
 * the adjustment is 2 chars, one for the brace and one for the space.
 * i.e "} <NL> else" changed to "} else".
 */
int ASFormatter::getCurrentLineCommentAdjustment()
{
	assert(foundClosingHeader && previousNonWSChar == '}');
	if (charNum < 1)
		return 2;
	size_t lastBrace = currentLine.rfind('}', charNum - 1);
	if (lastBrace == string::npos)
		return 2;
	return 0;
}

/**
 * get the previous word on a line
 * the argument 'currPos' must point to the current position.
 *
 * @return is the previous word or an empty string if none found.
 */
string ASFormatter::getPreviousWord(const string& line, int currPos) const
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
 * check if a line break is needed when a closing brace
 * is followed by a closing header.
 * the break depends on the braceFormatMode and other factors.
 */
void ASFormatter::isLineBreakBeforeClosingHeader()
{
	assert(foundClosingHeader && previousNonWSChar == '}');

	if (currentHeader == &AS_WHILE && shouldAttachClosingWhile)
	{
		appendClosingHeader();
		return;
	}

	if (braceFormatMode == BREAK_MODE
	        || braceFormatMode == RUN_IN_MODE
	        || attachClosingBraceMode)
	{
		isInLineBreak = true;
	}
	else if (braceFormatMode == NONE_MODE)
	{
		if (shouldBreakClosingHeaderBraces
		        || getBraceIndent() || getBlockIndent())
		{
			isInLineBreak = true;
		}
		else
		{
			appendSpacePad();
			// is closing brace broken?
			size_t i = currentLine.find_first_not_of(" \t");
			if (i != string::npos && currentLine[i] == '}')
				isInLineBreak = false;

			if (shouldBreakBlocks)
				isAppendPostBlockEmptyLineRequested = false;
		}
	}
	// braceFormatMode == ATTACH_MODE, LINUX_MODE
	else
	{
		if (shouldBreakClosingHeaderBraces
		        || getBraceIndent() || getBlockIndent())
		{
			isInLineBreak = true;
		}
		else
		{
			appendClosingHeader();
			if (shouldBreakBlocks)
				isAppendPostBlockEmptyLineRequested = false;
		}
	}
}

/**
 * Append a closing header to the previous closing brace, if possible
 */
void ASFormatter::appendClosingHeader()
{
	// if a blank line does not precede this
	// or last line is not a one line block, attach header
	bool previousLineIsEmpty = isEmptyLine(formattedLine);
	int previousLineIsOneLineBlock = 0;
	size_t firstBrace = findNextChar(formattedLine, '{');
	if (firstBrace != string::npos)
		previousLineIsOneLineBlock = isOneLineBlockReached(formattedLine, firstBrace);
	if (!previousLineIsEmpty
	        && previousLineIsOneLineBlock == 0)
	{
		isInLineBreak = false;
		appendSpacePad();
		spacePadNum = 0;	// don't count as comment padding
	}
}

/**
 * Add braces to a single line statement following a header.
 * braces are not added if the proper conditions are not met.
 * braces are added to the currentLine.
 */
bool ASFormatter::addBracesToStatement()
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

	// do not brace an empty statement
	if (currentChar == ';')
		return false;

	// do not add if a header follows
	if (isCharPotentialHeader(currentLine, charNum))
		if (findHeader(headers) != nullptr)
			return false;

	// find the next semi-colon
	size_t nextSemiColon = charNum;
	if (currentChar != ';')
		nextSemiColon = findNextChar(currentLine, ';', charNum + 1);
	if (nextSemiColon == string::npos)
		return false;

	// add closing brace before changing the line length
	if (nextSemiColon == currentLine.length() - 1)
		currentLine.append(" }");
	else
		currentLine.insert(nextSemiColon + 1, " }");
	// add opening brace
	currentLine.insert(charNum, "{ ");
	assert(computeChecksumIn("{}"));
	currentChar = '{';
	if ((int) currentLine.find_first_not_of(" \t") == charNum)
		currentLineBeginsWithBrace = true;
	// remove extra spaces
	if (!shouldAddOneLineBraces)
	{
		size_t lastText = formattedLine.find_last_not_of(" \t");
		if ((formattedLine.length() - 1) - lastText > 1)
			formattedLine.erase(lastText + 1);
	}
	return true;
}

/**
 * Remove braces from a single line statement following a header.
 * braces are not removed if the proper conditions are not met.
 * The first brace is replaced by a space.
 */
bool ASFormatter::removeBracesFromStatement()
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
	string nextLine_;
	// leave nextLine_ empty if end of line comment follows
	if (!isBeforeAnyLineEndComment(charNum) || currentLineBeginsWithBrace)
		nextLine_ = currentLine.substr(charNum + 1);
	size_t nextChar = 0;

	// find the first non-blank text
	ASPeekStream stream(sourceIterator);
	while (stream.hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
		{
			nextLine_ = stream.peekNextLine();
			nextChar = 0;
		}

		nextChar = nextLine_.find_first_not_of(" \t", nextChar);
		if (nextChar != string::npos)
			break;
	}

	// don't remove if comments or a header follow the brace
	if ((nextLine_.compare(nextChar, 2, "/*") == 0)
	        || (nextLine_.compare(nextChar, 2, "//") == 0)
	        || (isCharPotentialHeader(nextLine_, nextChar)
	            && ASBase::findHeader(nextLine_, nextChar, headers) != nullptr))
		return false;

	// find the next semi-colon
	size_t nextSemiColon = nextChar;
	if (nextLine_[nextChar] != ';')
		nextSemiColon = findNextChar(nextLine_, ';', nextChar + 1);
	if (nextSemiColon == string::npos)
		return false;

	// find the closing brace
	isFirstLine = true;
	nextChar = nextSemiColon + 1;
	while (stream.hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
		{
			nextLine_ = stream.peekNextLine();
			nextChar = 0;
		}
		nextChar = nextLine_.find_first_not_of(" \t", nextChar);
		if (nextChar != string::npos)
			break;
	}
	if (nextLine_.length() == 0 || nextLine_[nextChar] != '}')
		return false;

	// remove opening brace
	currentLine[charNum] = currentChar = ' ';
	assert(adjustChecksumIn(-'{'));
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
size_t ASFormatter::findNextChar(const string& line, char searchChar, int searchStart /*0*/) const
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
		if (line[i] == '"'
		        || (line[i] == '\'' && !isDigitSeparator(line, i)))
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

		// for now don't process C# 'delegate' braces
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
bool ASFormatter::isStructAccessModified(const string& firstLine, size_t index) const
{
	assert(firstLine[index] == '{');
	assert(isCStyle());

	bool isFirstLine = true;
	size_t braceCount = 1;
	string nextLine_ = firstLine.substr(index + 1);
	ASPeekStream stream(sourceIterator);

	// find the first non-blank text, bypassing all comments and quotes.
	bool isInComment_ = false;
	bool isInQuote_ = false;
	char quoteChar_ = ' ';
	while (stream.hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
			nextLine_ = stream.peekNextLine();
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

			if (nextLine_[i] == '"'
			        || (nextLine_[i] == '\'' && !isDigitSeparator(nextLine_, i)))
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
			// handle braces
			if (nextLine_[i] == '{')
				++braceCount;
			if (nextLine_[i] == '}')
				--braceCount;
			if (braceCount == 0)
				return false;
			// check for access modifiers
			if (isCharPotentialHeader(nextLine_, i))
			{
				if (findKeyword(nextLine_, i, AS_PUBLIC)
				        || findKeyword(nextLine_, i, AS_PRIVATE)
				        || findKeyword(nextLine_, i, AS_PROTECTED))
					return true;
				string name = getCurrentWord(nextLine_, i);
				i += name.length() - 1;
			}
		}	// end of for loop
	}	// end of while loop

	return false;
}

/**
* Look ahead in the file to see if a preprocessor block is indentable.
*
* @param firstLine     a reference to the line to indent.
* @param index         the current line index.
* @return              true if the block is indentable.
*/
bool ASFormatter::isIndentablePreprocessorBlock(const string& firstLine, size_t index)
{
	assert(firstLine[index] == '#');

	bool isFirstLine = true;
	bool isInIndentableBlock = false;
	bool blockContainsBraces = false;
	bool blockContainsDefineContinuation = false;
	bool isInClassConstructor = false;
	bool isPotentialHeaderGuard = false;	// ifndef is first preproc statement
	bool isPotentialHeaderGuard2 = false;	// define is within the first proproc
	int  numBlockIndents = 0;
	int  lineParenCount = 0;
	string nextLine_ = firstLine.substr(index);
	auto stream = make_shared<ASPeekStream>(sourceIterator);

	// find end of the block, bypassing all comments and quotes.
	bool isInComment_ = false;
	bool isInQuote_ = false;
	char quoteChar_ = ' ';
	while (stream->hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
			nextLine_ = stream->peekNextLine();
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

			if (nextLine_[i] == '"'
			        || (nextLine_[i] == '\'' && !isDigitSeparator(nextLine_, i)))
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
						if (isNDefPreprocStatement(nextLine_, preproc))
							isPotentialHeaderGuard = true;
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
				else if (preproc == "define")
				{
					if (nextLine_[nextLine_.length() - 1] == '\\')
						blockContainsDefineContinuation = true;
					// check for potential header include guards
					else if (isPotentialHeaderGuard && numBlockIndents == 1)
						isPotentialHeaderGuard2 = true;
				}
				i = nextLine_.length();
				continue;
			}
			// handle exceptions
			if (nextLine_[i] == '{' || nextLine_[i] == '}')
				blockContainsBraces = true;
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
			if (blockContainsBraces || isInClassConstructor || blockContainsDefineContinuation)
				goto EndOfWhileLoop;
		}	// end of for loop, end of line
		if (lineParenCount != 0)
			break;
	}	// end of while loop
EndOfWhileLoop:
	preprocBlockEnd = sourceIterator->tellg();
	if (preprocBlockEnd < 0)
		preprocBlockEnd = sourceIterator->getStreamLength();
	if (blockContainsBraces
	        || isInClassConstructor
	        || blockContainsDefineContinuation
	        || lineParenCount != 0
	        || numBlockIndents != 0)
		isInIndentableBlock = false;
	// find next executable instruction
	// this WILL RESET the get pointer
	string nextText = peekNextText("", false, stream);
	// bypass header include guards
	if (isFirstPreprocConditional)
	{
		isFirstPreprocConditional = false;
		if (nextText.empty() && isPotentialHeaderGuard2)
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

bool ASFormatter::isNDefPreprocStatement(const string& nextLine_, const string& preproc) const
{
	if (preproc == "ifndef")
		return true;
	// check for '!defined'
	if (preproc == "if")
	{
		size_t i = nextLine_.find('!');
		if (i == string::npos)
			return false;
		i = nextLine_.find_first_not_of(" \t", ++i);
		if (i != string::npos && nextLine_.compare(i, 7, "defined") == 0)
			return true;
	}
	return false;
}

/**
 * Check to see if this is an EXEC SQL statement.
 *
 * @param line          a reference to the line to indent.
 * @param index         the current line index.
 * @return              true if the statement is EXEC SQL.
 */
bool ASFormatter::isExecSQL(const string& line, size_t index) const
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
	size_t paren = currentLine.rfind(')', charNum);
	if (paren != string::npos)
		line = currentLine;
	// if not on currentLine it must be on the previous line
	else
	{
		line = readyFormattedLine;
		paren = line.rfind(')');
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
	int parenDepth_ = 0;
	int maxTemplateDepth = 0;
	templateDepth = 0;
	string nextLine_ = currentLine.substr(charNum);
	ASPeekStream stream(sourceIterator);

	// find the angle braces, bypassing all comments and quotes.
	bool isInComment_ = false;
	bool isInQuote_ = false;
	char quoteChar_ = ' ';
	while (stream.hasMoreLines() || isFirstLine)
	{
		if (isFirstLine)
			isFirstLine = false;
		else
			nextLine_ = stream.peekNextLine();
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

			if (currentChar_ == '"'
			        || (currentChar_ == '\'' && !isDigitSeparator(nextLine_, i)))
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
					return;
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
				templateDepth = 0;
				return;
			}
			else if (nextLine_.compare(i, 2, AS_AND) == 0
			         || nextLine_.compare(i, 2, AS_OR) == 0)
			{
				// this is not a template -> leave...
				isInTemplate = false;
				templateDepth = 0;
				return;
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
				return;
			}
			string name = getCurrentWord(nextLine_, i);
			i += name.length() - 1;
		}	// end for loop
	}	// end while loop
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

	// don't split before or after a brace
	if (appendedChar == '{' || appendedChar == '}'
	        || previousNonWSChar == '{' || previousNonWSChar == '}'
	        || nextChar == '{' || nextChar == '}'
	        || currentChar == '{' || currentChar == '}')	// currentChar tests for an appended brace
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
				parenNum = formattedLine.length() - 1;
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

void ASFormatter::updateFormattedLineSplitPointsOperator(const string& sequence)
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
	if (sequence == "||" || sequence == "&&" || sequence == "or" || sequence == "and")
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
	else if (sequence == "==" || sequence == "!=" || sequence == ">=" || sequence == "<=")
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
		        && !(sequence == "+" && isInExponent())
		        && !(sequence == "-"  && isInExponent())
		        && (isLegalNameChar(currentLine[charNum - 1])
		            || currentLine[charNum - 1] == ')'
		            || currentLine[charNum - 1] == ']'
		            || currentLine[charNum - 1] == '\"'))
		{
			if (formattedLine.length() - 1 <= maxCodeLength)
				maxWhiteSpace = formattedLine.length() - 1;
			else
				maxWhiteSpacePending = formattedLine.length() - 1;
		}
	}
	// unpadded operators that will USUALLY split AFTER the operator (counts as whitespace)
	else if (sequence == "=" || sequence == ":")
	{
		// split BEFORE if the line is too long
		// do NOT use <= here, must allow for a brace attached to an array
		size_t splitPoint = 0;
		if (formattedLine.length() < maxCodeLength)
			splitPoint = formattedLine.length();
		else
			splitPoint = formattedLine.length() - 1;
		// padded or unpadded arrays
		if (previousNonWSChar == ']')
		{
			if (formattedLine.length() - 1 <= maxCodeLength)
				maxWhiteSpace = splitPoint;
			else
				maxWhiteSpacePending = splitPoint;
		}
		else if (charNum > 0
		         && (isLegalNameChar(currentLine[charNum - 1])
		             || currentLine[charNum - 1] == ')'
		             || currentLine[charNum - 1] == ']'))
		{
			if (formattedLine.length() <= maxCodeLength)
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

	if (!isOkToBreakBlock(braceTypeStack->back()) && currentChar != '{')
	{
		shouldKeepLineUnbroken = true;
		clearFormattedLineSplitPoints();
		return false;
	}
	if (isBraceType(braceTypeStack->back(), ARRAY_TYPE))
	{
		shouldKeepLineUnbroken = true;
		if (!isBraceType(braceTypeStack->back(), ARRAY_NIS_TYPE))
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
bool ASFormatter::computeChecksumIn(const string& currentLine_)
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
bool ASFormatter::computeChecksumOut(const string& beautifiedLine)
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
		return nullptr;

	if (!isLegalNameChar(currentLine[nextNum]))
		return nullptr;

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
		return nullptr;

	const string* newOperator = ASBase::findOperator(currentLine, nextNum, operators);
	return newOperator;
}

// Check following data to determine if the current character is an array operator.
bool ASFormatter::isArrayOperator() const
{
	assert(currentChar == '*' || currentChar == '&' || currentChar == '^');
	assert(isBraceType(braceTypeStack->back(), ARRAY_TYPE));

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
	foundTrailingReturnType = false;
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
	nonInStatementBrace = 0;
	while (!questionMarkStack->empty())
		questionMarkStack->pop_back();
}

// Find the colon alignment for Objective-C method definitions and method calls.
int ASFormatter::findObjCColonAlignment() const
{
	assert(currentChar == '+' || currentChar == '-' || currentChar == '[');
	assert(getAlignMethodColon());

	bool isFirstLine = true;
	bool haveFirstColon = false;
	bool foundMethodColon = false;
	bool isInComment_ = false;
	bool isInQuote_ = false;
	char quoteChar_ = ' ';
	int  sqBracketCount = 0;
	int  colonAdjust = 0;
	int  colonAlign = 0;
	string nextLine_ = currentLine;
	ASPeekStream stream(sourceIterator);

	// peek next line
	while (sourceIterator->hasMoreLines() || isFirstLine)
	{
		if (!isFirstLine)
			nextLine_ = stream.peekNextLine();
		// parse the line
		haveFirstColon = false;
		nextLine_ = ASBeautifier::trim(nextLine_);
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

			if (nextLine_[i] == '"'
			        || (nextLine_[i] == '\'' && !isDigitSeparator(nextLine_, i)))
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
			// process the current char
			if ((nextLine_[i] == '{' && (currentChar == '-' || currentChar == '+'))
			        || nextLine_[i] == ';')
				goto EndOfWhileLoop;       // end of method definition
			if (nextLine_[i] == ']')
			{
				--sqBracketCount;
				if (sqBracketCount == 0)
					goto EndOfWhileLoop;   // end of method call
			}
			if (nextLine_[i] == '[')
				++sqBracketCount;
			if (isFirstLine)	 // colon align does not include the first line
				continue;
			if (sqBracketCount > 1)
				continue;
			if (haveFirstColon)  // multiple colons per line
				continue;
			// compute colon adjustment
			if (nextLine_[i] == ':')
			{
				haveFirstColon = true;
				foundMethodColon = true;
				if (shouldPadMethodColon)
				{
					int spacesStart;
					for (spacesStart = i; spacesStart > 0; spacesStart--)
						if (!isWhiteSpace(nextLine_[spacesStart - 1]))
							break;
					int spaces = i - spacesStart;
					if (objCColonPadMode == COLON_PAD_ALL || objCColonPadMode == COLON_PAD_BEFORE)
						colonAdjust = 1 - spaces;
					else if (objCColonPadMode == COLON_PAD_NONE || objCColonPadMode == COLON_PAD_AFTER)
						colonAdjust = 0 - spaces;
				}
				// compute alignment
				int colonPosition = i + colonAdjust;
				if (colonPosition > colonAlign)
					colonAlign = colonPosition;
			}
		}	// end of for loop
		isFirstLine = false;
	}	// end of while loop
EndOfWhileLoop:
	if (!foundMethodColon)
		colonAlign = -1;
	return colonAlign;
}

// pad an Objective-C method colon
void ASFormatter::padObjCMethodColon()
{
	assert(currentChar == ':');
	int commentAdjust = 0;
	char nextChar = peekNextChar();
	if (objCColonPadMode == COLON_PAD_NONE
	        || objCColonPadMode == COLON_PAD_AFTER
	        || nextChar == ')')
	{
		// remove spaces before
		for (int i = formattedLine.length() - 1; (i > -1) && isWhiteSpace(formattedLine[i]); i--)
		{
			formattedLine.erase(i);
			--commentAdjust;
		}
	}
	else
	{
		// pad space before
		for (int i = formattedLine.length() - 1; (i > 0) && isWhiteSpace(formattedLine[i]); i--)
			if (isWhiteSpace(formattedLine[i - 1]))
			{
				formattedLine.erase(i);
				--commentAdjust;
			}
		appendSpacePad();
	}
	if (objCColonPadMode == COLON_PAD_NONE
	        || objCColonPadMode == COLON_PAD_BEFORE
	        || nextChar == ')')
	{
		// remove spaces after
		int nextText = currentLine.find_first_not_of(" \t", charNum + 1);
		if (nextText == (int)string::npos)
			nextText = currentLine.length();
		int spaces = nextText - charNum - 1;
		if (spaces > 0)
		{
			// do not use goForward here
			currentLine.erase(charNum + 1, spaces);
			spacePadNum -= spaces;
		}
	}
	else
	{
		// pad space after
		int nextText = currentLine.find_first_not_of(" \t", charNum + 1);
		if (nextText == (int)string::npos)
			nextText = currentLine.length();
		int spaces = nextText - charNum - 1;
		if (spaces == 0)
		{
			currentLine.insert(charNum + 1, 1, ' ');
			spacePadNum += 1;
		}
		else if (spaces > 1)
		{
			// do not use goForward here
			currentLine.erase(charNum + 1, spaces - 1);
			spacePadNum -= spaces - 1;
		}
	}
	spacePadNum += commentAdjust;
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
	if (formattedLine[firstChar] == '*')
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
