// ASEnhancer.cpp
// Copyright (c) 2017 by Jim Pattee <jimp03@email.com>.
// This code is licensed under the MIT License.
// License.md describes the conditions under which this software may be distributed.

//-----------------------------------------------------------------------------
// headers
//-----------------------------------------------------------------------------

#include "astyle.h"

//-----------------------------------------------------------------------------
// astyle namespace
//-----------------------------------------------------------------------------

namespace astyle {
//
//-----------------------------------------------------------------------------
// ASEnhancer class
//-----------------------------------------------------------------------------

/**
 * ASEnhancer constructor
 */
ASEnhancer::ASEnhancer()
{
}

/**
 * Destructor of ASEnhancer
 */
ASEnhancer::~ASEnhancer()
{
}

/**
 * initialize the ASEnhancer.
 *
 * init() is called each time an ASFormatter object is initialized.
 */
void ASEnhancer::init(int  _fileType,
                      int  _indentLength,
                      int  _tabLength,
                      bool _useTabs,
                      bool _forceTab,
                      bool _namespaceIndent,
                      bool _caseIndent,
                      bool _preprocBlockIndent,
                      bool _preprocDefineIndent,
                      bool _emptyLineFill,
                      vector<const pair<const string, const string>* >* _indentableMacros)
{
	// formatting variables from ASFormatter and ASBeautifier
	ASBase::init(_fileType);
	indentLength = _indentLength;
	tabLength = _tabLength;
	useTabs = _useTabs;
	forceTab = _forceTab;
	namespaceIndent = _namespaceIndent;
	caseIndent = _caseIndent;
	preprocBlockIndent = _preprocBlockIndent;
	preprocDefineIndent = _preprocDefineIndent;
	emptyLineFill = _emptyLineFill;
	indentableMacros = _indentableMacros;
	quoteChar = '\'';

	// unindent variables
	lineNumber = 0;
	braceCount = 0;
	isInComment = false;
	isInQuote = false;
	switchDepth = 0;
	eventPreprocDepth = 0;
	lookingForCaseBrace = false;
	unindentNextLine = false;
	shouldUnindentLine = false;
	shouldUnindentComment = false;

	// switch struct and vector
	sw.switchBraceCount = 0;
	sw.unindentDepth = 0;
	sw.unindentCase = false;
	switchStack.clear();

	// other variables
	nextLineIsEventIndent = false;
	isInEventTable = false;
	nextLineIsDeclareIndent = false;
	isInDeclareSection = false;
}

/**
 * additional formatting for line of source code.
 * every line of source code in a source code file should be sent
 *     one after the other to this function.
 * indents event tables
 * unindents the case blocks
 *
 * @param line       the original formatted line will be updated if necessary.
 */
void ASEnhancer::enhance(string& line, bool isInNamespace, bool isInPreprocessor, bool isInSQL)
{
	shouldUnindentLine = true;
	shouldUnindentComment = false;
	lineNumber++;

	// check for beginning of event table
	if (nextLineIsEventIndent)
	{
		isInEventTable = true;
		nextLineIsEventIndent = false;
	}

	// check for beginning of SQL declare section
	if (nextLineIsDeclareIndent)
	{
		isInDeclareSection = true;
		nextLineIsDeclareIndent = false;
	}

	if (line.length() == 0
	        && !isInEventTable
	        && !isInDeclareSection
	        && !emptyLineFill)
		return;

	// test for unindent on attached braces
	if (unindentNextLine)
	{
		sw.unindentDepth++;
		sw.unindentCase = true;
		unindentNextLine = false;
	}

	// parse characters in the current line
	parseCurrentLine(line, isInPreprocessor, isInSQL);

	// check for SQL indentable lines
	if (isInDeclareSection)
	{
		size_t firstText = line.find_first_not_of(" \t");
		if (firstText == string::npos || line[firstText] != '#')
			indentLine(line, 1);
	}

	// check for event table indentable lines
	if (isInEventTable
	        && (eventPreprocDepth == 0
	            || (namespaceIndent && isInNamespace)))
	{
		size_t firstText = line.find_first_not_of(" \t");
		if (firstText == string::npos || line[firstText] != '#')
			indentLine(line, 1);
	}

	if (shouldUnindentComment && sw.unindentDepth > 0)
		unindentLine(line, sw.unindentDepth - 1);
	else if (shouldUnindentLine && sw.unindentDepth > 0)
		unindentLine(line, sw.unindentDepth);
}

/**
 * convert a force-tab indent to spaces
 *
 * @param line          a reference to the line that will be converted.
 */
void ASEnhancer::convertForceTabIndentToSpaces(string& line) const
{
	// replace tab indents with spaces
	for (size_t i = 0; i < line.length(); i++)
	{
		if (!isWhiteSpace(line[i]))
			break;
		if (line[i] == '\t')
		{
			line.erase(i, 1);
			line.insert(i, tabLength, ' ');
			i += tabLength - 1;
		}
	}
}

/**
 * convert a space indent to force-tab
 *
 * @param line          a reference to the line that will be converted.
 */
void ASEnhancer::convertSpaceIndentToForceTab(string& line) const
{
	assert(tabLength > 0);

	// replace leading spaces with tab indents
	size_t newSpaceIndentLength = line.find_first_not_of(" \t");
	size_t tabCount = newSpaceIndentLength / tabLength;		// truncate extra spaces
	line.replace(0U, tabCount * tabLength, tabCount, '\t');
}

/**
 * find the colon following a 'case' statement
 *
 * @param line          a reference to the line.
 * @param caseIndex     the line index of the case statement.
 * @return              the line index of the colon.
 */
size_t ASEnhancer::findCaseColon(const string& line, size_t caseIndex) const
{
	size_t i = caseIndex;
	bool isInQuote_ = false;
	char quoteChar_ = ' ';
	for (; i < line.length(); i++)
	{
		if (isInQuote_)
		{
			if (line[i] == '\\')
			{
				i++;
				continue;
			}
			else if (line[i] == quoteChar_)          // check ending quote
			{
				isInQuote_ = false;
				quoteChar_ = ' ';
				continue;
			}
			else
			{
				continue;                           // must close quote before continuing
			}
		}
		if (line[i] == '"' 		// check opening quote
		        || (line[i] == '\'' && !isDigitSeparator(line, i)))
		{
			isInQuote_ = true;
			quoteChar_ = line[i];
			continue;
		}
		if (line[i] == ':')
		{
			if ((i + 1 < line.length()) && (line[i + 1] == ':'))
				i++;                                // bypass scope resolution operator
			else
				break;                              // found it
		}
	}
	return i;
}

/**
* indent a line by a given number of tabsets
 *    by inserting leading whitespace to the line argument.
 *
 * @param line          a reference to the line to indent.
 * @param indent        the number of tabsets to insert.
 * @return              the number of characters inserted.
 */
int ASEnhancer::indentLine(string& line, int indent) const
{
	if (line.length() == 0
	        && !emptyLineFill)
		return 0;

	size_t charsToInsert = 0;

	if (forceTab && indentLength != tabLength)
	{
		// replace tab indents with spaces
		convertForceTabIndentToSpaces(line);
		// insert the space indents
		charsToInsert = indent * indentLength;
		line.insert(line.begin(), charsToInsert, ' ');
		// replace leading spaces with tab indents
		convertSpaceIndentToForceTab(line);
	}
	else if (useTabs)
	{
		charsToInsert = indent;
		line.insert(line.begin(), charsToInsert, '\t');
	}
	else // spaces
	{
		charsToInsert = indent * indentLength;
		line.insert(line.begin(), charsToInsert, ' ');
	}

	return charsToInsert;
}

/**
 * check for SQL "BEGIN DECLARE SECTION".
 * must compare case insensitive and allow any spacing between words.
 *
 * @param line          a reference to the line to indent.
 * @param index         the current line index.
 * @return              true if a hit.
 */
bool ASEnhancer::isBeginDeclareSectionSQL(const string& line, size_t index) const
{
	string word;
	size_t hits = 0;
	size_t i;
	for (i = index; i < line.length(); i++)
	{
		i = line.find_first_not_of(" \t", i);
		if (i == string::npos)
			return false;
		if (line[i] == ';')
			break;
		if (!isCharPotentialHeader(line, i))
			continue;
		word = getCurrentWord(line, i);
		for (size_t j = 0; j < word.length(); j++)
			word[j] = (char) toupper(word[j]);
		if (word == "EXEC" || word == "SQL")
		{
			i += word.length() - 1;
			continue;
		}
		if (word == "DECLARE" || word == "SECTION")
		{
			hits++;
			i += word.length() - 1;
			continue;
		}
		if (word == "BEGIN")
		{
			hits++;
			i += word.length() - 1;
			continue;
		}
		return false;
	}
	if (hits == 3)
		return true;
	return false;
}

/**
 * check for SQL "END DECLARE SECTION".
 * must compare case insensitive and allow any spacing between words.
 *
 * @param line          a reference to the line to indent.
 * @param index         the current line index.
 * @return              true if a hit.
 */
bool ASEnhancer::isEndDeclareSectionSQL(const string& line, size_t index) const
{
	string word;
	size_t hits = 0;
	size_t i;
	for (i = index; i < line.length(); i++)
	{
		i = line.find_first_not_of(" \t", i);
		if (i == string::npos)
			return false;
		if (line[i] == ';')
			break;
		if (!isCharPotentialHeader(line, i))
			continue;
		word = getCurrentWord(line, i);
		for (size_t j = 0; j < word.length(); j++)
			word[j] = (char) toupper(word[j]);
		if (word == "EXEC" || word == "SQL")
		{
			i += word.length() - 1;
			continue;
		}
		if (word == "DECLARE" || word == "SECTION")
		{
			hits++;
			i += word.length() - 1;
			continue;
		}
		if (word == "END")
		{
			hits++;
			i += word.length() - 1;
			continue;
		}
		return false;
	}
	if (hits == 3)
		return true;
	return false;
}

/**
 * check if a one-line brace has been reached,
 * i.e. if the currently reached '{' character is closed
 * with a complimentary '}' elsewhere on the current line,
 *.
 * @return     false = one-line brace has not been reached.
 *             true  = one-line brace has been reached.
 */
bool ASEnhancer::isOneLineBlockReached(const string& line, int startChar) const
{
	assert(line[startChar] == '{');

	bool isInComment_ = false;
	bool isInQuote_ = false;
	int _braceCount = 1;
	int lineLength = line.length();
	char quoteChar_ = ' ';
	char ch = ' ';

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
			++_braceCount;
		else if (ch == '}')
			--_braceCount;

		if (_braceCount == 0)
			return true;
	}

	return false;
}

/**
 * parse characters in the current line to determine if an indent
 * or unindent is needed.
 */
void ASEnhancer::parseCurrentLine(string& line, bool isInPreprocessor, bool isInSQL)
{
	bool isSpecialChar = false;			// is a backslash escape character

	for (size_t i = 0; i < line.length(); i++)
	{
		char ch = line[i];

		// bypass whitespace
		if (isWhiteSpace(ch))
			continue;

		// handle special characters (i.e. backslash+character such as \n, \t, ...)
		if (isSpecialChar)
		{
			isSpecialChar = false;
			continue;
		}
		if (!(isInComment) && line.compare(i, 2, "\\\\") == 0)
		{
			i++;
			continue;
		}
		if (!(isInComment) && ch == '\\')
		{
			isSpecialChar = true;
			continue;
		}

		// handle quotes (such as 'x' and "Hello Dolly")
		if (!isInComment
		        && (ch == '"'
		            || (ch == '\'' && !isDigitSeparator(line, i))))
		{
			if (!isInQuote)
			{
				quoteChar = ch;
				isInQuote = true;
			}
			else if (quoteChar == ch)
			{
				isInQuote = false;
				continue;
			}
		}

		if (isInQuote)
			continue;

		// handle comments

		if (!(isInComment) && line.compare(i, 2, "//") == 0)
		{
			// check for windows line markers
			if (line.compare(i + 2, 1, "\xf0") > 0)
				lineNumber--;
			// unindent if not in case braces
			if (line.find_first_not_of(" \t") == i
			        && sw.switchBraceCount == 1
			        && sw.unindentCase)
				shouldUnindentComment = true;
			break;                 // finished with the line
		}
		else if (!(isInComment) && line.compare(i, 2, "/*") == 0)
		{
			// unindent if not in case braces
			if (sw.switchBraceCount == 1 && sw.unindentCase)
				shouldUnindentComment = true;
			isInComment = true;
			size_t commentEnd = line.find("*/", i);
			if (commentEnd == string::npos)
				i = line.length() - 1;
			else
				i = commentEnd - 1;
			continue;
		}
		else if ((isInComment) && line.compare(i, 2, "*/") == 0)
		{
			// unindent if not in case braces
			if (sw.switchBraceCount == 1 && sw.unindentCase)
				shouldUnindentComment = true;
			isInComment = false;
			i++;
			continue;
		}

		if (isInComment)
		{
			// unindent if not in case braces
			if (sw.switchBraceCount == 1 && sw.unindentCase)
				shouldUnindentComment = true;
			size_t commentEnd = line.find("*/", i);
			if (commentEnd == string::npos)
				i = line.length() - 1;
			else
				i = commentEnd - 1;
			continue;
		}

		// if we have reached this far then we are NOT in a comment or string of special characters

		if (line[i] == '{')
			braceCount++;

		if (line[i] == '}')
			braceCount--;

		// check for preprocessor within an event table
		if (isInEventTable && line[i] == '#' && preprocBlockIndent)
		{
			string preproc;
			preproc = line.substr(i + 1);
			if (preproc.substr(0, 2) == "if") // #if, #ifdef, #ifndef)
				eventPreprocDepth += 1;
			if (preproc.substr(0, 5) == "endif" && eventPreprocDepth > 0)
				eventPreprocDepth -= 1;
		}

		bool isPotentialKeyword = isCharPotentialHeader(line, i);

		// ----------------  wxWidgets and MFC macros  ----------------------------------

		if (isPotentialKeyword)
		{
			for (size_t j = 0; j < indentableMacros->size(); j++)
			{
				// 'first' is the beginning macro
				if (findKeyword(line, i, indentableMacros->at(j)->first))
				{
					nextLineIsEventIndent = true;
					break;
				}
			}
			for (size_t j = 0; j < indentableMacros->size(); j++)
			{
				// 'second' is the ending macro
				if (findKeyword(line, i, indentableMacros->at(j)->second))
				{
					isInEventTable = false;
					eventPreprocDepth = 0;
					break;
				}
			}
		}

		// ----------------  process SQL  -----------------------------------------------

		if (isInSQL)
		{
			if (isBeginDeclareSectionSQL(line, i))
				nextLineIsDeclareIndent = true;
			if (isEndDeclareSectionSQL(line, i))
				isInDeclareSection = false;
			break;
		}

		// ----------------  process switch statements  ---------------------------------

		if (isPotentialKeyword && findKeyword(line, i, ASResource::AS_SWITCH))
		{
			switchDepth++;
			switchStack.emplace_back(sw);                      // save current variables
			sw.switchBraceCount = 0;
			sw.unindentCase = false;                        // don't clear case until end of switch
			i += 5;                                         // bypass switch statement
			continue;
		}

		// just want unindented case statements from this point

		if (caseIndent
		        || switchDepth == 0
		        || (isInPreprocessor && !preprocDefineIndent))
		{
			// bypass the entire word
			if (isPotentialKeyword)
			{
				string name = getCurrentWord(line, i);
				i += name.length() - 1;
			}
			continue;
		}

		i = processSwitchBlock(line, i);

	}   // end of for loop * end of for loop * end of for loop * end of for loop
}

/**
 * process the character at the current index in a switch block.
 *
 * @param line          a reference to the line to indent.
 * @param index         the current line index.
 * @return              the new line index.
 */
size_t ASEnhancer::processSwitchBlock(string& line, size_t index)
{
	size_t i = index;
	bool isPotentialKeyword = isCharPotentialHeader(line, i);

	if (line[i] == '{')
	{
		sw.switchBraceCount++;
		if (lookingForCaseBrace)                      // if 1st after case statement
		{
			sw.unindentCase = true;                     // unindenting this case
			sw.unindentDepth++;
			lookingForCaseBrace = false;              // not looking now
		}
		return i;
	}
	lookingForCaseBrace = false;                      // no opening brace, don't indent

	if (line[i] == '}')
	{
		sw.switchBraceCount--;
		assert(sw.switchBraceCount <= braceCount);
		if (sw.switchBraceCount == 0)                 // if end of switch statement
		{
			int lineUnindent = sw.unindentDepth;
			if (line.find_first_not_of(" \t") == i
			        && !switchStack.empty())
				lineUnindent = switchStack[switchStack.size() - 1].unindentDepth;
			if (shouldUnindentLine)
			{
				if (lineUnindent > 0)
					i -= unindentLine(line, lineUnindent);
				shouldUnindentLine = false;
			}
			switchDepth--;
			sw = switchStack.back();
			switchStack.pop_back();
		}
		return i;
	}

	if (isPotentialKeyword
	        && (findKeyword(line, i, ASResource::AS_CASE)
	            || findKeyword(line, i, ASResource::AS_DEFAULT)))
	{
		if (sw.unindentCase)					// if unindented last case
		{
			sw.unindentCase = false;			// stop unindenting previous case
			sw.unindentDepth--;
		}

		i = findCaseColon(line, i);

		i++;
		for (; i < line.length(); i++)			// bypass whitespace
		{
			if (!isWhiteSpace(line[i]))
				break;
		}
		if (i < line.length())
		{
			if (line[i] == '{')
			{
				braceCount++;
				sw.switchBraceCount++;
				if (!isOneLineBlockReached(line, i))
					unindentNextLine = true;
				return i;
			}
		}
		lookingForCaseBrace = true;
		i--;									// need to process this char
		return i;
	}
	if (isPotentialKeyword)
	{
		string name = getCurrentWord(line, i);          // bypass the entire name
		i += name.length() - 1;
	}
	return i;
}

/**
 * unindent a line by a given number of tabsets
 *    by erasing the leading whitespace from the line argument.
 *
 * @param line          a reference to the line to unindent.
 * @param unindent      the number of tabsets to erase.
 * @return              the number of characters erased.
 */
int ASEnhancer::unindentLine(string& line, int unindent) const
{
	size_t whitespace = line.find_first_not_of(" \t");

	if (whitespace == string::npos)         // if line is blank
		whitespace = line.length();         // must remove padding, if any

	if (whitespace == 0)
		return 0;

	size_t charsToErase = 0;

	if (forceTab && indentLength != tabLength)
	{
		// replace tab indents with spaces
		convertForceTabIndentToSpaces(line);
		// remove the space indents
		size_t spaceIndentLength = line.find_first_not_of(" \t");
		charsToErase = unindent * indentLength;
		if (charsToErase <= spaceIndentLength)
			line.erase(0, charsToErase);
		else
			charsToErase = 0;
		// replace leading spaces with tab indents
		convertSpaceIndentToForceTab(line);
	}
	else if (useTabs)
	{
		charsToErase = unindent;
		if (charsToErase <= whitespace)
			line.erase(0, charsToErase);
		else
			charsToErase = 0;
	}
	else // spaces
	{
		charsToErase = unindent * indentLength;
		if (charsToErase <= whitespace)
			line.erase(0, charsToErase);
		else
			charsToErase = 0;
	}

	return charsToErase;
}

}   // end namespace astyle
