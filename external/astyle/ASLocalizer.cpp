// ASLocalizer.cpp
// Copyright (c) 2018 by Jim Pattee <jimp03@email.com>.
// This code is licensed under the MIT License.
// License.md describes the conditions under which this software may be distributed.
//
// File encoding for this file is UTF-8 WITHOUT a byte order mark (BOM).
//    русский     中文（简体）    日本語     한국의
//
// Windows:
// Add the required "Language" to the system.
// The settings do NOT need to be changed to the added language.
// Change the "Region" settings.
// Change both the "Format" and the "Current Language..." settings.
// A restart is required if the codepage has changed.
//		Windows problems:
//		Hindi    - no available locale, language pack removed
//		Japanese - language pack install error
//		Ukranian - displays a ? instead of i
//
// Linux:
// Change the LANG environment variable: LANG=fr_FR.UTF-8.
// setlocale() will use the LANG environment variable on Linux.
//
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   To add a new language to this source module:
 *
 *   Add a new translation class to ASLocalizer.h.
 *   Update the WinLangCode array in ASLocalizer.cpp.
 *   Add the language code to setTranslationClass() in ASLocalizer.cpp.
 *   Add the English-Translation pair to the constructor in ASLocalizer.cpp.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

//----------------------------------------------------------------------------
// headers
//----------------------------------------------------------------------------

#include "ASLocalizer.h"

#ifdef _WIN32
	#include <windows.h>
#endif

#ifdef __VMS
	#define __USE_STD_IOSTREAM 1
	#include <assert>
#else
	#include <cassert>
#endif

#include <cstdio>
#include <iostream>
#include <clocale>		// needed by some compilers
#include <cstdlib>
#include <typeinfo>

#ifdef _MSC_VER
	#pragma warning(disable: 4996)  // secure version deprecation warnings
#endif

#ifdef __BORLANDC__
	#pragma warn -8104	    // Local Static with constructor dangerous for multi-threaded apps
#endif

#ifdef __INTEL_COMPILER
	#pragma warning(disable:  383)  // value copied to temporary, reference to temporary used
	#pragma warning(disable:  981)  // operands are evaluated in unspecified order
#endif

#ifdef __clang__
	#pragma clang diagnostic ignored "-Wdeprecated-declarations"  // wcstombs
#endif

namespace astyle {

#ifndef ASTYLE_LIB

//----------------------------------------------------------------------------
// ASLocalizer class methods.
//----------------------------------------------------------------------------

ASLocalizer::ASLocalizer()
// Set the locale information.
{
	// set language default values to english (ascii)
	// this will be used if a locale or a language cannot be found
	m_localeName = "UNKNOWN";
	m_langID = "en";
	m_lcid = 0;
	m_subLangID.clear();
	m_translation = nullptr;

	// Not all compilers support the C++ function locale::global(locale(""));
	char* localeName = setlocale(LC_ALL, "");
	if (localeName == nullptr)		// use the english (ascii) defaults
	{
		fprintf(stderr, "\n%s\n\n", "Cannot set native locale, reverting to English");
		setTranslationClass();
		return;
	}
	// set the class variables
#ifdef _WIN32
	size_t lcid = GetUserDefaultLCID();
	setLanguageFromLCID(lcid);
#else
	setLanguageFromName(localeName);
#endif
}

ASLocalizer::~ASLocalizer()
// Delete dynamically allocated memory.
{
	delete m_translation;
}

#ifdef _WIN32

struct WinLangCode
{
	size_t winLang;
	char canonicalLang[3];
};

static WinLangCode wlc[] =
// primary language identifier http://msdn.microsoft.com/en-us/library/aa912554.aspx
// sublanguage identifier http://msdn.microsoft.com/en-us/library/aa913256.aspx
// language ID http://msdn.microsoft.com/en-us/library/ee797784%28v=cs.20%29.aspx
{
	{ LANG_BULGARIAN,  "bg" },		//	bg-BG	1251
	{ LANG_CHINESE,    "zh" },		//	zh-CHS, zh-CHT
	{ LANG_DUTCH,      "nl" },		//	nl-NL	1252
	{ LANG_ENGLISH,    "en" },		//	en-US	1252
	{ LANG_ESTONIAN,   "et" },		//	et-EE
	{ LANG_FINNISH,    "fi" },		//	fi-FI	1252
	{ LANG_FRENCH,     "fr" },		//	fr-FR	1252
	{ LANG_GERMAN,     "de" },		//	de-DE	1252
	{ LANG_GREEK,      "el" },		//	el-GR	1253
	{ LANG_HINDI,      "hi" },		//	hi-IN
	{ LANG_HUNGARIAN,  "hu" },		//	hu-HU	1250
	{ LANG_ITALIAN,    "it" },		//	it-IT	1252
	{ LANG_JAPANESE,   "ja" },		//	ja-JP
	{ LANG_KOREAN,     "ko" },		//	ko-KR
	{ LANG_NORWEGIAN,  "nn" },		//	nn-NO	1252
	{ LANG_POLISH,     "pl" },		//	pl-PL	1250
	{ LANG_PORTUGUESE, "pt" },		//	pt-PT	1252
	{ LANG_ROMANIAN,   "ro" },		//	ro-RO	1250
	{ LANG_RUSSIAN,    "ru" },		//	ru-RU	1251
	{ LANG_SPANISH,    "es" },		//	es-ES	1252
	{ LANG_SWEDISH,    "sv" },		//	sv-SE	1252
	{ LANG_UKRAINIAN,  "uk" },		//	uk-UA	1251
};

void ASLocalizer::setLanguageFromLCID(size_t lcid)
// Windows get the language to use from the user locale.
// NOTE: GetUserDefaultLocaleName() gets nearly the same name as Linux.
//       But it needs Windows Vista or higher.
//       Same with LCIDToLocaleName().
{
	m_lcid = lcid;
	m_langID = "en";	// default to english

	size_t lang = PRIMARYLANGID(LANGIDFROMLCID(m_lcid));
	size_t sublang = SUBLANGID(LANGIDFROMLCID(m_lcid));
	// find language in the wlc table
	size_t count = sizeof(wlc) / sizeof(wlc[0]);
	for (size_t i = 0; i < count; i++)
	{
		if (wlc[i].winLang == lang)
		{
			m_langID = wlc[i].canonicalLang;
			break;
		}
	}
	if (m_langID == "zh")
	{
		if (sublang == SUBLANG_CHINESE_SIMPLIFIED || sublang == SUBLANG_CHINESE_SINGAPORE)
			m_subLangID = "CHS";
		else
			m_subLangID = "CHT";	// default
	}
	setTranslationClass();
}

#endif	// _WIN32

string ASLocalizer::getLanguageID() const
// Returns the language ID in m_langID.
{
	return m_langID;
}

const Translation* ASLocalizer::getTranslationClass() const
// Returns the name of the translation class in m_translation.  Used for testing.
{
	assert(m_translation);
	return m_translation;
}

void ASLocalizer::setLanguageFromName(const char* langID)
// Linux set the language to use from the langID.
//
// the language string has the following form
//
//      lang[_LANG][.encoding][@modifier]
//
// (see environ(5) in the Open Unix specification)
//
// where lang is the primary language, LANG is a sublang/territory,
// encoding is the charset to use and modifier "allows the user to select
// a specific instance of localization data within a single category"
//
// for example, the following strings are valid:
//      fr
//      fr_FR
//      de_DE.iso88591
//      de_DE@euro
//      de_DE.iso88591@euro
{
	// the constants describing the format of lang_LANG locale string
	m_lcid = 0;
	string langStr = langID;
	m_langID = langStr.substr(0, 2);

	// need the sublang for chinese
	if (m_langID == "zh" && langStr[2] == '_')
	{
		string subLang = langStr.substr(3, 2);
		if (subLang == "CN" || subLang == "SG")
			m_subLangID = "CHS";
		else
			m_subLangID = "CHT";	// default
	}
	setTranslationClass();
}

const char* ASLocalizer::settext(const char* textIn) const
// Call the settext class and return the value.
{
	assert(m_translation);
	const string stringIn = textIn;
	return m_translation->translate(stringIn).c_str();
}

void ASLocalizer::setTranslationClass()
// Return the required translation class.
// Sets the class variable m_translation from the value of m_langID.
// Get the language ID at http://msdn.microsoft.com/en-us/library/ee797784%28v=cs.20%29.aspx
{
	assert(m_langID.length());
	// delete previously set (--ascii option)
	if (m_translation != nullptr)
	{
		delete m_translation;
		m_translation = nullptr;
	}
	if (m_langID == "bg")
		m_translation = new Bulgarian;
	else if (m_langID == "zh" && m_subLangID == "CHS")
		m_translation = new ChineseSimplified;
	else if (m_langID == "zh" && m_subLangID == "CHT")
		m_translation = new ChineseTraditional;
	else if (m_langID == "nl")
		m_translation = new Dutch;
	else if (m_langID == "en")
		m_translation = new English;
	else if (m_langID == "et")
		m_translation = new Estonian;
	else if (m_langID == "fi")
		m_translation = new Finnish;
	else if (m_langID == "fr")
		m_translation = new French;
	else if (m_langID == "de")
		m_translation = new German;
	else if (m_langID == "el")
		m_translation = new Greek;
	else if (m_langID == "hi")
		m_translation = new Hindi;
	else if (m_langID == "hu")
		m_translation = new Hungarian;
	else if (m_langID == "it")
		m_translation = new Italian;
	else if (m_langID == "ja")
		m_translation = new Japanese;
	else if (m_langID == "ko")
		m_translation = new Korean;
	else if (m_langID == "nn")
		m_translation = new Norwegian;
	else if (m_langID == "pl")
		m_translation = new Polish;
	else if (m_langID == "pt")
		m_translation = new Portuguese;
	else if (m_langID == "ro")
		m_translation = new Romanian;
	else if (m_langID == "ru")
		m_translation = new Russian;
	else if (m_langID == "es")
		m_translation = new Spanish;
	else if (m_langID == "sv")
		m_translation = new Swedish;
	else if (m_langID == "uk")
		m_translation = new Ukrainian;
	else	// default
		m_translation = new English;
}

//----------------------------------------------------------------------------
// Translation base class methods.
//----------------------------------------------------------------------------

void Translation::addPair(const string& english, const wstring& translated)
// Add a string pair to the translation vector.
{
	pair<string, wstring> entry(english, translated);
	m_translation.emplace_back(entry);
}

string Translation::convertToMultiByte(const wstring& wideStr) const
// Convert wchar_t to a multibyte string using the currently assigned locale.
// Return an empty string if an error occurs.
{
	static bool msgDisplayed = false;
	// get length of the output excluding the nullptr and validate the parameters
	size_t mbLen = wcstombs(nullptr, wideStr.c_str(), 0);
	if (mbLen == string::npos)
	{
		if (!msgDisplayed)
		{
			fprintf(stderr, "\n%s\n\n", "Cannot convert to multi-byte string, reverting to English");
			msgDisplayed = true;
		}
		return "";
	}
	// convert the characters
	char* mbStr = new (nothrow) char[mbLen + 1];
	if (mbStr == nullptr)
	{
		if (!msgDisplayed)
		{
			fprintf(stderr, "\n%s\n\n", "Bad memory alloc for multi-byte string, reverting to English");
			msgDisplayed = true;
		}
		return "";
	}
	wcstombs(mbStr, wideStr.c_str(), mbLen + 1);
	// return the string
	string mbTranslation = mbStr;
	delete[] mbStr;
	return mbTranslation;
}

string Translation::getTranslationString(size_t i) const
// Return the translation ascii value. Used for testing.
{
	if (i >= m_translation.size())
		return string();
	return m_translation[i].first;
}

size_t Translation::getTranslationVectorSize() const
// Return the translation vector size.  Used for testing.
{
	return m_translation.size();
}

bool Translation::getWideTranslation(const string& stringIn, wstring& wideOut) const
// Get the wide translation string. Used for testing.
{
	for (size_t i = 0; i < m_translation.size(); i++)
	{
		if (m_translation[i].first == stringIn)
		{
			wideOut = m_translation[i].second;
			return true;
		}
	}
	// not found
	wideOut = L"";
	return false;
}

string& Translation::translate(const string& stringIn) const
// Translate a string.
// Return a mutable string so the method can have a "const" designation.
// This allows "settext" to be called from a "const" method.
{
	m_mbTranslation.clear();
	for (size_t i = 0; i < m_translation.size(); i++)
	{
		if (m_translation[i].first == stringIn)
		{
			m_mbTranslation = convertToMultiByte(m_translation[i].second);
			break;
		}
	}
	// not found, return english
	if (m_mbTranslation.empty())
		m_mbTranslation = stringIn;
	return m_mbTranslation;
}

//----------------------------------------------------------------------------
// Translation class methods.
// These classes have only a constructor which builds the language vector.
//----------------------------------------------------------------------------

Bulgarian::Bulgarian()	// български
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Форматиран  %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"Непроменен  %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"директория  %s\n");
	addPair("Default option file  %s\n", L"Файл с опции по подразбиране  %s\n");
	addPair("Project option file  %s\n", L"Файл с опции за проекта  %s\n");
	addPair("Exclude  %s\n", L"Изключвам  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Изключване (несравнимо)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s форматиран   %s hепроменен   ");
	addPair(" seconds   ", L" секунди   ");
	addPair("%d min %d sec   ", L"%d мин %d сек   ");
	addPair("%s lines\n", L"%s линии\n");
	addPair("Opening HTML documentation %s\n", L"Откриване HTML документация %s\n");
	addPair("Invalid default options:", L"Невалидни опции по подразбиране:");
	addPair("Invalid project options:", L"Невалидни опции за проекти:");
	addPair("Invalid command line options:", L"Невалидни опции за командния ред:");
	addPair("For help on options type 'astyle -h'", L"За помощ относно възможностите тип 'astyle -h'");
	addPair("Cannot open default option file", L"Не може да се отвори файлът с опции по подразбиране");
	addPair("Cannot open project option file", L"Не може да се отвори файла с опции за проекта");
	addPair("Cannot open directory", L"Не може да се отвори директория");
	addPair("Cannot open HTML file %s\n", L"Не може да се отвори HTML файл %s\n");
	addPair("Command execute failure", L"Command изпълни недостатъчност");
	addPair("Command is not installed", L"Command не е инсталиран");
	addPair("Missing filename in %s\n", L"Липсва името на файла в %s\n");
	addPair("Recursive option with no wildcard", L"Рекурсивно опция, без маска");
	addPair("Did you intend quote the filename", L"Знаете ли намерение да цитирам името на файла");
	addPair("No file to process %s\n", L"Не файл за обработка %s\n");
	addPair("Did you intend to use --recursive", L"Знаете ли възнамерявате да използвате --recursive");
	addPair("Cannot process UTF-32 encoding", L"Не може да са UTF-32 кодиране");
	addPair("Artistic Style has terminated\n", L"Artistic Style е прекратено\n");
}

ChineseSimplified::ChineseSimplified()	// 中文（简体）
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"格式化  %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"未改变  %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"目录  %s\n");
	addPair("Default option file  %s\n", L"默认选项文件  %s\n");
	addPair("Project option file  %s\n", L"项目选项文件  %s\n");
	addPair("Exclude  %s\n", L"排除  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"排除（无匹配项）  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s 格式化   %s 未改变   ");
	addPair(" seconds   ", L" 秒   ");
	addPair("%d min %d sec   ", L"%d 分 %d 秒   ");
	addPair("%s lines\n", L"%s 行\n");
	addPair("Opening HTML documentation %s\n", L"打开HTML文档 %s\n");
	addPair("Invalid default options:", L"默认选项无效:");
	addPair("Invalid project options:", L"项目选项无效:");
	addPair("Invalid command line options:", L"无效的命令行选项:");
	addPair("For help on options type 'astyle -h'", L"输入 'astyle -h' 以获得有关命令行的帮助");
	addPair("Cannot open default option file", L"无法打开默认选项文件");
	addPair("Cannot open project option file", L"无法打开项目选项文件");
	addPair("Cannot open directory", L"无法打开目录");
	addPair("Cannot open HTML file %s\n", L"无法打开HTML文件 %s\n");
	addPair("Command execute failure", L"执行命令失败");
	addPair("Command is not installed", L"未安装命令");
	addPair("Missing filename in %s\n", L"在%s缺少文件名\n");
	addPair("Recursive option with no wildcard", L"递归选项没有通配符");
	addPair("Did you intend quote the filename", L"你打算引用文件名");
	addPair("No file to process %s\n", L"没有文件可处理 %s\n");
	addPair("Did you intend to use --recursive", L"你打算使用 --recursive");
	addPair("Cannot process UTF-32 encoding", L"不能处理UTF-32编码");
	addPair("Artistic Style has terminated\n", L"Artistic Style 已经终止运行\n");
}

ChineseTraditional::ChineseTraditional()	// 中文（繁體）
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"格式化  %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"未改變  %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"目錄  %s\n");
	addPair("Default option file  %s\n", L"默認選項文件  %s\n");
	addPair("Project option file  %s\n", L"項目選項文件  %s\n");
	addPair("Exclude  %s\n", L"排除  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"排除（無匹配項）  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s 格式化   %s 未改變   ");
	addPair(" seconds   ", L" 秒   ");
	addPair("%d min %d sec   ", L"%d 分 %d 秒   ");
	addPair("%s lines\n", L"%s 行\n");
	addPair("Opening HTML documentation %s\n", L"打開HTML文檔 %s\n");
	addPair("Invalid default options:", L"默認選項無效:");
	addPair("Invalid project options:", L"項目選項無效:");
	addPair("Invalid command line options:", L"無效的命令行選項:");
	addPair("For help on options type 'astyle -h'", L"輸入'astyle -h'以獲得有關命令行的幫助:");
	addPair("Cannot open default option file", L"無法打開默認選項文件");
	addPair("Cannot open project option file", L"無法打開項目選項文件");
	addPair("Cannot open directory", L"無法打開目錄");
	addPair("Cannot open HTML file %s\n", L"無法打開HTML文件 %s\n");
	addPair("Command execute failure", L"執行命令失敗");
	addPair("Command is not installed", L"未安裝命令");
	addPair("Missing filename in %s\n", L"在%s缺少文件名\n");
	addPair("Recursive option with no wildcard", L"遞歸選項沒有通配符");
	addPair("Did you intend quote the filename", L"你打算引用文件名");
	addPair("No file to process %s\n", L"沒有文件可處理 %s\n");
	addPair("Did you intend to use --recursive", L"你打算使用 --recursive");
	addPair("Cannot process UTF-32 encoding", L"不能處理UTF-32編碼");
	addPair("Artistic Style has terminated\n", L"Artistic Style 已經終止運行\n");
}

Dutch::Dutch()	// Nederlandse
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Geformatteerd  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Onveranderd    %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Directory  %s\n");
	addPair("Default option file  %s\n", L"Standaard optie bestand  %s\n");
	addPair("Project option file  %s\n", L"Project optie bestand  %s\n");
	addPair("Exclude  %s\n", L"Uitsluiten  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Uitgesloten (ongeëvenaarde)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s geformatteerd   %s onveranderd   ");
	addPair(" seconds   ", L" seconden   ");
	addPair("%d min %d sec   ", L"%d min %d sec   ");
	addPair("%s lines\n", L"%s lijnen\n");
	addPair("Opening HTML documentation %s\n", L"Het openen van HTML-documentatie %s\n");
	addPair("Invalid default options:", L"Ongeldige standaardopties:");
	addPair("Invalid project options:", L"Ongeldige projectopties:");
	addPair("Invalid command line options:", L"Ongeldige command line opties:");
	addPair("For help on options type 'astyle -h'", L"Voor hulp bij 'astyle-h' opties het type");
	addPair("Cannot open default option file", L"Kan het standaardoptiesbestand niet openen");
	addPair("Cannot open project option file", L"Kan het project optie bestand niet openen");
	addPair("Cannot open directory", L"Kan niet open directory");
	addPair("Cannot open HTML file %s\n", L"Kan HTML-bestand niet openen %s\n");
	addPair("Command execute failure", L"Voeren commando falen");
	addPair("Command is not installed", L"Command is niet geïnstalleerd");
	addPair("Missing filename in %s\n", L"Ontbrekende bestandsnaam in %s\n");
	addPair("Recursive option with no wildcard", L"Recursieve optie met geen wildcard");
	addPair("Did you intend quote the filename", L"Heeft u van plan citaat van de bestandsnaam");
	addPair("No file to process %s\n", L"Geen bestand te verwerken %s\n");
	addPair("Did you intend to use --recursive", L"Hebt u van plan bent te gebruiken --recursive");
	addPair("Cannot process UTF-32 encoding", L"Kan niet verwerken UTF-32 codering");
	addPair("Artistic Style has terminated\n", L"Artistic Style heeft beëindigd\n");
}

English::English()
// this class is NOT translated
{}

Estonian::Estonian()	// Eesti
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formaadis  %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"Muutumatu  %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"Kataloog  %s\n");
	addPair("Default option file  %s\n", L"Vaikefunktsioonifail  %s\n");
	addPair("Project option file  %s\n", L"Projekti valiku fail  %s\n");
	addPair("Exclude  %s\n", L"Välista  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Välista (tasakaalustamata)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formaadis   %s muutumatu   ");
	addPair(" seconds   ", L" sekundit   ");
	addPair("%d min %d sec   ", L"%d min %d sek   ");
	addPair("%s lines\n", L"%s read\n");
	addPair("Opening HTML documentation %s\n", L"Avamine HTML dokumentatsioon %s\n");
	addPair("Invalid default options:", L"Vaikevalikud on sobimatud:");
	addPair("Invalid project options:", L"Projekti valikud on sobimatud:");
	addPair("Invalid command line options:", L"Vale käsureavõtmetega:");
	addPair("For help on options type 'astyle -h'", L"Abiks võimaluste tüüp 'astyle -h'");
	addPair("Cannot open default option file", L"Vaikimisi valitud faili ei saa avada");
	addPair("Cannot open project option file", L"Projektivaliku faili ei saa avada");
	addPair("Cannot open directory", L"Ei saa avada kataloogi");
	addPair("Cannot open HTML file %s\n", L"Ei saa avada HTML-faili %s\n");
	addPair("Command execute failure", L"Käsk täita rike");
	addPair("Command is not installed", L"Käsk ei ole paigaldatud");
	addPair("Missing filename in %s\n", L"Kadunud failinimi %s\n");
	addPair("Recursive option with no wildcard", L"Rekursiivne võimalus ilma metamärgi");
	addPair("Did you intend quote the filename", L"Kas te kavatsete tsiteerida failinimi");
	addPair("No file to process %s\n", L"No faili töötlema %s\n");
	addPair("Did you intend to use --recursive", L"Kas te kavatsete kasutada --recursive");
	addPair("Cannot process UTF-32 encoding", L"Ei saa töödelda UTF-32 kodeeringus");
	addPair("Artistic Style has terminated\n", L"Artistic Style on lõpetatud\n");
}

Finnish::Finnish()	// Suomeksi
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Muotoiltu  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Ennallaan  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Directory  %s\n");
	addPair("Default option file  %s\n", L"Oletusasetustiedosto  %s\n");
	addPair("Project option file  %s\n", L"Projektin valintatiedosto  %s\n");
	addPair("Exclude  %s\n", L"Sulkea  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Sulkea (verraton)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s muotoiltu   %s ennallaan   ");
	addPair(" seconds   ", L" sekuntia   ");
	addPair("%d min %d sec   ", L"%d min %d sek   ");
	addPair("%s lines\n", L"%s linjat\n");
	addPair("Opening HTML documentation %s\n", L"Avaaminen HTML asiakirjat %s\n");
	addPair("Invalid default options:", L"Virheelliset oletusasetukset:");
	addPair("Invalid project options:", L"Virheelliset hankevalinnat:");
	addPair("Invalid command line options:", L"Virheellinen komentorivin:");
	addPair("For help on options type 'astyle -h'", L"Apua vaihtoehdoista tyyppi 'astyle -h'");
	addPair("Cannot open default option file", L"Et voi avata oletusasetustiedostoa");
	addPair("Cannot open project option file", L"Projektin asetustiedostoa ei voi avata");
	addPair("Cannot open directory", L"Ei Open Directory");
	addPair("Cannot open HTML file %s\n", L"Ei voi avata HTML-tiedoston %s\n");
	addPair("Command execute failure", L"Suorita komento vika");
	addPair("Command is not installed", L"Komento ei ole asennettu");
	addPair("Missing filename in %s\n", L"Puuttuvat tiedostonimi %s\n");
	addPair("Recursive option with no wildcard", L"Rekursiivinen vaihtoehto ilman wildcard");
	addPair("Did you intend quote the filename", L"Oletko aio lainata tiedostonimi");
	addPair("No file to process %s\n", L"Ei tiedostoa käsitellä %s\n");
	addPair("Did you intend to use --recursive", L"Oliko aiot käyttää --recursive");
	addPair("Cannot process UTF-32 encoding", L"Ei voi käsitellä UTF-32 koodausta");
	addPair("Artistic Style has terminated\n", L"Artistic Style on päättynyt\n");
}

French::French()	// Française
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formaté    %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Inchangée  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Répertoire  %s\n");
	addPair("Default option file  %s\n", L"Fichier d'option par défaut  %s\n");
	addPair("Project option file  %s\n", L"Fichier d'option de projet  %s\n");
	addPair("Exclude  %s\n", L"Exclure  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Exclure (non appariés)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formaté   %s inchangée   ");
	addPair(" seconds   ", L" seconde   ");
	addPair("%d min %d sec   ", L"%d min %d sec   ");
	addPair("%s lines\n", L"%s lignes\n");
	addPair("Opening HTML documentation %s\n", L"Ouverture documentation HTML %s\n");
	addPair("Invalid default options:", L"Options par défaut invalides:");
	addPair("Invalid project options:", L"Options de projet non valides:");
	addPair("Invalid command line options:", L"Blancs options ligne de commande:");
	addPair("For help on options type 'astyle -h'", L"Pour de l'aide sur les options tapez 'astyle -h'");
	addPair("Cannot open default option file", L"Impossible d'ouvrir le fichier d'option par défaut");
	addPair("Cannot open project option file", L"Impossible d'ouvrir le fichier d'option de projet");
	addPair("Cannot open directory", L"Impossible d'ouvrir le répertoire");
	addPair("Cannot open HTML file %s\n", L"Impossible d'ouvrir le fichier HTML %s\n");
	addPair("Command execute failure", L"Exécuter échec de la commande");
	addPair("Command is not installed", L"Commande n'est pas installé");
	addPair("Missing filename in %s\n", L"Nom de fichier manquant dans %s\n");
	addPair("Recursive option with no wildcard", L"Option récursive sans joker");
	addPair("Did you intend quote the filename", L"Avez-vous l'intention de citer le nom de fichier");
	addPair("No file to process %s\n", L"Aucun fichier à traiter %s\n");
	addPair("Did you intend to use --recursive", L"Avez-vous l'intention d'utiliser --recursive");
	addPair("Cannot process UTF-32 encoding", L"Impossible de traiter codage UTF-32");
	addPair("Artistic Style has terminated\n", L"Artistic Style a mis fin\n");
}

German::German()	// Deutsch
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formatiert   %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Unverändert  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Verzeichnis  %s\n");
	addPair("Default option file  %s\n", L"Standard-Optionsdatei  %s\n");
	addPair("Project option file  %s\n", L"Projektoptionsdatei  %s\n");
	addPair("Exclude  %s\n", L"Ausschließen  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Ausschließen (unerreichte)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formatiert   %s unverändert   ");
	addPair(" seconds   ", L" sekunden   ");
	addPair("%d min %d sec   ", L"%d min %d sek   ");
	addPair("%s lines\n", L"%s linien\n");
	addPair("Opening HTML documentation %s\n", L"Öffnen HTML-Dokumentation %s\n");
	addPair("Invalid default options:", L"Ungültige Standardoptionen:");
	addPair("Invalid project options:", L"Ungültige Projektoptionen:");
	addPair("Invalid command line options:", L"Ungültige Kommandozeilen-Optionen:");
	addPair("For help on options type 'astyle -h'", L"Für Hilfe zu den Optionen geben Sie 'astyle -h'");
	addPair("Cannot open default option file", L"Die Standardoptionsdatei kann nicht geöffnet werden");
	addPair("Cannot open project option file", L"Die Projektoptionsdatei kann nicht geöffnet werden");
	addPair("Cannot open directory", L"Kann nicht geöffnet werden Verzeichnis");
	addPair("Cannot open HTML file %s\n", L"Kann nicht öffnen HTML-Datei %s\n");
	addPair("Command execute failure", L"Execute Befehl Scheitern");
	addPair("Command is not installed", L"Befehl ist nicht installiert");
	addPair("Missing filename in %s\n", L"Missing in %s Dateiname\n");
	addPair("Recursive option with no wildcard", L"Rekursive Option ohne Wildcard");
	addPair("Did you intend quote the filename", L"Haben Sie die Absicht Inhalte der Dateiname");
	addPair("No file to process %s\n", L"Keine Datei zu verarbeiten %s\n");
	addPair("Did you intend to use --recursive", L"Haben Sie verwenden möchten --recursive");
	addPair("Cannot process UTF-32 encoding", L"Nicht verarbeiten kann UTF-32 Codierung");
	addPair("Artistic Style has terminated\n", L"Artistic Style ist beendet\n");
}

Greek::Greek()	// ελληνικά
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Διαμορφωμένη  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Αμετάβλητος   %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Κατάλογος  %s\n");
	addPair("Default option file  %s\n", L"Προεπιλεγμένο αρχείο επιλογών  %s\n");
	addPair("Project option file  %s\n", L"Αρχείο επιλογής έργου  %s\n");
	addPair("Exclude  %s\n", L"Αποκλείω  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Ausschließen (unerreichte)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s σχηματοποιημένη   %s αμετάβλητες   ");
	addPair(" seconds   ", L" δευτερόλεπτα   ");
	addPair("%d min %d sec   ", L"%d λεπ %d δευ   ");
	addPair("%s lines\n", L"%s γραμμές\n");
	addPair("Opening HTML documentation %s\n", L"Εγκαίνια έγγραφα HTML %s\n");
	addPair("Invalid default options:", L"Μη έγκυρες επιλογές προεπιλογής:");
	addPair("Invalid project options:", L"Μη έγκυρες επιλογές έργου:");
	addPair("Invalid command line options:", L"Μη έγκυρη επιλογές γραμμής εντολών:");
	addPair("For help on options type 'astyle -h'", L"Για βοήθεια σχετικά με το είδος επιλογές 'astyle -h'");
	addPair("Cannot open default option file", L"Δεν είναι δυνατό να ανοίξει το προεπιλεγμένο αρχείο επιλογών");
	addPair("Cannot open project option file", L"Δεν είναι δυνατό να ανοίξει το αρχείο επιλογής έργου");
	addPair("Cannot open directory", L"Δεν μπορείτε να ανοίξετε τον κατάλογο");
	addPair("Cannot open HTML file %s\n", L"Δεν μπορείτε να ανοίξετε το αρχείο HTML %s\n");
	addPair("Command execute failure", L"Εντολή να εκτελέσει την αποτυχία");
	addPair("Command is not installed", L"Η εντολή δεν έχει εγκατασταθεί");
	addPair("Missing filename in %s\n", L"Λείπει το όνομα αρχείου σε %s\n");
	addPair("Recursive option with no wildcard", L"Αναδρομικές επιλογή χωρίς μπαλαντέρ");
	addPair("Did you intend quote the filename", L"Μήπως σκοπεύετε να αναφέρετε το όνομα του αρχείου");
	addPair("No file to process %s\n", L"Δεν υπάρχει αρχείο για την επεξεργασία %s\n");
	addPair("Did you intend to use --recursive", L"Μήπως σκοπεύετε να χρησιμοποιήσετε --recursive");
	addPair("Cannot process UTF-32 encoding", L"δεν μπορεί να επεξεργαστεί UTF-32 κωδικοποίηση");
	addPair("Artistic Style has terminated\n", L"Artistic Style έχει λήξει\n");
}

Hindi::Hindi()	// हिन्दी
// build the translation vector in the Translation base class
{
	// NOTE: Scintilla based editors (CodeBlocks) cannot always edit Hindi.
	//       Use Visual Studio instead.
	addPair("Formatted  %s\n", L"स्वरूपित किया  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"अपरिवर्तित     %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"निर्देशिका  %s\n");
	addPair("Default option file  %s\n", L"डिफ़ॉल्ट विकल्प फ़ाइल  %s\n");
	addPair("Project option file  %s\n", L"प्रोजेक्ट विकल्प फ़ाइल  %s\n");
	addPair("Exclude  %s\n", L"निकालना  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"अपवर्जित (बेजोड़)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s स्वरूपित किया   %s अपरिवर्तित   ");
	addPair(" seconds   ", L" सेकंड   ");
	addPair("%d min %d sec   ", L"%d मिनट %d सेकंड   ");
	addPair("%s lines\n", L"%s लाइनों\n");
	addPair("Opening HTML documentation %s\n", L"एचटीएमएल प्रलेखन खोलना %s\n");
	addPair("Invalid default options:", L"अमान्य डिफ़ॉल्ट विकल्प:");
	addPair("Invalid project options:", L"अमान्य प्रोजेक्ट विकल्प:");
	addPair("Invalid command line options:", L"कमांड लाइन विकल्प अवैध:");
	addPair("For help on options type 'astyle -h'", L"विकल्पों पर मदद के लिए प्रकार 'astyle -h'");
	addPair("Cannot open default option file", L"डिफ़ॉल्ट विकल्प फ़ाइल नहीं खोल सकता");
	addPair("Cannot open project option file", L"परियोजना विकल्प फ़ाइल नहीं खोल सकता");
	addPair("Cannot open directory", L"निर्देशिका नहीं खोल सकता");
	addPair("Cannot open HTML file %s\n", L"HTML फ़ाइल नहीं खोल सकता %s\n");
	addPair("Command execute failure", L"आदेश विफलता निष्पादित");
	addPair("Command is not installed", L"कमान स्थापित नहीं है");
	addPair("Missing filename in %s\n", L"लापता में फ़ाइलनाम %s\n");
	addPair("Recursive option with no wildcard", L"कोई वाइल्डकार्ड साथ पुनरावर्ती विकल्प");
	addPair("Did you intend quote the filename", L"क्या आप बोली फ़ाइलनाम का इरादा");
	addPair("No file to process %s\n", L"कोई फ़ाइल %s प्रक्रिया के लिए\n");
	addPair("Did you intend to use --recursive", L"क्या आप उपयोग करना चाहते हैं --recursive");
	addPair("Cannot process UTF-32 encoding", L"UTF-32 कूटबन्धन प्रक्रिया नहीं कर सकते");
	addPair("Artistic Style has terminated\n", L"Artistic Style समाप्त किया है\n");
}

Hungarian::Hungarian()	// Magyar
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formázott    %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Változatlan  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Címjegyzék  %s\n");
	addPair("Default option file  %s\n", L"Alapértelmezett beállítási fájl  %s\n");
	addPair("Project option file  %s\n", L"Projekt opciófájl  %s\n");
	addPair("Exclude  %s\n", L"Kizár  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Escludere (senza pari)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formázott   %s változatlan   ");
	addPair(" seconds   ", L" másodperc   ");
	addPair("%d min %d sec   ", L"%d jeg %d más   ");
	addPair("%s lines\n", L"%s vonalak\n");
	addPair("Opening HTML documentation %s\n", L"Nyitó HTML dokumentáció %s\n");
	addPair("Invalid default options:", L"Érvénytelen alapértelmezett beállítások:");
	addPair("Invalid project options:", L"Érvénytelen projektbeállítások:");
	addPair("Invalid command line options:", L"Érvénytelen parancssori opciók:");
	addPair("For help on options type 'astyle -h'", L"Ha segítségre van lehetőség típus 'astyle-h'");
	addPair("Cannot open default option file", L"Nem lehet megnyitni az alapértelmezett beállítási fájlt");
	addPair("Cannot open project option file", L"Nem lehet megnyitni a projekt opció fájlt");
	addPair("Cannot open directory", L"Nem lehet megnyitni könyvtár");
	addPair("Cannot open HTML file %s\n", L"Nem lehet megnyitni a HTML fájlt %s\n");
	addPair("Command execute failure", L"Command végre hiba");
	addPair("Command is not installed", L"Parancs nincs telepítve");
	addPair("Missing filename in %s\n", L"Hiányzó fájlnév %s\n");
	addPair("Recursive option with no wildcard", L"Rekurzív kapcsolót nem wildcard");
	addPair("Did you intend quote the filename", L"Esetleg kívánja idézni a fájlnév");
	addPair("No file to process %s\n", L"Nincs fájl feldolgozása %s\n");
	addPair("Did you intend to use --recursive", L"Esetleg a használni kívánt --recursive");
	addPair("Cannot process UTF-32 encoding", L"Nem tudja feldolgozni UTF-32 kódolással");
	addPair("Artistic Style has terminated\n", L"Artistic Style megszűnt\n");
}

Italian::Italian()	// Italiano
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formattata  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Immutato    %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Elenco  %s\n");
	addPair("Default option file  %s\n", L"File di opzione predefinito  %s\n");
	addPair("Project option file  %s\n", L"File di opzione del progetto  %s\n");
	addPair("Exclude  %s\n", L"Escludere  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Escludere (senza pari)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s ormattata   %s immutato   ");
	addPair(" seconds   ", L" secondo   ");
	addPair("%d min %d sec   ", L"%d min %d seg   ");
	addPair("%s lines\n", L"%s linee\n");
	addPair("Opening HTML documentation %s\n", L"Apertura di documenti HTML %s\n");
	addPair("Invalid default options:", L"Opzioni di default non valide:");
	addPair("Invalid project options:", L"Opzioni di progetto non valide:");
	addPair("Invalid command line options:", L"Opzioni della riga di comando non valido:");
	addPair("For help on options type 'astyle -h'", L"Per informazioni sulle opzioni di tipo 'astyle-h'");
	addPair("Cannot open default option file", L"Impossibile aprire il file di opzione predefinito");
	addPair("Cannot open project option file", L"Impossibile aprire il file di opzione del progetto");
	addPair("Cannot open directory", L"Impossibile aprire la directory");
	addPair("Cannot open HTML file %s\n", L"Impossibile aprire il file HTML %s\n");
	addPair("Command execute failure", L"Esegui fallimento comando");
	addPair("Command is not installed", L"Il comando non è installato");
	addPair("Missing filename in %s\n", L"Nome del file mancante in %s\n");
	addPair("Recursive option with no wildcard", L"Opzione ricorsiva senza jolly");
	addPair("Did you intend quote the filename", L"Avete intenzione citare il nome del file");
	addPair("No file to process %s\n", L"Nessun file al processo %s\n");
	addPair("Did you intend to use --recursive", L"Hai intenzione di utilizzare --recursive");
	addPair("Cannot process UTF-32 encoding", L"Non è possibile processo di codifica UTF-32");
	addPair("Artistic Style has terminated\n", L"Artistic Style ha terminato\n");
}

Japanese::Japanese()	// 日本語
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"フォーマット済みの  %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"変わりません        %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"ディレクトリ  %s\n");
	addPair("Default option file  %s\n", L"デフォルトオプションファイル  %s\n");
	addPair("Project option file  %s\n", L"プロジェクトオプションファイル  %s\n");
	addPair("Exclude  %s\n", L"除外する  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"除外する（一致しません）  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s フフォーマット済みの   %s 変わりません   ");
	addPair(" seconds   ", L" 秒   ");
	addPair("%d min %d sec   ", L"%d 分 %d 秒   ");
	addPair("%s lines\n", L"%s ライン\n");
	addPair("Opening HTML documentation %s\n", L"オープニングHTMLドキュメント %s\n");
	addPair("Invalid default options:", L"無効なデフォルトオプション:");
	addPair("Invalid project options:", L"無効なプロジェクトオプション:");
	addPair("Invalid command line options:", L"無効なコマンドラインオプション：");
	addPair("For help on options type 'astyle -h'", L"コオプションの種類のヘルプについて'astyle- h'を入力してください");
	addPair("Cannot open default option file", L"デフォルトのオプションファイルを開くことができません");
	addPair("Cannot open project option file", L"プロジェクトオプションファイルを開くことができません");
	addPair("Cannot open directory", L"ディレクトリを開くことができません。");
	addPair("Cannot open HTML file %s\n", L"HTMLファイルを開くことができません %s\n");
	addPair("Command execute failure", L"コマンドが失敗を実行します");
	addPair("Command is not installed", L"コマンドがインストールされていません");
	addPair("Missing filename in %s\n", L"%s で、ファイル名がありません\n");
	addPair("Recursive option with no wildcard", L"無ワイルドカードを使用して再帰的なオプション");
	addPair("Did you intend quote the filename", L"あなたはファイル名を引用するつもりでした");
	addPair("No file to process %s\n", L"いいえファイルは処理しないように %s\n");
	addPair("Did you intend to use --recursive", L"あなたは--recursive使用するつもりでした");
	addPair("Cannot process UTF-32 encoding", L"UTF - 32エンコーディングを処理できません");
	addPair("Artistic Style has terminated\n", L"Artistic Style 終了しました\n");
}

Korean::Korean()	// 한국의
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"수정됨    %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"변경없음  %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"디렉토리  %s\n");
	addPair("Default option file  %s\n", L"기본 옵션 파일  %s\n");
	addPair("Project option file  %s\n", L"프로젝트 옵션 파일  %s\n");
	addPair("Exclude  %s\n", L"제외됨  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"제외 (NO 일치)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s 수정됨   %s 변경없음   ");
	addPair(" seconds   ", L" 초   ");
	addPair("%d min %d sec   ", L"%d 분 %d 초   ");
	addPair("%s lines\n", L"%s 라인\n");
	addPair("Opening HTML documentation %s\n", L"HTML 문서를 열기 %s\n");
	addPair("Invalid default options:", L"잘못된 기본 옵션:");
	addPair("Invalid project options:", L"잘못된 프로젝트 옵션:");
	addPair("Invalid command line options:", L"잘못된 명령줄 옵션 :");
	addPair("For help on options type 'astyle -h'", L"도움말을 보려면 옵션 유형 'astyle - H'를 사용합니다");
	addPair("Cannot open default option file", L"기본 옵션 파일을 열 수 없습니다.");
	addPair("Cannot open project option file", L"프로젝트 옵션 파일을 열 수 없습니다.");
	addPair("Cannot open directory", L"디렉토리를 열지 못했습니다");
	addPair("Cannot open HTML file %s\n", L"HTML 파일을 열 수 없습니다 %s\n");
	addPair("Command execute failure", L"명령 실패를 실행");
	addPair("Command is not installed", L"명령이 설치되어 있지 않습니다");
	addPair("Missing filename in %s\n", L"%s 에서 누락된 파일 이름\n");
	addPair("Recursive option with no wildcard", L"와일드 카드없이 재귀 옵션");
	addPair("Did you intend quote the filename", L"당신은 파일 이름을 인용하고자하나요");
	addPair("No file to process %s\n", L"처리할 파일이 없습니다 %s\n");
	addPair("Did you intend to use --recursive", L"--recursive 를 사용하고자 하십니까");
	addPair("Cannot process UTF-32 encoding", L"UTF-32 인코딩을 처리할 수 없습니다");
	addPair("Artistic Style has terminated\n", L"Artistic Style를 종료합니다\n");
}

Norwegian::Norwegian()	// Norsk
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formatert  %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"Uendret    %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"Katalog  %s\n");
	addPair("Default option file  %s\n", L"Standard alternativfil  %s\n");
	addPair("Project option file  %s\n", L"Prosjekt opsjonsfil  %s\n");
	addPair("Exclude  %s\n", L"Ekskluder  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Ekskluder (uovertruffen)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formatert   %s uendret   ");
	addPair(" seconds   ", L" sekunder   ");
	addPair("%d min %d sec   ", L"%d min %d sek?   ");
	addPair("%s lines\n", L"%s linjer\n");
	addPair("Opening HTML documentation %s\n", L"Åpning HTML dokumentasjon %s\n");
	addPair("Invalid default options:", L"Ugyldige standardalternativer:");
	addPair("Invalid project options:", L"Ugyldige prosjektalternativer:");
	addPair("Invalid command line options:", L"Kommandolinjevalg Ugyldige:");
	addPair("For help on options type 'astyle -h'", L"For hjelp til alternativer type 'astyle -h'");
	addPair("Cannot open default option file", L"Kan ikke åpne standardvalgsfilen");
	addPair("Cannot open project option file", L"Kan ikke åpne prosjektvalgsfilen");
	addPair("Cannot open directory", L"Kan ikke åpne katalog");
	addPair("Cannot open HTML file %s\n", L"Kan ikke åpne HTML-fil %s\n");
	addPair("Command execute failure", L"Command utføre svikt");
	addPair("Command is not installed", L"Command er ikke installert");
	addPair("Missing filename in %s\n", L"Mangler filnavn i %s\n");
	addPair("Recursive option with no wildcard", L"Rekursiv alternativ uten wildcard");
	addPair("Did you intend quote the filename", L"Har du tenkt sitere filnavnet");
	addPair("No file to process %s\n", L"Ingen fil å behandle %s\n");
	addPair("Did you intend to use --recursive", L"Har du tenkt å bruke --recursive");
	addPair("Cannot process UTF-32 encoding", L"Kan ikke behandle UTF-32 koding");
	addPair("Artistic Style has terminated\n", L"Artistic Style har avsluttet\n");
}

Polish::Polish()	// Polski
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Sformatowany  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Niezmienione  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Katalog  %s\n");
	addPair("Default option file  %s\n", L"Domyślny plik opcji  %s\n");
	addPair("Project option file  %s\n", L"Plik opcji projektu  %s\n");
	addPair("Exclude  %s\n", L"Wykluczać  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Wyklucz (niezrównany)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s sformatowany   %s niezmienione   ");
	addPair(" seconds   ", L" sekund   ");
	addPair("%d min %d sec   ", L"%d min %d sek   ");
	addPair("%s lines\n", L"%s linii\n");
	addPair("Opening HTML documentation %s\n", L"Otwarcie dokumentacji HTML %s\n");
	addPair("Invalid default options:", L"Nieprawidłowe opcje domyślne:");
	addPair("Invalid project options:", L"Nieprawidłowe opcje projektu:");
	addPair("Invalid command line options:", L"Nieprawidłowe opcje wiersza polecenia:");
	addPair("For help on options type 'astyle -h'", L"Aby uzyskać pomoc od rodzaju opcji 'astyle -h'");
	addPair("Cannot open default option file", L"Nie można otworzyć pliku opcji domyślnych");
	addPair("Cannot open project option file", L"Nie można otworzyć pliku opcji projektu");
	addPair("Cannot open directory", L"Nie można otworzyć katalogu");
	addPair("Cannot open HTML file %s\n", L"Nie można otworzyć pliku HTML %s\n");
	addPair("Command execute failure", L"Wykonaj polecenia niepowodzenia");
	addPair("Command is not installed", L"Polecenie nie jest zainstalowany");
	addPair("Missing filename in %s\n", L"Brakuje pliku w %s\n");
	addPair("Recursive option with no wildcard", L"Rekurencyjne opcja bez symboli");
	addPair("Did you intend quote the filename", L"Czy zamierza Pan podać nazwę pliku");
	addPair("No file to process %s\n", L"Brak pliku do procesu %s\n");
	addPair("Did you intend to use --recursive", L"Czy masz zamiar używać --recursive");
	addPair("Cannot process UTF-32 encoding", L"Nie można procesu kodowania UTF-32");
	addPair("Artistic Style has terminated\n", L"Artistic Style został zakończony\n");
}

Portuguese::Portuguese()	// Português
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formatado   %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Inalterado  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Diretório  %s\n");
	addPair("Default option file  %s\n", L"Arquivo de opção padrão  %s\n");
	addPair("Project option file  %s\n", L"Arquivo de opção de projeto  %s\n");
	addPair("Exclude  %s\n", L"Excluir  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Excluir (incomparável)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formatado   %s inalterado   ");
	addPair(" seconds   ", L" segundo   ");
	addPair("%d min %d sec   ", L"%d min %d seg   ");
	addPair("%s lines\n", L"%s linhas\n");
	addPair("Opening HTML documentation %s\n", L"Abrindo a documentação HTML %s\n");
	addPair("Invalid default options:", L"Opções padrão inválidas:");
	addPair("Invalid project options:", L"Opções de projeto inválidas:");
	addPair("Invalid command line options:", L"Opções de linha de comando inválida:");
	addPair("For help on options type 'astyle -h'", L"Para obter ajuda sobre as opções de tipo 'astyle -h'");
	addPair("Cannot open default option file", L"Não é possível abrir o arquivo de opção padrão");
	addPair("Cannot open project option file", L"Não é possível abrir o arquivo de opção do projeto");
	addPair("Cannot open directory", L"Não é possível abrir diretório");
	addPair("Cannot open HTML file %s\n", L"Não é possível abrir arquivo HTML %s\n");
	addPair("Command execute failure", L"Executar falha de comando");
	addPair("Command is not installed", L"Comando não está instalado");
	addPair("Missing filename in %s\n", L"Filename faltando em %s\n");
	addPair("Recursive option with no wildcard", L"Opção recursiva sem curinga");
	addPair("Did you intend quote the filename", L"Será que você pretende citar o nome do arquivo");
	addPair("No file to process %s\n", L"Nenhum arquivo para processar %s\n");
	addPair("Did you intend to use --recursive", L"Será que você pretende usar --recursive");
	addPair("Cannot process UTF-32 encoding", L"Não pode processar a codificação UTF-32");
	addPair("Artistic Style has terminated\n", L"Artistic Style terminou\n");
}

Romanian::Romanian()	// Română
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formatat    %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Neschimbat  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Director  %s\n");
	addPair("Default option file  %s\n", L"Fișier opțional implicit  %s\n");
	addPair("Project option file  %s\n", L"Fișier opțiune proiect  %s\n");
	addPair("Exclude  %s\n", L"Excludeți  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Excludeți (necompensată)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formatat   %s neschimbat   ");
	addPair(" seconds   ", L" secunde   ");
	addPair("%d min %d sec   ", L"%d min %d sec   ");
	addPair("%s lines\n", L"%s linii\n");
	addPair("Opening HTML documentation %s\n", L"Documentație HTML deschidere %s\n");
	addPair("Invalid default options:", L"Opțiuni implicite nevalide:");
	addPair("Invalid project options:", L"Opțiunile de proiect nevalide:");
	addPair("Invalid command line options:", L"Opțiuni de linie de comandă nevalide:");
	addPair("For help on options type 'astyle -h'", L"Pentru ajutor cu privire la tipul de opțiuni 'astyle -h'");
	addPair("Cannot open default option file", L"Nu se poate deschide fișierul cu opțiuni implicite");
	addPair("Cannot open project option file", L"Nu se poate deschide fișierul cu opțiuni de proiect");
	addPair("Cannot open directory", L"Nu se poate deschide directorul");
	addPair("Cannot open HTML file %s\n", L"Nu se poate deschide fișierul HTML %s\n");
	addPair("Command execute failure", L"Comandă executa eșec");
	addPair("Command is not installed", L"Comanda nu este instalat");
	addPair("Missing filename in %s\n", L"Lipsă nume de fișier %s\n");
	addPair("Recursive option with no wildcard", L"Opțiunea recursiv cu nici un wildcard");
	addPair("Did you intend quote the filename", L"V-intentionati cita numele de fișier");
	addPair("No file to process %s\n", L"Nu există un fișier pentru a procesa %s\n");
	addPair("Did you intend to use --recursive", L"V-ați intenționați să utilizați --recursive");
	addPair("Cannot process UTF-32 encoding", L"Nu se poate procesa codificarea UTF-32");
	addPair("Artistic Style has terminated\n", L"Artistic Style a terminat\n");
}

Russian::Russian()	// русский
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Форматированный  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"без изменений    %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"каталог  %s\n");
	addPair("Default option file  %s\n", L"Файл с опцией по умолчанию  %s\n");
	addPair("Project option file  %s\n", L"Файл опций проекта  %s\n");
	addPair("Exclude  %s\n", L"исключать  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Исключить (непревзойденный)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s Форматированный   %s без изменений   ");
	addPair(" seconds   ", L" секунды   ");
	addPair("%d min %d sec   ", L"%d мин %d сек   ");
	addPair("%s lines\n", L"%s линий\n");
	addPair("Opening HTML documentation %s\n", L"Открытие HTML документации %s\n");
	addPair("Invalid default options:", L"Недействительные параметры по умолчанию:");
	addPair("Invalid project options:", L"Недопустимые параметры проекта:");
	addPair("Invalid command line options:", L"Недопустимые параметры командной строки:");
	addPair("For help on options type 'astyle -h'", L"Для получения справки по 'astyle -h' опций типа");
	addPair("Cannot open default option file", L"Не удается открыть файл параметров по умолчанию");
	addPair("Cannot open project option file", L"Не удается открыть файл опций проекта");
	addPair("Cannot open directory", L"Не могу открыть каталог");
	addPair("Cannot open HTML file %s\n", L"Не удается открыть файл HTML %s\n");
	addPair("Command execute failure", L"Выполнить команду недостаточности");
	addPair("Command is not installed", L"Не установлен Команда");
	addPair("Missing filename in %s\n", L"Отсутствует имя файла в %s\n");
	addPair("Recursive option with no wildcard", L"Рекурсивный вариант без каких-либо шаблона");
	addPair("Did you intend quote the filename", L"Вы намерены цитатой файла");
	addPair("No file to process %s\n", L"Нет файлов для обработки %s\n");
	addPair("Did you intend to use --recursive", L"Неужели вы собираетесь использовать --recursive");
	addPair("Cannot process UTF-32 encoding", L"Не удается обработать UTF-32 кодировке");
	addPair("Artistic Style has terminated\n", L"Artistic Style прекратил\n");
}

Spanish::Spanish()	// Español
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formato     %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Inalterado  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Directorio  %s\n");
	addPair("Default option file  %s\n", L"Archivo de opciones predeterminado  %s\n");
	addPair("Project option file  %s\n", L"Archivo de opciones del proyecto  %s\n");
	addPair("Exclude  %s\n", L"Excluir  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Excluir (incomparable)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formato   %s inalterado   ");
	addPair(" seconds   ", L" segundo   ");
	addPair("%d min %d sec   ", L"%d min %d seg   ");
	addPair("%s lines\n", L"%s líneas\n");
	addPair("Opening HTML documentation %s\n", L"Apertura de documentación HTML %s\n");
	addPair("Invalid default options:", L"Opciones predeterminadas no válidas:");
	addPair("Invalid project options:", L"Opciones de proyecto no válidas:");
	addPair("Invalid command line options:", L"No válido opciones de línea de comando:");
	addPair("For help on options type 'astyle -h'", L"Para obtener ayuda sobre las opciones tipo 'astyle -h'");
	addPair("Cannot open default option file", L"No se puede abrir el archivo de opciones predeterminado");
	addPair("Cannot open project option file", L"No se puede abrir el archivo de opciones del proyecto");
	addPair("Cannot open directory", L"No se puede abrir el directorio");
	addPair("Cannot open HTML file %s\n", L"No se puede abrir el archivo HTML %s\n");
	addPair("Command execute failure", L"Ejecutar el fracaso de comandos");
	addPair("Command is not installed", L"El comando no está instalado");
	addPair("Missing filename in %s\n", L"Falta nombre del archivo en %s\n");
	addPair("Recursive option with no wildcard", L"Recursiva opción sin comodín");
	addPair("Did you intend quote the filename", L"Se tiene la intención de citar el nombre de archivo");
	addPair("No file to process %s\n", L"No existe el fichero a procesar %s\n");
	addPair("Did you intend to use --recursive", L"Se va a utilizar --recursive");
	addPair("Cannot process UTF-32 encoding", L"No se puede procesar la codificación UTF-32");
	addPair("Artistic Style has terminated\n", L"Artistic Style ha terminado\n");
}

Swedish::Swedish()	// Svenska
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formaterade  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Oförändrade  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Katalog  %s\n");
	addPair("Default option file  %s\n", L"Standardalternativsfil  %s\n");
	addPair("Project option file  %s\n", L"Projektalternativ fil  %s\n");
	addPair("Exclude  %s\n", L"Uteslut  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Uteslut (oöverträffad)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formaterade   %s oförändrade   ");
	addPair(" seconds   ", L" sekunder   ");
	addPair("%d min %d sec   ", L"%d min %d sek   ");
	addPair("%s lines\n", L"%s linjer\n");
	addPair("Opening HTML documentation %s\n", L"Öppna HTML-dokumentation %s\n");
	addPair("Invalid default options:", L"Ogiltiga standardalternativ:");
	addPair("Invalid project options:", L"Ogiltiga projektalternativ:");
	addPair("Invalid command line options:", L"Ogiltig kommandoraden alternativ:");
	addPair("For help on options type 'astyle -h'", L"För hjälp om alternativ typ 'astyle -h'");
	addPair("Cannot open default option file", L"Kan inte öppna standardalternativsfilen");
	addPair("Cannot open project option file", L"Kan inte öppna projektalternativsfilen");
	addPair("Cannot open directory", L"Kan inte öppna katalog");
	addPair("Cannot open HTML file %s\n", L"Kan inte öppna HTML-filen %s\n");
	addPair("Command execute failure", L"Utför kommando misslyckande");
	addPair("Command is not installed", L"Kommandot är inte installerat");
	addPair("Missing filename in %s\n", L"Saknade filnamn i %s\n");
	addPair("Recursive option with no wildcard", L"Rekursiva alternativ utan jokertecken");
	addPair("Did you intend quote the filename", L"Visste du tänker citera filnamnet");
	addPair("No file to process %s\n", L"Ingen fil att bearbeta %s\n");
	addPair("Did you intend to use --recursive", L"Har du för avsikt att använda --recursive");
	addPair("Cannot process UTF-32 encoding", L"Kan inte hantera UTF-32 kodning");
	addPair("Artistic Style has terminated\n", L"Artistic Style har upphört\n");
}

Ukrainian::Ukrainian()	// Український
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"форматований  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"без змін      %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Каталог  %s\n");
	addPair("Default option file  %s\n", L"Файл параметра за замовчуванням  %s\n");
	addPair("Project option file  %s\n", L"Файл варіанту проекту  %s\n");
	addPair("Exclude  %s\n", L"Виключити  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Виключити (неперевершений)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s відформатований   %s без змін   ");
	addPair(" seconds   ", L" секунди   ");
	addPair("%d min %d sec   ", L"%d хви %d cek   ");
	addPair("%s lines\n", L"%s ліній\n");
	addPair("Opening HTML documentation %s\n", L"Відкриття HTML документації %s\n");
	addPair("Invalid default options:", L"Недійсні параметри за умовчанням:");
	addPair("Invalid project options:", L"Недійсні параметри проекту:");
	addPair("Invalid command line options:", L"Неприпустима параметри командного рядка:");
	addPair("For help on options type 'astyle -h'", L"Для отримання довідки по 'astyle -h' опцій типу");
	addPair("Cannot open default option file", L"Неможливо відкрити файл параметрів за замовчуванням");
	addPair("Cannot open project option file", L"Неможливо відкрити файл параметрів проекту");
	addPair("Cannot open directory", L"Не можу відкрити каталог");
	addPair("Cannot open HTML file %s\n", L"Не вдається відкрити файл HTML %s\n");
	addPair("Command execute failure", L"Виконати команду недостатності");
	addPair("Command is not installed", L"Не встановлений Команда");
	addPair("Missing filename in %s\n", L"Відсутня назва файлу в %s\n");
	addPair("Recursive option with no wildcard", L"Рекурсивний варіант без будь-яких шаблону");
	addPair("Did you intend quote the filename", L"Ви маєте намір цитатою файлу");
	addPair("No file to process %s\n", L"Немає файлів для обробки %s\n");
	addPair("Did you intend to use --recursive", L"Невже ви збираєтеся використовувати --recursive");
	addPair("Cannot process UTF-32 encoding", L"Не вдається обробити UTF-32 кодуванні");
	addPair("Artistic Style has terminated\n", L"Artistic Style припинив\n");
}


#endif	// ASTYLE_LIB

}   // end of namespace astyle

