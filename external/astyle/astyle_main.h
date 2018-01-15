// astyle_main.h
// Copyright (c) 2017 by Jim Pattee <jimp03@email.com>.
// This code is licensed under the MIT License.
// License.md describes the conditions under which this software may be distributed.

#ifndef ASTYLE_MAIN_H
#define ASTYLE_MAIN_H

//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------

#include "astyle.h"

#include <sstream>
#include <ctime>

#if defined(__BORLANDC__) && __BORLANDC__ < 0x0650
	// Embarcadero needs this for the following utime.h
	// otherwise "struct utimbuf" gets an error on time_t
	// 0x0650 for C++Builder XE3
	using std::time_t;
#endif

#if defined(_MSC_VER)
	#include <sys/utime.h>
	#include <sys/stat.h>
#else
	#include <utime.h>
	#include <sys/stat.h>
#endif                         // end compiler checks

#ifdef ASTYLE_JNI
	#include <jni.h>
	#ifndef ASTYLE_LIB    // ASTYLE_LIB must be defined for ASTYLE_JNI
		#define ASTYLE_LIB
	#endif
#endif  //  ASTYLE_JNI

#ifndef ASTYLE_LIB
	// for console build only
	#include "ASLocalizer.h"
	#define _(a) localizer.settext(a)
#endif	// ASTYLE_LIB

//-----------------------------------------------------------------------------
// declarations
//-----------------------------------------------------------------------------

// for G++ implementation of string.compare:
#if defined(__GNUC__) && __GNUC__ < 3
	#error - Use GNU C compiler release 3 or higher
#endif

// for getenv and localtime
#if defined(_MSC_VER)
	#pragma warning(disable: 4996)  // secure version deprecation warnings
#endif

// for namespace problem in version 5.0
#if defined(_MSC_VER) && _MSC_VER < 1200        // check for V6.0
	#error - Use Microsoft compiler version 6 or higher
#endif

#ifdef __clang__
	#pragma clang diagnostic ignored "-Wdeprecated-declarations"  // getenv, localtime
	#pragma clang diagnostic ignored "-Wmissing-braces"
#endif

// for mingw BOM, UTF-16, and Unicode functions
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
	#if (__MINGW32_MAJOR_VERSION > 3) || \
		((__MINGW32_MAJOR_VERSION == 3) && (__MINGW32_MINOR_VERSION < 16))
		#error - Use MinGW compiler version 4 or higher
	#endif
#endif

#ifdef ASTYLE_LIB

	// define STDCALL and EXPORT for Windows
	// MINGW defines STDCALL in Windows.h (actually windef.h)
	// EXPORT has no value if ASTYLE_NO_EXPORT is defined
	#ifdef _WIN32
		#ifndef STDCALL
			#define STDCALL __stdcall
		#endif
		// define this to prevent compiler warning and error messages
		#ifdef ASTYLE_NO_EXPORT
			#define EXPORT
		#else
			#define EXPORT __declspec(dllexport)
		#endif
		// define STDCALL and EXPORT for non-Windows
		// visibility attribute allows "-fvisibility=hidden" compiler option
	#else
		#define STDCALL
		#if __GNUC__ >= 4
			#define EXPORT __attribute__ ((visibility ("default")))
		#else
			#define EXPORT
		#endif
	#endif	// #ifdef _WIN32

	// define utf-16 bit text for the platform
	typedef unsigned short utf16_t;
	// define pointers to callback error handler and memory allocation
	typedef void (STDCALL* fpError)(int errorNumber, const char* errorMessage);
	typedef char* (STDCALL* fpAlloc)(unsigned long memoryNeeded);

#endif  // #ifdef ASTYLE_LIB

//----------------------------------------------------------------------------
// astyle namespace
//----------------------------------------------------------------------------

namespace astyle {
//
//----------------------------------------------------------------------------
// ASStreamIterator class
// typename will be istringstream for GUI and istream otherwise
// ASSourceIterator is an abstract class defined in astyle.h
//----------------------------------------------------------------------------

template<typename T>
class ASStreamIterator : public ASSourceIterator
{
public:
	bool checkForEmptyLine;

	// function declarations
	explicit ASStreamIterator(T* in);
	virtual ~ASStreamIterator();
	bool getLineEndChange(int lineEndFormat) const;
	int  getStreamLength() const;
	string nextLine(bool emptyLineWasDeleted);
	string peekNextLine();
	void peekReset();
	void saveLastInputLine();
	streamoff tellg();

private:
	ASStreamIterator(const ASStreamIterator& copy);       // copy constructor not to be implemented
	ASStreamIterator& operator=(ASStreamIterator&);       // assignment operator not to be implemented
	T* inStream;            // pointer to the input stream
	string buffer;          // current input line
	string prevBuffer;      // previous input line
	string outputEOL;       // next output end of line char
	int eolWindows;         // number of Windows line endings, CRLF
	int eolLinux;           // number of Linux line endings, LF
	int eolMacOld;          // number of old Mac line endings. CR
	streamoff streamLength; // length of the input file stream
	streamoff peekStart;    // starting position for peekNextLine
	bool prevLineDeleted;   // the previous input line was deleted

public:	// inline functions
	bool compareToInputBuffer(const string& nextLine_) const
	{ return (nextLine_ == prevBuffer); }
	const string& getOutputEOL() const { return outputEOL; }
	bool hasMoreLines() const { return !inStream->eof(); }
};

//----------------------------------------------------------------------------
// ASEncoding class for utf8/16 conversions
// used by both console and library builds
//----------------------------------------------------------------------------

class ASEncoding
{
private:
	typedef unsigned short utf16; // 16 bits
	typedef unsigned char utf8;   // 8 bits
	typedef unsigned char ubyte;  // 8 bits
	enum { SURROGATE_LEAD_FIRST = 0xD800 };
	enum { SURROGATE_LEAD_LAST = 0xDBFF };
	enum { SURROGATE_TRAIL_FIRST = 0xDC00 };
	enum { SURROGATE_TRAIL_LAST = 0xDFFF };
	enum { SURROGATE_FIRST_VALUE = 0x10000 };
	enum eState { eStart, eSecondOf4Bytes, ePenultimate, eFinal };

public:
	bool   getBigEndian() const;
	int    swap16bit(int value) const;
	size_t utf16len(const utf16* utf16In) const;
	size_t utf8LengthFromUtf16(const char* utf16In, size_t inLen, bool isBigEndian) const;
	size_t utf8ToUtf16(char* utf8In, size_t inLen, bool isBigEndian, char* utf16Out) const;
	size_t utf16LengthFromUtf8(const char* utf8In, size_t len) const;
	size_t utf16ToUtf8(char* utf16In, size_t inLen, bool isBigEndian,
	                   bool firstBlock, char* utf8Out) const;
};

//----------------------------------------------------------------------------
// ASOptions class for options processing
// used by both console and library builds
//----------------------------------------------------------------------------
class ASConsole;

class ASOptions
{
public:
#ifdef ASTYLE_LIB
	ASOptions(ASFormatter& formatterArg);
#else
	ASOptions(ASFormatter& formatterArg, ASConsole& consoleArg);
#endif
	string getOptionErrors() const;
	void importOptions(istream& in, vector<string>& optionsVector);
	bool parseOptions(vector<string>& optionsVector, const string& errorInfo);

private:
	// variables
	ASFormatter& formatter;
	stringstream optionErrors;		// option error messages
#ifndef ASTYLE_LIB
	ASConsole&   console;			// DO NOT USE for ASTYLE_LIB
#endif

	// functions
	ASOptions(const ASOptions&);           // copy constructor not to be implemented
	ASOptions& operator=(ASOptions&);      // assignment operator not to be implemented
	string getParam(const string& arg, const char* op);
	string getParam(const string& arg, const char* op1, const char* op2);
	bool isOption(const string& arg, const char* op);
	bool isOption(const string& arg, const char* op1, const char* op2);
	void isOptionError(const string& arg, const string& errorInfo);
	bool isParamOption(const string& arg, const char* option);
	bool isParamOption(const string& arg, const char* option1, const char* option2);
	void parseOption(const string& arg, const string& errorInfo);
};

#ifndef	ASTYLE_LIB

//----------------------------------------------------------------------------
// ASConsole class for console build
//----------------------------------------------------------------------------

class ASConsole
{
private:    // variables
	ASFormatter& formatter;             // reference to the ASFormatter object
	ASLocalizer localizer;              // ASLocalizer object
	ostream* errorStream;               // direct error messages to cerr or cout
	// command line options
	bool isRecursive;                   // recursive option
	bool isDryRun;                      // dry-run option
	bool noBackup;                      // suffix=none option
	bool preserveDate;                  // preserve-date option
	bool isVerbose;                     // verbose option
	bool isQuiet;                       // quiet option
	bool isFormattedOnly;               // formatted lines only option
	bool ignoreExcludeErrors;           // don't abort on unmatched excludes
	bool ignoreExcludeErrorsDisplay;    // don't display unmatched excludes
	bool optionsFileRequired;           // options= option
	bool useAscii;                      // ascii option
	// other variables
	bool bypassBrowserOpen;             // don't open the browser on html options
	bool hasWildcard;                   // file name includes a wildcard
	size_t mainDirectoryLength;         // directory length to be excluded in displays
	bool filesAreIdentical;             // input and output files are identical
	int  filesFormatted;                // number of files formatted
	int  filesUnchanged;                // number of files unchanged
	bool lineEndsMixed;                 // output has mixed line ends
	int  linesOut;                      // number of output lines

	ASEncoding utf8_16;                 // utf8/16 conversion methods

	string outputEOL;                   // current line end
	string prevEOL;                     // previous line end
	string optionsFileName;             // file path and name of the options file to use
	string origSuffix;                  // suffix= option
	string stdPathIn;                   // path to input from stdin=
	string stdPathOut;                  // path to output from stdout=
	string targetDirectory;             // path to the directory being processed
	string targetFilename;              // file name being processed

	vector<string> excludeVector;       // exclude from wildcard hits
	vector<bool>   excludeHitsVector;   // exclude flags for error reporting
	vector<string> fileNameVector;      // file paths and names from the command line
	vector<string> optionsVector;       // options from the command line
	vector<string> fileOptionsVector;   // options from the options file
	vector<string> fileName;            // files to be processed including path

public:     // functions
	explicit ASConsole(ASFormatter& formatterArg);
	~ASConsole();
	void convertLineEnds(ostringstream& out, int lineEnd);
	FileEncoding detectEncoding(const char* data, size_t dataSize) const;
	void error() const;
	void error(const char* why, const char* what) const;
	void formatCinToCout();
	vector<string> getArgvOptions(int argc, char** argv) const;
	bool fileNameVectorIsEmpty() const;
	ostream* getErrorStream() const;
	bool getFilesAreIdentical() const;
	int  getFilesFormatted() const;
	bool getIgnoreExcludeErrors() const;
	bool getIgnoreExcludeErrorsDisplay() const;
	bool getIsDryRun() const;
	bool getIsFormattedOnly() const;
	bool getIsQuiet() const;
	bool getIsRecursive() const;
	bool getIsVerbose() const;
	bool getLineEndsMixed() const;
	bool getNoBackup() const;
	bool getPreserveDate() const;
	string getLanguageID() const;
	string getNumberFormat(int num, size_t lcid = 0) const;
	string getNumberFormat(int num, const char* groupingArg, const char* separator) const;
	string getOptionsFileName() const;
	string getOrigSuffix() const;
	string getStdPathIn() const;
	string getStdPathOut() const;
	void processFiles();
	void processOptions(const vector<string>& argvOptions);
	void setBypassBrowserOpen(bool state);
	void setErrorStream(ostream* errStreamPtr);
	void setIgnoreExcludeErrors(bool state);
	void setIgnoreExcludeErrorsAndDisplay(bool state);
	void setIsDryRun(bool state);
	void setIsFormattedOnly(bool state);
	void setIsQuiet(bool state);
	void setIsRecursive(bool state);
	void setIsVerbose(bool state);
	void setNoBackup(bool state);
	void setOptionsFileName(const string& name);
	void setOrigSuffix(const string& suffix);
	void setPreserveDate(bool state);
	void setStdPathIn(const string& path);
	void setStdPathOut(const string& path);
	void standardizePath(string& path, bool removeBeginningSeparator = false) const;
	bool stringEndsWith(const string& str, const string& suffix) const;
	void updateExcludeVector(const string& suffixParam);
	vector<string> getExcludeVector() const;
	vector<bool>   getExcludeHitsVector() const;
	vector<string> getFileNameVector() const;
	vector<string> getOptionsVector() const;
	vector<string> getFileOptionsVector() const;
	vector<string> getFileName() const;

private:	// functions
	ASConsole(const ASConsole&);           // copy constructor not to be implemented
	ASConsole& operator=(ASConsole&);      // assignment operator not to be implemented
	void correctMixedLineEnds(ostringstream& out);
	void formatFile(const string& fileName_);
	string getCurrentDirectory(const string& fileName_) const;
	void getFileNames(const string& directory, const string& wildcard);
	void getFilePaths(const string& filePath);
	string getParam(const string& arg, const char* op);
	void initializeOutputEOL(LineEndFormat lineEndFormat);
	bool isOption(const string& arg, const char* op);
	bool isOption(const string& arg, const char* a, const char* b);
	bool isParamOption(const string& arg, const char* option);
	bool isPathExclued(const string& subPath);
	void launchDefaultBrowser(const char* filePathIn = nullptr) const;
	void printHelp() const;
	void printMsg(const char* msg, const string& data) const;
	void printSeparatingLine() const;
	void printVerboseHeader() const;
	void printVerboseStats(clock_t startTime) const;
	FileEncoding readFile(const string& fileName_, stringstream& in) const;
	void removeFile(const char* fileName_, const char* errMsg) const;
	void renameFile(const char* oldFileName, const char* newFileName, const char* errMsg) const;
	void setOutputEOL(LineEndFormat lineEndFormat, const string& currentEOL);
	void sleep(int seconds) const;
	int  waitForRemove(const char* newFileName) const;
	int  wildcmp(const char* wild, const char* data) const;
	void writeFile(const string& fileName_, FileEncoding encoding, ostringstream& out) const;
#ifdef _WIN32
	void displayLastError();
#endif
};
#else	// ASTYLE_LIB

//----------------------------------------------------------------------------
// ASLibrary class for library build
//----------------------------------------------------------------------------

class ASLibrary
{
public:
	ASLibrary() {}
	virtual ~ASLibrary() {}
	// virtual functions are mocked in testing
	utf16_t* formatUtf16(const utf16_t*, const utf16_t*, fpError, fpAlloc) const;
	virtual utf16_t* convertUtf8ToUtf16(const char* utf8In, fpAlloc fpMemoryAlloc) const;
	virtual char* convertUtf16ToUtf8(const utf16_t* utf16In) const;

private:
	static char* STDCALL tempMemoryAllocation(unsigned long memoryNeeded);

private:
	ASEncoding utf8_16;         // utf8/16 conversion methods
};

#endif	// ASTYLE_LIB

//----------------------------------------------------------------------------

}   // end of namespace astyle

//----------------------------------------------------------------------------
// declarations for java native interface (JNI) build
// they are called externally and are NOT part of the namespace
//----------------------------------------------------------------------------
#ifdef ASTYLE_JNI
void  STDCALL javaErrorHandler(int errorNumber, const char* errorMessage);
char* STDCALL javaMemoryAlloc(unsigned long memoryNeeded);
// the following function names are constructed from method names in the calling java program
extern "C" EXPORT
jstring STDCALL Java_AStyleInterface_AStyleGetVersion(JNIEnv* env, jclass);
extern "C" EXPORT
jstring STDCALL Java_AStyleInterface_AStyleMain(JNIEnv* env,
                                                jobject obj,
                                                jstring textInJava,
                                                jstring optionsJava);
#endif //  ASTYLE_JNI

//----------------------------------------------------------------------------
// declarations for UTF-16 interface
// they are called externally and are NOT part of the namespace
//----------------------------------------------------------------------------
#ifdef ASTYLE_LIB
extern "C" EXPORT
utf16_t* STDCALL AStyleMainUtf16(const utf16_t* pSourceIn,
                                 const utf16_t* pOptions,
                                 fpError fpErrorHandler,
                                 fpAlloc fpMemoryAlloc);
#endif	// ASTYLE_LIB

//-----------------------------------------------------------------------------
// declarations for standard DLL interface
// they are called externally and are NOT part of the namespace
//-----------------------------------------------------------------------------
#ifdef ASTYLE_LIB
extern "C" EXPORT char* STDCALL AStyleMain(const char* pSourceIn,
                                           const char* pOptions,
                                           fpError fpErrorHandler,
                                           fpAlloc fpMemoryAlloc);
extern "C" EXPORT const char* STDCALL AStyleGetVersion(void);
#endif	// ASTYLE_LIB

//-----------------------------------------------------------------------------

#endif // closes ASTYLE_MAIN_H
