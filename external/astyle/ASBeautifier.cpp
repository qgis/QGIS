// ASBeautifier.cpp
// Copyright (c) 2017 by Jim Pattee <jimp03@email.com>.
// This code is licensed under the MIT License.
// License.md describes the conditions under which this software may be distributed.

//-----------------------------------------------------------------------------
// headers
//-----------------------------------------------------------------------------

#include "astyle.h"

#include <algorithm>

//-----------------------------------------------------------------------------
// astyle namespace
//-----------------------------------------------------------------------------

namespace astyle {
//
// this must be global
static int g_preprocessorCppExternCBrace;

//-----------------------------------------------------------------------------
// ASBeautifier class
//-----------------------------------------------------------------------------

/**
 * ASBeautifier's constructor
 * This constructor is called only once for each source file.
 * The cloned ASBeautifier objects are created with the copy constructor.
 */
ASBeautifier::ASBeautifier()
{
	waitingBeautifierStack = nullptr;
	activeBeautifierStack = nullptr;
	waitingBeautifierStackLengthStack = nullptr;
	activeBeautifierStackLengthStack = nullptr;

	headerStack = nullptr;
	tempStacks = nullptr;
	squareBracketDepthStack = nullptr;
	blockStatementStack = nullptr;
	parenStatementStack = nullptr;
	braceBlockStateStack = nullptr;
	continuationIndentStack = nullptr;
	continuationIndentStackSizeStack = nullptr;
	parenIndentStack = nullptr;
	preprocIndentStack = nullptr;
	sourceIterator = nullptr;
	isModeManuallySet = false;
	shouldForceTabIndentation = false;
	setSpaceIndentation(4);
	setContinuationIndentation(1);
	setMinConditionalIndentOption(MINCOND_TWO);
	setMaxContinuationIndentLength(40);
	classInitializerIndents = 1;
	tabLength = 0;
	setClassIndent(false);
	setModifierIndent(false);
	setSwitchIndent(false);
	setCaseIndent(false);
	setBlockIndent(false);
	setBraceIndent(false);
	setBraceIndentVtk(false);
	setNamespaceIndent(false);
	setAfterParenIndent(false);
	setLabelIndent(false);
	setEmptyLineFill(false);
	setCStyle();
	setPreprocDefineIndent(false);
	setPreprocConditionalIndent(false);
	setAlignMethodColon(false);

	// initialize ASBeautifier member vectors
	beautifierFileType = 9;		// reset to an invalid type
	headers = new vector<const string*>;
	nonParenHeaders = new vector<const string*>;
	assignmentOperators = new vector<const string*>;
	nonAssignmentOperators = new vector<const string*>;
	preBlockStatements = new vector<const string*>;
	preCommandHeaders = new vector<const string*>;
	indentableHeaders = new vector<const string*>;
}

/**
 * ASBeautifier's copy constructor
 * Copy the vector objects to vectors in the new ASBeautifier
 * object so the new object can be destroyed without deleting
 * the vector objects in the copied vector.
 * This is the reason a copy constructor is needed.
 *
 * Must explicitly call the base class copy constructor.
 */
ASBeautifier::ASBeautifier(const ASBeautifier& other) : ASBase(other)
{
	// these don't need to copy the stack
	waitingBeautifierStack = nullptr;
	activeBeautifierStack = nullptr;
	waitingBeautifierStackLengthStack = nullptr;
	activeBeautifierStackLengthStack = nullptr;

	// vector '=' operator performs a DEEP copy of all elements in the vector

	headerStack = new vector<const string*>;
	*headerStack = *other.headerStack;

	tempStacks = copyTempStacks(other);

	squareBracketDepthStack = new vector<int>;
	*squareBracketDepthStack = *other.squareBracketDepthStack;

	blockStatementStack = new vector<bool>;
	*blockStatementStack = *other.blockStatementStack;

	parenStatementStack = new vector<bool>;
	*parenStatementStack = *other.parenStatementStack;

	braceBlockStateStack = new vector<bool>;
	*braceBlockStateStack = *other.braceBlockStateStack;

	continuationIndentStack = new vector<int>;
	*continuationIndentStack = *other.continuationIndentStack;

	continuationIndentStackSizeStack = new vector<int>;
	*continuationIndentStackSizeStack = *other.continuationIndentStackSizeStack;

	parenIndentStack = new vector<int>;
	*parenIndentStack = *other.parenIndentStack;

	preprocIndentStack = new vector<pair<int, int> >;
	*preprocIndentStack = *other.preprocIndentStack;

	// Copy the pointers to vectors.
	// This is ok because the original ASBeautifier object
	// is not deleted until end of job.
	beautifierFileType = other.beautifierFileType;
	headers = other.headers;
	nonParenHeaders = other.nonParenHeaders;
	assignmentOperators = other.assignmentOperators;
	nonAssignmentOperators = other.nonAssignmentOperators;
	preBlockStatements = other.preBlockStatements;
	preCommandHeaders = other.preCommandHeaders;
	indentableHeaders = other.indentableHeaders;

	// protected variables
	// variables set by ASFormatter
	// must also be updated in activeBeautifierStack
	inLineNumber = other.inLineNumber;
	runInIndentContinuation = other.runInIndentContinuation;
	nonInStatementBrace = other.nonInStatementBrace;
	objCColonAlignSubsequent = other.objCColonAlignSubsequent;
	lineCommentNoBeautify = other.lineCommentNoBeautify;
	isElseHeaderIndent = other.isElseHeaderIndent;
	isCaseHeaderCommentIndent = other.isCaseHeaderCommentIndent;
	isNonInStatementArray = other.isNonInStatementArray;
	isSharpAccessor = other.isSharpAccessor;
	isSharpDelegate = other.isSharpDelegate;
	isInExternC = other.isInExternC;
	isInBeautifySQL = other.isInBeautifySQL;
	isInIndentableStruct = other.isInIndentableStruct;
	isInIndentablePreproc = other.isInIndentablePreproc;

	// private variables
	sourceIterator = other.sourceIterator;
	currentHeader = other.currentHeader;
	previousLastLineHeader = other.previousLastLineHeader;
	probationHeader = other.probationHeader;
	lastLineHeader = other.lastLineHeader;
	indentString = other.indentString;
	verbatimDelimiter = other.verbatimDelimiter;
	isInQuote = other.isInQuote;
	isInVerbatimQuote = other.isInVerbatimQuote;
	haveLineContinuationChar = other.haveLineContinuationChar;
	isInAsm = other.isInAsm;
	isInAsmOneLine = other.isInAsmOneLine;
	isInAsmBlock = other.isInAsmBlock;
	isInComment = other.isInComment;
	isInPreprocessorComment = other.isInPreprocessorComment;
	isInRunInComment = other.isInRunInComment;
	isInCase = other.isInCase;
	isInQuestion = other.isInQuestion;
	isContinuation = other.isContinuation;
	isInHeader = other.isInHeader;
	isInTemplate = other.isInTemplate;
	isInDefine = other.isInDefine;
	isInDefineDefinition = other.isInDefineDefinition;
	classIndent = other.classIndent;
	isIndentModeOff = other.isIndentModeOff;
	isInClassHeader = other.isInClassHeader;
	isInClassHeaderTab = other.isInClassHeaderTab;
	isInClassInitializer = other.isInClassInitializer;
	isInClass = other.isInClass;
	isInObjCMethodDefinition = other.isInObjCMethodDefinition;
	isInObjCMethodCall = other.isInObjCMethodCall;
	isInObjCMethodCallFirst = other.isInObjCMethodCallFirst;
	isImmediatelyPostObjCMethodDefinition = other.isImmediatelyPostObjCMethodDefinition;
	isImmediatelyPostObjCMethodCall = other.isImmediatelyPostObjCMethodCall;
	isInIndentablePreprocBlock = other.isInIndentablePreprocBlock;
	isInObjCInterface = other.isInObjCInterface;
	isInEnum = other.isInEnum;
	isInEnumTypeID = other.isInEnumTypeID;
	isInLet = other.isInLet;
	modifierIndent = other.modifierIndent;
	switchIndent = other.switchIndent;
	caseIndent = other.caseIndent;
	namespaceIndent = other.namespaceIndent;
	braceIndent = other.braceIndent;
	braceIndentVtk = other.braceIndentVtk;
	blockIndent = other.blockIndent;
	shouldIndentAfterParen = other.shouldIndentAfterParen;
	labelIndent = other.labelIndent;
	isInConditional = other.isInConditional;
	isModeManuallySet = other.isModeManuallySet;
	shouldForceTabIndentation = other.shouldForceTabIndentation;
	emptyLineFill = other.emptyLineFill;
	lineOpensWithLineComment = other.lineOpensWithLineComment;
	lineOpensWithComment = other.lineOpensWithComment;
	lineStartsInComment = other.lineStartsInComment;
	backslashEndsPrevLine = other.backslashEndsPrevLine;
	blockCommentNoIndent = other.blockCommentNoIndent;
	blockCommentNoBeautify = other.blockCommentNoBeautify;
	previousLineProbationTab = other.previousLineProbationTab;
	lineBeginsWithOpenBrace = other.lineBeginsWithOpenBrace;
	lineBeginsWithCloseBrace = other.lineBeginsWithCloseBrace;
	lineBeginsWithComma = other.lineBeginsWithComma;
	lineIsCommentOnly = other.lineIsCommentOnly;
	lineIsLineCommentOnly = other.lineIsLineCommentOnly;
	shouldIndentBracedLine = other.shouldIndentBracedLine;
	isInSwitch = other.isInSwitch;
	foundPreCommandHeader = other.foundPreCommandHeader;
	foundPreCommandMacro = other.foundPreCommandMacro;
	shouldAlignMethodColon = other.shouldAlignMethodColon;
	shouldIndentPreprocDefine = other.shouldIndentPreprocDefine;
	shouldIndentPreprocConditional = other.shouldIndentPreprocConditional;
	indentCount = other.indentCount;
	spaceIndentCount = other.spaceIndentCount;
	spaceIndentObjCMethodAlignment = other.spaceIndentObjCMethodAlignment;
	bracePosObjCMethodAlignment = other.bracePosObjCMethodAlignment;
	colonIndentObjCMethodAlignment = other.colonIndentObjCMethodAlignment;
	lineOpeningBlocksNum = other.lineOpeningBlocksNum;
	lineClosingBlocksNum = other.lineClosingBlocksNum;
	fileType = other.fileType;
	minConditionalOption = other.minConditionalOption;
	minConditionalIndent = other.minConditionalIndent;
	parenDepth = other.parenDepth;
	indentLength = other.indentLength;
	tabLength = other.tabLength;
	continuationIndent = other.continuationIndent;
	blockTabCount = other.blockTabCount;
	maxContinuationIndent = other.maxContinuationIndent;
	classInitializerIndents = other.classInitializerIndents;
	templateDepth = other.templateDepth;
	squareBracketCount = other.squareBracketCount;
	prevFinalLineSpaceIndentCount = other.prevFinalLineSpaceIndentCount;
	prevFinalLineIndentCount = other.prevFinalLineIndentCount;
	defineIndentCount = other.defineIndentCount;
	preprocBlockIndent = other.preprocBlockIndent;
	quoteChar = other.quoteChar;
	prevNonSpaceCh = other.prevNonSpaceCh;
	currentNonSpaceCh = other.currentNonSpaceCh;
	currentNonLegalCh = other.currentNonLegalCh;
	prevNonLegalCh = other.prevNonLegalCh;
}

/**
 * ASBeautifier's destructor
 */
ASBeautifier::~ASBeautifier()
{
	deleteBeautifierContainer(waitingBeautifierStack);
	deleteBeautifierContainer(activeBeautifierStack);
	deleteContainer(waitingBeautifierStackLengthStack);
	deleteContainer(activeBeautifierStackLengthStack);
	deleteContainer(headerStack);
	deleteTempStacksContainer(tempStacks);
	deleteContainer(squareBracketDepthStack);
	deleteContainer(blockStatementStack);
	deleteContainer(parenStatementStack);
	deleteContainer(braceBlockStateStack);
	deleteContainer(continuationIndentStack);
	deleteContainer(continuationIndentStackSizeStack);
	deleteContainer(parenIndentStack);
	deleteContainer(preprocIndentStack);
}

/**
 * initialize the ASBeautifier.
 *
 * This init() should be called every time a ABeautifier object is to start
 * beautifying a NEW source file.
 * It is called only when a new ASFormatter object is created.
 * init() receives a pointer to a ASSourceIterator object that will be
 * used to iterate through the source code.
 *
 * @param iter     a pointer to the ASSourceIterator or ASStreamIterator object.
 */
void ASBeautifier::init(ASSourceIterator* iter)
{
	sourceIterator = iter;
	initVectors();
	ASBase::init(getFileType());
	g_preprocessorCppExternCBrace = 0;

	initContainer(waitingBeautifierStack, new vector<ASBeautifier*>);
	initContainer(activeBeautifierStack, new vector<ASBeautifier*>);

	initContainer(waitingBeautifierStackLengthStack, new vector<int>);
	initContainer(activeBeautifierStackLengthStack, new vector<int>);

	initContainer(headerStack, new vector<const string*>);

	initTempStacksContainer(tempStacks, new vector<vector<const string*>*>);
	tempStacks->emplace_back(new vector<const string*>);

	initContainer(squareBracketDepthStack, new vector<int>);
	initContainer(blockStatementStack, new vector<bool>);
	initContainer(parenStatementStack, new vector<bool>);
	initContainer(braceBlockStateStack, new vector<bool>);
	braceBlockStateStack->push_back(true);
	initContainer(continuationIndentStack, new vector<int>);
	initContainer(continuationIndentStackSizeStack, new vector<int>);
	continuationIndentStackSizeStack->emplace_back(0);
	initContainer(parenIndentStack, new vector<int>);
	initContainer(preprocIndentStack, new vector<pair<int, int> >);

	previousLastLineHeader = nullptr;
	currentHeader = nullptr;

	isInQuote = false;
	isInVerbatimQuote = false;
	haveLineContinuationChar = false;
	isInAsm = false;
	isInAsmOneLine = false;
	isInAsmBlock = false;
	isInComment = false;
	isInPreprocessorComment = false;
	isInRunInComment = false;
	isContinuation = false;
	isInCase = false;
	isInQuestion = false;
	isIndentModeOff = false;
	isInClassHeader = false;
	isInClassHeaderTab = false;
	isInClassInitializer = false;
	isInClass = false;
	isInObjCMethodDefinition = false;
	isInObjCMethodCall = false;
	isInObjCMethodCallFirst = false;
	isImmediatelyPostObjCMethodDefinition = false;
	isImmediatelyPostObjCMethodCall = false;
	isInIndentablePreprocBlock = false;
	isInObjCInterface = false;
	isInEnum = false;
	isInEnumTypeID = false;
	isInLet = false;
	isInHeader = false;
	isInTemplate = false;
	isInConditional = false;

	indentCount = 0;
	spaceIndentCount = 0;
	spaceIndentObjCMethodAlignment = 0;
	bracePosObjCMethodAlignment = 0;
	colonIndentObjCMethodAlignment = 0;
	lineOpeningBlocksNum = 0;
	lineClosingBlocksNum = 0;
	templateDepth = 0;
	squareBracketCount = 0;
	parenDepth = 0;
	blockTabCount = 0;
	prevFinalLineSpaceIndentCount = 0;
	prevFinalLineIndentCount = 0;
	defineIndentCount = 0;
	preprocBlockIndent = 0;
	prevNonSpaceCh = '{';
	currentNonSpaceCh = '{';
	prevNonLegalCh = '{';
	currentNonLegalCh = '{';
	quoteChar = ' ';
	probationHeader = nullptr;
	lastLineHeader = nullptr;
	backslashEndsPrevLine = false;
	lineOpensWithLineComment = false;
	lineOpensWithComment = false;
	lineStartsInComment = false;
	isInDefine = false;
	isInDefineDefinition = false;
	lineCommentNoBeautify = false;
	isElseHeaderIndent = false;
	isCaseHeaderCommentIndent = false;
	blockCommentNoIndent = false;
	blockCommentNoBeautify = false;
	previousLineProbationTab = false;
	lineBeginsWithOpenBrace = false;
	lineBeginsWithCloseBrace = false;
	lineBeginsWithComma = false;
	lineIsCommentOnly = false;
	lineIsLineCommentOnly = false;
	shouldIndentBracedLine = true;
	isInSwitch = false;
	foundPreCommandHeader = false;
	foundPreCommandMacro = false;

	isNonInStatementArray = false;
	isSharpAccessor = false;
	isSharpDelegate = false;
	isInExternC = false;
	isInBeautifySQL = false;
	isInIndentableStruct = false;
	isInIndentablePreproc = false;
	inLineNumber = 0;
	runInIndentContinuation = 0;
	nonInStatementBrace = 0;
	objCColonAlignSubsequent = 0;
}

/*
 * initialize the vectors
 */
void ASBeautifier::initVectors()
{
	if (fileType == beautifierFileType)    // don't build unless necessary
		return;

	beautifierFileType = fileType;

	headers->clear();
	nonParenHeaders->clear();
	assignmentOperators->clear();
	nonAssignmentOperators->clear();
	preBlockStatements->clear();
	preCommandHeaders->clear();
	indentableHeaders->clear();

	ASResource::buildHeaders(headers, fileType, true);
	ASResource::buildNonParenHeaders(nonParenHeaders, fileType, true);
	ASResource::buildAssignmentOperators(assignmentOperators);
	ASResource::buildNonAssignmentOperators(nonAssignmentOperators);
	ASResource::buildPreBlockStatements(preBlockStatements, fileType);
	ASResource::buildPreCommandHeaders(preCommandHeaders, fileType);
	ASResource::buildIndentableHeaders(indentableHeaders);
}

/**
 * set indentation style to C/C++.
 */
void ASBeautifier::setCStyle()
{
	fileType = C_TYPE;
}

/**
 * set indentation style to Java.
 */
void ASBeautifier::setJavaStyle()
{
	fileType = JAVA_TYPE;
}

/**
 * set indentation style to C#.
 */
void ASBeautifier::setSharpStyle()
{
	fileType = SHARP_TYPE;
}

/**
 * set mode manually set flag
 */
void ASBeautifier::setModeManuallySet(bool state)
{
	isModeManuallySet = state;
}

/**
 * set tabLength equal to indentLength.
 * This is done when tabLength is not explicitly set by
 * "indent=force-tab-x"
 *
 */
void ASBeautifier::setDefaultTabLength()
{
	tabLength = indentLength;
}

/**
 * indent using a different tab setting for indent=force-tab
 *
 * @param   length     number of spaces per tab.
 */
void ASBeautifier::setForceTabXIndentation(int length)
{
	// set tabLength instead of indentLength
	indentString = "\t";
	tabLength = length;
	shouldForceTabIndentation = true;
}

/**
 * indent using one tab per indentation
 */
void ASBeautifier::setTabIndentation(int length, bool forceTabs)
{
	indentString = "\t";
	indentLength = length;
	shouldForceTabIndentation = forceTabs;
}

/**
 * indent using a number of spaces per indentation.
 *
 * @param   length     number of spaces per indent.
 */
void ASBeautifier::setSpaceIndentation(int length)
{
	indentString = string(length, ' ');
	indentLength = length;
}

/**
* indent continuation lines using a number of indents.
*
* @param   indent     number of indents per line.
*/
void ASBeautifier::setContinuationIndentation(int indent)
{
	continuationIndent = indent;
}

/**
 * set the maximum indentation between two lines in a multi-line statement.
 *
 * @param   max     maximum indentation length.
 */
void ASBeautifier::setMaxContinuationIndentLength(int max)
{
	maxContinuationIndent = max;
}

// retained for compatability with release 2.06
// "MaxInStatementIndent" has been changed to "MaxContinuationIndent" in 3.0
// it is referenced only by the old "MaxInStatementIndent" options
void ASBeautifier::setMaxInStatementIndentLength(int max)
{
	setMaxContinuationIndentLength(max);
}

/**
 * set the minimum conditional indentation option.
 *
 * @param   min     minimal indentation option.
 */
void ASBeautifier::setMinConditionalIndentOption(int min)
{
	minConditionalOption = min;
}

/**
 * set minConditionalIndent from the minConditionalOption.
 */
void ASBeautifier::setMinConditionalIndentLength()
{
	if (minConditionalOption == MINCOND_ZERO)
		minConditionalIndent = 0;
	else if (minConditionalOption == MINCOND_ONE)
		minConditionalIndent = indentLength;
	else if (minConditionalOption == MINCOND_ONEHALF)
		minConditionalIndent = indentLength / 2;
	// minConditionalOption = INDENT_TWO
	else
		minConditionalIndent = indentLength * 2;
}

/**
 * set the state of the brace indent option. If true, braces will
 * be indented one additional indent.
 *
 * @param   state             state of option.
 */
void ASBeautifier::setBraceIndent(bool state)
{
	braceIndent = state;
}

/**
* set the state of the brace indent VTK option. If true, braces will
* be indented one additional indent, except for the opening brace.
*
* @param   state             state of option.
*/
void ASBeautifier::setBraceIndentVtk(bool state)
{
	// need to set both of these
	setBraceIndent(state);
	braceIndentVtk = state;
}

/**
 * set the state of the block indentation option. If true, entire blocks
 * will be indented one additional indent, similar to the GNU indent style.
 *
 * @param   state             state of option.
 */
void ASBeautifier::setBlockIndent(bool state)
{
	blockIndent = state;
}

/**
 * set the state of the class indentation option. If true, C++ class
 * definitions will be indented one additional indent.
 *
 * @param   state             state of option.
 */
void ASBeautifier::setClassIndent(bool state)
{
	classIndent = state;
}

/**
 * set the state of the modifier indentation option. If true, C++ class
 * access modifiers will be indented one-half an indent.
 *
 * @param   state             state of option.
 */
void ASBeautifier::setModifierIndent(bool state)
{
	modifierIndent = state;
}

/**
 * set the state of the switch indentation option. If true, blocks of 'switch'
 * statements will be indented one additional indent.
 *
 * @param   state             state of option.
 */
void ASBeautifier::setSwitchIndent(bool state)
{
	switchIndent = state;
}

/**
 * set the state of the case indentation option. If true, lines of 'case'
 * statements will be indented one additional indent.
 *
 * @param   state             state of option.
 */
void ASBeautifier::setCaseIndent(bool state)
{
	caseIndent = state;
}

/**
 * set the state of the namespace indentation option.
 * If true, blocks of 'namespace' statements will be indented one
 * additional indent. Otherwise, NO indentation will be added.
 *
 * @param   state             state of option.
 */
void ASBeautifier::setNamespaceIndent(bool state)
{
	namespaceIndent = state;
}

/**
* set the state of the indent after parens option.
*
* @param   state             state of option.
*/
void ASBeautifier::setAfterParenIndent(bool state)
{
	shouldIndentAfterParen = state;
}

/**
 * set the state of the label indentation option.
 * If true, labels will be indented one indent LESS than the
 * current indentation level.
 * If false, labels will be flushed to the left with NO
 * indent at all.
 *
 * @param   state             state of option.
 */
void ASBeautifier::setLabelIndent(bool state)
{
	labelIndent = state;
}

/**
 * set the state of the preprocessor indentation option.
 * If true, multi-line #define statements will be indented.
 *
 * @param   state             state of option.
 */
void ASBeautifier::setPreprocDefineIndent(bool state)
{
	shouldIndentPreprocDefine = state;
}

void ASBeautifier::setPreprocConditionalIndent(bool state)
{
	shouldIndentPreprocConditional = state;
}

/**
 * set the state of the empty line fill option.
 * If true, empty lines will be filled with the whitespace.
 * of their previous lines.
 * If false, these lines will remain empty.
 *
 * @param   state             state of option.
 */
void ASBeautifier::setEmptyLineFill(bool state)
{
	emptyLineFill = state;
}

void ASBeautifier::setAlignMethodColon(bool state)
{
	shouldAlignMethodColon = state;
}

/**
 * get the file type.
 */
int ASBeautifier::getFileType() const
{
	return fileType;
}

/**
 * get the number of spaces per indent
 *
 * @return   value of indentLength option.
 */
int ASBeautifier::getIndentLength() const
{
	return indentLength;
}

/**
 * get the char used for indentation, space or tab
 *
 * @return   the char used for indentation.
 */
string ASBeautifier::getIndentString() const
{
	return indentString;
}

/**
 * get mode manually set flag
 */
bool ASBeautifier::getModeManuallySet() const
{
	return isModeManuallySet;
}

/**
 * get the state of the force tab indentation option.
 *
 * @return   state of force tab indentation.
 */
bool ASBeautifier::getForceTabIndentation() const
{
	return shouldForceTabIndentation;
}

/**
* Get the state of the Objective-C align method colon option.
*
* @return   state of shouldAlignMethodColon option.
*/
bool ASBeautifier::getAlignMethodColon() const
{
	return shouldAlignMethodColon;
}

/**
 * get the state of the block indentation option.
 *
 * @return   state of blockIndent option.
 */
bool ASBeautifier::getBlockIndent() const
{
	return blockIndent;
}

/**
 * get the state of the brace indentation option.
 *
 * @return   state of braceIndent option.
 */
bool ASBeautifier::getBraceIndent() const
{
	return braceIndent;
}

/**
* Get the state of the namespace indentation option. If true, blocks
* of the 'namespace' statement will be indented one additional indent.
*
* @return   state of namespaceIndent option.
*/
bool ASBeautifier::getNamespaceIndent() const
{
	return namespaceIndent;
}

/**
 * Get the state of the class indentation option. If true, blocks of
 * the 'class' statement will be indented one additional indent.
 *
 * @return   state of classIndent option.
 */
bool ASBeautifier::getClassIndent() const
{
	return classIndent;
}

/**
 * Get the state of the class access modifier indentation option.
 * If true, the class access modifiers will be indented one-half indent.
 *
 * @return   state of modifierIndent option.
 */
bool ASBeautifier::getModifierIndent() const
{
	return modifierIndent;
}

/**
 * get the state of the switch indentation option. If true, blocks of
 * the 'switch' statement will be indented one additional indent.
 *
 * @return   state of switchIndent option.
 */
bool ASBeautifier::getSwitchIndent() const
{
	return switchIndent;
}

/**
 * get the state of the case indentation option. If true, lines of 'case'
 * statements will be indented one additional indent.
 *
 * @return   state of caseIndent option.
 */
bool ASBeautifier::getCaseIndent() const
{
	return caseIndent;
}

/**
 * get the state of the empty line fill option.
 * If true, empty lines will be filled with the whitespace.
 * of their previous lines.
 * If false, these lines will remain empty.
 *
 * @return   state of emptyLineFill option.
 */
bool ASBeautifier::getEmptyLineFill() const
{
	return emptyLineFill;
}

/**
 * get the state of the preprocessor indentation option.
 * If true, preprocessor "define" lines will be indented.
 * If false, preprocessor "define" lines will be unchanged.
 *
 * @return   state of shouldIndentPreprocDefine option.
 */
bool ASBeautifier::getPreprocDefineIndent() const
{
	return shouldIndentPreprocDefine;
}

/**
 * get the length of the tab indentation option.
 *
 * @return   length of tab indent option.
 */
int ASBeautifier::getTabLength() const
{
	return tabLength;
}

/**
 * beautify a line of source code.
 * every line of source code in a source code file should be sent
 * one after the other to the beautify method.
 *
 * @return      the indented line.
 * @param originalLine       the original unindented line.
 */
string ASBeautifier::beautify(const string& originalLine)
{
	string line;
	bool isInQuoteContinuation = isInVerbatimQuote || haveLineContinuationChar;

	currentHeader = nullptr;
	lastLineHeader = nullptr;
	blockCommentNoBeautify = blockCommentNoIndent;
	isInClass = false;
	isInSwitch = false;
	lineBeginsWithOpenBrace = false;
	lineBeginsWithCloseBrace = false;
	lineBeginsWithComma = false;
	lineIsCommentOnly = false;
	lineIsLineCommentOnly = false;
	shouldIndentBracedLine = true;
	isInAsmOneLine = false;
	lineOpensWithLineComment = false;
	lineOpensWithComment = false;
	lineStartsInComment = isInComment;
	previousLineProbationTab = false;
	lineOpeningBlocksNum = 0;
	lineClosingBlocksNum = 0;
	if (isImmediatelyPostObjCMethodDefinition)
		clearObjCMethodDefinitionAlignment();
	if (isImmediatelyPostObjCMethodCall)
	{
		isImmediatelyPostObjCMethodCall = false;
		isInObjCMethodCall = false;
		objCColonAlignSubsequent = 0;
	}

	// handle and remove white spaces around the line:
	// If not in comment, first find out size of white space before line,
	// so that possible comments starting in the line continue in
	// relation to the preliminary white-space.
	if (isInQuoteContinuation)
	{
		// trim a single space added by ASFormatter, otherwise leave it alone
		if (!(originalLine.length() == 1 && originalLine[0] == ' '))
			line = originalLine;
	}
	else if (isInComment || isInBeautifySQL)
	{
		// trim the end of comment and SQL lines
		line = originalLine;
		size_t trimEnd = line.find_last_not_of(" \t");
		if (trimEnd == string::npos)
			trimEnd = 0;
		else
			trimEnd++;
		if (trimEnd < line.length())
			line.erase(trimEnd);
		// does a brace open the line
		size_t firstChar = line.find_first_not_of(" \t");
		if (firstChar != string::npos)
		{
			if (line[firstChar] == '{')
				lineBeginsWithOpenBrace = true;
			else if (line[firstChar] == '}')
				lineBeginsWithCloseBrace = true;
			else if (line[firstChar] == ',')
				lineBeginsWithComma = true;
		}
	}
	else
	{
		line = trim(originalLine);
		if (line.length() > 0)
		{
			if (line[0] == '{')
				lineBeginsWithOpenBrace = true;
			else if (line[0] == '}')
				lineBeginsWithCloseBrace = true;
			else if (line[0] == ',')
				lineBeginsWithComma = true;
			else if (line.compare(0, 2, "//") == 0)
				lineIsLineCommentOnly = true;
			else if (line.compare(0, 2, "/*") == 0)
			{
				if (line.find("*/", 2) != string::npos)
					lineIsCommentOnly = true;
			}
		}

		isInRunInComment = false;
		size_t j = line.find_first_not_of(" \t{");
		if (j != string::npos && line.compare(j, 2, "//") == 0)
			lineOpensWithLineComment = true;
		if (j != string::npos && line.compare(j, 2, "/*") == 0)
		{
			lineOpensWithComment = true;
			size_t k = line.find_first_not_of(" \t");
			if (k != string::npos && line.compare(k, 1, "{") == 0)
				isInRunInComment = true;
		}
	}

	// When indent is OFF the lines must still be processed by ASBeautifier.
	// Otherwise the lines immediately following may not be indented correctly.
	if ((lineIsLineCommentOnly || lineIsCommentOnly)
	        && line.find("*INDENT-OFF*", 0) != string::npos)
		isIndentModeOff = true;

	if (line.length() == 0)
	{
		if (backslashEndsPrevLine)
		{
			backslashEndsPrevLine = false;
			// check if this line ends a multi-line #define
			// if so, remove the #define's cloned beautifier from the active
			// beautifier stack and delete it.
			if (isInDefineDefinition && !isInDefine)
			{
				isInDefineDefinition = false;
				ASBeautifier* defineBeautifier = activeBeautifierStack->back();
				activeBeautifierStack->pop_back();
				delete defineBeautifier;
			}
		}
		if (emptyLineFill && !isInQuoteContinuation)
		{
			if (isInIndentablePreprocBlock)
				return preLineWS(preprocBlockIndent, 0);
			if (!headerStack->empty() || isInEnum)
				return preLineWS(prevFinalLineIndentCount, prevFinalLineSpaceIndentCount);
			// must fall thru here
		}
		else
			return line;
	}

	// handle preprocessor commands
	if (isInIndentablePreprocBlock
	        && line.length() > 0
	        && line[0] != '#')
	{
		string indentedLine;
		if (isInClassHeaderTab || isInClassInitializer)
		{
			// parsing is turned off in ASFormatter by indent-off
			// the originalLine will probably never be returned here
			indentedLine = preLineWS(prevFinalLineIndentCount, prevFinalLineSpaceIndentCount) + line;
			return getIndentedLineReturn(indentedLine, originalLine);
		}
		indentedLine = preLineWS(preprocBlockIndent, 0) + line;
		return getIndentedLineReturn(indentedLine, originalLine);
	}

	if (!isInComment
	        && !isInQuoteContinuation
	        && line.length() > 0
	        && ((line[0] == '#' && !isIndentedPreprocessor(line, 0))
	            || backslashEndsPrevLine))
	{
		if (line[0] == '#' && !isInDefine)
		{
			string preproc = extractPreprocessorStatement(line);
			processPreprocessor(preproc, line);
			if (isInIndentablePreprocBlock || isInIndentablePreproc)
			{
				string indentedLine;
				if ((preproc.length() >= 2 && preproc.substr(0, 2) == "if")) // #if, #ifdef, #ifndef
				{
					indentedLine = preLineWS(preprocBlockIndent, 0) + line;
					preprocBlockIndent += 1;
					isInIndentablePreprocBlock = true;
				}
				else if (preproc == "else" || preproc == "elif")
				{
					indentedLine = preLineWS(preprocBlockIndent - 1, 0) + line;
				}
				else if (preproc == "endif")
				{
					preprocBlockIndent -= 1;
					indentedLine = preLineWS(preprocBlockIndent, 0) + line;
					if (preprocBlockIndent == 0)
						isInIndentablePreprocBlock = false;
				}
				else
					indentedLine = preLineWS(preprocBlockIndent, 0) + line;
				return getIndentedLineReturn(indentedLine, originalLine);
			}
			if (shouldIndentPreprocConditional && preproc.length() > 0)
			{
				string indentedLine;
				if (preproc.length() >= 2 && preproc.substr(0, 2) == "if") // #if, #ifdef, #ifndef
				{
					pair<int, int> entry;	// indentCount, spaceIndentCount
					if (!isInDefine && activeBeautifierStack != nullptr && !activeBeautifierStack->empty())
						entry = activeBeautifierStack->back()->computePreprocessorIndent();
					else
						entry = computePreprocessorIndent();
					preprocIndentStack->emplace_back(entry);
					indentedLine = preLineWS(preprocIndentStack->back().first,
					                         preprocIndentStack->back().second) + line;
					return getIndentedLineReturn(indentedLine, originalLine);
				}
				if (preproc == "else" || preproc == "elif")
				{
					if (!preprocIndentStack->empty())	// if no entry don't indent
					{
						indentedLine = preLineWS(preprocIndentStack->back().first,
						                         preprocIndentStack->back().second) + line;
						return getIndentedLineReturn(indentedLine, originalLine);
					}
				}
				else if (preproc == "endif")
				{
					if (!preprocIndentStack->empty())	// if no entry don't indent
					{
						indentedLine = preLineWS(preprocIndentStack->back().first,
						                         preprocIndentStack->back().second) + line;
						preprocIndentStack->pop_back();
						return getIndentedLineReturn(indentedLine, originalLine);
					}
				}
			}
		}

		// check if the last char is a backslash
		if (line.length() > 0)
			backslashEndsPrevLine = (line[line.length() - 1] == '\\');
		// comments within the definition line can be continued without the backslash
		if (isInPreprocessorUnterminatedComment(line))
			backslashEndsPrevLine = true;

		// check if this line ends a multi-line #define
		// if so, use the #define's cloned beautifier for the line's indentation
		// and then remove it from the active beautifier stack and delete it.
		if (!backslashEndsPrevLine && isInDefineDefinition && !isInDefine)
		{
			isInDefineDefinition = false;
			ASBeautifier* defineBeautifier = activeBeautifierStack->back();
			activeBeautifierStack->pop_back();

			string indentedLine = defineBeautifier->beautify(line);
			delete defineBeautifier;
			return getIndentedLineReturn(indentedLine, originalLine);
		}

		// unless this is a multi-line #define, return this precompiler line as is.
		if (!isInDefine && !isInDefineDefinition)
			return originalLine;
	}

	// if there exists any worker beautifier in the activeBeautifierStack,
	// then use it instead of me to indent the current line.
	// variables set by ASFormatter must be updated.
	if (!isInDefine && activeBeautifierStack != nullptr && !activeBeautifierStack->empty())
	{
		activeBeautifierStack->back()->inLineNumber = inLineNumber;
		activeBeautifierStack->back()->runInIndentContinuation = runInIndentContinuation;
		activeBeautifierStack->back()->nonInStatementBrace = nonInStatementBrace;
		activeBeautifierStack->back()->objCColonAlignSubsequent = objCColonAlignSubsequent;
		activeBeautifierStack->back()->lineCommentNoBeautify = lineCommentNoBeautify;
		activeBeautifierStack->back()->isElseHeaderIndent = isElseHeaderIndent;
		activeBeautifierStack->back()->isCaseHeaderCommentIndent = isCaseHeaderCommentIndent;
		activeBeautifierStack->back()->isNonInStatementArray = isNonInStatementArray;
		activeBeautifierStack->back()->isSharpAccessor = isSharpAccessor;
		activeBeautifierStack->back()->isSharpDelegate = isSharpDelegate;
		activeBeautifierStack->back()->isInExternC = isInExternC;
		activeBeautifierStack->back()->isInBeautifySQL = isInBeautifySQL;
		activeBeautifierStack->back()->isInIndentableStruct = isInIndentableStruct;
		activeBeautifierStack->back()->isInIndentablePreproc = isInIndentablePreproc;
		// must return originalLine not the trimmed line
		return activeBeautifierStack->back()->beautify(originalLine);
	}

	// Flag an indented header in case this line is a one-line block.
	// The header in the header stack will be deleted by a one-line block.
	bool isInExtraHeaderIndent = false;
	if (!headerStack->empty()
	        && lineBeginsWithOpenBrace
	        && (headerStack->back() != &AS_OPEN_BRACE
	            || probationHeader != nullptr))
		isInExtraHeaderIndent = true;

	size_t iPrelim = headerStack->size();

	// calculate preliminary indentation based on headerStack and data from past lines
	computePreliminaryIndentation();

	// parse characters in the current line.
	parseCurrentLine(line);

	// handle special cases of indentation
	adjustParsedLineIndentation(iPrelim, isInExtraHeaderIndent);

	if (isInObjCMethodDefinition)
		adjustObjCMethodDefinitionIndentation(line);

	if (isInObjCMethodCall)
		adjustObjCMethodCallIndentation(line);

	if (isInDefine)
	{
		if (line.length() > 0 && line[0] == '#')
		{
			// the 'define' does not have to be attached to the '#'
			string preproc = trim(line.substr(1));
			if (preproc.compare(0, 6, "define") == 0)
			{
				if (!continuationIndentStack->empty()
				        && continuationIndentStack->back() > 0)
				{
					defineIndentCount = indentCount;
				}
				else
				{
					defineIndentCount = indentCount - 1;
					--indentCount;
				}
			}
		}

		indentCount -= defineIndentCount;
	}

	if (indentCount < 0)
		indentCount = 0;

	if (lineCommentNoBeautify || blockCommentNoBeautify || isInQuoteContinuation)
		indentCount = spaceIndentCount = 0;

	// finally, insert indentations into beginning of line

	string indentedLine = preLineWS(indentCount, spaceIndentCount) + line;
	indentedLine = getIndentedLineReturn(indentedLine, originalLine);

	prevFinalLineSpaceIndentCount = spaceIndentCount;
	prevFinalLineIndentCount = indentCount;

	if (lastLineHeader != nullptr)
		previousLastLineHeader = lastLineHeader;

	if ((lineIsLineCommentOnly || lineIsCommentOnly)
	        && line.find("*INDENT-ON*", 0) != string::npos)
		isIndentModeOff = false;

	return indentedLine;
}

const string& ASBeautifier::getIndentedLineReturn(const string& newLine, const string& originalLine) const
{
	if (isIndentModeOff)
		return originalLine;
	return newLine;
}

string ASBeautifier::preLineWS(int lineIndentCount, int lineSpaceIndentCount) const
{
	if (shouldForceTabIndentation)
	{
		if (tabLength != indentLength)
		{
			// adjust for different tab length
			int indentCountOrig = lineIndentCount;
			int spaceIndentCountOrig = lineSpaceIndentCount;
			lineIndentCount = ((indentCountOrig * indentLength) + spaceIndentCountOrig) / tabLength;
			lineSpaceIndentCount = ((indentCountOrig * indentLength) + spaceIndentCountOrig) % tabLength;
		}
		else
		{
			lineIndentCount += lineSpaceIndentCount / indentLength;
			lineSpaceIndentCount = lineSpaceIndentCount % indentLength;
		}
	}

	string ws;
	for (int i = 0; i < lineIndentCount; i++)
		ws += indentString;
	while ((lineSpaceIndentCount--) > 0)
		ws += string(" ");
	return ws;
}

/**
 * register a continuation indent.
 */
void ASBeautifier::registerContinuationIndent(const string& line, int i, int spaceIndentCount_,
                                              int tabIncrementIn, int minIndent, bool updateParenStack)
{
	int remainingCharNum = line.length() - i;
	int nextNonWSChar = getNextProgramCharDistance(line, i);

	// if indent is around the last char in the line OR indent-after-paren is requested,
	// indent with the continuation indent
	if (nextNonWSChar == remainingCharNum || shouldIndentAfterParen)
	{
		int previousIndent = spaceIndentCount_;
		if (!continuationIndentStack->empty())
			previousIndent = continuationIndentStack->back();
		int currIndent = continuationIndent * indentLength + previousIndent;
		if (currIndent > maxContinuationIndent && line[i] != '{')
			currIndent = indentLength * 2 + spaceIndentCount_;
		continuationIndentStack->emplace_back(currIndent);
		if (updateParenStack)
			parenIndentStack->emplace_back(previousIndent);
		return;
	}

	if (updateParenStack)
		parenIndentStack->emplace_back(i + spaceIndentCount_ - runInIndentContinuation);

	int tabIncrement = tabIncrementIn;

	// check for following tabs
	for (int j = i + 1; j < (i + nextNonWSChar); j++)
	{
		if (line[j] == '\t')
			tabIncrement += convertTabToSpaces(j, tabIncrement);
	}

	int continuationIndentCount = i + nextNonWSChar + spaceIndentCount_ + tabIncrement;

	// check for run-in statement
	if (i > 0 && line[0] == '{')
		continuationIndentCount -= indentLength;

	if (continuationIndentCount < minIndent)
		continuationIndentCount = minIndent + spaceIndentCount_;

	// this is not done for an in-statement array
	if (continuationIndentCount > maxContinuationIndent
	        && !(prevNonLegalCh == '=' && currentNonLegalCh == '{'))
		continuationIndentCount = indentLength * 2 + spaceIndentCount_;

	if (!continuationIndentStack->empty()
	        && continuationIndentCount < continuationIndentStack->back())
		continuationIndentCount = continuationIndentStack->back();

	// the block opener is not indented for a NonInStatementArray
	if ((isNonInStatementArray && line[i] == '{')
	        && !isInEnum && !braceBlockStateStack->empty() && braceBlockStateStack->back())
		continuationIndentCount = 0;

	continuationIndentStack->emplace_back(continuationIndentCount);
}

/**
* Register a continuation indent for a class header or a class initializer colon.
*/
void ASBeautifier::registerContinuationIndentColon(const string& line, int i, int tabIncrementIn)
{
	assert(line[i] == ':');
	assert(isInClassInitializer || isInClassHeaderTab);

	// register indent at first word after the colon
	size_t firstChar = line.find_first_not_of(" \t");
	if (firstChar == (size_t) i)		// firstChar is ':'
	{
		size_t firstWord = line.find_first_not_of(" \t", firstChar + 1);
		if (firstChar != string::npos)
		{
			int continuationIndentCount = firstWord + spaceIndentCount + tabIncrementIn;
			continuationIndentStack->emplace_back(continuationIndentCount);
			isContinuation = true;
		}
	}
}

/**
 * Compute indentation for a preprocessor #if statement.
 * This may be called for the activeBeautiferStack
 * instead of the active ASBeautifier object.
 */
pair<int, int> ASBeautifier::computePreprocessorIndent()
{
	computePreliminaryIndentation();
	pair<int, int> entry(indentCount, spaceIndentCount);
	if (!headerStack->empty()
	        && entry.first > 0
	        && (headerStack->back() == &AS_IF
	            || headerStack->back() == &AS_ELSE
	            || headerStack->back() == &AS_FOR
	            || headerStack->back() == &AS_WHILE))
		--entry.first;
	return entry;
}

/**
 * get distance to the next non-white space, non-comment character in the line.
 * if no such character exists, return the length remaining to the end of the line.
 */
int ASBeautifier::getNextProgramCharDistance(const string& line, int i) const
{
	bool inComment = false;
	int  remainingCharNum = line.length() - i;
	int  charDistance;
	char ch;

	for (charDistance = 1; charDistance < remainingCharNum; charDistance++)
	{
		ch = line[i + charDistance];
		if (inComment)
		{
			if (line.compare(i + charDistance, 2, "*/") == 0)
			{
				charDistance++;
				inComment = false;
			}
			continue;
		}
		else if (isWhiteSpace(ch))
			continue;
		else if (ch == '/')
		{
			if (line.compare(i + charDistance, 2, "//") == 0)
				return remainingCharNum;
			if (line.compare(i + charDistance, 2, "/*") == 0)
			{
				charDistance++;
				inComment = true;
			}
		}
		else
			return charDistance;
	}

	return charDistance;
}

/**
 * find the index number of a string element in a container of strings
 *
 * @return              the index number of element in the container. -1 if element not found.
 * @param container     a vector of strings.
 * @param element       the element to find .
 */
int ASBeautifier::indexOf(const vector<const string*>& container, const string* element) const
{
	vector<const string*>::const_iterator where;

	where = find(container.begin(), container.end(), element);
	if (where == container.end())
		return -1;
	return (int) (where - container.begin());
}

/**
 * convert tabs to spaces.
 * i is the position of the character to convert to spaces.
 * tabIncrementIn is the increment that must be added for tab indent characters
 *     to get the correct column for the current tab.
 */
int ASBeautifier::convertTabToSpaces(int i, int tabIncrementIn) const
{
	int tabToSpacesAdjustment = indentLength - 1 - ((tabIncrementIn + i) % indentLength);
	return tabToSpacesAdjustment;
}

/**
 * trim removes the white space surrounding a line.
 *
 * @return          the trimmed line.
 * @param str       the line to trim.
 */
string ASBeautifier::trim(const string& str) const
{
	int start = 0;
	int end = str.length() - 1;

	while (start < end && isWhiteSpace(str[start]))
		start++;

	while (start <= end && isWhiteSpace(str[end]))
		end--;

	// don't trim if it ends in a continuation
	if (end > -1 && str[end] == '\\')
		end = str.length() - 1;

	string returnStr(str, start, end + 1 - start);
	return returnStr;
}

/**
 * rtrim removes the white space from the end of a line.
 *
 * @return          the trimmed line.
 * @param str       the line to trim.
 */
string ASBeautifier::rtrim(const string& str) const
{
	size_t len = str.length();
	size_t end = str.find_last_not_of(" \t");
	if (end == string::npos
	        || end == len - 1)
		return str;
	string returnStr(str, 0, end + 1);
	return returnStr;
}

/**
 * Copy tempStacks for the copy constructor.
 * The value of the vectors must also be copied.
 */
vector<vector<const string*>*>* ASBeautifier::copyTempStacks(const ASBeautifier& other) const
{
	vector<vector<const string*>*>* tempStacksNew = new vector<vector<const string*>*>;
	vector<vector<const string*>*>::iterator iter;
	for (iter = other.tempStacks->begin();
	        iter != other.tempStacks->end();
	        ++iter)
	{
		vector<const string*>* newVec = new vector<const string*>;
		*newVec = **iter;
		tempStacksNew->emplace_back(newVec);
	}
	return tempStacksNew;
}

/**
 * delete a member vectors to eliminate memory leak reporting
 */
void ASBeautifier::deleteBeautifierVectors()
{
	beautifierFileType = 9;		// reset to an invalid type
	delete headers;
	delete nonParenHeaders;
	delete preBlockStatements;
	delete preCommandHeaders;
	delete assignmentOperators;
	delete nonAssignmentOperators;
	delete indentableHeaders;
}

/**
 * delete a vector object
 * T is the type of vector
 * used for all vectors except tempStacks
 */
template<typename T>
void ASBeautifier::deleteContainer(T& container)
{
	if (container != nullptr)
	{
		container->clear();
		delete (container);
		container = nullptr;
	}
}

/**
 * Delete the ASBeautifier vector object.
 * This is a vector of pointers to ASBeautifier objects allocated with the 'new' operator.
 * Therefore the ASBeautifier objects have to be deleted in addition to the
 * ASBeautifier pointer entries.
 */
void ASBeautifier::deleteBeautifierContainer(vector<ASBeautifier*>*& container)
{
	if (container != nullptr)
	{
		vector<ASBeautifier*>::iterator iter = container->begin();
		while (iter < container->end())
		{
			delete *iter;
			++iter;
		}
		container->clear();
		delete (container);
		container = nullptr;
	}
}

/**
 * Delete the tempStacks vector object.
 * The tempStacks is a vector of pointers to strings allocated with the 'new' operator.
 * Therefore the strings have to be deleted in addition to the tempStacks entries.
 */
void ASBeautifier::deleteTempStacksContainer(vector<vector<const string*>*>*& container)
{
	if (container != nullptr)
	{
		vector<vector<const string*>*>::iterator iter = container->begin();
		while (iter < container->end())
		{
			delete *iter;
			++iter;
		}
		container->clear();
		delete (container);
		container = nullptr;
	}
}

/**
 * initialize a vector object
 * T is the type of vector used for all vectors
 */
template<typename T>
void ASBeautifier::initContainer(T& container, T value)
{
	// since the ASFormatter object is never deleted,
	// the existing vectors must be deleted before creating new ones
	if (container != nullptr)
		deleteContainer(container);
	container = value;
}

/**
 * Initialize the tempStacks vector object.
 * The tempStacks is a vector of pointers to strings allocated with the 'new' operator.
 * Any residual entries are deleted before the vector is initialized.
 */
void ASBeautifier::initTempStacksContainer(vector<vector<const string*>*>*& container,
                                           vector<vector<const string*>*>* value)
{
	if (container != nullptr)
		deleteTempStacksContainer(container);
	container = value;
}

/**
 * Determine if an assignment statement ends with a comma
 *     that is not in a function argument. It ends with a
 *     comma if a comma is the last char on the line.
 *
 * @return  true if line ends with a comma, otherwise false.
 */
bool ASBeautifier::statementEndsWithComma(const string& line, int index) const
{
	assert(line[index] == '=');

	bool isInComment_ = false;
	bool isInQuote_ = false;
	int parenCount = 0;
	size_t lineLength = line.length();
	size_t i = 0;
	char quoteChar_ = ' ';

	for (i = index + 1; i < lineLength; ++i)
	{
		char ch = line[i];

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
			if (isLineEndComment(line, i))
				break;
			else
			{
				isInComment_ = true;
				++i;
				continue;
			}
		}

		if (ch == '(')
			parenCount++;
		if (ch == ')')
			parenCount--;
	}
	if (isInComment_
	        || isInQuote_
	        || parenCount > 0)
		return false;

	size_t lastChar = line.find_last_not_of(" \t", i - 1);

	if (lastChar == string::npos || line[lastChar] != ',')
		return false;

	return true;
}

/**
 * check if current comment is a line-end comment
 *
 * @return     is before a line-end comment.
 */
bool ASBeautifier::isLineEndComment(const string& line, int startPos) const
{
	assert(line.compare(startPos, 2, "/*") == 0);

	// comment must be closed on this line with nothing after it
	size_t endNum = line.find("*/", startPos + 2);
	if (endNum != string::npos)
	{
		size_t nextChar = line.find_first_not_of(" \t", endNum + 2);
		if (nextChar == string::npos)
			return true;
	}
	return false;
}

/**
 * get the previous word index for an assignment operator
 *
 * @return is the index to the previous word (the in statement indent).
 */
int ASBeautifier::getContinuationIndentAssign(const string& line, size_t currPos) const
{
	assert(line[currPos] == '=');

	if (currPos == 0)
		return 0;

	// get the last legal word (may be a number)
	size_t end = line.find_last_not_of(" \t", currPos - 1);
	if (end == string::npos || !isLegalNameChar(line[end]))
		return 0;

	int start;          // start of the previous word
	for (start = end; start > -1; start--)
	{
		if (!isLegalNameChar(line[start]) || line[start] == '.')
			break;
	}
	start++;

	return start;
}

/**
 * get the instatement indent for a comma
 *
 * @return is the indent to the second word on the line (the in statement indent).
 */
int ASBeautifier::getContinuationIndentComma(const string& line, size_t currPos) const
{
	assert(line[currPos] == ',');

	// get first word on a line
	size_t indent = line.find_first_not_of(" \t");
	if (indent == string::npos || !isLegalNameChar(line[indent]))
		return 0;

	// bypass first word
	for (; indent < currPos; indent++)
	{
		if (!isLegalNameChar(line[indent]))
			break;
	}
	indent++;
	if (indent >= currPos || indent < 4)
		return 0;

	// point to second word or assignment operator
	indent = line.find_first_not_of(" \t", indent);
	if (indent == string::npos || indent >= currPos)
		return 0;

	return indent;
}

/**
 * get the next word on a line
 * the argument 'currPos' must point to the current position.
 *
 * @return is the next word or an empty string if none found.
 */
string ASBeautifier::getNextWord(const string& line, size_t currPos) const
{
	size_t lineLength = line.length();
	// get the last legal word (may be a number)
	if (currPos == lineLength - 1)
		return string();

	size_t start = line.find_first_not_of(" \t", currPos + 1);
	if (start == string::npos || !isLegalNameChar(line[start]))
		return string();

	size_t end;			// end of the current word
	for (end = start + 1; end <= lineLength; end++)
	{
		if (!isLegalNameChar(line[end]) || line[end] == '.')
			break;
	}

	return line.substr(start, end - start);
}

/**
 * Check if a preprocessor directive is always indented.
 * C# "region" and "endregion" are always indented.
 * C/C++ "pragma omp" is always indented.
 *
 * @return is true or false.
 */
bool ASBeautifier::isIndentedPreprocessor(const string& line, size_t currPos) const
{
	assert(line[0] == '#');
	string nextWord = getNextWord(line, currPos);
	if (nextWord == "region" || nextWord == "endregion")
		return true;
	// is it #pragma omp
	if (nextWord == "pragma")
	{
		// find pragma
		size_t start = line.find("pragma");
		if (start == string::npos || !isLegalNameChar(line[start]))
			return false;
		// bypass pragma
		for (; start < line.length(); start++)
		{
			if (!isLegalNameChar(line[start]))
				break;
		}
		start++;
		if (start >= line.length())
			return false;
		// point to start of second word
		start = line.find_first_not_of(" \t", start);
		if (start == string::npos)
			return false;
		// point to end of second word
		size_t end;
		for (end = start; end < line.length(); end++)
		{
			if (!isLegalNameChar(line[end]))
				break;
		}
		// check for "pragma omp"
		string word = line.substr(start, end - start);
		if (word == "omp" || word == "region" || word == "endregion")
			return true;
	}
	return false;
}

/**
 * Check if a preprocessor directive is checking for __cplusplus defined.
 *
 * @return is true or false.
 */
bool ASBeautifier::isPreprocessorConditionalCplusplus(const string& line) const
{
	string preproc = trim(line.substr(1));
	if (preproc.compare(0, 5, "ifdef") == 0 && getNextWord(preproc, 4) == "__cplusplus")
		return true;
	if (preproc.compare(0, 2, "if") == 0)
	{
		// check for " #if defined(__cplusplus)"
		size_t charNum = 2;
		charNum = preproc.find_first_not_of(" \t", charNum);
		if (charNum != string::npos && preproc.compare(charNum, 7, "defined") == 0)
		{
			charNum += 7;
			charNum = preproc.find_first_not_of(" \t", charNum);
			if (preproc.compare(charNum, 1, "(") == 0)
			{
				++charNum;
				charNum = preproc.find_first_not_of(" \t", charNum);
				if (preproc.compare(charNum, 11, "__cplusplus") == 0)
					return true;
			}
		}
	}
	return false;
}

/**
 * Check if a preprocessor definition contains an unterminated comment.
 * Comments within a preprocessor definition can be continued without the backslash.
 *
 * @return is true or false.
 */
bool ASBeautifier::isInPreprocessorUnterminatedComment(const string& line)
{
	if (!isInPreprocessorComment)
	{
		size_t startPos = line.find("/*");
		if (startPos == string::npos)
			return false;
	}
	size_t endNum = line.find("*/");
	if (endNum != string::npos)
	{
		isInPreprocessorComment = false;
		return false;
	}
	isInPreprocessorComment = true;
	return true;
}

void ASBeautifier::popLastContinuationIndent()
{
	assert(!continuationIndentStackSizeStack->empty());
	int previousIndentStackSize = continuationIndentStackSizeStack->back();
	if (continuationIndentStackSizeStack->size() > 1)
		continuationIndentStackSizeStack->pop_back();
	while (previousIndentStackSize < (int) continuationIndentStack->size())
		continuationIndentStack->pop_back();
}

// for unit testing
int ASBeautifier::getBeautifierFileType() const
{ return beautifierFileType; }

/**
 * Process preprocessor statements and update the beautifier stacks.
 */
void ASBeautifier::processPreprocessor(const string& preproc, const string& line)
{
	// When finding a multi-lined #define statement, the original beautifier
	// 1. sets its isInDefineDefinition flag
	// 2. clones a new beautifier that will be used for the actual indentation
	//    of the #define. This clone is put into the activeBeautifierStack in order
	//    to be called for the actual indentation.
	// The original beautifier will have isInDefineDefinition = true, isInDefine = false
	// The cloned beautifier will have   isInDefineDefinition = true, isInDefine = true
	if (shouldIndentPreprocDefine && preproc == "define" && line[line.length() - 1] == '\\')
	{
		if (!isInDefineDefinition)
		{
			// this is the original beautifier
			isInDefineDefinition = true;

			// push a new beautifier into the active stack
			// this beautifier will be used for the indentation of this define
			ASBeautifier* defineBeautifier = new ASBeautifier(*this);
			activeBeautifierStack->emplace_back(defineBeautifier);
		}
		else
		{
			// the is the cloned beautifier that is in charge of indenting the #define.
			isInDefine = true;
		}
	}
	else if (preproc.length() >= 2 && preproc.substr(0, 2) == "if")
	{
		if (isPreprocessorConditionalCplusplus(line) && !g_preprocessorCppExternCBrace)
			g_preprocessorCppExternCBrace = 1;
		// push a new beautifier into the stack
		waitingBeautifierStackLengthStack->push_back(waitingBeautifierStack->size());
		activeBeautifierStackLengthStack->push_back(activeBeautifierStack->size());
		if (activeBeautifierStackLengthStack->back() == 0)
			waitingBeautifierStack->emplace_back(new ASBeautifier(*this));
		else
			waitingBeautifierStack->emplace_back(new ASBeautifier(*activeBeautifierStack->back()));
	}
	else if (preproc == "else")
	{
		if ((waitingBeautifierStack != nullptr) && !waitingBeautifierStack->empty())
		{
			// MOVE current waiting beautifier to active stack.
			activeBeautifierStack->emplace_back(waitingBeautifierStack->back());
			waitingBeautifierStack->pop_back();
		}
	}
	else if (preproc == "elif")
	{
		if ((waitingBeautifierStack != nullptr) && !waitingBeautifierStack->empty())
		{
			// append a COPY current waiting beautifier to active stack, WITHOUT deleting the original.
			activeBeautifierStack->emplace_back(new ASBeautifier(*(waitingBeautifierStack->back())));
		}
	}
	else if (preproc == "endif")
	{
		int stackLength = 0;
		ASBeautifier* beautifier = nullptr;

		if (waitingBeautifierStackLengthStack != nullptr && !waitingBeautifierStackLengthStack->empty())
		{
			stackLength = waitingBeautifierStackLengthStack->back();
			waitingBeautifierStackLengthStack->pop_back();
			while ((int) waitingBeautifierStack->size() > stackLength)
			{
				beautifier = waitingBeautifierStack->back();
				waitingBeautifierStack->pop_back();
				delete beautifier;
			}
		}

		if (!activeBeautifierStackLengthStack->empty())
		{
			stackLength = activeBeautifierStackLengthStack->back();
			activeBeautifierStackLengthStack->pop_back();
			while ((int) activeBeautifierStack->size() > stackLength)
			{
				beautifier = activeBeautifierStack->back();
				activeBeautifierStack->pop_back();
				delete beautifier;
			}
		}
	}
}

// Compute the preliminary indentation based on data in the headerStack
// and data from previous lines.
// Update the class variable indentCount.
void ASBeautifier::computePreliminaryIndentation()
{
	indentCount = 0;
	spaceIndentCount = 0;
	isInClassHeaderTab = false;

	if (isInObjCMethodDefinition && !continuationIndentStack->empty())
		spaceIndentObjCMethodAlignment = continuationIndentStack->back();

	if (!continuationIndentStack->empty())
		spaceIndentCount = continuationIndentStack->back();

	for (size_t i = 0; i < headerStack->size(); i++)
	{
		isInClass = false;

		if (blockIndent)
		{
			// do NOT indent opening block for these headers
			if (!((*headerStack)[i] == &AS_NAMESPACE
			        || (*headerStack)[i] == &AS_MODULE
			        || (*headerStack)[i] == &AS_CLASS
			        || (*headerStack)[i] == &AS_STRUCT
			        || (*headerStack)[i] == &AS_UNION
			        || (*headerStack)[i] == &AS_INTERFACE
			        || (*headerStack)[i] == &AS_THROWS
			        || (*headerStack)[i] == &AS_STATIC))
				++indentCount;
		}
		else if (!(i > 0 && (*headerStack)[i - 1] != &AS_OPEN_BRACE
		           && (*headerStack)[i] == &AS_OPEN_BRACE))
			++indentCount;

		if (!isJavaStyle() && !namespaceIndent && i > 0
		        && ((*headerStack)[i - 1] == &AS_NAMESPACE
		            || (*headerStack)[i - 1] == &AS_MODULE)
		        && (*headerStack)[i] == &AS_OPEN_BRACE)
			--indentCount;

		if (isCStyle() && i >= 1
		        && (*headerStack)[i - 1] == &AS_CLASS
		        && (*headerStack)[i] == &AS_OPEN_BRACE)
		{
			if (classIndent)
				++indentCount;
			isInClass = true;
		}

		// is the switchIndent option is on, indent switch statements an additional indent.
		else if (switchIndent && i > 1
		         && (*headerStack)[i - 1] == &AS_SWITCH
		         && (*headerStack)[i] == &AS_OPEN_BRACE)
		{
			++indentCount;
			isInSwitch = true;
		}

	}	// end of for loop

	if (isInClassHeader)
	{
		if (!isJavaStyle())
			isInClassHeaderTab = true;
		if (lineOpensWithLineComment || lineStartsInComment || lineOpensWithComment)
		{
			if (!lineBeginsWithOpenBrace)
				--indentCount;
			if (!continuationIndentStack->empty())
				spaceIndentCount -= continuationIndentStack->back();
		}
		else if (blockIndent)
		{
			if (!lineBeginsWithOpenBrace)
				++indentCount;
		}
	}

	if (isInClassInitializer || isInEnumTypeID)
	{
		indentCount += classInitializerIndents;
	}

	if (isInEnum && lineBeginsWithComma && !continuationIndentStack->empty())
	{
		// unregister '=' indent from the previous line
		continuationIndentStack->pop_back();
		isContinuation = false;
		spaceIndentCount = 0;
	}

	// Objective-C interface continuation line
	if (isInObjCInterface)
		++indentCount;

	// unindent a class closing brace...
	if (!lineStartsInComment
	        && isCStyle()
	        && isInClass
	        && classIndent
	        && headerStack->size() >= 2
	        && (*headerStack)[headerStack->size() - 2] == &AS_CLASS
	        && (*headerStack)[headerStack->size() - 1] == &AS_OPEN_BRACE
	        && lineBeginsWithCloseBrace
	        && braceBlockStateStack->back())
		--indentCount;

	// unindent an indented switch closing brace...
	else if (!lineStartsInComment
	         && isInSwitch
	         && switchIndent
	         && headerStack->size() >= 2
	         && (*headerStack)[headerStack->size() - 2] == &AS_SWITCH
	         && (*headerStack)[headerStack->size() - 1] == &AS_OPEN_BRACE
	         && lineBeginsWithCloseBrace)
		--indentCount;

	// handle special case of run-in comment in an indented class statement
	if (isInClass
	        && classIndent
	        && isInRunInComment
	        && !lineOpensWithComment
	        && headerStack->size() > 1
	        && (*headerStack)[headerStack->size() - 2] == &AS_CLASS)
		--indentCount;

	if (isInConditional)
		--indentCount;
	if (g_preprocessorCppExternCBrace >= 4)
		--indentCount;
}

void ASBeautifier::adjustParsedLineIndentation(size_t iPrelim, bool isInExtraHeaderIndent)
{
	if (lineStartsInComment)
		return;

	// unindent a one-line statement in a header indent
	if (!blockIndent
	        && lineBeginsWithOpenBrace
	        && headerStack->size() < iPrelim
	        && isInExtraHeaderIndent
	        && (lineOpeningBlocksNum > 0 && lineOpeningBlocksNum <= lineClosingBlocksNum)
	        && shouldIndentBracedLine)
		--indentCount;

	/*
	 * if '{' doesn't follow an immediately previous '{' in the headerStack
	 * (but rather another header such as "for" or "if", then unindent it
	 * by one indentation relative to its block.
	 */
	else if (!blockIndent
	         && lineBeginsWithOpenBrace
	         && !(lineOpeningBlocksNum > 0 && lineOpeningBlocksNum <= lineClosingBlocksNum)
	         && (headerStack->size() > 1 && (*headerStack)[headerStack->size() - 2] != &AS_OPEN_BRACE)
	         && shouldIndentBracedLine)
		--indentCount;

	// must check one less in headerStack if more than one header on a line (allow-addins)...
	else if (headerStack->size() > iPrelim + 1
	         && !blockIndent
	         && lineBeginsWithOpenBrace
	         && !(lineOpeningBlocksNum > 0 && lineOpeningBlocksNum <= lineClosingBlocksNum)
	         && (headerStack->size() > 2 && (*headerStack)[headerStack->size() - 3] != &AS_OPEN_BRACE)
	         && shouldIndentBracedLine)
		--indentCount;

	// unindent a closing brace...
	else if (lineBeginsWithCloseBrace
	         && shouldIndentBracedLine)
		--indentCount;

	// correctly indent one-line-blocks...
	else if (lineOpeningBlocksNum > 0
	         && lineOpeningBlocksNum == lineClosingBlocksNum
	         && previousLineProbationTab)
		--indentCount;

	if (indentCount < 0)
		indentCount = 0;

	// take care of extra brace indentation option...
	if (!lineStartsInComment
	        && braceIndent
	        && shouldIndentBracedLine
	        && (lineBeginsWithOpenBrace || lineBeginsWithCloseBrace))
	{
		if (!braceIndentVtk)
			++indentCount;
		else
		{
			// determine if a style VTK brace is indented
			bool haveUnindentedBrace = false;
			for (size_t i = 0; i < headerStack->size(); i++)
			{
				if (((*headerStack)[i] == &AS_NAMESPACE
				        || (*headerStack)[i] == &AS_MODULE
				        || (*headerStack)[i] == &AS_CLASS
				        || (*headerStack)[i] == &AS_STRUCT)
				        && i + 1 < headerStack->size()
				        && (*headerStack)[i + 1] == &AS_OPEN_BRACE)
					i++;
				else if (lineBeginsWithOpenBrace)
				{
					// don't double count the current brace
					if (i + 1 < headerStack->size()
					        && (*headerStack)[i] == &AS_OPEN_BRACE)
						haveUnindentedBrace = true;
				}
				else if ((*headerStack)[i] == &AS_OPEN_BRACE)
					haveUnindentedBrace = true;
			}	// end of for loop
			if (haveUnindentedBrace)
				++indentCount;
		}
	}
}

/**
 * Compute indentCount adjustment when in a series of else-if statements
 * and shouldBreakElseIfs is requested.
 * It increments by one for each 'else' in the tempStack.
 */
int ASBeautifier::adjustIndentCountForBreakElseIfComments() const
{
	assert(isElseHeaderIndent && !tempStacks->empty());
	int indentCountIncrement = 0;
	vector<const string*>* lastTempStack = tempStacks->back();
	if (lastTempStack != nullptr)
	{
		for (size_t i = 0; i < lastTempStack->size(); i++)
		{
			if (*lastTempStack->at(i) == AS_ELSE)
				indentCountIncrement++;
		}
	}
	return indentCountIncrement;
}

/**
 * Extract a preprocessor statement without the #.
 * If a error occurs an empty string is returned.
 */
string ASBeautifier::extractPreprocessorStatement(const string& line) const
{
	string preproc;
	size_t start = line.find_first_not_of("#/ \t");
	if (start == string::npos)
		return preproc;
	size_t end = line.find_first_of("/ \t", start);
	if (end == string::npos)
		end = line.length();
	preproc = line.substr(start, end - start);
	return preproc;
}

void ASBeautifier::adjustObjCMethodDefinitionIndentation(const string& line_)
{
	// register indent for Objective-C continuation line
	if (line_.length() > 0
	        && (line_[0] == '-' || line_[0] == '+'))
	{
		if (shouldAlignMethodColon && objCColonAlignSubsequent != -1)
		{
			string convertedLine = getIndentedSpaceEquivalent(line_);
			colonIndentObjCMethodAlignment = convertedLine.find(':');
			int objCColonAlignSubsequentIndent = objCColonAlignSubsequent + indentLength;
			if (objCColonAlignSubsequentIndent > colonIndentObjCMethodAlignment)
				colonIndentObjCMethodAlignment = objCColonAlignSubsequentIndent;
		}
		else if (continuationIndentStack->empty()
		         || continuationIndentStack->back() == 0)
		{
			continuationIndentStack->emplace_back(indentLength);
			isContinuation = true;
		}
	}
	// set indent for last definition line
	else if (!lineBeginsWithOpenBrace)
	{
		if (shouldAlignMethodColon)
			spaceIndentCount = computeObjCColonAlignment(line_, colonIndentObjCMethodAlignment);
		else if (continuationIndentStack->empty())
			spaceIndentCount = spaceIndentObjCMethodAlignment;
	}
}

void ASBeautifier::adjustObjCMethodCallIndentation(const string& line_)
{
	static int keywordIndentObjCMethodAlignment = 0;
	if (shouldAlignMethodColon && objCColonAlignSubsequent != -1)
	{
		if (isInObjCMethodCallFirst)
		{
			isInObjCMethodCallFirst = false;
			string convertedLine = getIndentedSpaceEquivalent(line_);
			bracePosObjCMethodAlignment = convertedLine.find('[');
			keywordIndentObjCMethodAlignment =
			    getObjCFollowingKeyword(convertedLine, bracePosObjCMethodAlignment);
			colonIndentObjCMethodAlignment = convertedLine.find(':');
			if (colonIndentObjCMethodAlignment >= 0)
			{
				int objCColonAlignSubsequentIndent = objCColonAlignSubsequent + indentLength;
				if (objCColonAlignSubsequentIndent > colonIndentObjCMethodAlignment)
					colonIndentObjCMethodAlignment = objCColonAlignSubsequentIndent;
				if (lineBeginsWithOpenBrace)
					colonIndentObjCMethodAlignment -= indentLength;
			}
		}
		else
		{
			if (line_.find(':') != string::npos)
			{
				if (colonIndentObjCMethodAlignment < 0)
					spaceIndentCount += computeObjCColonAlignment(line_, objCColonAlignSubsequent);
				else if (objCColonAlignSubsequent > colonIndentObjCMethodAlignment)
					spaceIndentCount = computeObjCColonAlignment(line_, objCColonAlignSubsequent);
				else
					spaceIndentCount = computeObjCColonAlignment(line_, colonIndentObjCMethodAlignment);
			}
			else
			{
				if (spaceIndentCount < colonIndentObjCMethodAlignment)
					spaceIndentCount += keywordIndentObjCMethodAlignment;
			}
		}
	}
	else    // align keywords instead of colons
	{
		if (isInObjCMethodCallFirst)
		{
			isInObjCMethodCallFirst = false;
			string convertedLine = getIndentedSpaceEquivalent(line_);
			bracePosObjCMethodAlignment = convertedLine.find('[');
			keywordIndentObjCMethodAlignment =
			    getObjCFollowingKeyword(convertedLine, bracePosObjCMethodAlignment);
		}
		else
		{
			if (spaceIndentCount < keywordIndentObjCMethodAlignment + bracePosObjCMethodAlignment)
				spaceIndentCount += keywordIndentObjCMethodAlignment;
		}
	}
}

/**
 * Clear the variables used to align the Objective-C method definitions.
 */
void ASBeautifier::clearObjCMethodDefinitionAlignment()
{
	assert(isImmediatelyPostObjCMethodDefinition);
	spaceIndentCount = 0;
	spaceIndentObjCMethodAlignment = 0;
	colonIndentObjCMethodAlignment = 0;
	isInObjCMethodDefinition = false;
	isImmediatelyPostObjCMethodDefinition = false;
	if (!continuationIndentStack->empty())
		continuationIndentStack->pop_back();
}

/**
 * Compute the spaceIndentCount necessary to align the current line colon
 * with the colon position in the argument.
 * If it cannot be aligned indentLength is returned and a new colon
 * position is calculated.
 */
int ASBeautifier::computeObjCColonAlignment(const string& line, int colonAlignPosition) const
{
	int colonPosition = line.find(':');
	if (colonPosition < 0 || colonPosition > colonAlignPosition)
		return indentLength;
	return (colonAlignPosition - colonPosition);
}

/*
 * Compute postition of the keyword following the method call object.
 */
int ASBeautifier::getObjCFollowingKeyword(const string& line, int bracePos) const
{
	assert(line[bracePos] == '[');
	size_t firstText = line.find_first_not_of(" \t", bracePos + 1);
	if (firstText == string::npos)
		return -(indentCount * indentLength - 1);
	size_t searchBeg = firstText;
	size_t objectEnd = 0;	// end of object text
	if (line[searchBeg] == '[')
	{
		objectEnd = line.find(']', searchBeg + 1);
		if (objectEnd == string::npos)
			return 0;
	}
	else
	{
		if (line[searchBeg] == '(')
		{
			searchBeg = line.find(')', searchBeg + 1);
			if (searchBeg == string::npos)
				return 0;
		}
		// bypass the object name
		objectEnd = line.find_first_of(" \t", searchBeg + 1);
		if (objectEnd == string::npos)
			return 0;
		--objectEnd;
	}
	size_t keyPos = line.find_first_not_of(" \t", objectEnd + 1);
	if (keyPos == string::npos)
		return 0;
	return keyPos - firstText;
}

/**
 * Get a line using the current space indent with all tabs replaced by spaces.
 * The indentCount is NOT included
 * Needed to compute an accurate alignment.
 */
string ASBeautifier::getIndentedSpaceEquivalent(const string& line_) const
{
	string spaceIndent;
	spaceIndent.append(spaceIndentCount, ' ');
	string convertedLine = spaceIndent + line_;
	for (size_t i = spaceIndent.length(); i < convertedLine.length(); i++)
	{
		if (convertedLine[i] == '\t')
		{
			size_t numSpaces = indentLength - (i % indentLength);
			convertedLine.replace(i, 1, numSpaces, ' ');
			i += indentLength - 1;
		}
	}
	return convertedLine;
}

/**
 * Parse the current line to update indentCount and spaceIndentCount.
 */
void ASBeautifier::parseCurrentLine(const string& line)
{
	bool isInLineComment = false;
	bool isInOperator = false;
	bool isSpecialChar = false;
	bool haveCaseIndent = false;
	bool haveAssignmentThisLine = false;
	bool closingBraceReached = false;
	bool previousLineProbation = (probationHeader != nullptr);
	char ch = ' ';
	int tabIncrementIn = 0;
	if (isInQuote
	        && !haveLineContinuationChar
	        && !isInVerbatimQuote
	        && !isInAsm)
		isInQuote = false;				// missing closing quote
	haveLineContinuationChar = false;

	for (size_t i = 0; i < line.length(); i++)
	{
		ch = line[i];

		if (isInBeautifySQL)
			continue;

		// handle special characters (i.e. backslash+character such as \n, \t, ...)
		if (isInQuote && !isInVerbatimQuote)
		{
			if (isSpecialChar)
			{
				isSpecialChar = false;
				continue;
			}
			if (line.compare(i, 2, "\\\\") == 0)
			{
				i++;
				continue;
			}
			if (ch == '\\')
			{
				if (peekNextChar(line, i) == ' ')   // is this '\' at end of line
					haveLineContinuationChar = true;
				else
					isSpecialChar = true;
				continue;
			}
		}
		else if (isInDefine && ch == '\\')
			continue;

		// bypass whitespace here
		if (isWhiteSpace(ch))
		{
			if (ch == '\t')
				tabIncrementIn += convertTabToSpaces(i, tabIncrementIn);
			continue;
		}

		// handle quotes (such as 'x' and "Hello Dolly")
		if (!(isInComment || isInLineComment)
		        && (ch == '"'
		            || (ch == '\'' && !isDigitSeparator(line, i))))
		{
			if (!isInQuote)
			{
				quoteChar = ch;
				isInQuote = true;
				char prevCh = i > 0 ? line[i - 1] : ' ';
				if (isCStyle() && prevCh == 'R')
				{
					int parenPos = line.find('(', i);
					if (parenPos != -1)
					{
						isInVerbatimQuote = true;
						verbatimDelimiter = line.substr(i + 1, parenPos - i - 1);
					}
				}
				else if (isSharpStyle() && prevCh == '@')
					isInVerbatimQuote = true;
				// check for "C" following "extern"
				else if (g_preprocessorCppExternCBrace == 2 && line.compare(i, 3, "\"C\"") == 0)
					++g_preprocessorCppExternCBrace;
			}
			else if (isInVerbatimQuote && ch == '"')
			{
				if (isCStyle())
				{
					string delim = ')' + verbatimDelimiter;
					int delimStart = i - delim.length();
					if (delimStart > 0 && line.substr(delimStart, delim.length()) == delim)
					{
						isInQuote = false;
						isInVerbatimQuote = false;
					}
				}
				else if (isSharpStyle())
				{
					if (line.compare(i, 2, "\"\"") == 0)
						i++;
					else
					{
						isInQuote = false;
						isInVerbatimQuote = false;
						continue;
					}
				}
			}
			else if (quoteChar == ch)
			{
				isInQuote = false;
				isContinuation = true;
				continue;
			}
		}
		if (isInQuote)
			continue;

		// handle comments

		if (!(isInComment || isInLineComment) && line.compare(i, 2, "//") == 0)
		{
			// if there is a 'case' statement after these comments unindent by 1
			if (isCaseHeaderCommentIndent)
				--indentCount;
			// isElseHeaderIndent is set by ASFormatter if shouldBreakElseIfs is requested
			// if there is an 'else' after these comments a tempStacks indent is required
			if (isElseHeaderIndent && lineOpensWithLineComment && !tempStacks->empty())
				indentCount += adjustIndentCountForBreakElseIfComments();
			isInLineComment = true;
			i++;
			continue;
		}
		else if (!(isInComment || isInLineComment) && line.compare(i, 2, "/*") == 0)
		{
			// if there is a 'case' statement after these comments unindent by 1
			if (isCaseHeaderCommentIndent && lineOpensWithComment)
				--indentCount;
			// isElseHeaderIndent is set by ASFormatter if shouldBreakElseIfs is requested
			// if there is an 'else' after these comments a tempStacks indent is required
			if (isElseHeaderIndent && lineOpensWithComment && !tempStacks->empty())
				indentCount += adjustIndentCountForBreakElseIfComments();
			isInComment = true;
			i++;
			if (!lineOpensWithComment)				// does line start with comment?
				blockCommentNoIndent = true;        // if no, cannot indent continuation lines
			continue;
		}
		else if ((isInComment || isInLineComment) && line.compare(i, 2, "*/") == 0)
		{
			size_t firstText = line.find_first_not_of(" \t");
			// if there is a 'case' statement after these comments unindent by 1
			// only if the ending comment is the first entry on the line
			if (isCaseHeaderCommentIndent && firstText == i)
				--indentCount;
			// if this comment close starts the line, must check for else-if indent
			// isElseHeaderIndent is set by ASFormatter if shouldBreakElseIfs is requested
			// if there is an 'else' after these comments a tempStacks indent is required
			if (firstText == i)
			{
				if (isElseHeaderIndent && !lineOpensWithComment && !tempStacks->empty())
					indentCount += adjustIndentCountForBreakElseIfComments();
			}
			isInComment = false;
			i++;
			blockCommentNoIndent = false;           // ok to indent next comment
			continue;
		}
		// treat indented preprocessor lines as a line comment
		else if (line[0] == '#' && isIndentedPreprocessor(line, i))
		{
			isInLineComment = true;
		}

		if (isInLineComment)
		{
			// bypass rest of the comment up to the comment end
			while (i + 1 < line.length())
				i++;

			continue;
		}
		if (isInComment)
		{
			// if there is a 'case' statement after these comments unindent by 1
			if (!lineOpensWithComment && isCaseHeaderCommentIndent)
				--indentCount;
			// isElseHeaderIndent is set by ASFormatter if shouldBreakElseIfs is requested
			// if there is an 'else' after these comments a tempStacks indent is required
			if (!lineOpensWithComment && isElseHeaderIndent && !tempStacks->empty())
				indentCount += adjustIndentCountForBreakElseIfComments();
			// bypass rest of the comment up to the comment end
			while (i + 1 < line.length()
			        && line.compare(i + 1, 2, "*/") != 0)
				i++;

			continue;
		}

		// if we have reached this far then we are NOT in a comment or string of special character...

		if (probationHeader != nullptr)
		{
			if ((probationHeader == &AS_STATIC && ch == '{')
			        || (probationHeader == &AS_SYNCHRONIZED && ch == '('))
			{
				// insert the probation header as a new header
				isInHeader = true;
				headerStack->emplace_back(probationHeader);

				// handle the specific probation header
				isInConditional = (probationHeader == &AS_SYNCHRONIZED);

				isContinuation = false;
				// if the probation comes from the previous line, then indent by 1 tab count.
				if (previousLineProbation
				        && ch == '{'
				        && !(blockIndent && probationHeader == &AS_STATIC))
				{
					++indentCount;
					previousLineProbationTab = true;
				}
				previousLineProbation = false;
			}

			// dismiss the probation header
			probationHeader = nullptr;
		}

		prevNonSpaceCh = currentNonSpaceCh;
		currentNonSpaceCh = ch;
		if (!isLegalNameChar(ch) && ch != ',' && ch != ';')
		{
			prevNonLegalCh = currentNonLegalCh;
			currentNonLegalCh = ch;
		}

		if (isInHeader)
		{
			isInHeader = false;
			currentHeader = headerStack->back();
		}
		else
			currentHeader = nullptr;

		if (isCStyle() && isInTemplate
		        && (ch == '<' || ch == '>')
		        && !(line.length() > i + 1 && line.compare(i, 2, ">=") == 0))
		{
			if (ch == '<')
			{
				++templateDepth;
				continuationIndentStackSizeStack->push_back(continuationIndentStack->size());
				registerContinuationIndent(line, i, spaceIndentCount, tabIncrementIn, 0, true);
			}
			else if (ch == '>')
			{
				popLastContinuationIndent();
				if (--templateDepth <= 0)
				{
					ch = ';';
					isInTemplate = false;
					templateDepth = 0;
				}
			}
		}

		// handle parentheses
		if (ch == '(' || ch == '[' || ch == ')' || ch == ']')
		{
			if (ch == '(' || ch == '[')
			{
				isInOperator = false;
				// if have a struct header, this is a declaration not a definition
				if (ch == '('
				        && !headerStack->empty()
				        && headerStack->back() == &AS_STRUCT)
				{
					headerStack->pop_back();
					isInClassHeader = false;
					if (line.find(AS_STRUCT, 0) > i)	// if not on this line
						indentCount -= classInitializerIndents;
					if (indentCount < 0)
						indentCount = 0;
				}

				if (parenDepth == 0)
				{
					parenStatementStack->push_back(isContinuation);
					isContinuation = true;
				}
				parenDepth++;
				if (ch == '[')
				{
					++squareBracketCount;
					if (squareBracketCount == 1 && isCStyle())
					{
						isInObjCMethodCall = true;
						isInObjCMethodCallFirst = true;
					}
				}

				continuationIndentStackSizeStack->push_back(continuationIndentStack->size());

				if (currentHeader != nullptr)
					registerContinuationIndent(line, i, spaceIndentCount, tabIncrementIn, minConditionalIndent, true);
				else
					registerContinuationIndent(line, i, spaceIndentCount, tabIncrementIn, 0, true);
			}
			else if (ch == ')' || ch == ']')
			{
				if (ch == ']')
					--squareBracketCount;
				if (squareBracketCount <= 0)
				{
					squareBracketCount = 0;
					if (isInObjCMethodCall)
						isImmediatelyPostObjCMethodCall = true;
				}
				foundPreCommandHeader = false;
				parenDepth--;
				if (parenDepth == 0)
				{
					if (!parenStatementStack->empty())      // in case of unmatched closing parens
					{
						isContinuation = parenStatementStack->back();
						parenStatementStack->pop_back();
					}
					isInAsm = false;
					isInConditional = false;
				}

				if (!continuationIndentStackSizeStack->empty())
				{
					popLastContinuationIndent();

					if (!parenIndentStack->empty())
					{
						int poppedIndent = parenIndentStack->back();
						parenIndentStack->pop_back();

						if (i == 0)
							spaceIndentCount = poppedIndent;
					}
				}
			}
			continue;
		}

		if (ch == '{')
		{
			// first, check if '{' is a block-opener or a static-array opener
			bool isBlockOpener = ((prevNonSpaceCh == '{' && braceBlockStateStack->back())
			                      || prevNonSpaceCh == '}'
			                      || prevNonSpaceCh == ')'
			                      || prevNonSpaceCh == ';'
			                      || peekNextChar(line, i) == '{'
			                      || foundPreCommandHeader
			                      || foundPreCommandMacro
			                      || isInClassHeader
			                      || (isInClassInitializer && !isLegalNameChar(prevNonSpaceCh))
			                      || isNonInStatementArray
			                      || isInObjCMethodDefinition
			                      || isInObjCInterface
			                      || isSharpAccessor
			                      || isSharpDelegate
			                      || isInExternC
			                      || isInAsmBlock
			                      || getNextWord(line, i) == AS_NEW
			                      || (isInDefine
			                          && (prevNonSpaceCh == '('
			                              || isLegalNameChar(prevNonSpaceCh))));

			if (isInObjCMethodDefinition)
			{
				objCColonAlignSubsequent = 0;
				isImmediatelyPostObjCMethodDefinition = true;
				if (lineBeginsWithOpenBrace)		// for run-in braces
					clearObjCMethodDefinitionAlignment();
			}

			if (!isBlockOpener && !isContinuation && !isInClassInitializer && !isInEnum)
			{
				if (headerStack->empty())
					isBlockOpener = true;
				else if (headerStack->back() == &AS_OPEN_BRACE
				         && headerStack->size() >= 2)
				{
					if ((*headerStack)[headerStack->size() - 2] == &AS_NAMESPACE
					        || (*headerStack)[headerStack->size() - 2] == &AS_MODULE
					        || (*headerStack)[headerStack->size() - 2] == &AS_CLASS
					        || (*headerStack)[headerStack->size() - 2] == &AS_INTERFACE
					        || (*headerStack)[headerStack->size() - 2] == &AS_STRUCT
					        || (*headerStack)[headerStack->size() - 2] == &AS_UNION)
						isBlockOpener = true;
				}
				else if (headerStack->back() == &AS_NAMESPACE
				         || headerStack->back() == &AS_MODULE
				         || headerStack->back() == &AS_CLASS
				         || headerStack->back() == &AS_INTERFACE
				         || headerStack->back() == &AS_STRUCT
				         || headerStack->back() == &AS_UNION)
					isBlockOpener = true;
			}

			if (!isBlockOpener && currentHeader != nullptr)
			{
				for (size_t n = 0; n < nonParenHeaders->size(); n++)
					if (currentHeader == (*nonParenHeaders)[n])
					{
						isBlockOpener = true;
						break;
					}
			}

			braceBlockStateStack->push_back(isBlockOpener);

			if (!isBlockOpener)
			{
				continuationIndentStackSizeStack->push_back(continuationIndentStack->size());
				registerContinuationIndent(line, i, spaceIndentCount, tabIncrementIn, 0, true);
				parenDepth++;
				if (i == 0)
					shouldIndentBracedLine = false;
				isInEnumTypeID = false;

				continue;
			}

			// this brace is a block opener...

			++lineOpeningBlocksNum;

			if (isInClassInitializer || isInEnumTypeID)
			{
				// decrease tab count if brace is broken
				if (lineBeginsWithOpenBrace)
				{
					indentCount -= classInitializerIndents;
					// decrease one more if an empty class
					if (!headerStack->empty()
					        && (*headerStack).back() == &AS_CLASS)
					{
						int nextChar = getNextProgramCharDistance(line, i);
						if ((int) line.length() > nextChar && line[nextChar] == '}')
							--indentCount;
					}
				}
			}

			if (isInObjCInterface)
			{
				isInObjCInterface = false;
				if (lineBeginsWithOpenBrace)
					--indentCount;
			}

			if (braceIndent && !namespaceIndent && !headerStack->empty()
			        && ((*headerStack).back() == &AS_NAMESPACE
			            || (*headerStack).back() == &AS_MODULE))
			{
				shouldIndentBracedLine = false;
				--indentCount;
			}

			// an indentable struct is treated like a class in the header stack
			if (!headerStack->empty()
			        && (*headerStack).back() == &AS_STRUCT
			        && isInIndentableStruct)
				(*headerStack).back() = &AS_CLASS;

			squareBracketDepthStack->emplace_back(parenDepth);
			blockStatementStack->push_back(isContinuation);

			if (!continuationIndentStack->empty())
			{
				// completely purge the inStatementIndentStack
				while (!continuationIndentStack->empty())
					popLastContinuationIndent();
				if (isInClassInitializer || isInClassHeaderTab)
				{
					if (lineBeginsWithOpenBrace || lineBeginsWithComma)
						spaceIndentCount = 0;
				}
				else
					spaceIndentCount = 0;
			}

			blockTabCount += (isContinuation ? 1 : 0);
			if (g_preprocessorCppExternCBrace == 3)
				++g_preprocessorCppExternCBrace;
			parenDepth = 0;
			isInClassHeader = false;
			isInClassHeaderTab = false;
			isInClassInitializer = false;
			isInEnumTypeID = false;
			isContinuation = false;
			isInQuestion = false;
			isInLet = false;
			foundPreCommandHeader = false;
			foundPreCommandMacro = false;
			isInExternC = false;

			tempStacks->emplace_back(new vector<const string*>);
			headerStack->emplace_back(&AS_OPEN_BRACE);
			lastLineHeader = &AS_OPEN_BRACE;

			continue;
		}	// end '{'

		//check if a header has been reached
		bool isPotentialHeader = isCharPotentialHeader(line, i);

		if (isPotentialHeader && squareBracketCount == 0)
		{
			const string* newHeader = findHeader(line, i, headers);

			// Qt headers may be variables in C++
			if (isCStyle()
			        && (newHeader == &AS_FOREVER || newHeader == &AS_FOREACH))
			{
				if (line.find_first_of("=;", i) != string::npos)
					newHeader = nullptr;
			}
			else if (newHeader == &AS_USING
			         && ASBeautifier::peekNextChar(line, i + (*newHeader).length() - 1) != '(')
				newHeader = nullptr;

			if (newHeader != nullptr)
			{
				// if we reached here, then this is a header...
				bool isIndentableHeader = true;

				isInHeader = true;

				vector<const string*>* lastTempStack = nullptr;;
				if (!tempStacks->empty())
					lastTempStack = tempStacks->back();

				// if a new block is opened, push a new stack into tempStacks to hold the
				// future list of headers in the new block.

				// take care of the special case: 'else if (...)'
				if (newHeader == &AS_IF && lastLineHeader == &AS_ELSE)
				{
					headerStack->pop_back();
				}

				// take care of 'else'
				else if (newHeader == &AS_ELSE)
				{
					if (lastTempStack != nullptr)
					{
						int indexOfIf = indexOf(*lastTempStack, &AS_IF);
						if (indexOfIf != -1)
						{
							// recreate the header list in headerStack up to the previous 'if'
							// from the temporary snapshot stored in lastTempStack.
							int restackSize = lastTempStack->size() - indexOfIf - 1;
							for (int r = 0; r < restackSize; r++)
							{
								headerStack->emplace_back(lastTempStack->back());
								lastTempStack->pop_back();
							}
							if (!closingBraceReached)
								indentCount += restackSize;
						}
						/*
						 * If the above if is not true, i.e. no 'if' before the 'else',
						 * then nothing beautiful will come out of this...
						 * I should think about inserting an Exception here to notify the caller of this...
						 */
					}
				}

				// check if 'while' closes a previous 'do'
				else if (newHeader == &AS_WHILE)
				{
					if (lastTempStack != nullptr)
					{
						int indexOfDo = indexOf(*lastTempStack, &AS_DO);
						if (indexOfDo != -1)
						{
							// recreate the header list in headerStack up to the previous 'do'
							// from the temporary snapshot stored in lastTempStack.
							int restackSize = lastTempStack->size() - indexOfDo - 1;
							for (int r = 0; r < restackSize; r++)
							{
								headerStack->emplace_back(lastTempStack->back());
								lastTempStack->pop_back();
							}
							if (!closingBraceReached)
								indentCount += restackSize;
						}
					}
				}
				// check if 'catch' closes a previous 'try' or 'catch'
				else if (newHeader == &AS_CATCH || newHeader == &AS_FINALLY)
				{
					if (lastTempStack != nullptr)
					{
						int indexOfTry = indexOf(*lastTempStack, &AS_TRY);
						if (indexOfTry == -1)
							indexOfTry = indexOf(*lastTempStack, &AS_CATCH);
						if (indexOfTry != -1)
						{
							// recreate the header list in headerStack up to the previous 'try'
							// from the temporary snapshot stored in lastTempStack.
							int restackSize = lastTempStack->size() - indexOfTry - 1;
							for (int r = 0; r < restackSize; r++)
							{
								headerStack->emplace_back(lastTempStack->back());
								lastTempStack->pop_back();
							}

							if (!closingBraceReached)
								indentCount += restackSize;
						}
					}
				}
				else if (newHeader == &AS_CASE)
				{
					isInCase = true;
					if (!haveCaseIndent)
					{
						haveCaseIndent = true;
						if (!lineBeginsWithOpenBrace)
							--indentCount;
					}
				}
				else if (newHeader == &AS_DEFAULT)
				{
					isInCase = true;
					--indentCount;
				}
				else if (newHeader == &AS_STATIC
				         || newHeader == &AS_SYNCHRONIZED)
				{
					if (!headerStack->empty()
					        && (headerStack->back() == &AS_STATIC
					            || headerStack->back() == &AS_SYNCHRONIZED))
					{
						isIndentableHeader = false;
					}
					else
					{
						isIndentableHeader = false;
						probationHeader = newHeader;
					}
				}
				else if (newHeader == &AS_TEMPLATE)
				{
					isInTemplate = true;
					isIndentableHeader = false;
				}

				if (isIndentableHeader)
				{
					headerStack->emplace_back(newHeader);
					isContinuation = false;
					if (indexOf(*nonParenHeaders, newHeader) == -1)
					{
						isInConditional = true;
					}
					lastLineHeader = newHeader;
				}
				else
					isInHeader = false;

				i += newHeader->length() - 1;

				continue;
			}  // newHeader != nullptr

			if (findHeader(line, i, preCommandHeaders) != nullptr)
				foundPreCommandHeader = true;

			// Objective-C NSException macros are preCommandHeaders
			if (isCStyle() && findKeyword(line, i, AS_NS_DURING))
				foundPreCommandMacro = true;
			if (isCStyle() && findKeyword(line, i, AS_NS_HANDLER))
				foundPreCommandMacro = true;

			if (parenDepth == 0 && findKeyword(line, i, AS_ENUM))
				isInEnum = true;

			if (isSharpStyle() && findKeyword(line, i, AS_LET))
				isInLet = true;

		}   // isPotentialHeader

		if (ch == '?')
			isInQuestion = true;

		// special handling of colons
		if (ch == ':')
		{
			if (line.length() > i + 1 && line[i + 1] == ':') // look for ::
			{
				++i;
				continue;
			}
			else if (isInQuestion)
			{
				// do nothing special
			}
			else if (parenDepth > 0)
			{
				// found a 'for' loop or an objective-C statement
				// so do nothing special
			}
			else if (isInEnum)
			{
				// found an enum with a base-type
				isInEnumTypeID = true;
				if (i == 0)
					indentCount += classInitializerIndents;
			}
			else if (isCStyle()
			         && !isInCase
			         && (prevNonSpaceCh == ')' || foundPreCommandHeader))
			{
				// found a 'class' c'tor initializer
				isInClassInitializer = true;
				registerContinuationIndentColon(line, i, tabIncrementIn);
				if (i == 0)
					indentCount += classInitializerIndents;
			}
			else if (isInClassHeader || isInObjCInterface)
			{
				// is in a 'class A : public B' definition
				isInClassHeaderTab = true;
				registerContinuationIndentColon(line, i, tabIncrementIn);
			}
			else if (isInAsm || isInAsmOneLine || isInAsmBlock)
			{
				// do nothing special
			}
			else if (isDigit(peekNextChar(line, i)))
			{
				// found a bit field - do nothing special
			}
			else if (isCStyle() && isInClass && prevNonSpaceCh != ')')
			{
				// found a 'private:' or 'public:' inside a class definition
				--indentCount;
				if (modifierIndent)
					spaceIndentCount += (indentLength / 2);
			}
			else if (isCStyle() && !isInClass
			         && headerStack->size() >= 2
			         && (*headerStack)[headerStack->size() - 2] == &AS_CLASS
			         && (*headerStack)[headerStack->size() - 1] == &AS_OPEN_BRACE)
			{
				// found a 'private:' or 'public:' inside a class definition
				// and on the same line as the class opening brace
				// do nothing
			}
			else if (isJavaStyle() && lastLineHeader == &AS_FOR)
			{
				// found a java for-each statement
				// so do nothing special
			}
			else
			{
				currentNonSpaceCh = ';'; // so that braces after the ':' will appear as block-openers
				char peekedChar = peekNextChar(line, i);
				if (isInCase)
				{
					isInCase = false;
					ch = ';'; // from here on, treat char as ';'
				}
				else if (isCStyle() || (isSharpStyle() && peekedChar == ';'))
				{
					// is in a label (e.g. 'label1:')
					if (labelIndent)
						--indentCount; // unindent label by one indent
					else if (!lineBeginsWithOpenBrace)
						indentCount = 0; // completely flush indent to left
				}
			}
		}

		if ((ch == ';' || (parenDepth > 0 && ch == ',')) && !continuationIndentStackSizeStack->empty())
			while ((int) continuationIndentStackSizeStack->back() + (parenDepth > 0 ? 1 : 0)
			        < (int) continuationIndentStack->size())
				continuationIndentStack->pop_back();

		else if (ch == ',' && isInEnum && isNonInStatementArray && !continuationIndentStack->empty())
			continuationIndentStack->pop_back();

		// handle commas
		// previous "isInStatement" will be from an assignment operator or class initializer
		if (ch == ',' && parenDepth == 0 && !isContinuation && !isNonInStatementArray)
		{
			// is comma at end of line
			size_t nextChar = line.find_first_not_of(" \t", i + 1);
			if (nextChar != string::npos)
			{
				if (line.compare(nextChar, 2, "//") == 0
				        || line.compare(nextChar, 2, "/*") == 0)
					nextChar = string::npos;
			}
			// register indent
			if (nextChar == string::npos)
			{
				// register indent at previous word
				if (isJavaStyle() && isInClassHeader)
				{
					// do nothing for now
				}
				// register indent at second word on the line
				else if (!isInTemplate && !isInClassHeaderTab && !isInClassInitializer)
				{
					int prevWord = getContinuationIndentComma(line, i);
					int continuationIndentCount = prevWord + spaceIndentCount + tabIncrementIn;
					continuationIndentStack->emplace_back(continuationIndentCount);
					isContinuation = true;
				}
			}
		}
		// handle comma first initializers
		if (ch == ',' && parenDepth == 0 && lineBeginsWithComma
		        && (isInClassInitializer || isInClassHeaderTab))
			spaceIndentCount = 0;

		// handle ends of statements
		if ((ch == ';' && parenDepth == 0) || ch == '}')
		{
			if (ch == '}')
			{
				// first check if this '}' closes a previous block, or a static array...
				if (braceBlockStateStack->size() > 1)
				{
					bool braceBlockState = braceBlockStateStack->back();
					braceBlockStateStack->pop_back();
					if (!braceBlockState)
					{
						if (!continuationIndentStackSizeStack->empty())
						{
							// this brace is a static array
							popLastContinuationIndent();
							parenDepth--;
							if (i == 0)
								shouldIndentBracedLine = false;

							if (!parenIndentStack->empty())
							{
								int poppedIndent = parenIndentStack->back();
								parenIndentStack->pop_back();
								if (i == 0)
									spaceIndentCount = poppedIndent;
							}
						}
						continue;
					}
				}

				// this brace is block closer...

				++lineClosingBlocksNum;

				if (!continuationIndentStackSizeStack->empty())
					popLastContinuationIndent();

				if (!squareBracketDepthStack->empty())
				{
					parenDepth = squareBracketDepthStack->back();
					squareBracketDepthStack->pop_back();
					isContinuation = blockStatementStack->back();
					blockStatementStack->pop_back();

					if (isContinuation)
						blockTabCount--;
				}

				closingBraceReached = true;
				if (i == 0)
					spaceIndentCount = 0;
				isInAsmBlock = false;
				isInAsm = isInAsmOneLine = isInQuote = false;	// close these just in case

				int headerPlace = indexOf(*headerStack, &AS_OPEN_BRACE);
				if (headerPlace != -1)
				{
					const string* popped = headerStack->back();
					while (popped != &AS_OPEN_BRACE)
					{
						headerStack->pop_back();
						popped = headerStack->back();
					}
					headerStack->pop_back();

					if (headerStack->empty())
						g_preprocessorCppExternCBrace = 0;

					// do not indent namespace brace unless namespaces are indented
					if (!namespaceIndent && !headerStack->empty()
					        && ((*headerStack).back() == &AS_NAMESPACE
					            || (*headerStack).back() == &AS_MODULE)
					        && i == 0)		// must be the first brace on the line
						shouldIndentBracedLine = false;

					if (!tempStacks->empty())
					{
						vector<const string*>* temp = tempStacks->back();
						tempStacks->pop_back();
						delete temp;
					}
				}

				ch = ' '; // needed due to cases such as '}else{', so that headers ('else' in this case) will be identified...
			}	// ch == '}'

			/*
			 * Create a temporary snapshot of the current block's header-list in the
			 * uppermost inner stack in tempStacks, and clear the headerStack up to
			 * the beginning of the block.
			 * Thus, the next future statement will think it comes one indent past
			 * the block's '{' unless it specifically checks for a companion-header
			 * (such as a previous 'if' for an 'else' header) within the tempStacks,
			 * and recreates the temporary snapshot by manipulating the tempStacks.
			 */
			if (!tempStacks->back()->empty())
				while (!tempStacks->back()->empty())
					tempStacks->back()->pop_back();
			while (!headerStack->empty() && headerStack->back() != &AS_OPEN_BRACE)
			{
				tempStacks->back()->emplace_back(headerStack->back());
				headerStack->pop_back();
			}

			if (parenDepth == 0 && ch == ';')
				isContinuation = false;

			if (isInObjCMethodDefinition)
			{
				objCColonAlignSubsequent = 0;
				isImmediatelyPostObjCMethodDefinition = true;
			}

			previousLastLineHeader = nullptr;
			isInClassHeader = false;		// for 'friend' class
			isInEnum = false;
			isInEnumTypeID = false;
			isInQuestion = false;
			isInTemplate = false;
			isInObjCInterface = false;
			foundPreCommandHeader = false;
			foundPreCommandMacro = false;
			squareBracketCount = 0;

			continue;
		}

		if (isPotentialHeader)
		{
			// check for preBlockStatements in C/C++ ONLY if not within parentheses
			// (otherwise 'struct XXX' statements would be wrongly interpreted...)
			if (!isInTemplate && !(isCStyle() && parenDepth > 0))
			{
				const string* newHeader = findHeader(line, i, preBlockStatements);
				// handle CORBA IDL module
				if (newHeader == &AS_MODULE)
				{
					char nextChar = peekNextChar(line, i + newHeader->length() - 1);
					if (prevNonSpaceCh == ')' || !isalpha(nextChar))
						newHeader = nullptr;
				}
				if (newHeader != nullptr
				        && !(isCStyle() && newHeader == &AS_CLASS && isInEnum))	// is not 'enum class'
				{
					if (!isSharpStyle())
						headerStack->emplace_back(newHeader);
					// do not need 'where' in the headerStack
					// do not need second 'class' statement in a row
					else if (!(newHeader == &AS_WHERE
					           || ((newHeader == &AS_CLASS || newHeader == &AS_STRUCT)
					               && !headerStack->empty()
					               && (headerStack->back() == &AS_CLASS
					                   || headerStack->back() == &AS_STRUCT))))
						headerStack->emplace_back(newHeader);

					if (!headerStack->empty())
					{
						if ((*headerStack).back() == &AS_CLASS
						        || (*headerStack).back() == &AS_STRUCT
						        || (*headerStack).back() == &AS_INTERFACE)
						{
							isInClassHeader = true;
						}
						else if ((*headerStack).back() == &AS_NAMESPACE
						         || (*headerStack).back() == &AS_MODULE)
						{
							// remove continuationIndent from namespace
							if (!continuationIndentStack->empty())
								continuationIndentStack->pop_back();
							isContinuation = false;
						}
					}

					i += newHeader->length() - 1;
					continue;
				}
			}
			const string* foundIndentableHeader = findHeader(line, i, indentableHeaders);

			if (foundIndentableHeader != nullptr)
			{
				// must bypass the header before registering the in statement
				i += foundIndentableHeader->length() - 1;
				if (!isInOperator && !isInTemplate && !isNonInStatementArray)
				{
					registerContinuationIndent(line, i, spaceIndentCount, tabIncrementIn, 0, false);
					isContinuation = true;
				}
				continue;
			}

			if (isCStyle() && findKeyword(line, i, AS_OPERATOR))
				isInOperator = true;

			if (g_preprocessorCppExternCBrace == 1 && findKeyword(line, i, AS_EXTERN))
				++g_preprocessorCppExternCBrace;

			if (g_preprocessorCppExternCBrace == 3)	// extern "C" is not followed by a '{'
				g_preprocessorCppExternCBrace = 0;

			// "new" operator is a pointer, not a calculation
			if (findKeyword(line, i, AS_NEW))
			{
				if (isContinuation && !continuationIndentStack->empty() && prevNonSpaceCh == '=')
					continuationIndentStack->back() = 0;
			}

			if (isCStyle())
			{
				if (findKeyword(line, i, AS_ASM)
				        || findKeyword(line, i, AS__ASM__))
				{
					isInAsm = true;
				}
				else if (findKeyword(line, i, AS_MS_ASM)		// microsoft specific
				         || findKeyword(line, i, AS_MS__ASM))
				{
					int index = 4;
					if (peekNextChar(line, i) == '_')		// check for __asm
						index = 5;

					char peekedChar = peekNextChar(line, i + index);
					if (peekedChar == '{' || peekedChar == ' ')
						isInAsmBlock = true;
					else
						isInAsmOneLine = true;
				}
			}

			// bypass the entire name for all others
			string name = getCurrentWord(line, i);
			i += name.length() - 1;
			continue;
		}

		// Handle Objective-C statements

		if (ch == '@' && !isWhiteSpace(line[i + 1])
		        && isCharPotentialHeader(line, i + 1))
		{
			string curWord = getCurrentWord(line, i + 1);
			if (curWord == AS_INTERFACE	&& headerStack->empty())
			{
				isInObjCInterface = true;
				string name = '@' + curWord;
				i += name.length() - 1;
				continue;
			}
			else if (isInObjCInterface)
			{
				--indentCount;
				isInObjCInterface = false;
			}

			if (curWord == AS_PUBLIC
			        || curWord == AS_PRIVATE
			        || curWord == AS_PROTECTED)
			{
				--indentCount;
				if (modifierIndent)
					spaceIndentCount += (indentLength / 2);
				string name = '@' + curWord;
				i += name.length() - 1;
				continue;
			}
			else if (curWord == AS_END)
			{
				popLastContinuationIndent();
				spaceIndentCount = 0;
				isInObjCMethodDefinition = false;
				string name = '@' + curWord;
				i += name.length() - 1;
				continue;
			}
		}
		else if ((ch == '-' || ch == '+')
		         && peekNextChar(line, i) == '('
		         && headerStack->empty()
		         && line.find_first_not_of(" \t") == i)
		{
			if (isInObjCInterface)
				--indentCount;
			isInObjCInterface = false;
			isInObjCMethodDefinition = true;
			continue;
		}

		// Handle operators

		bool isPotentialOperator = isCharPotentialOperator(ch);

		if (isPotentialOperator)
		{
			// Check if an operator has been reached.
			const string* foundAssignmentOp = findOperator(line, i, assignmentOperators);
			const string* foundNonAssignmentOp = findOperator(line, i, nonAssignmentOperators);

			if (foundNonAssignmentOp != nullptr)
			{
				if (foundNonAssignmentOp == &AS_LAMBDA)
					foundPreCommandHeader = true;
				if (isInTemplate && foundNonAssignmentOp == &AS_GR_GR)
					foundNonAssignmentOp = nullptr;
			}

			// Since findHeader's boundary checking was not used above, it is possible
			// that both an assignment op and a non-assignment op where found,
			// e.g. '>>' and '>>='. If this is the case, treat the LONGER one as the
			// found operator.
			if (foundAssignmentOp != nullptr && foundNonAssignmentOp != nullptr)
			{
				if (foundAssignmentOp->length() < foundNonAssignmentOp->length())
					foundAssignmentOp = nullptr;
				else
					foundNonAssignmentOp = nullptr;
			}

			if (foundNonAssignmentOp != nullptr)
			{
				if (foundNonAssignmentOp->length() > 1)
					i += foundNonAssignmentOp->length() - 1;

				// For C++ input/output, operator<< and >> should be
				// aligned, if we are not in a statement already and
				// also not in the "operator<<(...)" header line
				if (!isInOperator
				        && continuationIndentStack->empty()
				        && isCStyle()
				        && (foundNonAssignmentOp == &AS_GR_GR
				            || foundNonAssignmentOp == &AS_LS_LS))
				{
					// this will be true if the line begins with the operator
					if (i < 2 && spaceIndentCount == 0)
						spaceIndentCount += 2 * indentLength;
					// align to the beginning column of the operator
					registerContinuationIndent(line, i - foundNonAssignmentOp->length(), spaceIndentCount, tabIncrementIn, 0, false);
				}
			}

			else if (foundAssignmentOp != nullptr)
			{
				foundPreCommandHeader = false;		// clears this for array assignments
				foundPreCommandMacro = false;

				if (foundAssignmentOp->length() > 1)
					i += foundAssignmentOp->length() - 1;

				if (!isInOperator && !isInTemplate && (!isNonInStatementArray || isInEnum))
				{
					// if multiple assignments, align on the previous word
					if (foundAssignmentOp == &AS_ASSIGN
					        && prevNonSpaceCh != ']'		// an array
					        && statementEndsWithComma(line, i))
					{
						if (!haveAssignmentThisLine)		// only one assignment indent per line
						{
							// register indent at previous word
							haveAssignmentThisLine = true;
							int prevWordIndex = getContinuationIndentAssign(line, i);
							int continuationIndentCount = prevWordIndex + spaceIndentCount + tabIncrementIn;
							continuationIndentStack->emplace_back(continuationIndentCount);
							isContinuation = true;
						}
					}
					// don't indent an assignment if 'let'
					else if (isInLet)
					{
						isInLet = false;
					}
					else if (!lineBeginsWithComma)
					{
						if (i == 0 && spaceIndentCount == 0)
							spaceIndentCount += indentLength;
						registerContinuationIndent(line, i, spaceIndentCount, tabIncrementIn, 0, false);
						isContinuation = true;
					}
				}
			}
		}
	}	// end of for loop * end of for loop * end of for loop * end of for loop * end of for loop *
}

}   // end namespace astyle
