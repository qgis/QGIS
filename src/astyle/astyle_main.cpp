/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   astyle_main.cpp
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
/*
    AStyle_main source file map.
    This source file contains several classes.
    They are arranged as follows.
    ---------------------------------------
    namespace astyle {
    ASStreamIterator methods
    ASConsole methods
        // Windows specific
        // Linux specific
    ASLibrary methods
        // Windows specific
        // Linux specific
    ASOptions methods
    Utf8_16 methods
    }   // end of astyle namespace
    Global Area ---------------------------
        Java Native Interface functions
        AStyleMainUtf16 entry point
        AStyleMain entry point
        AStyleGetVersion entry point
        main entry point
    ---------------------------------------
*/

#include "astyle_main.h"

#include <algorithm>
#include <cstdlib>
#include <errno.h>
#include <fstream>
#include <sstream>

// includes for recursive getFileNames() function
#ifdef _WIN32
	#undef UNICODE		// use ASCII windows functions
	#include <windows.h>
#else
	#include <dirent.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#ifdef __VMS
		#include <unixlib.h>
		#include <rms.h>
		#include <ssdef.h>
		#include <stsdef.h>
		#include <lib$routines.h>
		#include <starlet.h>
	#endif /* __VMS */
#endif

#ifdef __DMC__
	#include <locale.h>
#endif

// turn off MinGW automatic file globbing
// this CANNOT be in the astyle namespace
#ifndef ASTYLE_LIB
	int _CRT_glob = 0;
#endif

//----------------------------------------------------------------------------
// astyle namespace
//----------------------------------------------------------------------------

namespace astyle {

// console build variables
#ifndef ASTYLE_LIB
	ASConsole* g_console = NULL;     // class to encapsulate console variables
	ostream* _err = &cerr;           // direct error messages to cerr
	#ifdef _WIN32
		char g_fileSeparator = '\\';     // Windows file separator
		bool g_isCaseSensitive = false;  // Windows IS case sensitive
	#else
		char g_fileSeparator = '/';      // Linux file separator
		bool g_isCaseSensitive = true;   // Linux IS NOT case sensitive
	#endif	// _WIN32
#endif	// ASTYLE_LIB

// java library build variables
#ifdef ASTYLE_JNI
	JNIEnv*   g_env;
	jobject   g_obj;
	jmethodID g_mid;
#endif

const char* g_version = "2.05.1";

//-----------------------------------------------------------------------------
// ASStreamIterator class
// typename will be istringstream for GUI and istream otherwise
//-----------------------------------------------------------------------------

template<typename T>
ASStreamIterator<T>::ASStreamIterator(T* in)
{
	inStream = in;
	buffer.reserve(200);
	eolWindows = 0;
	eolLinux = 0;
	eolMacOld = 0;
	outputEOL[0] = '\0';
	peekStart = 0;
	prevLineDeleted = false;
	checkForEmptyLine = false;
	// get length of stream
	inStream->seekg(0, inStream->end);
	streamLength = inStream->tellg();
	inStream->seekg(0, inStream->beg);
}

template<typename T>
ASStreamIterator<T>::~ASStreamIterator()
{
}

/**
* get the length of the input stream.
* streamLength variable is set by the constructor.
*
* @return     length of the input file stream, converted to an int.
*/
template<typename T>
int ASStreamIterator<T>::getStreamLength() const
{
	return static_cast<int>(streamLength);
}

/**
 * read the input stream, delete any end of line characters,
 *     and build a string that contains the input line.
 *
 * @return        string containing the next input line minus any end of line characters
 */
template<typename T>
string ASStreamIterator<T>::nextLine(bool emptyLineWasDeleted)
{
	// verify that the current position is correct
	assert(peekStart == 0);

	// a deleted line may be replaced if break-blocks is requested
	// this sets up the compare to check for a replaced empty line
	if (prevLineDeleted)
	{
		prevLineDeleted = false;
		checkForEmptyLine = true;
	}
	if (!emptyLineWasDeleted)
		prevBuffer = buffer;
	else
		prevLineDeleted = true;

	// read the next record
	buffer.clear();
	char ch;
	inStream->get(ch);

	while (!inStream->eof() && ch != '\n' && ch != '\r')
	{
		buffer.append(1, ch);
		inStream->get(ch);
	}

	if (inStream->eof())
	{
		return buffer;
	}

	int peekCh = inStream->peek();

	// find input end-of-line characters
	if (!inStream->eof())
	{
		if (ch == '\r')         // CR+LF is windows otherwise Mac OS 9
		{
			if (peekCh == '\n')
			{
				inStream->get();
				eolWindows++;
			}
			else
				eolMacOld++;
		}
		else                    // LF is Linux, allow for improbable LF/CR
		{
			if (peekCh == '\r')
			{
				inStream->get();
				eolWindows++;
			}
			else
				eolLinux++;
		}
	}
	else
	{
		inStream->clear();
	}

	// set output end of line characters
	if (eolWindows >= eolLinux)
	{
		if (eolWindows >= eolMacOld)
			strcpy(outputEOL, "\r\n");  // Windows (CR+LF)
		else
			strcpy(outputEOL, "\r");    // MacOld (CR)
	}
	else if (eolLinux >= eolMacOld)
		strcpy(outputEOL, "\n");		// Linux (LF)
	else
		strcpy(outputEOL, "\r");		// MacOld (CR)

	return buffer;
}

// save the current position and get the next line
// this can be called for multiple reads
// when finished peeking you MUST call peekReset()
// call this function from ASFormatter ONLY
template<typename T>
string ASStreamIterator<T>::peekNextLine()
{
	assert(hasMoreLines());
	string nextLine_;
	char ch;

	if (peekStart == 0)
		peekStart = inStream->tellg();

	// read the next record
	inStream->get(ch);
	while (!inStream->eof() && ch != '\n' && ch != '\r')
	{
		nextLine_.append(1, ch);
		inStream->get(ch);
	}

	if (inStream->eof())
	{
		return nextLine_;
	}

	int peekCh = inStream->peek();

	// remove end-of-line characters
	if (!inStream->eof())
	{
		if ((peekCh == '\n' || peekCh == '\r') && peekCh != ch)
			inStream->get();
	}

	return nextLine_;
}

// reset current position and EOF for peekNextLine()
template<typename T>
void ASStreamIterator<T>::peekReset()
{
	assert(peekStart != 0);
	inStream->clear();
	inStream->seekg(peekStart);
	peekStart = 0;
}

// save the last input line after input has reached EOF
template<typename T>
void ASStreamIterator<T>::saveLastInputLine()
{
	assert(inStream->eof());
	prevBuffer = buffer;
}

// return position of the get pointer
template<typename T>
streamoff ASStreamIterator<T>::tellg()
{
	return inStream->tellg();
}

// check for a change in line ends
template<typename T>
bool ASStreamIterator<T>::getLineEndChange(int lineEndFormat) const
{
	assert(lineEndFormat == LINEEND_DEFAULT
	       || lineEndFormat == LINEEND_WINDOWS
	       || lineEndFormat == LINEEND_LINUX
	       || lineEndFormat == LINEEND_MACOLD);

	bool lineEndChange = false;
	if (lineEndFormat == LINEEND_WINDOWS)
		lineEndChange = (eolLinux + eolMacOld != 0);
	else if (lineEndFormat == LINEEND_LINUX)
		lineEndChange = (eolWindows + eolMacOld != 0);
	else if (lineEndFormat == LINEEND_MACOLD)
		lineEndChange = (eolWindows + eolLinux != 0);
	else
	{
		if (eolWindows > 0)
			lineEndChange = (eolLinux + eolMacOld != 0);
		else if (eolLinux > 0)
			lineEndChange = (eolWindows + eolMacOld != 0);
		else if (eolMacOld > 0)
			lineEndChange = (eolWindows + eolLinux != 0);
	}
	return lineEndChange;
}

//-----------------------------------------------------------------------------
// ASConsole class
// main function will be included only in the console build
//-----------------------------------------------------------------------------

#ifndef ASTYLE_LIB

// rewrite a stringstream converting the line ends
void ASConsole::convertLineEnds(ostringstream &out, int lineEnd)
{
	assert(lineEnd == LINEEND_WINDOWS || lineEnd == LINEEND_LINUX || lineEnd == LINEEND_MACOLD);
	const string &inStr = out.str();	// avoids strange looking syntax
	string outStr;						// the converted output
	int inLength = inStr.length();
	for (int pos = 0; pos < inLength; pos++)
	{
		if (inStr[pos] == '\r')
		{
			if (inStr[pos + 1] == '\n')
			{
				// CRLF
				if (lineEnd == LINEEND_CR)
				{
					outStr += inStr[pos];		// Delete the LF
					pos++;
					continue;
				}
				else if (lineEnd == LINEEND_LF)
				{
					outStr += inStr[pos + 1];		// Delete the CR
					pos++;
					continue;
				}
				else
				{
					outStr += inStr[pos];		// Do not change
					outStr += inStr[pos + 1];
					pos++;
					continue;
				}
			}
			else
			{
				// CR
				if (lineEnd == LINEEND_CRLF)
				{
					outStr += inStr[pos];		// Insert the CR
					outStr += '\n';				// Insert the LF
					continue;
				}
				else if (lineEnd == LINEEND_LF)
				{
					outStr += '\n';				// Insert the LF
					continue;
				}
				else
				{
					outStr += inStr[pos];		// Do not change
					continue;
				}
			}
		}
		else if (inStr[pos] == '\n')
		{
			// LF
			if (lineEnd == LINEEND_CRLF)
			{
				outStr += '\r';				// Insert the CR
				outStr += inStr[pos];		// Insert the LF
				continue;
			}
			else if (lineEnd == LINEEND_CR)
			{
				outStr += '\r';				// Insert the CR
				continue;
			}
			else
			{
				outStr += inStr[pos];		// Do not change
				continue;
			}
		}
		else
		{
			outStr += inStr[pos];		// Write the current char
		}
	}
	// replace the stream
	out.str(outStr);
}

void ASConsole::correctMixedLineEnds(ostringstream &out)
{
	LineEndFormat lineEndFormat = LINEEND_DEFAULT;
	if (strcmp(outputEOL, "\r\n") == 0)
		lineEndFormat = LINEEND_WINDOWS;
	if (strcmp(outputEOL, "\n") == 0)
		lineEndFormat = LINEEND_LINUX;
	if (strcmp(outputEOL, "\r") == 0)
		lineEndFormat = LINEEND_MACOLD;
	convertLineEnds(out, lineEndFormat);
}

// check files for 16 or 32 bit encoding
// the file must have a Byte Order Mark (BOM)
// NOTE: some string functions don't work with NULLs (e.g. length())
FileEncoding ASConsole::detectEncoding(const char* data, size_t dataSize) const
{
	FileEncoding encoding = ENCODING_8BIT;

	if (dataSize >= 4 && memcmp(data, "\x00\x00\xFE\xFF", 4) == 0)
		encoding = UTF_32BE;
	else if (dataSize >= 4 && memcmp(data, "\xFF\xFE\x00\x00", 4) == 0)
		encoding = UTF_32LE;
	else if (dataSize >= 2 && memcmp(data, "\xFE\xFF", 2) == 0)
		encoding = UTF_16BE;
	else if (dataSize >= 2 && memcmp(data, "\xFF\xFE", 2) == 0)
		encoding = UTF_16LE;

	return encoding;
}

// error exit without a message
void ASConsole::error() const
{
	(*_err) << _("\nArtistic Style has terminated") << endl;
	exit(EXIT_FAILURE);
}

// error exit with a message
void ASConsole::error(const char* why, const char* what) const
{
	(*_err) << why << ' ' << what << endl;
	error();
}

/**
 * If no files have been given, use cin for input and cout for output.
 *
 * This is used to format text for text editors like TextWrangler (Mac).
 * Do NOT display any console messages when this function is used.
 */
void ASConsole::formatCinToCout()
{
	// Using cin.tellg() causes problems with both Windows and Linux.
	// The Windows problem occurs when the input is not Windows line-ends.
	// The tellg() will be out of sequence with the get() statements.
	// The Linux cin.tellg() will return -1 (invalid).
	// Copying the input sequentially to a stringstream before
	// formatting solves the problem for both.
	istream* inStream = &cin;
	stringstream outStream;
	char ch;
	inStream->get(ch);
	while (!inStream->eof())
	{
		outStream.put(ch);
		inStream->get(ch);
	}
	ASStreamIterator<stringstream> streamIterator(&outStream);
	// Windows pipe or redirection always outputs Windows line-ends.
	// Linux pipe or redirection will output any line end.
	LineEndFormat lineEndFormat = formatter.getLineEndFormat();
	initializeOutputEOL(lineEndFormat);
	formatter.init(&streamIterator);

	while (formatter.hasMoreLines())
	{
		cout << formatter.nextLine();
		if (formatter.hasMoreLines())
		{
			setOutputEOL(lineEndFormat, streamIterator.getOutputEOL());
			cout << outputEOL;
		}
		else
		{
			// this can happen if the file if missing a closing bracket and break-blocks is requested
			if (formatter.getIsLineReady())
			{
				setOutputEOL(lineEndFormat, streamIterator.getOutputEOL());
				cout << outputEOL;
				cout << formatter.nextLine();
			}
		}
	}
	cout.flush();
}

/**
 * Open input file, format it, and close the output.
 *
 * @param fileName_     The path and name of the file to be processed.
 */
void ASConsole::formatFile(const string &fileName_)
{
	stringstream in;
	ostringstream out;
	FileEncoding encoding = readFile(fileName_, in);

	// Unless a specific language mode has been set, set the language mode
	// according to the file's suffix.
	if (!formatter.getModeManuallySet())
	{
		if (stringEndsWith(fileName_, string(".java")))
			formatter.setJavaStyle();
		else if (stringEndsWith(fileName_, string(".cs")))
			formatter.setSharpStyle();
		else
			formatter.setCStyle();
	}

	// set line end format
	string nextLine;				// next output line
	filesAreIdentical = true;		// input and output files are identical
	LineEndFormat lineEndFormat = formatter.getLineEndFormat();
	initializeOutputEOL(lineEndFormat);
	// do this AFTER setting the file mode
	ASStreamIterator<stringstream> streamIterator(&in);
	formatter.init(&streamIterator);

	// format the file
	while (formatter.hasMoreLines())
	{
		nextLine = formatter.nextLine();
		out << nextLine;
		linesOut++;
		if (formatter.hasMoreLines())
		{
			setOutputEOL(lineEndFormat, streamIterator.getOutputEOL());
			out << outputEOL;
		}
		else
		{
			streamIterator.saveLastInputLine();     // to compare the last input line
			// this can happen if the file if missing a closing bracket and break-blocks is requested
			if (formatter.getIsLineReady())
			{
				setOutputEOL(lineEndFormat, streamIterator.getOutputEOL());
				out << outputEOL;
				nextLine = formatter.nextLine();
				out << nextLine;
				linesOut++;
				streamIterator.saveLastInputLine();
			}
		}

		if (filesAreIdentical)
		{
			if (streamIterator.checkForEmptyLine)
			{
				if (nextLine.find_first_not_of(" \t") != string::npos)
					filesAreIdentical = false;
			}
			else if (!streamIterator.compareToInputBuffer(nextLine))
				filesAreIdentical = false;
			streamIterator.checkForEmptyLine = false;
		}
	}
	// correct for mixed line ends
	if (lineEndsMixed)
	{
		correctMixedLineEnds(out);
		filesAreIdentical = false;
	}

	// remove targetDirectory from filename if required by print
	string displayName;
	if (hasWildcard)
		displayName = fileName_.substr(targetDirectory.length() + 1);
	else
		displayName = fileName_;

	// if file has changed, write the new file
	if (!filesAreIdentical || streamIterator.getLineEndChange(lineEndFormat))
	{
		if (!isDryRun)
			writeFile(fileName_, encoding, out);
		printMsg(_("Formatted  %s\n"), displayName);
		filesFormatted++;
	}
	else
	{
		if (!isFormattedOnly)
			printMsg(_("Unchanged  %s\n"), displayName);
		filesUnchanged++;
	}

	assert(formatter.getChecksumDiff() == 0);
}

// build a vector of argv options
// the program path argv[0] is excluded
vector<string> ASConsole::getArgvOptions(int argc, char** argv) const
{
	vector<string> argvOptions;
	for (int i = 1; i < argc; i++)
	{
		argvOptions.push_back(string(argv[i]));
	}
	return argvOptions;
}

// for unit testing
vector<bool> ASConsole::getExcludeHitsVector() const
{ return excludeHitsVector; }

// for unit testing
vector<string> ASConsole::getExcludeVector() const
{ return excludeVector; }

// for unit testing
vector<string> ASConsole::getFileName() const
{ return fileName; }

// for unit testing
vector<string> ASConsole::getFileNameVector() const
{ return fileNameVector; }

// for unit testing
vector<string> ASConsole::getFileOptionsVector() const
{ return fileOptionsVector; }

// for unit testing
bool ASConsole::getFilesAreIdentical() const
{ return filesAreIdentical; }

// for unit testing
int ASConsole::getFilesFormatted() const
{ return filesFormatted; }

// for unit testing
bool ASConsole::getIgnoreExcludeErrors() const
{ return ignoreExcludeErrors; }

// for unit testing
bool ASConsole::getIgnoreExcludeErrorsDisplay() const
{ return ignoreExcludeErrorsDisplay; }

// for unit testing
bool ASConsole::getIsDryRun() const
{ return isDryRun; }

// for unit testing
bool ASConsole::getIsFormattedOnly() const
{ return isFormattedOnly; }

// for unit testing
string ASConsole::getLanguageID() const
{ return localizer.getLanguageID(); }

// for unit testing
bool ASConsole::getIsQuiet() const
{ return isQuiet; }

// for unit testing
bool ASConsole::getIsRecursive() const
{ return isRecursive; }

// for unit testing
bool ASConsole::getIsVerbose() const
{ return isVerbose; }

// for unit testing
bool ASConsole::getLineEndsMixed() const
{ return lineEndsMixed; }

// for unit testing
bool ASConsole::getNoBackup() const
{ return noBackup; }

// for unit testing
string ASConsole::getOptionsFileName() const
{ return optionsFileName; }

// for unit testing
vector<string> ASConsole::getOptionsVector() const
{ return optionsVector; }

// for unit testing
string ASConsole::getOrigSuffix() const
{ return origSuffix; }

// for unit testing
bool ASConsole::getPreserveDate() const
{ return preserveDate; }

// for unit testing
void ASConsole::setBypassBrowserOpen(bool state)
{ bypassBrowserOpen = state; }

string ASConsole::getParam(const string &arg, const char* op)
{
	return arg.substr(strlen(op));
}

// initialize output end of line
void ASConsole::initializeOutputEOL(LineEndFormat lineEndFormat)
{
	assert(lineEndFormat == LINEEND_DEFAULT
	       || lineEndFormat == LINEEND_WINDOWS
	       || lineEndFormat == LINEEND_LINUX
	       || lineEndFormat == LINEEND_MACOLD);

	outputEOL[0] = '\0';		// current line end
	prevEOL[0] = '\0';			// previous line end
	lineEndsMixed = false;		// output has mixed line ends, LINEEND_DEFAULT only

	if (lineEndFormat == LINEEND_WINDOWS)
		strcpy(outputEOL, "\r\n");
	else if (lineEndFormat == LINEEND_LINUX)
		strcpy(outputEOL, "\n");
	else if (lineEndFormat == LINEEND_MACOLD)
		strcpy(outputEOL, "\r");
	else
		outputEOL[0] = '\0';
}

FileEncoding ASConsole::readFile(const string &fileName_, stringstream &in) const
{
	const int blockSize = 65536;	// 64 KB
	ifstream fin(fileName_.c_str(), ios::binary);
	if (!fin)
		error("Cannot open input file", fileName_.c_str());
	char* data = new(nothrow) char[blockSize];
	if (!data)
		error("Cannot allocate memory for input file", fileName_.c_str());
	fin.read(data, blockSize);
	if (fin.bad())
		error("Cannot read input file", fileName_.c_str());
	size_t dataSize = static_cast<size_t>(fin.gcount());
	FileEncoding encoding = detectEncoding(data, dataSize);
	if (encoding ==  UTF_32BE || encoding ==  UTF_32LE)
		error(_("Cannot process UTF-32 encoding"), fileName_.c_str());
	bool firstBlock = true;
	bool isBigEndian = (encoding == UTF_16BE);
	while (dataSize)
	{
		if (encoding == UTF_16LE || encoding == UTF_16BE)
		{
			// convert utf-16 to utf-8
			size_t utf8Size = utf8_16.Utf8LengthFromUtf16(data, dataSize, isBigEndian);
			char* utf8Out = new(nothrow) char[utf8Size];
			if (!utf8Out)
				error("Cannot allocate memory for utf-8 conversion", fileName_.c_str());
			size_t utf8Len = utf8_16.Utf16ToUtf8(data, dataSize, isBigEndian, firstBlock, utf8Out);
			assert(utf8Len == utf8Size);
			in << string(utf8Out, utf8Len);
			delete [] utf8Out;
		}
		else
			in << string(data, dataSize);
		fin.read(data, blockSize);
		if (fin.bad())
			error("Cannot read input file", fileName_.c_str());
		dataSize = static_cast<size_t>(fin.gcount());
		firstBlock = false;
	}
	fin.close();
	delete [] data;
	return encoding;
}

void ASConsole::setIgnoreExcludeErrors(bool state)
{ ignoreExcludeErrors = state; }

void ASConsole::setIgnoreExcludeErrorsAndDisplay(bool state)
{ ignoreExcludeErrors = state; ignoreExcludeErrorsDisplay = state; }

void ASConsole::setIsFormattedOnly(bool state)
{ isFormattedOnly = state; }

void ASConsole::setIsQuiet(bool state)
{ isQuiet = state; }

void ASConsole::setIsRecursive(bool state)
{ isRecursive = state; }

void ASConsole::setIsDryRun(bool state)
{ isDryRun = state; }

void ASConsole::setIsVerbose(bool state)
{ isVerbose = state; }

void ASConsole::setNoBackup(bool state)
{ noBackup = state; }

void ASConsole::setOptionsFileName(string name)
{ optionsFileName = name; }

void ASConsole::setOrigSuffix(string suffix)
{ origSuffix = suffix; }

void ASConsole::setPreserveDate(bool state)
{ preserveDate = state; }

// set outputEOL variable
void ASConsole::setOutputEOL(LineEndFormat lineEndFormat, const char* currentEOL)
{
	if (lineEndFormat == LINEEND_DEFAULT)
	{
		strcpy(outputEOL, currentEOL);
		if (strlen(prevEOL) == 0)
			strcpy(prevEOL, outputEOL);
		if (strcmp(prevEOL, outputEOL) != 0)
		{
			lineEndsMixed = true;
			filesAreIdentical = false;
			strcpy(prevEOL, outputEOL);
		}
	}
	else
	{
		strcpy(prevEOL, currentEOL);
		if (strcmp(prevEOL, outputEOL) != 0)
			filesAreIdentical = false;
	}
}

#ifdef _WIN32  // Windows specific

/**
 * WINDOWS function to display the last system error.
 */
void ASConsole::displayLastError()
{
	LPSTR msgBuf;
	DWORD lastError = GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	              NULL,
	              lastError,
	              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Default language
	              (LPSTR) &msgBuf,
	              0,
	              NULL
	             );
	// Display the string.
	(*_err) << "Error (" << lastError << ") " << msgBuf << endl;
	// Free the buffer.
	LocalFree(msgBuf);
}

/**
 * WINDOWS function to get the current directory.
 * NOTE: getenv("CD") does not work for Windows Vista.
 *        The Windows function GetCurrentDirectory is used instead.
 *
 * @return              The path of the current directory
 */
string ASConsole::getCurrentDirectory(const string &fileName_) const
{
	char currdir[MAX_PATH];
	currdir[0] = '\0';
	if (!GetCurrentDirectory(sizeof(currdir), currdir))
		error("Cannot find file", fileName_.c_str());
	return string(currdir);
}

/**
 * WINDOWS function to resolve wildcards and recurse into sub directories.
 * The fileName vector is filled with the path and names of files to process.
 *
 * @param directory     The path of the directory to be processed.
 * @param wildcard      The wildcard to be processed (e.g. *.cpp).
 */
void ASConsole::getFileNames(const string &directory, const string &wildcard)
{
	vector<string> subDirectory;    // sub directories of directory
	WIN32_FIND_DATA findFileData;   // for FindFirstFile and FindNextFile

	// Find the first file in the directory
	// Find will get at least "." and "..".
	string firstFile = directory + "\\*";
	HANDLE hFind = FindFirstFile(firstFile.c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		// Error (3) The system cannot find the path specified.
		// Error (123) The filename, directory name, or volume label syntax is incorrect.
		// ::FindClose(hFind); before exiting
		displayLastError();
		error(_("Cannot open directory"), directory.c_str());
	}

	// save files and sub directories
	do
	{
		// skip hidden or read only
		if (findFileData.cFileName[0] == '.'
		        || (findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		        || (findFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
			continue;

		// is this a sub directory
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!isRecursive)
				continue;
			// if a sub directory and recursive, save sub directory
			string subDirectoryPath = directory + g_fileSeparator + findFileData.cFileName;
			if (isPathExclued(subDirectoryPath))
				printMsg(_("Exclude  %s\n"), subDirectoryPath.substr(mainDirectoryLength));
			else
				subDirectory.push_back(subDirectoryPath);
			continue;
		}

		// save the file name
		string filePathName = directory + g_fileSeparator + findFileData.cFileName;
		// check exclude before wildcmp to avoid "unmatched exclude" error
		bool isExcluded = isPathExclued(filePathName);
		// save file name if wildcard match
		if (wildcmp(wildcard.c_str(), findFileData.cFileName))
		{
			if (isExcluded)
				printMsg(_("Exclude  %s\n"), filePathName.substr(mainDirectoryLength));
			else
				fileName.push_back(filePathName);
		}
	}
	while (FindNextFile(hFind, &findFileData) != 0);

	// check for processing error
	::FindClose(hFind);
	DWORD dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
		error("Error processing directory", directory.c_str());

	// recurse into sub directories
	// if not doing recursive subDirectory is empty
	for (unsigned i = 0; i < subDirectory.size(); i++)
		getFileNames(subDirectory[i], wildcard);

	return;
}

/**
 * WINDOWS function to format a number according to the current locale.
 * This formats positive integers only, no float.
 *
 * @param num		The number to be formatted.
 * @param lcid		The LCID of the locale to be used for testing.
 * @return			The formatted number.
 */
string ASConsole::getNumberFormat(int num, size_t lcid) const
{
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(__GNUC__)
	// Compilers that don't support C++ locales should still support this assert.
	// The C locale should be set but not the C++.
	// This function is not necessary if the C++ locale is set.
	// The locale().name() return value is not portable to all compilers.
	assert(locale().name() == "C");
#endif
	// convert num to a string
	stringstream alphaNum;
	alphaNum << num;
	string number = alphaNum.str();
	if (useAscii)
		return number;

	// format the number using the Windows API
	if (lcid == 0)
		lcid = LOCALE_USER_DEFAULT;
	int outSize = ::GetNumberFormat(lcid, 0, number.c_str(), NULL, NULL, 0);
	char* outBuf = new(nothrow) char[outSize];
	if (outBuf == NULL)
		return number;
	::GetNumberFormat(lcid, 0, number.c_str(), NULL, outBuf, outSize);
	string formattedNum(outBuf);
	delete [] outBuf;
	// remove the decimal
	int decSize = ::GetLocaleInfo(lcid, LOCALE_SDECIMAL, NULL, 0);
	char* decBuf = new(nothrow) char[decSize];
	if (decBuf == NULL)
		return number;
	::GetLocaleInfo(lcid, LOCALE_SDECIMAL, decBuf, decSize);
	size_t i = formattedNum.rfind(decBuf);
	delete [] decBuf;
	if (i != string::npos)
		formattedNum.erase(i);
	if (!formattedNum.length())
		formattedNum = "0";
	return formattedNum;
}

/**
 * WINDOWS function to open a HTML file in the default browser.
 */
void ASConsole::launchDefaultBrowser(const char* filePathIn /*NULL*/) const
{
	struct stat statbuf;
	const char* envPaths[] = {  "PROGRAMFILES(X86)", "PROGRAMFILES" };
	size_t pathsLen = sizeof(envPaths) / sizeof(envPaths[0]);
	string htmlDefaultPath;
	for (size_t i = 0; i < pathsLen; i++)
	{
		const char* envPath = getenv(envPaths[i]);
		if (envPath == NULL)
			continue;
		htmlDefaultPath = envPath;
		if (htmlDefaultPath.length() > 0
		        && htmlDefaultPath[htmlDefaultPath.length() - 1] == g_fileSeparator)
			htmlDefaultPath.erase(htmlDefaultPath.length() - 1);
		htmlDefaultPath.append("\\AStyle\\doc");
		if (stat(htmlDefaultPath.c_str(), &statbuf) == 0 && statbuf.st_mode & S_IFDIR)
			break;
	}
	htmlDefaultPath.append("\\");

	// build file path
	string htmlFilePath;
	if (filePathIn == NULL)
		htmlFilePath = htmlDefaultPath + "astyle.html";
	else
	{
		if (strpbrk(filePathIn, "\\/") == NULL)
			htmlFilePath = htmlDefaultPath + filePathIn;
		else
			htmlFilePath = filePathIn;
	}
	standardizePath(htmlFilePath);
	if (stat(htmlFilePath.c_str(), &statbuf) != 0 || !(statbuf.st_mode & S_IFREG))
	{
		printf(_("Cannot open HTML file %s\n"), htmlFilePath.c_str());
		return;
	}

	SHELLEXECUTEINFO sei = { sizeof(sei), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	sei.fMask = SEE_MASK_FLAG_NO_UI;
	sei.lpVerb = "open";
	sei.lpFile = htmlFilePath.c_str();
	sei.nShow = SW_SHOWNORMAL;

	// browser open will be bypassed in test programs
	printf(_("Opening HTML documentation %s\n"), htmlFilePath.c_str());
	if (!bypassBrowserOpen)
	{
		int ret = ShellExecuteEx(&sei);
		if (!ret)
			error(_("Command execute failure"), htmlFilePath.c_str());
	}
}

#else  // Linux specific

/**
 * LINUX function to get the current directory.
 * This is done if the fileName does not contain a path.
 * It is probably from an editor sending a single file.
 *
 * @param fileName_     The filename is used only for  the error message.
 * @return              The path of the current directory
 */
string ASConsole::getCurrentDirectory(const string &fileName_) const
{
	char* currdir = getenv("PWD");
	if (currdir == NULL)
		error("Cannot find file", fileName_.c_str());
	return string(currdir);
}

/**
 * LINUX function to resolve wildcards and recurse into sub directories.
 * The fileName vector is filled with the path and names of files to process.
 *
 * @param directory     The path of the directory to be processed.
 * @param wildcard      The wildcard to be processed (e.g. *.cpp).
 */
void ASConsole::getFileNames(const string &directory, const string &wildcard)
{
	struct dirent* entry;           // entry from readdir()
	struct stat statbuf;            // entry from stat()
	vector<string> subDirectory;    // sub directories of this directory

	// errno is defined in <errno.h> and is set for errors in opendir, readdir, or stat
	errno = 0;

	DIR* dp = opendir(directory.c_str());
	if (dp == NULL)
		error(_("Cannot open directory"), directory.c_str());

	// save the first fileName entry for this recursion
	const unsigned firstEntry = fileName.size();

	// save files and sub directories
	while ((entry = readdir(dp)) != NULL)
	{
		// get file status
		string entryFilepath = directory + g_fileSeparator + entry->d_name;
		if (stat(entryFilepath.c_str(), &statbuf) != 0)
		{
			if (errno == EOVERFLOW)         // file over 2 GB is OK
			{
				errno = 0;
				continue;
			}
			perror("errno message");
			error("Error getting file status in directory", directory.c_str());
		}
		// skip hidden or read only
		if (entry->d_name[0] == '.' || !(statbuf.st_mode & S_IWUSR))
			continue;
		// if a sub directory and recursive, save sub directory
		if (S_ISDIR(statbuf.st_mode) && isRecursive)
		{
			if (isPathExclued(entryFilepath))
				printMsg(_("Exclude  %s\n"), entryFilepath.substr(mainDirectoryLength));
			else
				subDirectory.push_back(entryFilepath);
			continue;
		}

		// if a file, save file name
		if (S_ISREG(statbuf.st_mode))
		{
			// check exclude before wildcmp to avoid "unmatched exclude" error
			bool isExcluded = isPathExclued(entryFilepath);
			// save file name if wildcard match
			if (wildcmp(wildcard.c_str(), entry->d_name))
			{
				if (isExcluded)
					printMsg(_("Exclude  %s\n"), entryFilepath.substr(mainDirectoryLength));
				else
					fileName.push_back(entryFilepath);
			}
		}
	}

	if (closedir(dp) != 0)
	{
		perror("errno message");
		error("Error reading directory", directory.c_str());
	}

	// sort the current entries for fileName
	if (firstEntry < fileName.size())
		sort(&fileName[firstEntry], &fileName[fileName.size()]);

	// recurse into sub directories
	// if not doing recursive, subDirectory is empty
	if (subDirectory.size() > 1)
		sort(subDirectory.begin(), subDirectory.end());
	for (unsigned i = 0; i < subDirectory.size(); i++)
	{
		getFileNames(subDirectory[i], wildcard);
	}

	return;
}

/**
 * LINUX function to get locale information and call getNumberFormat.
 * This formats positive integers only, no float.
 *
 * @param num		The number to be formatted.
 *                  size_t is for compatibility with the Windows function.
 * @return			The formatted number.
 */
string ASConsole::getNumberFormat(int num, size_t) const
{
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(__GNUC__)
	// Compilers that don't support C++ locales should still support this assert.
	// The C locale should be set but not the C++.
	// This function is not necessary if the C++ locale is set.
	// The locale().name() return value is not portable to all compilers.
	assert(locale().name() == "C");
#endif

	// get the locale info
	struct lconv* lc;
	lc = localeconv();

	// format the number
	return getNumberFormat(num, lc->grouping, lc->thousands_sep);
}

/**
 * LINUX function to format a number according to the current locale.
 * This formats positive integers only, no float.
 *
 * @param num			The number to be formatted.
 * @param groupingArg   The grouping string from the locale.
 * @param  separator	The thousands group separator from the locale.
 * @return				The formatted number.
 */
string ASConsole::getNumberFormat(int num, const char* groupingArg, const char* separator) const
{
	// convert num to a string
	stringstream alphaNum;
	alphaNum << num;
	string number = alphaNum.str();
	// format the number from right to left
	string formattedNum;
	size_t ig = 0;	// grouping index
	int grouping = groupingArg[ig];
	int i = number.length();
	// check for no grouping
	if (grouping == 0)
		grouping = number.length();
	while (i > 0)
	{
		// extract a group of numbers
		string group;
		if (i < grouping)
			group = number;
		else
			group = number.substr(i - grouping);
		// update formatted number
		formattedNum.insert(0, group);
		i -= grouping;
		if (i < 0)
			i = 0;
		if (i > 0)
			formattedNum.insert(0, separator);
		number.erase(i);
		// update grouping
		if (groupingArg[ig] != '\0'
		        && groupingArg[ig + 1] != '\0')
			grouping = groupingArg[++ig];
	}
	return formattedNum;
}

/**
 * LINUX function to open a HTML file in the default browser.
 * Use xdg-open from freedesktop.org cross-desktop compatibility suite xdg-utils.
 * see http://portland.freedesktop.org/wiki/
 * This is installed on most modern distributions.
 */
void ASConsole::launchDefaultBrowser(const char* filePathIn /*NULL*/) const
{
	struct stat statbuf;
	string htmlDefaultPath = "/usr/share/doc/astyle/html/";
	string htmlDefaultFile = "astyle.html";

	// build file path
	string htmlFilePath;
	if (filePathIn == NULL)
		htmlFilePath = htmlDefaultPath + htmlDefaultFile;
	else
	{
		if (strpbrk(filePathIn, "\\/") == NULL)
			htmlFilePath = htmlDefaultPath + filePathIn;
		else
			htmlFilePath = filePathIn;
	}
	standardizePath(htmlFilePath);
	if (stat(htmlFilePath.c_str(), &statbuf) != 0 || !(statbuf.st_mode & S_IFREG))
	{
		printf(_("Cannot open HTML file %s\n"), htmlFilePath.c_str());
		return;
	}

	// get search paths
	const char* envPaths = getenv("PATH");
	if (envPaths == NULL)
		envPaths = "?";
	size_t envlen = strlen(envPaths);
	char* paths = new char[envlen + 1];
	strcpy(paths, envPaths);
	// find xdg-open (usually in /usr/bin)
	// Mac uses open instead
#ifdef __APPLE__
	const char* FILE_OPEN = "open";
#else
	const char* FILE_OPEN = "xdg-open";
#endif
	string searchPath;
	char* searchDir = strtok(paths, ":");
	while (searchDir != NULL)
	{
		searchPath = searchDir;
		if (searchPath.length() > 0
		        && searchPath[searchPath.length() - 1] != g_fileSeparator)
			searchPath.append(string(1, g_fileSeparator));
		searchPath.append(FILE_OPEN);
		if (stat(searchPath.c_str(), &statbuf) == 0 && (statbuf.st_mode & S_IFREG))
			break;
		searchDir = strtok(NULL, ":");
	}
	delete[] paths;
	if (searchDir == NULL)
		error(_("Command is not installed"), FILE_OPEN);

	// browser open will be bypassed in test programs
	printf(_("Opening HTML documentation %s\n"), htmlFilePath.c_str());
	if (!bypassBrowserOpen)
	{
		execlp(FILE_OPEN, FILE_OPEN, htmlFilePath.c_str(), NULL);
		// execlp will NOT return if successful
		error(_("Command execute failure"), FILE_OPEN);
	}
}

#endif  // _WIN32

// get individual file names from the command-line file path
void ASConsole::getFilePaths(string &filePath)
{
	fileName.clear();
	targetDirectory = string();
	targetFilename = string();

	// separate directory and file name
	size_t separator = filePath.find_last_of(g_fileSeparator);
	if (separator == string::npos)
	{
		// if no directory is present, use the currently active directory
		targetDirectory = getCurrentDirectory(filePath);
		targetFilename  = filePath;
		mainDirectoryLength = targetDirectory.length() + 1;    // +1 includes trailing separator
	}
	else
	{
		targetDirectory = filePath.substr(0, separator);
		targetFilename  = filePath.substr(separator + 1);
		mainDirectoryLength = targetDirectory.length() + 1;    // +1 includes trailing separator
	}

	if (targetFilename.length() == 0)
	{
		fprintf(stderr, _("Missing filename in %s\n"), filePath.c_str());
		error();
	}

	// check filename for wildcards
	hasWildcard = false;
	if (targetFilename.find_first_of("*?") != string::npos)
		hasWildcard = true;

	// clear exclude hits vector
	for (size_t ix = 0; ix < excludeHitsVector.size(); ix++)
		excludeHitsVector[ix] = false;

	// If the filename is not quoted on Linux, bash will replace the
	// wildcard instead of passing it to the program.
	if (isRecursive && !hasWildcard)
	{
		fprintf(stderr, "%s\n", _("Recursive option with no wildcard"));
#ifndef _WIN32
		fprintf(stderr, "%s\n", _("Did you intend quote the filename"));
#endif
		error();
	}

	// display directory name for wildcard processing
	if (hasWildcard)
	{
		printSeparatingLine();
		printMsg(_("Directory  %s\n"), targetDirectory + g_fileSeparator + targetFilename);
	}

	// create a vector of paths and file names to process
	if (hasWildcard || isRecursive)
		getFileNames(targetDirectory, targetFilename);
	else
	{
		// verify a single file is not a directory (needed on Linux)
		string entryFilepath = targetDirectory + g_fileSeparator + targetFilename;
		struct stat statbuf;
		if (stat(entryFilepath.c_str(), &statbuf) == 0 && (statbuf.st_mode & S_IFREG))
			fileName.push_back(entryFilepath);
	}

	// check for unprocessed excludes
	bool excludeErr = false;
	for (size_t ix = 0; ix < excludeHitsVector.size(); ix++)
	{
		if (excludeHitsVector[ix] == false)
		{
			excludeErr = true;
			if (!ignoreExcludeErrorsDisplay)
			{
				if (ignoreExcludeErrors)
					printMsg(_("Exclude (unmatched)  %s\n"), excludeVector[ix]);
				else
					fprintf(stderr, _("Exclude (unmatched)  %s\n"), excludeVector[ix].c_str());
			}
			else
			{
				if (!ignoreExcludeErrors)
					fprintf(stderr, _("Exclude (unmatched)  %s\n"), excludeVector[ix].c_str());
			}
		}
	}

	if (excludeErr && !ignoreExcludeErrors)
	{
		if (hasWildcard && !isRecursive)
			fprintf(stderr, "%s\n", _("Did you intend to use --recursive"));
		error();
	}

	// check if files were found (probably an input error if not)
	if (fileName.empty())
	{
		fprintf(stderr, _("No file to process %s\n"), filePath.c_str());
		if (hasWildcard && !isRecursive)
			fprintf(stderr, "%s\n", _("Did you intend to use --recursive"));
		error();
	}

	if (hasWildcard)
		printSeparatingLine();
}

bool ASConsole::fileNameVectorIsEmpty() const
{
	return fileNameVector.empty();
}

bool ASConsole::isOption(const string &arg, const char* op)
{
	return arg.compare(op) == 0;
}

bool ASConsole::isOption(const string &arg, const char* a, const char* b)
{
	return (isOption(arg, a) || isOption(arg, b));
}

bool ASConsole::isParamOption(const string &arg, const char* option)
{
	bool retVal = arg.compare(0, strlen(option), option) == 0;
	// if comparing for short option, 2nd char of arg must be numeric
	if (retVal && strlen(option) == 1 && arg.length() > 1)
		if (!isdigit((unsigned char)arg[1]))
			retVal = false;
	return retVal;
}

// compare a path to the exclude vector
// used for both directories and filenames
// updates the g_excludeHitsVector
// return true if a match
bool ASConsole::isPathExclued(const string &subPath)
{
	bool retVal = false;

	// read the exclude vector checking for a match
	for (size_t i = 0; i < excludeVector.size(); i++)
	{
		string exclude = excludeVector[i];

		if (subPath.length() < exclude.length())
			continue;

		size_t compareStart = subPath.length() - exclude.length();
		// subPath compare must start with a directory name
		if (compareStart > 0)
		{
			char lastPathChar = subPath[compareStart - 1];
			if (lastPathChar != g_fileSeparator)
				continue;
		}

		string compare = subPath.substr(compareStart);
		if (!g_isCaseSensitive)
		{
			// make it case insensitive for Windows
			for (size_t j = 0; j < compare.length(); j++)
				compare[j] = (char)tolower(compare[j]);
			for (size_t j = 0; j < exclude.length(); j++)
				exclude[j] = (char)tolower(exclude[j]);
		}
		// compare sub directory to exclude data - must check them all
		if (compare == exclude)
		{
			excludeHitsVector[i] = true;
			retVal = true;
			break;
		}
	}
	return retVal;
}

void ASConsole::printHelp() const
{
	cout << endl;
	cout << "                     Artistic Style " << g_version << endl;
	cout << "                     Maintained by: Jim Pattee\n";
	cout << "                     Original Author: Tal Davidson\n";
	cout << endl;
	cout << "Usage:\n";
	cout << "------\n";
	cout << "            astyle [OPTIONS] File1 File2 File3 [...]\n";
	cout << endl;
	cout << "            astyle [OPTIONS] < Original > Beautified\n";
	cout << endl;
	cout << "    When indenting a specific file, the resulting indented file RETAINS\n";
	cout << "    the original file-name. The original pre-indented file is renamed,\n";
	cout << "    with a suffix of \'.orig\' added to the original filename.\n";
	cout << endl;
	cout << "    Wildcards (* and ?) may be used in the filename.\n";
	cout << "    A \'recursive\' option can process directories recursively.\n";
	cout << endl;
	cout << "    By default, astyle is set up to indent with four spaces per indent,\n";
	cout << "    a maximal indentation of 40 spaces inside continuous statements,\n";
	cout << "    a minimum indentation of eight spaces inside conditional statements,\n";
	cout << "    and NO formatting options.\n";
	cout << endl;
	cout << "Options:\n";
	cout << "--------\n";
	cout << "    This  program  follows  the  usual  GNU  command line syntax.\n";
	cout << "    Long options (starting with '--') must be written one at a time.\n";
	cout << "    Short options (starting with '-') may be appended together.\n";
	cout << "    Thus, -bps4 is the same as -b -p -s4.\n";
	cout << endl;
	cout << "Options File:\n";
	cout << "-------------\n";
	cout << "    Artistic Style looks for a default options file in the\n";
	cout << "    following order:\n";
	cout << "    1. The contents of the ARTISTIC_STYLE_OPTIONS environment\n";
	cout << "       variable if it exists.\n";
	cout << "    2. The file called .astylerc in the directory pointed to by the\n";
	cout << "       HOME environment variable ( i.e. $HOME/.astylerc ).\n";
	cout << "    3. The file called astylerc in the directory pointed to by the\n";
	cout << "       USERPROFILE environment variable (i.e. %USERPROFILE%\\astylerc).\n";
	cout << "    If a default options file is found, the options in this file will\n";
	cout << "    be parsed BEFORE the command-line options.\n";
	cout << "    Long options within the default option file may be written without\n";
	cout << "    the preliminary '--'.\n";
	cout << endl;
	cout << "Disable Formatting:\n";
	cout << "----------------------\n";
	cout << "    Disable Block\n";
	cout << "    Blocks of code can be disabled with the comment tags *INDENT-OFF*\n";
	cout << "    and *INDENT-ON*. It must be contained in a one-line comment.\n";
	cout << endl;
	cout << "    Disable Line\n";
	cout << "    Padding of operators can be disabled on a single line using the\n";
	cout << "    comment tag *NOPAD*. It must be contained in a line-end comment.\n";
	cout << endl;
	cout << "Bracket Style Options:\n";
	cout << "----------------------\n";
	cout << "    default bracket style\n";
	cout << "    If no bracket style is requested, the opening brackets will not be\n";
	cout << "    changed and closing brackets will be broken from the preceding line.\n";
	cout << endl;
	cout << "    --style=allman  OR  --style=bsd  OR  --style=break  OR  -A1\n";
	cout << "    Allman style formatting/indenting.\n";
	cout << "    Broken brackets.\n";
	cout << endl;
	cout << "    --style=java  OR  --style=attach  OR  -A2\n";
	cout << "    Java style formatting/indenting.\n";
	cout << "    Attached brackets.\n";
	cout << endl;
	cout << "    --style=kr  OR  --style=k&r  OR  --style=k/r  OR  -A3\n";
	cout << "    Kernighan & Ritchie style formatting/indenting.\n";
	cout << "    Linux brackets.\n";
	cout << endl;
	cout << "    --style=stroustrup  OR  -A4\n";
	cout << "    Stroustrup style formatting/indenting.\n";
	cout << "    Stroustrup brackets.\n";
	cout << endl;
	cout << "    --style=whitesmith  OR  -A5\n";
	cout << "    Whitesmith style formatting/indenting.\n";
	cout << "    Broken, indented brackets.\n";
	cout << "    Indented class blocks and switch blocks.\n";
	cout << endl;
	cout << "    --style=vtk  OR  -A15\n";
	cout << "    VTK style formatting/indenting.\n";
	cout << "    Broken, indented brackets, except for opening brackets.\n";
	cout << endl;
	cout << "    --style=banner  OR  -A6\n";
	cout << "    Banner style formatting/indenting.\n";
	cout << "    Attached, indented brackets.\n";
	cout << endl;
	cout << "    --style=gnu  OR  -A7\n";
	cout << "    GNU style formatting/indenting.\n";
	cout << "    Broken brackets, indented blocks.\n";
	cout << endl;
	cout << "    --style=linux  OR  --style=knf  OR  -A8\n";
	cout << "    Linux style formatting/indenting.\n";
	cout << "    Linux brackets, minimum conditional indent is one-half indent.\n";
	cout << endl;
	cout << "    --style=horstmann  OR  -A9\n";
	cout << "    Horstmann style formatting/indenting.\n";
	cout << "    Run-in brackets, indented switches.\n";
	cout << endl;
	cout << "    --style=1tbs  OR  --style=otbs  OR  -A10\n";
	cout << "    One True Brace Style formatting/indenting.\n";
	cout << "    Linux brackets, add brackets to all conditionals.\n";
	cout << endl;
	cout << "    --style=google  OR  -A14\n";
	cout << "    Google style formatting/indenting.\n";
	cout << "    Attached brackets, indented class modifiers.\n";
	cout << endl;
	cout << "    --style=pico  OR  -A11\n";
	cout << "    Pico style formatting/indenting.\n";
	cout << "    Run-in opening brackets and attached closing brackets.\n";
	cout << "    Uses keep one line blocks and keep one line statements.\n";
	cout << endl;
	cout << "    --style=lisp  OR  -A12\n";
	cout << "    Lisp style formatting/indenting.\n";
	cout << "    Attached opening brackets and attached closing brackets.\n";
	cout << "    Uses keep one line statements.\n";
	cout << endl;
	cout << "Tab Options:\n";
	cout << "------------\n";
	cout << "    default indent option\n";
	cout << "    If no indentation option is set, the default\n";
	cout << "    option of 4 spaces per indent will be used.\n";
	cout << endl;
	cout << "    --indent=spaces=#  OR  -s#\n";
	cout << "    Indent using # spaces per indent. Not specifying #\n";
	cout << "    will result in a default of 4 spaces per indent.\n";
	cout << endl;
	cout << "    --indent=tab  OR  --indent=tab=#  OR  -t  OR  -t#\n";
	cout << "    Indent using tab characters, assuming that each\n";
	cout << "    indent is # spaces long. Not specifying # will result\n";
	cout << "    in a default assumption of 4 spaces per indent.\n";
	cout << endl;
	cout << "    --indent=force-tab=#  OR  -T#\n";
	cout << "    Indent using tab characters, assuming that each\n";
	cout << "    indent is # spaces long. Force tabs to be used in areas\n";
	cout << "    AStyle would prefer to use spaces.\n";
	cout << endl;
	cout << "    --indent=force-tab-x=#  OR  -xT#\n";
	cout << "    Allows the tab length to be set to a length that is different\n";
	cout << "    from the indent length. This may cause the indentation to be\n";
	cout << "    a mix of both spaces and tabs. This option sets the tab length.\n";
	cout << endl;
	cout << "Bracket Modify Options:\n";
	cout << "-----------------------\n";
	cout << "    --attach-namespaces  OR  -xn\n";
	cout << "    Attach brackets to a namespace statement.\n";
	cout << endl;
	cout << "    --attach-classes  OR  -xc\n";
	cout << "    Attach brackets to a class statement.\n";
	cout << endl;
	cout << "    --attach-inlines  OR  -xl\n";
	cout << "    Attach brackets to class inline function definitions.\n";
	cout << endl;
	cout << "    --attach-extern-c  OR  -xk\n";
	cout << "    Attach brackets to an extern \"C\" statement.\n";
	cout << endl;
	cout << "Indentation Options:\n";
	cout << "--------------------\n";
	cout << "    --indent-classes  OR  -C\n";
	cout << "    Indent 'class' blocks so that the entire block is indented.\n";
	cout << endl;
	cout << "    --indent-modifiers  OR  -xG\n";
	cout << "    Indent 'class' access modifiers, 'public:', 'protected:' or\n";
	cout << "    'private:', one half indent. The rest of the class is not\n";
	cout << "    indented. \n";
	cout << endl;
	cout << "    --indent-switches  OR  -S\n";
	cout << "    Indent 'switch' blocks, so that the inner 'case XXX:'\n";
	cout << "    headers are indented in relation to the switch block.\n";
	cout << endl;
	cout << "    --indent-cases  OR  -K\n";
	cout << "    Indent case blocks from the 'case XXX:' headers.\n";
	cout << "    Case statements not enclosed in blocks are NOT indented.\n";
	cout << endl;
	cout << "    --indent-namespaces  OR  -N\n";
	cout << "    Indent the contents of namespace blocks.\n";
	cout << endl;
	cout << "    --indent-labels  OR  -L\n";
	cout << "    Indent labels so that they appear one indent less than\n";
	cout << "    the current indentation level, rather than being\n";
	cout << "    flushed completely to the left (which is the default).\n";
	cout << endl;
	cout << "    --indent-preproc-block  OR  -xW\n";
	cout << "    Indent preprocessor blocks at bracket level 0.\n";
	cout << "    Without this option the preprocessor block is not indented.\n";
	cout << endl;
	cout << "    --indent-preproc-cond  OR  -xw\n";
	cout << "    Indent preprocessor conditional statements #if/#else/#endif\n";
	cout << "    to the same level as the source code.\n";
	cout << endl;
	cout << "    --indent-preproc-define  OR  -w\n";
	cout << "    Indent multi-line preprocessor #define statements.\n";
	cout << endl;
	cout << "    --indent-col1-comments  OR  -Y\n";
	cout << "    Indent line comments that start in column one.\n";
	cout << endl;
	cout << "    --min-conditional-indent=#  OR  -m#\n";
	cout << "    Indent a minimal # spaces in a continuous conditional\n";
	cout << "    belonging to a conditional header.\n";
	cout << "    The valid values are:\n";
	cout << "    0 - no minimal indent.\n";
	cout << "    1 - indent at least one additional indent.\n";
	cout << "    2 - indent at least two additional indents.\n";
	cout << "    3 - indent at least one-half an additional indent.\n";
	cout << "    The default value is 2, two additional indents.\n";
	cout << endl;
	cout << "    --max-instatement-indent=#  OR  -M#\n";
	cout << "    Indent a maximal # spaces in a continuous statement,\n";
	cout << "    relative to the previous line.\n";
	cout << "    The valid values are 40 thru 120.\n";
	cout << "    The default value is 40.\n";
	cout << endl;
	cout << "Padding Options:\n";
	cout << "----------------\n";
	cout << "    --break-blocks  OR  -f\n";
	cout << "    Insert empty lines around unrelated blocks, labels, classes, ...\n";
	cout << endl;
	cout << "    --break-blocks=all  OR  -F\n";
	cout << "    Like --break-blocks, except also insert empty lines \n";
	cout << "    around closing headers (e.g. 'else', 'catch', ...).\n";
	cout << endl;
	cout << "    --pad-oper  OR  -p\n";
	cout << "    Insert space padding around operators.\n";
	cout << endl;
	cout << "    --pad-paren  OR  -P\n";
	cout << "    Insert space padding around parenthesis on both the outside\n";
	cout << "    and the inside.\n";
	cout << endl;
	cout << "    --pad-paren-out  OR  -d\n";
	cout << "    Insert space padding around parenthesis on the outside only.\n";
	cout << endl;
	cout << "    --pad-first-paren-out  OR  -xd\n";
	cout << "    Insert space padding around first parenthesis in a series on\n";
	cout << "    the outside only.\n";
	cout << endl;
	cout << "    --pad-paren-in  OR  -D\n";
	cout << "    Insert space padding around parenthesis on the inside only.\n";
	cout << endl;
	cout << "    --pad-header  OR  -H\n";
	cout << "    Insert space padding after paren headers (e.g. 'if', 'for'...).\n";
	cout << endl;
	cout << "    --unpad-paren  OR  -U\n";
	cout << "    Remove unnecessary space padding around parenthesis. This\n";
	cout << "    can be used in combination with the 'pad' options above.\n";
	cout << endl;
	cout << "    --delete-empty-lines  OR  -xd\n";
	cout << "    Delete empty lines within a function or method.\n";
	cout << "    It will NOT delete lines added by the break-blocks options.\n";
	cout << endl;
	cout << "    --fill-empty-lines  OR  -E\n";
	cout << "    Fill empty lines with the white space of their\n";
	cout << "    previous lines.\n";
	cout << endl;
	cout << "    --align-pointer=type    OR  -k1\n";
	cout << "    --align-pointer=middle  OR  -k2\n";
	cout << "    --align-pointer=name    OR  -k3\n";
	cout << "    Attach a pointer or reference operator (*, &, or ^) to either\n";
	cout << "    the operator type (left), middle, or operator name (right).\n";
	cout << "    To align the reference separately use --align-reference.\n";
	cout << endl;
	cout << "    --align-reference=none    OR  -W0\n";
	cout << "    --align-reference=type    OR  -W1\n";
	cout << "    --align-reference=middle  OR  -W2\n";
	cout << "    --align-reference=name    OR  -W3\n";
	cout << "    Attach a reference operator (&) to either\n";
	cout << "    the operator type (left), middle, or operator name (right).\n";
	cout << "    If not set, follow pointer alignment.\n";
	cout << endl;
	cout << "Formatting Options:\n";
	cout << "-------------------\n";
	cout << "    --break-closing-brackets  OR  -y\n";
	cout << "    Break brackets before closing headers (e.g. 'else', 'catch', ...).\n";
	cout << "    Use with --style=java, --style=kr, --style=stroustrup,\n";
	cout << "    --style=linux, or --style=1tbs.\n";
	cout << endl;
	cout << "    --break-elseifs  OR  -e\n";
	cout << "    Break 'else if()' statements into two different lines.\n";
	cout << endl;
	cout << "    --add-brackets  OR  -j\n";
	cout << "    Add brackets to unbracketed one line conditional statements.\n";
	cout << endl;
	cout << "    --add-one-line-brackets  OR  -J\n";
	cout << "    Add one line brackets to unbracketed one line conditional\n";
	cout << "    statements.\n";
	cout << endl;
	cout << "    --remove-brackets  OR  -xj\n";
	cout << "    Remove brackets from a bracketed one line conditional statements.\n";
	cout << endl;
	cout << "    --keep-one-line-blocks  OR  -O\n";
	cout << "    Don't break blocks residing completely on one line.\n";
	cout << endl;
	cout << "    --keep-one-line-statements  OR  -o\n";
	cout << "    Don't break lines containing multiple statements into\n";
	cout << "    multiple single-statement lines.\n";
	cout << endl;
	cout << "    --convert-tabs  OR  -c\n";
	cout << "    Convert tabs to the appropriate number of spaces.\n";
	cout << endl;
	cout << "    --close-templates  OR  -xy\n";
	cout << "    Close ending angle brackets on template definitions.\n";
	cout << endl;
	cout << "    --remove-comment-prefix  OR  -xp\n";
	cout << "    Remove the leading '*' prefix on multi-line comments and\n";
	cout << "    indent the comment text one indent.\n";
	cout << endl;
	cout << "    --max-code-length=#    OR  -xC#\n";
	cout << "    --break-after-logical  OR  -xL\n";
	cout << "    max-code-length=# will break the line if it exceeds more than\n";
	cout << "    # characters. The valid values are 50 thru 200.\n";
	cout << "    If the line contains logical conditionals they will be placed\n";
	cout << "    first on the new line. The option break-after-logical will\n";
	cout << "    cause the logical conditional to be placed last on the\n";
	cout << "    previous line.\n";
	cout << endl;
	cout << "    --mode=c\n";
	cout << "    Indent a C or C++ source file (this is the default).\n";
	cout << endl;
	cout << "    --mode=java\n";
	cout << "    Indent a Java source file.\n";
	cout << endl;
	cout << "    --mode=cs\n";
	cout << "    Indent a C# source file.\n";
	cout << endl;
	cout << "Objective-C Options:\n";
	cout << "--------------------\n";
	cout << "    --align-method-colon  OR  -xM\n";
	cout << "    Align the colons in an Objective-C method definition.\n";
	cout << endl;
	cout << "    --pad-method-prefix  OR  -xQ\n";
	cout << "    Insert space padding after the '-' or '+' Objective-C\n";
	cout << "    method prefix.\n";
	cout << endl;
	cout << "    --unpad-method-prefix  OR  -xR\n";
	cout << "    Remove all space padding after the '-' or '+' Objective-C\n";
	cout << "    method prefix.\n";
	cout << endl;
	cout << "    --pad-method-colon=none    OR  -xP\n";
	cout << "    --pad-method-colon=all     OR  -xP1\n";
	cout << "    --pad-method-colon=after   OR  -xP2\n";
	cout << "    --pad-method-colon=before  OR  -xP3\n";
	cout << "    Add or remove space padding before or after the colons in an\n";
	cout << "    Objective-C method call.\n";
	cout << endl;
	cout << "Other Options:\n";
	cout << "--------------\n";
	cout << "    --suffix=####\n";
	cout << "    Append the suffix #### instead of '.orig' to original filename.\n";
	cout << endl;
	cout << "    --suffix=none  OR  -n\n";
	cout << "    Do not retain a backup of the original file.\n";
	cout << endl;
	cout << "    --recursive  OR  -r  OR  -R\n";
	cout << "    Process subdirectories recursively.\n";
	cout << endl;
	cout << "    --dry-run\n";
	cout << "    Perform a trial run with no changes made to check for formatting.\n";
	cout << endl;
	cout << "    --exclude=####\n";
	cout << "    Specify a file or directory #### to be excluded from processing.\n";
	cout << endl;
	cout << "    --ignore-exclude-errors  OR  -i\n";
	cout << "    Allow processing to continue if there are errors in the exclude=####\n";
	cout << "    options. It will display the unmatched excludes.\n";
	cout << endl;
	cout << "    --ignore-exclude-errors-x  OR  -xi\n";
	cout << "    Allow processing to continue if there are errors in the exclude=####\n";
	cout << "    options. It will NOT display the unmatched excludes.\n";
	cout << endl;
	cout << "    --errors-to-stdout  OR  -X\n";
	cout << "    Print errors and help information to standard-output rather than\n";
	cout << "    to standard-error.\n";
	cout << endl;
	cout << "    --preserve-date  OR  -Z\n";
	cout << "    Preserve the original file's date and time modified. The time\n";
	cout << "     modified will be changed a few micro seconds to force a compile.\n";
	cout << endl;
	cout << "    --verbose  OR  -v\n";
	cout << "    Verbose mode. Extra informational messages will be displayed.\n";
	cout << endl;
	cout << "    --formatted  OR  -Q\n";
	cout << "    Formatted display mode. Display only the files that have been\n";
	cout << "    formatted.\n";
	cout << endl;
	cout << "    --quiet  OR  -q\n";
	cout << "    Quiet mode. Suppress all output except error messages.\n";
	cout << endl;
	cout << "    --lineend=windows  OR  -z1\n";
	cout << "    --lineend=linux    OR  -z2\n";
	cout << "    --lineend=macold   OR  -z3\n";
	cout << "    Force use of the specified line end style. Valid options\n";
	cout << "    are windows (CRLF), linux (LF), and macold (CR).\n";
	cout << endl;
	cout << "Command Line Only:\n";
	cout << "------------------\n";
	cout << "    --options=####\n";
	cout << "    Specify an options file #### to read and use.\n";
	cout << endl;
	cout << "    --options=none\n";
	cout << "    Disable the default options file.\n";
	cout << "    Only the command-line parameters will be used.\n";
	cout << endl;
	cout << "    --ascii  OR  -I\n";
	cout << "    The displayed output will be ascii characters only.\n";
	cout << endl;
	cout << "    --version  OR  -V\n";
	cout << "    Print version number.\n";
	cout << endl;
	cout << "    --help  OR  -h  OR  -?\n";
	cout << "    Print this help message.\n";
	cout << endl;
	cout << "    --html  OR  -!\n";
	cout << "    Open the HTML help file \"astyle.html\" in the default browser.\n";
	cout << "    The documentation must be installed in the standard install path.\n";
	cout << endl;
	cout << "    --html=####\n";
	cout << "    Open a HTML help file in the default browser using the file path\n";
	cout << "    ####. The path may include a directory path and a file name, or a\n";
	cout << "    file name only. Paths containing spaces must be enclosed in quotes.\n";
	cout << endl;
	cout << endl;
}

/**
 * Process files in the fileNameVector.
 */
void ASConsole::processFiles()
{
	if (isVerbose)
		printVerboseHeader();

	clock_t startTime = clock();     // start time of file formatting

	// loop thru input fileNameVector and process the files
	for (size_t i = 0; i < fileNameVector.size(); i++)
	{
		getFilePaths(fileNameVector[i]);

		// loop thru fileName vector formatting the files
		for (size_t j = 0; j < fileName.size(); j++)
			formatFile(fileName[j]);
	}

	// files are processed, display stats
	if (isVerbose)
		printVerboseStats(startTime);
}

// process options from the command line and options file
// build the vectors fileNameVector, excludeVector, optionsVector, and fileOptionsVector
void ASConsole::processOptions(vector<string> &argvOptions)
{
	string arg;
	bool ok = true;
	bool shouldParseOptionsFile = true;

	// get command line options
	for (size_t i = 0; i < argvOptions.size(); i++)
	{
		arg = argvOptions[i];

		if ( isOption(arg, "-I" )
		        || isOption(arg, "--ascii") )
		{
			useAscii = true;
			setlocale(LC_ALL, "C");		// use English decimal indicator
			localizer.setLanguageFromName("en");
		}
		else if ( isOption(arg, "--options=none") )
		{
			shouldParseOptionsFile = false;
		}
		else if ( isParamOption(arg, "--options=") )
		{
			optionsFileName = getParam(arg, "--options=");
			optionsFileRequired = true;
			if (optionsFileName.compare("") == 0)
				setOptionsFileName(" ");
		}
		else if ( isOption(arg, "-h")
		          || isOption(arg, "--help")
		          || isOption(arg, "-?") )
		{
			printHelp();
			exit(EXIT_SUCCESS);
		}
		else if ( isOption(arg, "-!")
		          || isOption(arg, "--html") )
		{
			launchDefaultBrowser();
			exit(EXIT_SUCCESS);
		}
		else if ( isParamOption(arg, "--html=") )
		{
			string htmlFilePath = getParam(arg, "--html=");
			launchDefaultBrowser(htmlFilePath.c_str());
			exit(EXIT_SUCCESS);
		}
		else if ( isOption(arg, "-V" )
		          || isOption(arg, "--version") )
		{
			printf("Artistic Style Version %s\n", g_version);
			exit(EXIT_SUCCESS);
		}
		else if (arg[0] == '-')
		{
			optionsVector.push_back(arg);
		}
		else // file-name
		{
			standardizePath(arg);
			fileNameVector.push_back(arg);
		}
	}

	// get options file path and name
	if (shouldParseOptionsFile)
	{
		if (optionsFileName.compare("") == 0)
		{
			char* env = getenv("ARTISTIC_STYLE_OPTIONS");
			if (env != NULL)
				setOptionsFileName(env);
		}
		if (optionsFileName.compare("") == 0)
		{
			char* env = getenv("HOME");
			if (env != NULL)
				setOptionsFileName(string(env) + "/.astylerc");
		}
		if (optionsFileName.compare("") == 0)
		{
			char* env = getenv("USERPROFILE");
			if (env != NULL)
				setOptionsFileName(string(env) + "/astylerc");
		}
		if (optionsFileName.compare("") != 0)
			standardizePath(optionsFileName);
	}

	// create the options file vector and parse the options for errors
	ASOptions options(formatter);
	if (optionsFileName.compare("") != 0)
	{
		ifstream optionsIn(optionsFileName.c_str());
		if (optionsIn)
		{
			options.importOptions(optionsIn, fileOptionsVector);
			ok = options.parseOptions(fileOptionsVector,
			                          string(_("Invalid option file options:")));
		}
		else
		{
			if (optionsFileRequired)
				error(_("Cannot open options file"), optionsFileName.c_str());
			optionsFileName.clear();
		}
		optionsIn.close();
	}
	if (!ok)
	{
		(*_err) << options.getOptionErrors() << endl;
		(*_err) << _("For help on options type 'astyle -h'") << endl;
		error();
	}

	// parse the command line options vector for errors
	ok = options.parseOptions(optionsVector,
	                          string(_("Invalid command line options:")));
	if (!ok)
	{
		(*_err) << options.getOptionErrors() << endl;
		(*_err) << _("For help on options type 'astyle -h'") << endl;
		error();
	}
}

// remove a file and check for an error
void ASConsole::removeFile(const char* fileName_, const char* errMsg) const
{
	if (remove(fileName_))
	{
		if (errno == ENOENT)        // no file is OK
			errno = 0;
		if (errno)
		{
			perror("errno message");
			error(errMsg, fileName_);
		}
	}
}

// rename a file and check for an error
void ASConsole::renameFile(const char* oldFileName, const char* newFileName, const char* errMsg) const
{
	int result = rename(oldFileName, newFileName);
	if (result != 0)
	{
		// if file still exists the remove needs more time - retry
		if (errno == EEXIST)
		{
			errno = 0;
			waitForRemove(newFileName);
			result = rename(oldFileName, newFileName);
		}
		if (result != 0)
		{
			perror("errno message");
			error(errMsg, oldFileName);
		}
	}
}

// make sure file separators are correct type (Windows or Linux)
// remove ending file separator
// remove beginning file separator if requested and NOT a complete file path
void ASConsole::standardizePath(string &path, bool removeBeginningSeparator /*false*/) const
{
#ifdef __VMS
	struct FAB fab;
	struct NAML naml;
	char less[NAML$C_MAXRSS];
	char sess[NAM$C_MAXRSS];
	int r0_status;

	// If we are on a VMS system, translate VMS style filenames to unix
	// style.
	fab = cc$rms_fab;
	fab.fab$l_fna = (char*) - 1;
	fab.fab$b_fns = 0;
	fab.fab$l_naml = &naml;
	naml = cc$rms_naml;
	strcpy(sess, path.c_str());
	naml.naml$l_long_filename = (char*)sess;
	naml.naml$l_long_filename_size = path.length();
	naml.naml$l_long_expand = less;
	naml.naml$l_long_expand_alloc = sizeof(less);
	naml.naml$l_esa = sess;
	naml.naml$b_ess = sizeof(sess);
	naml.naml$v_no_short_upcase = 1;
	r0_status = sys$parse(&fab);
	if (r0_status == RMS$_SYN)
	{
		error("File syntax error", path.c_str());
	}
	else
	{
		if (!$VMS_STATUS_SUCCESS(r0_status))
		{
			(void)lib$signal (r0_status);
		}
	}
	less[naml.naml$l_long_expand_size - naml.naml$b_ver] = '\0';
	sess[naml.naml$b_esl - naml.naml$b_ver] = '\0';
	if (naml.naml$l_long_expand_size > naml.naml$b_esl)
	{
		path = decc$translate_vms (less);
	}
	else
	{
		path = decc$translate_vms(sess);
	}
#endif /* __VMS */

	// make sure separators are correct type (Windows or Linux)
	for (size_t i = 0; i < path.length(); i++)
	{
		i = path.find_first_of("/\\", i);
		if (i == string::npos)
			break;
		path[i] = g_fileSeparator;
	}
	// remove beginning separator if requested
	if (removeBeginningSeparator && (path[0] == g_fileSeparator))
		path.erase(0, 1);
}

void ASConsole::printMsg(const char* msg, const string &data) const
{
	if (isQuiet)
		return;
	printf(msg, data.c_str());
}

void ASConsole::printSeparatingLine() const
{
	string line;
	for (size_t i = 0; i < 60; i++)
		line.append("-");
	printMsg("%s\n", line);
}

void ASConsole::printVerboseHeader() const
{
	assert(isVerbose);
	if (isQuiet)
		return;
	// get the date
	struct tm* ptr;
	time_t lt;
	char str[20];
	lt = time(NULL);
	ptr = localtime(&lt);
	strftime(str, 20, "%x", ptr);
	// print the header
	printf("Artistic Style %s     %s\n", g_version, str);
	// print options file
	if (!optionsFileName.empty())
		printf(_("Using default options file %s\n"), optionsFileName.c_str());
}

void ASConsole::printVerboseStats(clock_t startTime) const
{
	assert(isVerbose);
	if (isQuiet)
		return;
	if (hasWildcard)
		printSeparatingLine();
	string formatted = getNumberFormat(filesFormatted);
	string unchanged = getNumberFormat(filesUnchanged);
	printf(_(" %s formatted   %s unchanged   "), formatted.c_str(), unchanged.c_str());

	// show processing time
	clock_t stopTime = clock();
	float secs = (stopTime - startTime) / float (CLOCKS_PER_SEC);
	if (secs < 60)
	{
		if (secs < 2.0)
			printf("%.2f", secs);
		else if (secs < 20.0)
			printf("%.1f", secs);
		else
			printf("%.0f", secs);
		printf("%s", _(" seconds   "));
	}
	else
	{
		// show minutes and seconds if time is greater than one minute
		int min = (int) secs / 60;
		secs -= min * 60;
		int minsec = int (secs + .5);
		printf(_("%d min %d sec   "), min, minsec);
	}

	string lines = getNumberFormat(linesOut);
	printf(_("%s lines\n"), lines.c_str());
}

void ASConsole::sleep(int seconds) const
{
	clock_t endwait;
	endwait = clock_t (clock () + seconds * CLOCKS_PER_SEC);
	while (clock() < endwait) {}
}

bool ASConsole::stringEndsWith(const string &str, const string &suffix) const
{
	int strIndex = (int) str.length() - 1;
	int suffixIndex = (int) suffix.length() - 1;

	while (strIndex >= 0 && suffixIndex >= 0)
	{
		if (tolower(str[strIndex]) != tolower(suffix[suffixIndex]))
			return false;

		--strIndex;
		--suffixIndex;
	}
	// suffix longer than string
	if (strIndex < 0 && suffixIndex >= 0)
		return false;
	return true;
}

void ASConsole::updateExcludeVector(string suffixParam)
{
	excludeVector.push_back(suffixParam);
	standardizePath(excludeVector.back(), true);
	excludeHitsVector.push_back(false);
}

int ASConsole::waitForRemove(const char* newFileName) const
{
	struct stat stBuf;
	int seconds;
	// sleep a max of 20 seconds for the remove
	for (seconds = 1; seconds <= 20; seconds++)
	{
		sleep(1);
		if (stat(newFileName, &stBuf) != 0)
			break;
	}
	errno = 0;
	return seconds;
}

// From The Code Project http://www.codeproject.com/string/wildcmp.asp
// Written by Jack Handy - jakkhandy@hotmail.com
// Modified to compare case insensitive for Windows
int ASConsole::wildcmp(const char* wild, const char* data) const
{
	const char* cp = NULL, *mp = NULL;
	bool cmpval;

	while ((*data) && (*wild != '*'))
	{
		if (!g_isCaseSensitive)
			cmpval = (tolower(*wild) != tolower(*data)) && (*wild != '?');
		else
			cmpval = (*wild != *data) && (*wild != '?');

		if (cmpval)
		{
			return 0;
		}
		wild++;
		data++;
	}

	while (*data)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return 1;
			}
			mp = wild;
			cp = data + 1;
		}
		else
		{
			if (!g_isCaseSensitive)
				cmpval = (tolower(*wild) == tolower(*data) || (*wild == '?'));
			else
				cmpval = (*wild == *data) || (*wild == '?');

			if (cmpval)
			{
				wild++;
				data++;
			}
			else
			{
				wild = mp;
				data = cp++;
			}
		}
	}

	while (*wild == '*')
	{
		wild++;
	}
	return !*wild;
}

void ASConsole::writeFile(const string &fileName_, FileEncoding encoding, ostringstream &out) const
{
	// save date accessed and date modified of original file
	struct stat stBuf;
	bool statErr = false;
	if (stat(fileName_.c_str(), &stBuf) == -1)
		statErr = true;

	// create a backup
	if (!noBackup)
	{
		string origFileName = fileName_ + origSuffix;
		removeFile(origFileName.c_str(), "Cannot remove pre-existing backup file");
		renameFile(fileName_.c_str(), origFileName.c_str(), "Cannot create backup file");
	}

	// write the output file
	ofstream fout(fileName_.c_str(), ios::binary | ios::trunc);
	if (!fout)
		error("Cannot open output file", fileName_.c_str());
	if (encoding == UTF_16LE || encoding == UTF_16BE)
	{
		// convert utf-8 to utf-16
		bool isBigEndian = (encoding == UTF_16BE);
		size_t utf16Size = utf8_16.Utf16LengthFromUtf8(out.str().c_str(), out.str().length());
		char* utf16Out = new char[utf16Size];
		size_t utf16Len = utf8_16.Utf8ToUtf16(const_cast<char*>(out.str().c_str()),
		                                      out.str().length(), isBigEndian, utf16Out);
		assert(utf16Len == utf16Size);
		fout << string(utf16Out, utf16Len);
		delete [] utf16Out;
	}
	else
		fout << out.str();

	fout.close();

	// change date modified to original file date
	// Embarcadero must be linked with cw32mt not cw32
	if (preserveDate)
	{
		if (!statErr)
		{
			struct utimbuf outBuf;
			outBuf.actime = stBuf.st_atime;
			// add ticks so 'make' will recognize a change
			// Visual Studio 2008 needs more than 1
			outBuf.modtime = stBuf.st_mtime + 10;
			if (utime(fileName_.c_str(), &outBuf) == -1)
				statErr = true;
		}
		if (statErr)
		{
			perror("errno message");
			(*_err) << "*********  Cannot preserve file date" << endl;
		}
	}
}

//-----------------------------------------------------------------------------
// ASLibrary class
// used by shared object (DLL) calls
//-----------------------------------------------------------------------------

#else	// ASTYLE_LIB

utf16_t* ASLibrary::formatUtf16(const utf16_t* pSourceIn,		// the source to be formatted
                                const utf16_t* pOptions,		// AStyle options
                                fpError fpErrorHandler,			// error handler function
                                fpAlloc fpMemoryAlloc) const	// memory allocation function)
{
	const char* utf8In = convertUtf16ToUtf8(pSourceIn);
	if (utf8In == NULL)
	{
		fpErrorHandler(121, "Cannot convert input utf-16 to utf-8.");
		return NULL;
	}
	const char* utf8Options = convertUtf16ToUtf8(pOptions);
	if (utf8Options == NULL)
	{
		delete [] utf8In;
		fpErrorHandler(122, "Cannot convert options utf-16 to utf-8.");
		return NULL;
	}
	// call the Artistic Style formatting function
	// cannot use the callers memory allocation here
	char* utf8Out = AStyleMain(utf8In,
	                           utf8Options,
	                           fpErrorHandler,
	                           ASLibrary::tempMemoryAllocation);
	// finished with these
	delete [] utf8In;
	delete [] utf8Options;
	utf8In = NULL;
	utf8Options = NULL;
	// AStyle error has already been sent
	if (utf8Out == NULL)
		return NULL;
	// convert text to wide char and return it
	utf16_t* utf16Out = convertUtf8ToUtf16(utf8Out, fpMemoryAlloc);
	delete [] utf8Out;
	utf8Out = NULL;
	if (utf16Out == NULL)
	{
		fpErrorHandler(123, "Cannot convert output utf-8 to utf-16.");
		return NULL;
	}
	return utf16Out;
}

// STATIC method to allocate temporary memory for AStyle formatting.
// The data will be converted before being returned to the calling program.
char* STDCALL ASLibrary::tempMemoryAllocation(unsigned long memoryNeeded)
{
	char* buffer = new(nothrow) char[memoryNeeded];
	return buffer;
}

/**
 * Convert utf-8 strings to utf16 strings.
 * Memory is allocated by the calling program memory allocation function.
 * The calling function must check for errors.
 */
utf16_t* ASLibrary::convertUtf8ToUtf16(const char* utf8In, fpAlloc fpMemoryAlloc) const
{
	if (utf8In == NULL)
		return NULL;
	char* data = const_cast<char*>(utf8In);
	size_t dataSize = strlen(utf8In);
	bool isBigEndian = utf8_16.getBigEndian();
	// return size is in number of CHARs, not utf16_t
	size_t utf16Size = (utf8_16.Utf16LengthFromUtf8(data, dataSize) + sizeof(utf16_t));
	char* utf16Out = fpMemoryAlloc(utf16Size);
	if (utf16Out == NULL)
		return NULL;
#ifdef NDEBUG
	utf8_16.Utf8ToUtf16(data, dataSize + 1, isBigEndian, utf16Out);
#else
	size_t utf16Len = utf8_16.Utf8ToUtf16(data, dataSize + 1, isBigEndian, utf16Out);
	assert(utf16Len == utf16Size);
#endif
	assert(utf16Size == (utf8_16.utf16len(reinterpret_cast<utf16_t*>(utf16Out)) + 1) * sizeof(utf16_t));
	return reinterpret_cast<utf16_t*>(utf16Out);
}

/**
 * Convert utf16 strings to utf-8.
 * The calling function must check for errors and delete the
 * allocated memory.
 */
char* ASLibrary::convertUtf16ToUtf8(const utf16_t* utf16In) const
{
	if (utf16In == NULL)
		return NULL;
	char* data = reinterpret_cast<char*>(const_cast<utf16_t*>(utf16In));
	// size must be in chars
	size_t dataSize = utf8_16.utf16len(utf16In) * sizeof(utf16_t);
	bool isBigEndian = utf8_16.getBigEndian();
	size_t utf8Size = utf8_16.Utf8LengthFromUtf16(data, dataSize, isBigEndian) + 1;
	char* utf8Out = new(nothrow) char[utf8Size];
	if (utf8Out == NULL)
		return NULL;
#ifdef NDEBUG
	utf8_16.Utf16ToUtf8(data, dataSize + 1, isBigEndian, true, utf8Out);
#else
	size_t utf8Len = utf8_16.Utf16ToUtf8(data, dataSize + 1, isBigEndian, true, utf8Out);
	assert(utf8Len == utf8Size);
#endif
	assert(utf8Size == strlen(utf8Out) + 1);
	return utf8Out;
}

#endif	// ASTYLE_LIB

//-----------------------------------------------------------------------------
// ASOptions class
// used by both console and library builds
//-----------------------------------------------------------------------------

/**
 * parse the options vector
 * optionsVector can be either a fileOptionsVector (options file) or an optionsVector (command line)
 *
 * @return        true if no errors, false if errors
 */
bool ASOptions::parseOptions(vector<string> &optionsVector, const string &errorInfo)
{
	vector<string>::iterator option;
	string arg, subArg;
	optionErrors.clear();

	for (option = optionsVector.begin(); option != optionsVector.end(); ++option)
	{
		arg = *option;

		if (arg.compare(0, 2, "--") == 0)
			parseOption(arg.substr(2), errorInfo);
		else if (arg[0] == '-')
		{
			size_t i;

			for (i = 1; i < arg.length(); ++i)
			{
				if (i > 1
				        && isalpha((unsigned char)arg[i])
				        && arg[i - 1] != 'x')
				{
					// parse the previous option in subArg
					parseOption(subArg, errorInfo);
					subArg = "";
				}
				// append the current option to subArg
				subArg.append(1, arg[i]);
			}
			// parse the last option
			parseOption(subArg, errorInfo);
			subArg = "";
		}
		else
		{
			parseOption(arg, errorInfo);
			subArg = "";
		}
	}
	if (optionErrors.str().length() > 0)
		return false;
	return true;
}

void ASOptions::parseOption(const string &arg, const string &errorInfo)
{
	if ( isOption(arg, "style=allman") || isOption(arg, "style=bsd") || isOption(arg, "style=break") )
	{
		formatter.setFormattingStyle(STYLE_ALLMAN);
	}
	else if ( isOption(arg, "style=java") || isOption(arg, "style=attach") )
	{
		formatter.setFormattingStyle(STYLE_JAVA);
	}
	else if ( isOption(arg, "style=k&r") || isOption(arg, "style=kr") || isOption(arg, "style=k/r") )
	{
		formatter.setFormattingStyle(STYLE_KR);
	}
	else if ( isOption(arg, "style=stroustrup") )
	{
		formatter.setFormattingStyle(STYLE_STROUSTRUP);
	}
	else if ( isOption(arg, "style=whitesmith") )
	{
		formatter.setFormattingStyle(STYLE_WHITESMITH);
	}
	else if (isOption(arg, "style=vtk"))
	{
		formatter.setFormattingStyle(STYLE_VTK);
	}
	else if ( isOption(arg, "style=banner") )
	{
		formatter.setFormattingStyle(STYLE_BANNER);
	}
	else if ( isOption(arg, "style=gnu") )
	{
		formatter.setFormattingStyle(STYLE_GNU);
	}
	else if ( isOption(arg, "style=linux") || isOption(arg, "style=knf") )
	{
		formatter.setFormattingStyle(STYLE_LINUX);
	}
	else if ( isOption(arg, "style=horstmann") )
	{
		formatter.setFormattingStyle(STYLE_HORSTMANN);
	}
	else if ( isOption(arg, "style=1tbs") || isOption(arg, "style=otbs") )
	{
		formatter.setFormattingStyle(STYLE_1TBS);
	}
	else if ( isOption(arg, "style=google") )
	{
		formatter.setFormattingStyle(STYLE_GOOGLE);
	}
	else if ( isOption(arg, "style=pico") )
	{
		formatter.setFormattingStyle(STYLE_PICO);
	}
	else if ( isOption(arg, "style=lisp") || isOption(arg, "style=python") )
	{
		formatter.setFormattingStyle(STYLE_LISP);
	}
	else if ( isParamOption(arg, "A") )
	{
		int style = 0;
		string styleParam = getParam(arg, "A");
		if (styleParam.length() > 0)
			style = atoi(styleParam.c_str());
		if (style == 1)
			formatter.setFormattingStyle(STYLE_ALLMAN);
		else if (style == 2)
			formatter.setFormattingStyle(STYLE_JAVA);
		else if (style == 3)
			formatter.setFormattingStyle(STYLE_KR);
		else if (style == 4)
			formatter.setFormattingStyle(STYLE_STROUSTRUP);
		else if (style == 5)
			formatter.setFormattingStyle(STYLE_WHITESMITH);
		else if (style == 6)
			formatter.setFormattingStyle(STYLE_BANNER);
		else if (style == 7)
			formatter.setFormattingStyle(STYLE_GNU);
		else if (style == 8)
			formatter.setFormattingStyle(STYLE_LINUX);
		else if (style == 9)
			formatter.setFormattingStyle(STYLE_HORSTMANN);
		else if (style == 10)
			formatter.setFormattingStyle(STYLE_1TBS);
		else if (style == 11)
			formatter.setFormattingStyle(STYLE_PICO);
		else if (style == 12)
			formatter.setFormattingStyle(STYLE_LISP);
		else if (style == 14)
			formatter.setFormattingStyle(STYLE_GOOGLE);
		else if (style == 15)
			formatter.setFormattingStyle(STYLE_VTK);
		else
			isOptionError(arg, errorInfo);
	}
	// must check for mode=cs before mode=c !!!
	else if ( isOption(arg, "mode=cs") )
	{
		formatter.setSharpStyle();
		formatter.setModeManuallySet(true);
	}
	else if ( isOption(arg, "mode=c") )
	{
		formatter.setCStyle();
		formatter.setModeManuallySet(true);
	}
	else if ( isOption(arg, "mode=java") )
	{
		formatter.setJavaStyle();
		formatter.setModeManuallySet(true);
	}
	else if ( isParamOption(arg, "t", "indent=tab=") )
	{
		int spaceNum = 4;
		string spaceNumParam = getParam(arg, "t", "indent=tab=");
		if (spaceNumParam.length() > 0)
			spaceNum = atoi(spaceNumParam.c_str());
		if (spaceNum < 2 || spaceNum > 20)
			isOptionError(arg, errorInfo);
		else
		{
			formatter.setTabIndentation(spaceNum, false);
		}
	}
	else if ( isOption(arg, "indent=tab") )
	{
		formatter.setTabIndentation(4);
	}
	else if ( isParamOption(arg, "T", "indent=force-tab=") )
	{
		int spaceNum = 4;
		string spaceNumParam = getParam(arg, "T", "indent=force-tab=");
		if (spaceNumParam.length() > 0)
			spaceNum = atoi(spaceNumParam.c_str());
		if (spaceNum < 2 || spaceNum > 20)
			isOptionError(arg, errorInfo);
		else
		{
			formatter.setTabIndentation(spaceNum, true);
		}
	}
	else if ( isOption(arg, "indent=force-tab") )
	{
		formatter.setTabIndentation(4, true);
	}
	else if ( isParamOption(arg, "xT", "indent=force-tab-x=") )
	{
		int tabNum = 8;
		string tabNumParam = getParam(arg, "xT", "indent=force-tab-x=");
		if (tabNumParam.length() > 0)
			tabNum = atoi(tabNumParam.c_str());
		if (tabNum < 2 || tabNum > 20)
			isOptionError(arg, errorInfo);
		else
		{
			formatter.setForceTabXIndentation(tabNum);
		}
	}
	else if ( isOption(arg, "indent=force-tab-x") )
	{
		formatter.setForceTabXIndentation(8);
	}
	else if ( isParamOption(arg, "s", "indent=spaces=") )
	{
		int spaceNum = 4;
		string spaceNumParam = getParam(arg, "s", "indent=spaces=");
		if (spaceNumParam.length() > 0)
			spaceNum = atoi(spaceNumParam.c_str());
		if (spaceNum < 2 || spaceNum > 20)
			isOptionError(arg, errorInfo);
		else
		{
			formatter.setSpaceIndentation(spaceNum);
		}
	}
	else if ( isOption(arg, "indent=spaces") )
	{
		formatter.setSpaceIndentation(4);
	}
	else if ( isParamOption(arg, "m", "min-conditional-indent=") )
	{
		int minIndent = MINCOND_TWO;
		string minIndentParam = getParam(arg, "m", "min-conditional-indent=");
		if (minIndentParam.length() > 0)
			minIndent = atoi(minIndentParam.c_str());
		if (minIndent >= MINCOND_END)
			isOptionError(arg, errorInfo);
		else
			formatter.setMinConditionalIndentOption(minIndent);
	}
	else if ( isParamOption(arg, "M", "max-instatement-indent=") )
	{
		int maxIndent = 40;
		string maxIndentParam = getParam(arg, "M", "max-instatement-indent=");
		if (maxIndentParam.length() > 0)
			maxIndent = atoi(maxIndentParam.c_str());
		if (maxIndent < 40)
			isOptionError(arg, errorInfo);
		else if (maxIndent > 120)
			isOptionError(arg, errorInfo);
		else
			formatter.setMaxInStatementIndentLength(maxIndent);
	}
	else if ( isOption(arg, "N", "indent-namespaces") )
	{
		formatter.setNamespaceIndent(true);
	}
	else if ( isOption(arg, "C", "indent-classes") )
	{
		formatter.setClassIndent(true);
	}
	else if ( isOption(arg, "xG", "indent-modifiers") )
	{
		formatter.setModifierIndent(true);
	}
	else if ( isOption(arg, "S", "indent-switches") )
	{
		formatter.setSwitchIndent(true);
	}
	else if ( isOption(arg, "K", "indent-cases") )
	{
		formatter.setCaseIndent(true);
	}
	else if ( isOption(arg, "L", "indent-labels") )
	{
		formatter.setLabelIndent(true);
	}
	else if (isOption(arg, "xW", "indent-preproc-block"))
	{
		formatter.setPreprocBlockIndent(true);
	}
	else if ( isOption(arg, "w", "indent-preproc-define") )
	{
		formatter.setPreprocDefineIndent(true);
	}
	else if ( isOption(arg, "xw", "indent-preproc-cond") )
	{
		formatter.setPreprocConditionalIndent(true);
	}
	else if ( isOption(arg, "y", "break-closing-brackets") )
	{
		formatter.setBreakClosingHeaderBracketsMode(true);
	}
	else if ( isOption(arg, "O", "keep-one-line-blocks") )
	{
		formatter.setBreakOneLineBlocksMode(false);
	}
	else if ( isOption(arg, "o", "keep-one-line-statements") )
	{
		formatter.setSingleStatementsMode(false);
	}
	else if ( isOption(arg, "P", "pad-paren") )
	{
		formatter.setParensOutsidePaddingMode(true);
		formatter.setParensInsidePaddingMode(true);
	}
	else if ( isOption(arg, "d", "pad-paren-out") )
	{
		formatter.setParensOutsidePaddingMode(true);
	}
	else if ( isOption(arg, "xd", "pad-first-paren-out") )
	{
		formatter.setParensFirstPaddingMode(true);
	}
	else if ( isOption(arg, "D", "pad-paren-in") )
	{
		formatter.setParensInsidePaddingMode(true);
	}
	else if ( isOption(arg, "H", "pad-header") )
	{
		formatter.setParensHeaderPaddingMode(true);
	}
	else if ( isOption(arg, "U", "unpad-paren") )
	{
		formatter.setParensUnPaddingMode(true);
	}
	else if ( isOption(arg, "p", "pad-oper") )
	{
		formatter.setOperatorPaddingMode(true);
	}
	else if ( isOption(arg, "xe", "delete-empty-lines") )
	{
		formatter.setDeleteEmptyLinesMode(true);
	}
	else if ( isOption(arg, "E", "fill-empty-lines") )
	{
		formatter.setEmptyLineFill(true);
	}
	else if ( isOption(arg, "c", "convert-tabs") )
	{
		formatter.setTabSpaceConversionMode(true);
	}
	else if ( isOption(arg, "xy", "close-templates") )
	{
		formatter.setCloseTemplatesMode(true);
	}
	else if ( isOption(arg, "F", "break-blocks=all") )
	{
		formatter.setBreakBlocksMode(true);
		formatter.setBreakClosingHeaderBlocksMode(true);
	}
	else if ( isOption(arg, "f", "break-blocks") )
	{
		formatter.setBreakBlocksMode(true);
	}
	else if ( isOption(arg, "e", "break-elseifs") )
	{
		formatter.setBreakElseIfsMode(true);
	}
	else if ( isOption(arg, "j", "add-brackets") )
	{
		formatter.setAddBracketsMode(true);
	}
	else if ( isOption(arg, "J", "add-one-line-brackets") )
	{
		formatter.setAddOneLineBracketsMode(true);
	}
	else if ( isOption(arg, "xj", "remove-brackets") )
	{
		formatter.setRemoveBracketsMode(true);
	}
	else if ( isOption(arg, "Y", "indent-col1-comments") )
	{
		formatter.setIndentCol1CommentsMode(true);
	}
	else if ( isOption(arg, "align-pointer=type") )
	{
		formatter.setPointerAlignment(PTR_ALIGN_TYPE);
	}
	else if ( isOption(arg, "align-pointer=middle") )
	{
		formatter.setPointerAlignment(PTR_ALIGN_MIDDLE);
	}
	else if ( isOption(arg, "align-pointer=name") )
	{
		formatter.setPointerAlignment(PTR_ALIGN_NAME);
	}
	else if ( isParamOption(arg, "k") )
	{
		int align = 0;
		string styleParam = getParam(arg, "k");
		if (styleParam.length() > 0)
			align = atoi(styleParam.c_str());
		if (align < 1 || align > 3)
			isOptionError(arg, errorInfo);
		else if (align == 1)
			formatter.setPointerAlignment(PTR_ALIGN_TYPE);
		else if (align == 2)
			formatter.setPointerAlignment(PTR_ALIGN_MIDDLE);
		else if (align == 3)
			formatter.setPointerAlignment(PTR_ALIGN_NAME);
	}
	else if ( isOption(arg, "align-reference=none") )
	{
		formatter.setReferenceAlignment(REF_ALIGN_NONE);
	}
	else if ( isOption(arg, "align-reference=type") )
	{
		formatter.setReferenceAlignment(REF_ALIGN_TYPE);
	}
	else if ( isOption(arg, "align-reference=middle") )
	{
		formatter.setReferenceAlignment(REF_ALIGN_MIDDLE);
	}
	else if ( isOption(arg, "align-reference=name") )
	{
		formatter.setReferenceAlignment(REF_ALIGN_NAME);
	}
	else if ( isParamOption(arg, "W") )
	{
		int align = 0;
		string styleParam = getParam(arg, "W");
		if (styleParam.length() > 0)
			align = atoi(styleParam.c_str());
		if (align < 0 || align > 3)
			isOptionError(arg, errorInfo);
		else if (align == 0)
			formatter.setReferenceAlignment(REF_ALIGN_NONE);
		else if (align == 1)
			formatter.setReferenceAlignment(REF_ALIGN_TYPE);
		else if (align == 2)
			formatter.setReferenceAlignment(REF_ALIGN_MIDDLE);
		else if (align == 3)
			formatter.setReferenceAlignment(REF_ALIGN_NAME);
	}
	else if ( isParamOption(arg, "max-code-length=") )
	{
		int maxLength = 50;
		string maxLengthParam = getParam(arg, "max-code-length=");
		if (maxLengthParam.length() > 0)
			maxLength = atoi(maxLengthParam.c_str());
		if (maxLength < 50)
			isOptionError(arg, errorInfo);
		else if (maxLength > 200)
			isOptionError(arg, errorInfo);
		else
			formatter.setMaxCodeLength(maxLength);
	}
	else if ( isParamOption(arg, "xC") )
	{
		int maxLength = 50;
		string maxLengthParam = getParam(arg, "xC");
		if (maxLengthParam.length() > 0)
			maxLength = atoi(maxLengthParam.c_str());
		if (maxLength > 200)
			isOptionError(arg, errorInfo);
		else
			formatter.setMaxCodeLength(maxLength);
	}
	else if ( isOption(arg, "xL", "break-after-logical") )
	{
		formatter.setBreakAfterMode(true);
	}
	else if ( isOption(arg, "xc", "attach-classes") )
	{
		formatter.setAttachClass(true);
	}
	else if ( isOption(arg, "xk", "attach-extern-c") )
	{
		formatter.setAttachExternC(true);
	}
	else if ( isOption(arg, "xn", "attach-namespaces") )
	{
		formatter.setAttachNamespace(true);
	}
	else if ( isOption(arg, "xl", "attach-inlines") )
	{
		formatter.setAttachInline(true);
	}
	else if ( isOption(arg, "xp", "remove-comment-prefix") )
	{
		formatter.setStripCommentPrefix(true);
	}
	// Objective-C options
	else if ( isOption(arg, "xM", "align-method-colon") )
	{
		formatter.setAlignMethodColon(true);
	}
	else if ( isOption(arg, "xQ", "pad-method-prefix") )
	{
		formatter.setMethodPrefixPaddingMode(true);
	}
	else if ( isOption(arg, "xR", "unpad-method-prefix") )
	{
		formatter.setMethodPrefixUnPaddingMode(true);
	}
	else if ( isOption(arg, "xP0", "pad-method-colon=none") )
	{
		formatter.setObjCColonPaddingMode(COLON_PAD_NONE);
	}
	else if ( isOption(arg, "xP1", "pad-method-colon=all") )
	{
		formatter.setObjCColonPaddingMode(COLON_PAD_ALL);
	}
	else if ( isOption(arg, "xP2", "pad-method-colon=after") )
	{
		formatter.setObjCColonPaddingMode(COLON_PAD_AFTER);
	}
	else if ( isOption(arg, "xP3", "pad-method-colon=before") )
	{
		formatter.setObjCColonPaddingMode(COLON_PAD_BEFORE);
	}
	// depreciated options ////////////////////////////////////////////////////////////////////////////////////////////
	else if ( isOption(arg, "indent-preprocessor") )	// depreciated release 2.04
	{
		formatter.setPreprocDefineIndent(true);
	}
	else if ( isOption(arg, "style=ansi") )				// depreciated release 2.05
	{
		formatter.setFormattingStyle(STYLE_ALLMAN);
	}
//  NOTE: Removed in release 2.04.
//	else if ( isOption(arg, "b", "brackets=break") )
//	{
//		formatter.setBracketFormatMode(BREAK_MODE);
//	}
//	else if ( isOption(arg, "a", "brackets=attach") )
//	{
//		formatter.setBracketFormatMode(ATTACH_MODE);
//	}
//	else if ( isOption(arg, "l", "brackets=linux") )
//	{
//		formatter.setBracketFormatMode(LINUX_MODE);
//	}
//	else if ( isOption(arg, "u", "brackets=stroustrup") )
//	{
//		formatter.setBracketFormatMode(STROUSTRUP_MODE);
//	}
//	else if ( isOption(arg, "g", "brackets=run-in") )
//	{
//		formatter.setBracketFormatMode(RUN_IN_MODE);
//	}
	// end depreciated options ////////////////////////////////////////////////////////////////////////////////////////
#ifdef ASTYLE_LIB
	// End of options used by GUI /////////////////////////////////////////////////////////////////////////////////////
	else
		isOptionError(arg, errorInfo);
#else
	// Options used by only console ///////////////////////////////////////////////////////////////////////////////////
	else if ( isOption(arg, "n", "suffix=none") )
	{
		g_console->setNoBackup(true);
	}
	else if ( isParamOption(arg, "suffix=") )
	{
		string suffixParam = getParam(arg, "suffix=");
		if (suffixParam.length() > 0)
		{
			g_console->setOrigSuffix(suffixParam);
		}
	}
	else if ( isParamOption(arg, "exclude=") )
	{
		string suffixParam = getParam(arg, "exclude=");
		if (suffixParam.length() > 0)
			g_console->updateExcludeVector(suffixParam);
	}
	else if ( isOption(arg, "r", "R") || isOption(arg, "recursive") )
	{
		g_console->setIsRecursive(true);
	}
	else if (isOption(arg, "dry-run"))
	{
		g_console->setIsDryRun(true);
	}
	else if ( isOption(arg, "Z", "preserve-date") )
	{
		g_console->setPreserveDate(true);
	}
	else if ( isOption(arg, "v", "verbose") )
	{
		g_console->setIsVerbose(true);
	}
	else if ( isOption(arg, "Q", "formatted") )
	{
		g_console->setIsFormattedOnly(true);
	}
	else if ( isOption(arg, "q", "quiet") )
	{
		g_console->setIsQuiet(true);
	}
	else if ( isOption(arg, "i", "ignore-exclude-errors") )
	{
		g_console->setIgnoreExcludeErrors(true);
	}
	else if ( isOption(arg, "xi", "ignore-exclude-errors-x") )
	{
		g_console->setIgnoreExcludeErrorsAndDisplay(true);
	}
	else if ( isOption(arg, "X", "errors-to-stdout") )
	{
		_err = &cout;
	}
	else if ( isOption(arg, "lineend=windows") )
	{
		formatter.setLineEndFormat(LINEEND_WINDOWS);
	}
	else if ( isOption(arg, "lineend=linux") )
	{
		formatter.setLineEndFormat(LINEEND_LINUX);
	}
	else if ( isOption(arg, "lineend=macold") )
	{
		formatter.setLineEndFormat(LINEEND_MACOLD);
	}
	else if ( isParamOption(arg, "z") )
	{
		int lineendType = 0;
		string lineendParam = getParam(arg, "z");
		if (lineendParam.length() > 0)
			lineendType = atoi(lineendParam.c_str());
		if (lineendType < 1 || lineendType > 3)
			isOptionError(arg, errorInfo);
		else if (lineendType == 1)
			formatter.setLineEndFormat(LINEEND_WINDOWS);
		else if (lineendType == 2)
			formatter.setLineEndFormat(LINEEND_LINUX);
		else if (lineendType == 3)
			formatter.setLineEndFormat(LINEEND_MACOLD);
	}
	else
		isOptionError(arg, errorInfo);
#endif
}	// End of parseOption function

// Parse options from the options file.
void ASOptions::importOptions(istream &in, vector<string> &optionsVector)
{
	char ch;
	bool isInQuote = false;
	char quoteChar = ' ';
	string currentToken;

	while (in)
	{
		currentToken = "";
		do
		{
			in.get(ch);
			if (in.eof())
				break;
			// treat '#' as line comments
			if (ch == '#')
				while (in)
				{
					in.get(ch);
					if (ch == '\n' || ch == '\r')
						break;
				}

			// break options on new-lines, tabs, commas, or spaces
			// remove quotes from output
			if (in.eof() || ch == '\n' || ch == '\r' || ch == '\t' || ch == ',')
				break;
			if (ch == ' ' && !isInQuote)
				break;
			if (ch == quoteChar && isInQuote)
				break;
			if (ch == '"' || ch == '\'')
			{
				isInQuote = true;
				quoteChar = ch;
				continue;
			}
			currentToken.append(1, ch);
		}
		while (in);

		if (currentToken.length() != 0)
			optionsVector.push_back(currentToken);
		isInQuote = false;
	}
}

string ASOptions::getOptionErrors() const
{
	return optionErrors.str();
}

string ASOptions::getParam(const string &arg, const char* op)
{
	return arg.substr(strlen(op));
}

string ASOptions::getParam(const string &arg, const char* op1, const char* op2)
{
	return isParamOption(arg, op1) ? getParam(arg, op1) : getParam(arg, op2);
}

bool ASOptions::isOption(const string &arg, const char* op)
{
	return arg.compare(op) == 0;
}

bool ASOptions::isOption(const string &arg, const char* op1, const char* op2)
{
	return (isOption(arg, op1) || isOption(arg, op2));
}

void ASOptions::isOptionError(const string &arg, const string &errorInfo)
{
	if (optionErrors.str().length() == 0)
		optionErrors << errorInfo << endl;   // need main error message
	optionErrors << arg << endl;
}

bool ASOptions::isParamOption(const string &arg, const char* option)
{
	bool retVal = arg.compare(0, strlen(option), option) == 0;
	// if comparing for short option, 2nd char of arg must be numeric
	if (retVal && strlen(option) == 1 && arg.length() > 1)
		if (!isdigit((unsigned char)arg[1]))
			retVal = false;
	return retVal;
}

bool ASOptions::isParamOption(const string &arg, const char* option1, const char* option2)
{
	return isParamOption(arg, option1) || isParamOption(arg, option2);
}

//----------------------------------------------------------------------------
// Utf8_16 class
//----------------------------------------------------------------------------

// Return true if an int is big endian.
bool Utf8_16::getBigEndian() const
{
	short int word = 0x0001;
	char* byte = (char*) &word;
	return (byte[0] ? false : true);
}

// Swap the two low order bytes of a 16 bit integer value.
int Utf8_16::swap16bit(int value) const
{
	return ( ((value & 0xff) << 8) | ((value & 0xff00) >> 8) );
}

// Return the length of a utf-16 C string.
// The length is in number of utf16_t.
size_t Utf8_16::utf16len(const utf16* utf16In) const
{
	size_t length = 0;
	while (*utf16In++ != '\0')
		length++;
	return length;
}

// Adapted from SciTE UniConversion.cxx.
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// Modified for Artistic Style by Jim Pattee.
// Compute the length of an output utf-8 file given a utf-16 file.
// Input inLen is the size in BYTES (not wchar_t).
size_t Utf8_16::Utf8LengthFromUtf16(const char* utf16In, size_t inLen, bool isBigEndian) const
{
	size_t len = 0;
	size_t wcharLen = inLen / 2;
	const short* uptr = reinterpret_cast<const short*>(utf16In);
	for (size_t i = 0; i < wcharLen && uptr[i];)
	{
		size_t uch = isBigEndian ? swap16bit(uptr[i]) : uptr[i];
		if (uch < 0x80)
			len++;
		else if (uch < 0x800)
			len += 2;
		else if ((uch >= SURROGATE_LEAD_FIRST) && (uch <= SURROGATE_TRAIL_LAST))
		{
			len += 4;
			i++;
		}
		else
			len += 3;
		i++;
	}
	return len;
}

// Adapted from SciTE Utf8_16.cxx.
// Copyright (C) 2002 Scott Kirkwood.
// Modified for Artistic Style by Jim Pattee.
// Convert a utf-8 file to utf-16.
size_t Utf8_16::Utf8ToUtf16(char* utf8In, size_t inLen, bool isBigEndian, char* utf16Out) const
{
	int nCur = 0;
	ubyte* pRead = reinterpret_cast<ubyte*>(utf8In);
	utf16* pCur = reinterpret_cast<utf16*>(utf16Out);
	const ubyte* pEnd = pRead + inLen;
	const utf16* pCurStart = pCur;
	eState state = eStart;

	// the BOM will automatically be converted to utf-16
	while (pRead < pEnd)
	{
		switch (state)
		{
		case eStart:
			if ((0xF0 & *pRead) == 0xF0)
			{
				nCur = (0x7 & *pRead) << 18;
				state = eSecondOf4Bytes;
			}
			else if ((0xE0 & *pRead) == 0xE0)
			{
				nCur = (~0xE0 & *pRead) << 12;
				state = ePenultimate;
			}
			else if ((0xC0 & *pRead) == 0xC0)
			{
				nCur = (~0xC0 & *pRead) << 6;
				state = eFinal;
			}
			else
			{
				nCur = *pRead;
				state = eStart;
			}
			break;
		case eSecondOf4Bytes:
			nCur |= (0x3F & *pRead) << 12;
			state = ePenultimate;
			break;
		case ePenultimate:
			nCur |= (0x3F & *pRead) << 6;
			state = eFinal;
			break;
		case eFinal:
			nCur |= (0x3F & *pRead);
			state = eStart;
			break;
		}
		++pRead;

		if (state == eStart)
		{
			int codePoint = nCur;
			if (codePoint >= SURROGATE_FIRST_VALUE)
			{
				codePoint -= SURROGATE_FIRST_VALUE;
				int lead = (codePoint >> 10) + SURROGATE_LEAD_FIRST;
				*pCur++ = static_cast<utf16>(isBigEndian ? swap16bit(lead) : lead);
				int trail = (codePoint & 0x3ff) + SURROGATE_TRAIL_FIRST;
				*pCur++ = static_cast<utf16>(isBigEndian ? swap16bit(trail) : trail);
			}
			else
				*pCur++ = static_cast<utf16>(isBigEndian ? swap16bit(codePoint) : codePoint);
		}
	}
	// return value is the output length in BYTES (not wchar_t)
	return (pCur - pCurStart) * 2;
}

// Adapted from SciTE UniConversion.cxx.
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// Modified for Artistic Style by Jim Pattee.
// Compute the length of an output utf-16 file given a utf-8 file.
// Return value is the size in BYTES (not wchar_t).
size_t Utf8_16::Utf16LengthFromUtf8(const char* utf8In, size_t len) const
{
	size_t ulen = 0;
	size_t charLen;
	for (size_t i = 0; i < len;)
	{
		unsigned char ch = static_cast<unsigned char>(utf8In[i]);
		if (ch < 0x80)
			charLen = 1;
		else if (ch < 0x80 + 0x40 + 0x20)
			charLen = 2;
		else if (ch < 0x80 + 0x40 + 0x20 + 0x10)
			charLen = 3;
		else
		{
			charLen = 4;
			ulen++;
		}
		i += charLen;
		ulen++;
	}
	// return value is the length in bytes (not wchar_t)
	return ulen * 2;
}

// Adapted from SciTE Utf8_16.cxx.
// Copyright (C) 2002 Scott Kirkwood.
// Modified for Artistic Style by Jim Pattee.
// Convert a utf-16 file to utf-8.
size_t Utf8_16::Utf16ToUtf8(char* utf16In, size_t inLen, bool isBigEndian,
                            bool firstBlock, char* utf8Out) const
{
	int nCur16 = 0;
	int nCur = 0;
	ubyte* pRead = reinterpret_cast<ubyte*>(utf16In);
	ubyte* pCur = reinterpret_cast<ubyte*>(utf8Out);
	const ubyte* pEnd = pRead + inLen;
	const ubyte* pCurStart = pCur;
	static eState state = eStart;	// state is retained for subsequent blocks
	if (firstBlock)
		state = eStart;

	// the BOM will automatically be converted to utf-8
	while (pRead < pEnd)
	{
		switch (state)
		{
		case eStart:
			if (pRead >= pEnd)
			{
				++pRead;
				break;
			}
			if (isBigEndian)
			{
				nCur16 = static_cast<utf16>(*pRead++ << 8);
				nCur16 |= static_cast<utf16>(*pRead);
			}
			else
			{
				nCur16 = *pRead++;
				nCur16 |= static_cast<utf16>(*pRead << 8);
			}
			if (nCur16 >= SURROGATE_LEAD_FIRST && nCur16 <= SURROGATE_LEAD_LAST)
			{
				++pRead;
				int trail;
				if (isBigEndian)
				{
					trail = static_cast<utf16>(*pRead++ << 8);
					trail |= static_cast<utf16>(*pRead);
				}
				else
				{
					trail = *pRead++;
					trail |= static_cast<utf16>(*pRead << 8);
				}
				nCur16 = (((nCur16 & 0x3ff) << 10) | (trail & 0x3ff)) + SURROGATE_FIRST_VALUE;
			}
			++pRead;

			if (nCur16 < 0x80)
			{
				nCur = static_cast<ubyte>(nCur16 & 0xFF);
				state = eStart;
			}
			else if (nCur16 < 0x800)
			{
				nCur = static_cast<ubyte>(0xC0 | (nCur16 >> 6));
				state = eFinal;
			}
			else if (nCur16 < SURROGATE_FIRST_VALUE)
			{
				nCur = static_cast<ubyte>(0xE0 | (nCur16 >> 12));
				state = ePenultimate;
			}
			else
			{
				nCur = static_cast<ubyte>(0xF0 | (nCur16 >> 18));
				state = eSecondOf4Bytes;
			}
			break;
		case eSecondOf4Bytes:
			nCur = static_cast<ubyte>(0x80 | ((nCur16 >> 12) & 0x3F));
			state = ePenultimate;
			break;
		case ePenultimate:
			nCur = static_cast<ubyte>(0x80 | ((nCur16 >> 6) & 0x3F));
			state = eFinal;
			break;
		case eFinal:
			nCur = static_cast<ubyte>(0x80 | (nCur16 & 0x3F));
			state = eStart;
			break;
		}
		*pCur++ = static_cast<ubyte>(nCur);
	}
	return pCur - pCurStart;
}

//----------------------------------------------------------------------------

}   // end of astyle namespace

//----------------------------------------------------------------------------

using namespace astyle;

//----------------------------------------------------------------------------
// ASTYLE_JNI functions for Java library builds
//----------------------------------------------------------------------------

#ifdef ASTYLE_JNI

// called by a java program to get the version number
// the function name is constructed from method names in the calling java program
extern "C"  EXPORT
jstring STDCALL Java_AStyleInterface_AStyleGetVersion(JNIEnv* env, jclass)
{
	return env->NewStringUTF(g_version);
}

// called by a java program to format the source code
// the function name is constructed from method names in the calling java program
extern "C"  EXPORT
jstring STDCALL Java_AStyleInterface_AStyleMain(JNIEnv* env,
                                                jobject obj,
                                                jstring textInJava,
                                                jstring optionsJava)
{
	g_env = env;                                // make object available globally
	g_obj = obj;                                // make object available globally

	jstring textErr = env->NewStringUTF("");    // zero length text returned if an error occurs

	// get the method ID
	jclass cls = env->GetObjectClass(obj);
	g_mid = env->GetMethodID(cls, "ErrorHandler", "(ILjava/lang/String;)V");
	if (g_mid == 0)
	{
		cout << "Cannot find java method ErrorHandler" << endl;
		return textErr;
	}

	// convert jstring to char*
	const char* textIn = env->GetStringUTFChars(textInJava, NULL);
	const char* options = env->GetStringUTFChars(optionsJava, NULL);

	// call the C++ formatting function
	char* textOut = AStyleMain(textIn, options, javaErrorHandler, javaMemoryAlloc);
	// if an error message occurred it was displayed by errorHandler
	if (textOut == NULL)
		return textErr;

	// release memory
	jstring textOutJava = env->NewStringUTF(textOut);
	delete [] textOut;
	env->ReleaseStringUTFChars(textInJava, textIn);
	env->ReleaseStringUTFChars(optionsJava, options);

	return textOutJava;
}

// Call the Java error handler
void STDCALL javaErrorHandler(int errorNumber, const char* errorMessage)
{
	jstring errorMessageJava = g_env->NewStringUTF(errorMessage);
	g_env->CallVoidMethod(g_obj, g_mid, errorNumber, errorMessageJava);
}

// Allocate memory for the formatted text
char* STDCALL javaMemoryAlloc(unsigned long memoryNeeded)
{
	// error condition is checked after return from AStyleMain
	char* buffer = new(nothrow) char[memoryNeeded];
	return buffer;
}
#endif	// ASTYLE_JNI

//----------------------------------------------------------------------------
// Entry point for AStyleMainUtf16 library builds
//----------------------------------------------------------------------------

#ifdef ASTYLE_LIB

extern "C" EXPORT utf16_t* STDCALL AStyleMainUtf16(const utf16_t* pSourceIn,	// the source to be formatted
                                                   const utf16_t* pOptions,		// AStyle options
                                                   fpError fpErrorHandler,		// error handler function
                                                   fpAlloc fpMemoryAlloc)		// memory allocation function
{
	if (fpErrorHandler == NULL)         // cannot display a message if no error handler
		return NULL;

	if (pSourceIn == NULL)
	{
		fpErrorHandler(101, "No pointer to source input.");
		return NULL;
	}
	if (pOptions == NULL)
	{
		fpErrorHandler(102, "No pointer to AStyle options.");
		return NULL;
	}
	if (fpMemoryAlloc == NULL)
	{
		fpErrorHandler(103, "No pointer to memory allocation function.");
		return NULL;
	}
#ifndef _WIN32
	// check size of utf16_t on Linux
	int sizeCheck = 2;
	if (sizeof(utf16_t) != sizeCheck)
	{
		fpErrorHandler(104, "Unsigned short is not the correct size.");
		return NULL;
	}
#endif

	ASLibrary library;
	utf16_t* utf16Out = library.formatUtf16(pSourceIn, pOptions, fpErrorHandler, fpMemoryAlloc);
	return utf16Out;
}

//----------------------------------------------------------------------------
// ASTYLE_LIB entry point for library builds
//----------------------------------------------------------------------------
/*
 * IMPORTANT VC DLL linker for WIN32 must have the parameter  /EXPORT:AStyleMain=_AStyleMain@16
 *                                                            /EXPORT:AStyleGetVersion=_AStyleGetVersion@0
 * No /EXPORT is required for x64
 */
extern "C" EXPORT char* STDCALL AStyleMain(const char* pSourceIn,		// the source to be formatted
                                           const char* pOptions,		// AStyle options
                                           fpError fpErrorHandler,		// error handler function
                                           fpAlloc fpMemoryAlloc)		// memory allocation function
{
	if (fpErrorHandler == NULL)         // cannot display a message if no error handler
		return NULL;

	if (pSourceIn == NULL)
	{
		fpErrorHandler(101, "No pointer to source input.");
		return NULL;
	}
	if (pOptions == NULL)
	{
		fpErrorHandler(102, "No pointer to AStyle options.");
		return NULL;
	}
	if (fpMemoryAlloc == NULL)
	{
		fpErrorHandler(103, "No pointer to memory allocation function.");
		return NULL;
	}

	ASFormatter formatter;
	ASOptions options(formatter);

	vector<string> optionsVector;
	istringstream opt(pOptions);

	options.importOptions(opt, optionsVector);

	bool ok = options.parseOptions(optionsVector, "Invalid Artistic Style options:");
	if (!ok)
		fpErrorHandler(130, options.getOptionErrors().c_str());

	istringstream in(pSourceIn);
	ASStreamIterator<istringstream> streamIterator(&in);
	ostringstream out;
	formatter.init(&streamIterator);

	while (formatter.hasMoreLines())
	{
		out << formatter.nextLine();
		if (formatter.hasMoreLines())
			out << streamIterator.getOutputEOL();
		else
		{
			// this can happen if the file if missing a closing bracket and break-blocks is requested
			if (formatter.getIsLineReady())
			{
				out << streamIterator.getOutputEOL();
				out << formatter.nextLine();
			}
		}
	}

	unsigned long textSizeOut = out.str().length();
	char* pTextOut = fpMemoryAlloc(textSizeOut + 1);     // call memory allocation function
	if (pTextOut == NULL)
	{
		fpErrorHandler(120, "Allocation failure on output.");
		return NULL;
	}

	strcpy(pTextOut, out.str().c_str());
#ifndef NDEBUG
	// The checksum is an assert in the console build and ASFormatter.
	// This error returns the incorrectly formatted file to the editor.
	// This is done to allow the file to be saved for debugging purposes.
	if (formatter.getChecksumDiff() != 0)
		fpErrorHandler(220,
		               "Checksum error.\n"
		               "The incorrectly formatted file will be returned for debugging.");
#endif
	return pTextOut;
}

extern "C" EXPORT const char* STDCALL AStyleGetVersion(void)
{
	return g_version;
}

// ASTYLECON_LIB is defined to exclude "main" from the test programs
#elif !defined(ASTYLECON_LIB)

//----------------------------------------------------------------------------
// main function for ASConsole build
//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
	// create objects
	ASFormatter formatter;
	g_console = new ASConsole(formatter);

	// process command line and options file
	// build the vectors fileNameVector, optionsVector, and fileOptionsVector
	vector<string> argvOptions;
	argvOptions = g_console->getArgvOptions(argc, argv);
	g_console->processOptions(argvOptions);

	// if no files have been given, use cin for input and cout for output
	if (g_console->fileNameVectorIsEmpty())
	{
		g_console->formatCinToCout();
		return EXIT_SUCCESS;
	}

	// process entries in the fileNameVector
	g_console->processFiles();

	delete g_console;
	return EXIT_SUCCESS;
}

#endif	// ASTYLE_LIB
