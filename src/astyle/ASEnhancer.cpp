/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   ASEnhancer.cpp
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

// can trace only if NDEBUG is not defined
#ifndef NDEBUG
// #define TRACEswitch
// #define TRACEcase
// #define TRACEmisc
#endif

#include "astyle.h"

#include <iostream>
#include <fstream>
#include <sstream>

#ifdef TRACEswitch
#define TRswitch(a,b)   *traceOut << lineNumber << a << b << endl;
#else
#define TRswitch(a,b)   ((void)0)
#endif // TRACEswitch
#ifdef TRACEcase
#define TRcase(a,b)     *traceOut << lineNumber << a << b << endl;
#else
#define TRcase(a,b)     ((void)0)
#endif // TRACEcase
#ifdef TRACEmisc
#define TRmisc(a)       *traceOut << lineNumber << a << endl;
#else
#define TRmisc(a)       ((void)0)
#endif // TRACEmisc


namespace astyle
{

// ---------------------------- functions for ASEnhancer Class -------------------------------------

/**
 * ASEnhancer constructor
 */
ASEnhancer::ASEnhancer()
{
	// variables are initialized by init()
	traceOut = new stringstream;
}

/**
 * Destructor of ASEnhancer
 * Display the TRACE entries.
 */
ASEnhancer::~ASEnhancer()
{
#if defined(TRACEswitch) || defined(TRACEcase) || defined(TRACEmisc)
	string line;
	string msg = "TRACE Entries\n\n";
	char countLine[50];
	int count = 0;

	while (getline(*traceOut, line))
	{
		msg += line + '\n';
		count++;
	}
	sprintf(countLine, "\n%d Entries", count);
	msg += countLine;
	// write a text file to "My Documents" (Windows)
	char filename [_MAX_PATH + _MAX_FNAME + _MAX_EXT + 1];   // full path and filename
	strcpy(filename, getenv("USERPROFILE"));
	strcat(filename, "\\My Documents\\tracee.txt");
	ofstream outfile(filename);
	outfile << msg;
	outfile.close();
#endif
	delete traceOut;
}

/**
 * initialize the ASEnhancer.
 *
 * init() is called each time an ASFormatter object is initialized.
 */
void ASEnhancer::init(int _indentLength,
                      string _indentString,
                      bool _isCStyle,
                      bool _isJavaStyle,
                      bool _isSharpStyle,
                      bool _caseIndent,
                      bool _emptyLineFill)
{
	// formatting variables from ASFormatter and ASBeautifier
	indentLengthX = _indentLength;
	if (_indentString.compare(0, 1, "\t") == 0)
		useTabsX = true;
	else
		useTabsX = false;
	isCStyleX      = _isCStyle;
	isJavaStyleX   = _isJavaStyle;
	isSharpStyleX  = _isSharpStyle;
	caseIndentX    = _caseIndent;
	emptyLineFillX = _emptyLineFill;

	// unindent variables
	lineNumber = 0;
	bracketCount = 0;
	isInComment = false;
	isInQuote = false;
	switchDepth = 0;
	lookingForCaseBracket = false;
	unindentNextLine = false;

	// switch struct and vector
	sw.switchBracketCount = 0;
	sw.unindentDepth = 0;
	sw.unindentCase = false;
	swVector.clear();

	nextLineIsEventTable = false;
	isInEventTable = false;

#if defined(TRACEswitch) || defined(TRACEcase) || defined(TRACEmisc)
	*traceOut << "New file -------------" << endl;
#endif
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
void ASEnhancer::enhance(string &line)
{
	bool   isSpecialChar = false;
	size_t  lineLength;                             // length of the line being parsed

	lineNumber++;
	lineLength = line.length();

	// check for beginning of event table
	if (nextLineIsEventTable)
	{
		isInEventTable = true;
		nextLineIsEventTable = false;
	}

	if (lineLength == 0
	        && ! isInEventTable
	        && ! emptyLineFillX)
		return;

	// test for unindent on attached brackets
	if (unindentNextLine)
	{
		sw.unindentDepth++;
		sw.unindentCase = true;
		unindentNextLine = false;
		TRcase(" unindent case ", sw.unindentDepth);
	}

	// parse characters in the current line.

	for (size_t i = 0; i < lineLength; i++)
	{
		char ch = line[i];

		// bypass whitespace
		if (isWhiteSpaceX(ch))
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
		if (!(isInComment) && (ch == '"' || ch == '\''))
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
			break;                 // finished with the line
		}
		else if (!(isInComment) && line.compare(i, 2, "/*") == 0)
		{
			isInComment = true;
			i++;
			continue;
		}
		else if ((isInComment) && line.compare(i, 2, "*/") == 0)
		{
			isInComment = false;
			i++;
			continue;
		}

		if (isInComment)
			continue;

		// if we have reached this far then we are NOT in a comment or string of special characters

		if (line[i] == '{')                                 // if open bracket
			bracketCount++;

		if (line[i] == '}')                     // if close bracket
			bracketCount--;

		// ----------------  process event tables  --------------------------------------

		// check for event table begin
		if (findKeyword(line, i, "BEGIN_EVENT_TABLE")
		        || findKeyword(line, i, "BEGIN_MESSAGE_MAP"))
			nextLineIsEventTable = true;

		// check for event table end
		if (findKeyword(line, i, "END_EVENT_TABLE")
		        || findKeyword(line, i, "END_MESSAGE_MAP"))
			isInEventTable = false;

		// ----------------  process switch statements  ---------------------------------

		if (findKeyword(line, i, "switch"))                 // if switch statement
		{
			switchDepth++;                                  // bump switch depth
			TRswitch(" switch ", switchDepth);
			swVector.push_back(sw);                         // save current variables
			sw.switchBracketCount = 0;
			sw.unindentCase = false;                        // don't clear case until end of switch
			i += 5;                                         // bypass switch statement
			continue;
		}

		// just want switch statements from this point

		if (caseIndentX || switchDepth == 0)               // from here just want switch statements
			continue;                                      // get next char

		if (line[i] == '{')                                 // if open bracket
		{
			sw.switchBracketCount++;
			if (lookingForCaseBracket)                      // if 1st after case statement
			{
				sw.unindentCase = true;                     // unindenting this case
				sw.unindentDepth++;                         // bump depth
				lookingForCaseBracket = false;              // not looking now
				TRcase(" unindent case ", sw.unindentDepth);
			}
			continue;
		}

		lookingForCaseBracket = false;                      // no opening bracket, don't indent

		if (line[i] == '}')                                 // if close bracket
		{
			sw.switchBracketCount--;
			if (sw.switchBracketCount == 0)                 // if end of switch statement
			{
				TRswitch("  endsw ", switchDepth);
				switchDepth--;                              // one less switch
				sw = swVector.back();                       // restore sw struct
				swVector.pop_back();                        // remove last entry from stack
			}
			continue;
		}

		// look for case or default header

		if (findKeyword(line, i, "case") || findKeyword(line, i, "default"))
		{
			if (sw.unindentCase)                            // if unindented last case
			{
				sw.unindentCase = false;                    // stop unindenting previous case
				sw.unindentDepth--;                         // reduce depth
			}
			bool isInQuote = false;
			char quoteChar = ' ';
			for (; i < lineLength; i++)                     // find colon
			{
				if (isInQuote)
				{
					if (line[i] == '\\')
					{
						i++;								// bypass next char
						continue;
					}
					else if (line[i] == quoteChar)			// check ending quote
					{
						isInQuote = false;
						quoteChar = ' ';
						continue;
					}
					else
					{
						continue;							// must close quote before continuing
					}
				}
				if (line[i] == '\'' || line[i] == '\"')	// check opening quote
				{
					isInQuote = true;
					quoteChar = line[i];
					continue;
				}
				if (line[i] == ':')
				{
					if ((i + 1 < lineLength) && (line[i + 1] == ':'))
						i++;								// bypass scope resolution operator
					else
						break;								// found it
				}
			}
			i++;
			for (; i < lineLength; i++)                     // bypass whitespace
			{
				if (!(isWhiteSpaceX(line[i])))
					break;
			}
			if (i < lineLength)                             // check for bracket
			{
				if (line[i] == '{')                         // if bracket found
				{
					sw.switchBracketCount++;
					unindentNextLine = true;                // start unindenting on next line
					continue;
				}
			}
			lookingForCaseBracket = true;                   // bracket must be on next line
			i--;                                            // need to check for comments
			continue;
		}
	}   // end of for loop

	if (isInEventTable) 									// if need to indent
		indentLine(line, 1);               					//    do it

	if (sw.unindentDepth > 0)                               // if need to unindent
		unindentLine(line, sw.unindentDepth);               //    do it
}

/**
 * indent a line by a given number of tabsets
 *    by inserting leading whitespace to the line argument.
 *
 * @param line          a pointer to the line to indent.
 * @param unindent      the number of tabsets to insert.
 * @return              the number of characters inserted.
 */
int ASEnhancer::indentLine(string  &line, const int indent) const
{
	if (line.length() == 0
	        && ! emptyLineFillX)
		return 0;

	size_t charsToInsert;                   	// number of chars to insert

	if (useTabsX)                    			// if formatted with tabs
	{
		charsToInsert = indent;             	// tabs to insert
		line.insert((size_t) 0, charsToInsert, '\t');    // insert the tabs
	}
	else
	{
		charsToInsert = indent * indentLengthX;  // compute chars to insert
		line.insert((size_t)0, charsToInsert, ' ');     // insert the spaces
	}

	return charsToInsert;
}

/**
 * unindent a line by a given number of tabsets
 *    by erasing the leading whitespace from the line argument.
 *
 * @param line          a pointer to the line to unindent.
 * @param unindent      the number of tabsets to erase.
 * @return              the number of characters erased.
 */
int ASEnhancer::unindentLine(string  &line, const int unindent) const
{
	size_t whitespace = line.find_first_not_of(" \t");

	if (whitespace == string::npos)         // if line is blank
		whitespace = line.length();         // must remove padding, if any

	if (whitespace == 0)
		return 0;

	size_t charsToErase;                    // number of chars to erase

	if (useTabsX)                    		// if formatted with tabs
	{
		charsToErase = unindent;            // tabs to erase
		if (charsToErase <= whitespace)     // if there is enough whitespace
			line.erase(0, charsToErase);    // erase the tabs
		else
			charsToErase = 0;
	}
	else
	{
		charsToErase = unindent * indentLengthX; // compute chars to erase
		if (charsToErase <= whitespace)         // if there is enough whitespace
			line.erase(0, charsToErase);        // erase the spaces
		else
			charsToErase = 0;
	}

	return charsToErase;
}

/**
 * check if a specific line position contains a keyword.
 *
 * @return    true if the word was found. false if the word was not found.
 */
bool ASEnhancer::findKeyword(const string &line, int i, const char *keyword) const
{
	if (line.compare(i, strlen(keyword), keyword) == 0)
	{
		// check that this is a header and not a part of a longer word
		// (e.g. not at its begining, not at its middle...)

		int lineLength = line.length();
		int wordEnd = i + strlen(keyword);
		char startCh = keyword[0];      // first char of header
		char endCh = 0;                // char just after header
		char prevCh = 0;               // char just before header

		if (wordEnd < lineLength)
		{
			endCh = line[wordEnd];
		}
		if (i > 0)
		{
			prevCh = line[i-1];
		}

		if (prevCh != 0
		        && isLegalNameCharX(startCh)
		        && isLegalNameCharX(prevCh))
		{
			return false;
		}
		else if (wordEnd >= lineLength
		         || !isLegalNameCharX(startCh)
		         || !isLegalNameCharX(endCh))
		{
			// is not a keyword if part of a definition
			char peekChar = peekNextChar(line, wordEnd - 1);
			if (peekChar == ',' || peekChar == ')')
				return false;
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}

/**
* peek at the next unread character.
*
* @return     	the next unread character.
* @param line   the line to check.
* @param i      the current char position on the line.
*/
char ASEnhancer::peekNextChar(const string &line, int i) const
{
	char ch = ' ';
	size_t peekNum = line.find_first_not_of(" \t", i + 1);

	if (peekNum == string::npos)
		return ch;

	ch = line[peekNum];

	return ch;
}


}   // end namespace astyle
