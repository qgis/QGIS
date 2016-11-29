//
//  FILE ENCODING IS UTF-8 WITHOUT A BOM.
//  русский    中文（简体）    日本    한국의
//
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   ASLocalizer.cpp
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
 *
 *   To add a new language:
 *
 *   Add a new translation class to ASLocalizer.h.
 *   Add the Add the English-Translation pair to the constructor in ASLocalizer.cpp.
 *   Update the WinLangCode array, if necessary.
 *   Add the language code to the function setTranslationClass().
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

#ifdef __DMC__
	#include <locale.h>
	// digital mars doesn't have these
	const size_t SUBLANG_CHINESE_MACAU = 5;
	const size_t LANG_HINDI = 57;
#endif

#ifdef __VMS
	#define __USE_STD_IOSTREAM 1
	#include <assert>
#else
	#include <cassert>
#endif

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <typeinfo>

#ifdef _MSC_VER
	#pragma warning(disable: 4996)  // secure version deprecation warnings
	// #pragma warning(disable: 4267)  // 64 bit signed/unsigned loss of data
#endif

#ifdef __BORLANDC__
	#pragma warn -8104	    // Local Static with constructor dangerous for multi-threaded apps
#endif

#ifdef __INTEL_COMPILER
	#pragma warning(disable:  383)  // value copied to temporary, reference to temporary used
	#pragma warning(disable:  981)  // operands are evaluated in unspecified order
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
	m_translation = NULL;

	// Not all compilers support the C++ function locale::global(locale(""));
	// For testing on Windows change the "Region and Language" settings or use AppLocale.
	// For testing on Linux change the LANG environment variable: LANG=fr_FR.UTF-8.
	// setlocale() will use the LANG environment variable on Linux.

	char* localeName = setlocale(LC_ALL, "");
	if (localeName == NULL)		// use the english (ascii) defaults
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
	{ LANG_CHINESE,    "zh" },
	{ LANG_DUTCH,      "nl" },
	{ LANG_ENGLISH,    "en" },
	{ LANG_FINNISH,    "fi" },
	{ LANG_FRENCH,     "fr" },
	{ LANG_GERMAN,     "de" },
	{ LANG_HINDI,      "hi" },
	{ LANG_ITALIAN,    "it" },
	{ LANG_JAPANESE,   "ja" },
	{ LANG_KOREAN,     "ko" },
	{ LANG_POLISH,     "pl" },
	{ LANG_PORTUGUESE, "pt" },
	{ LANG_RUSSIAN,    "ru" },
	{ LANG_SPANISH,    "es" },
	{ LANG_SWEDISH,    "sv" },
	{ LANG_UKRAINIAN,  "uk" },
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

#endif	// _win32

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
	static const size_t LEN_LANG = 2;

	m_lcid = 0;
	string langStr = langID;
	m_langID = langStr.substr(0, LEN_LANG);

	// need the sublang for chinese
	if (m_langID == "zh" && langStr[LEN_LANG] == '_')
	{
		string subLang = langStr.substr(LEN_LANG + 1, LEN_LANG);
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
	if (m_translation)
	{
		delete m_translation;
		m_translation = NULL;
	}
	if (m_langID == "zh" && m_subLangID == "CHS")
		m_translation = new ChineseSimplified;
	else if (m_langID == "zh" && m_subLangID == "CHT")
		m_translation = new ChineseTraditional;
	else if (m_langID == "nl")
		m_translation = new Dutch;
	else if (m_langID == "en")
		m_translation = new English;
	else if (m_langID == "fi")
		m_translation = new Finnish;
	else if (m_langID == "fr")
		m_translation = new French;
	else if (m_langID == "de")
		m_translation = new German;
	else if (m_langID == "hi")
		m_translation = new Hindi;
	else if (m_langID == "it")
		m_translation = new Italian;
	else if (m_langID == "ja")
		m_translation = new Japanese;
	else if (m_langID == "ko")
		m_translation = new Korean;
	else if (m_langID == "pl")
		m_translation = new Polish;
	else if (m_langID == "pt")
		m_translation = new Portuguese;
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

void Translation::addPair(const string &english, const wstring &translated)
// Add a string pair to the translation vector.
{
	pair<string, wstring> entry(english, translated);
	m_translation.push_back(entry);
}

string Translation::convertToMultiByte(const wstring &wideStr) const
// Convert wchar_t to a multibyte string using the currently assigned locale.
// Return an empty string if an error occurs.
{
	static bool msgDisplayed = false;
	// get length of the output excluding the NULL and validate the parameters
	size_t mbLen = wcstombs(NULL, wideStr.c_str(), 0);
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
	char* mbStr = new(nothrow) char[mbLen + 1];
	if (mbStr == NULL)
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
	delete [] mbStr;
	return mbTranslation;
}

size_t Translation::getTranslationVectorSize() const
// Return the translation vector size.  Used for testing.
{
	return m_translation.size();
}

bool Translation::getWideTranslation(const string &stringIn, wstring &wideOut) const
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

string &Translation::translate(const string &stringIn) const
// Translate a string.
// Return a static string instead of a member variable so the method can have a "const" designation.
// This allows "settext" to be called from a "const" method.
{
	static string mbTranslation;
	mbTranslation.clear();
	for (size_t i = 0; i < m_translation.size(); i++)
	{
		if (m_translation[i].first == stringIn)
		{
			mbTranslation = convertToMultiByte(m_translation[i].second);
			break;
		}
	}
	// not found, return english
	if (mbTranslation.empty())
		mbTranslation = stringIn;
	return mbTranslation;
}

//----------------------------------------------------------------------------
// Translation class methods.
// These classes have only a constructor which builds the language vector.
//----------------------------------------------------------------------------

ChineseSimplified::ChineseSimplified()	// 中文（简体）
{
	addPair("Formatted  %s\n", L"格式化  %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"未改变  %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"目录  %s\n");
	addPair("Exclude  %s\n", L"排除  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"排除（无匹配项） %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s 格式化   %s 未改变   ");
	addPair(" seconds   ", L" 秒   ");
	addPair("%d min %d sec   ", L"%d 分 %d 秒   ");
	addPair("%s lines\n", L"%s 行\n");
	addPair("Using default options file %s\n", L"使用默认配置文件 %s\n");
	addPair("Opening HTML documentation %s\n", L"打开HTML文档 %s\n");
	addPair("Invalid option file options:", L"无效的配置文件选项:");
	addPair("Invalid command line options:", L"无效的命令行选项:");
	addPair("For help on options type 'astyle -h'", L"输入 'astyle -h' 以获得有关命令行的帮助");
	addPair("Cannot open options file", L"无法打开配置文件");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style 已经终止运行");
}

ChineseTraditional::ChineseTraditional()	// 中文（繁體）
{
	addPair("Formatted  %s\n", L"格式化  %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"未改變  %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"目錄  %s\n");
	addPair("Exclude  %s\n", L"排除  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"排除（無匹配項） %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s 格式化 %s 未改變   ");
	addPair(" seconds   ", L" 秒   ");
	addPair("%d min %d sec   ", L"%d 分 %d 秒   ");
	addPair("%s lines\n", L"%s 行\n");
	addPair("Using default options file %s\n", L"使用默認配置文件 %s\n");
	addPair("Opening HTML documentation %s\n", L"打開HTML文檔 %s\n");
	addPair("Invalid option file options:", L"無效的配置文件選項:");
	addPair("Invalid command line options:", L"無效的命令行選項:");
	addPair("For help on options type 'astyle -h'", L"輸入'astyle -h'以獲得有關命令行的幫助:");
	addPair("Cannot open options file", L"無法打開配置文件");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style 已經終止運行");
}

Dutch::Dutch()	// Nederlandse
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Geformatteerd  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Onveranderd    %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Directory  %s\n");
	addPair("Exclude  %s\n", L"Uitsluiten  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Uitgesloten (ongeëvenaarde)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s geformatteerd   %s onveranderd   ");
	addPair(" seconds   ", L" seconden   ");
	addPair("%d min %d sec   ", L"%d min %d sec   ");
	addPair("%s lines\n", L"%s lijnen\n");
	addPair("Using default options file %s\n", L"Met behulp van standaard opties bestand %s\n");
	addPair("Opening HTML documentation %s\n", L"Het openen van HTML-documentatie %s\n");
	addPair("Invalid option file options:", L"Ongeldige optie file opties:");
	addPair("Invalid command line options:", L"Ongeldige command line opties:");
	addPair("For help on options type 'astyle -h'", L"Voor hulp bij 'astyle-h' opties het type");
	addPair("Cannot open options file", L"Kan niet worden geopend options bestand");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style heeft beëindigd");
}

English::English()
// this class is NOT translated
{}

Finnish::Finnish()	// Suomeksi
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Muotoiltu  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Ennallaan  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Directory  %s\n");
	addPair("Exclude  %s\n", L"Sulkea  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Sulkea (verraton)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s muotoiltu   %s ennallaan   ");
	addPair(" seconds   ", L" sekuntia   ");
	addPair("%d min %d sec   ", L"%d min %d sek   ");
	addPair("%s lines\n", L"%s linjat\n");
	addPair("Using default options file %s\n", L"Käyttämällä oletusasetuksia tiedosto %s\n");
	addPair("Opening HTML documentation %s\n", L"Avaaminen HTML asiakirjat %s\n");
	addPair("Invalid option file options:", L"Virheellinen vaihtoehto tiedosto vaihtoehtoja:");
	addPair("Invalid command line options:", L"Virheellinen komentorivin:");
	addPair("For help on options type 'astyle -h'", L"Apua vaihtoehdoista tyyppi 'astyle -h'");
	addPair("Cannot open options file", L"Ei voi avata vaihtoehtoja tiedostoa");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style on päättynyt");
}

French::French()	// Française
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formaté    %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Inchangée  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Répertoire  %s\n");
	addPair("Exclude  %s\n", L"Exclure  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Exclure (non appariés)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formaté   %s inchangée   ");
	addPair(" seconds   ", L" seconde   ");
	addPair("%d min %d sec   ", L"%d min %d sec   ");
	addPair("%s lines\n", L"%s lignes\n");
	addPair("Using default options file %s\n", L"Options par défaut utilisation du fichier %s\n");
	addPair("Opening HTML documentation %s\n", L"Ouverture documentation HTML %s\n");
	addPair("Invalid option file options:", L"Options Blancs option du fichier:");
	addPair("Invalid command line options:", L"Blancs options ligne de commande:");
	addPair("For help on options type 'astyle -h'", L"Pour de l'aide sur les options tapez 'astyle -h'");
	addPair("Cannot open options file", L"Impossible d'ouvrir le fichier d'options");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style a mis fin");
}

German::German()	// Deutsch
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formatiert   %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Unverändert  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Verzeichnis  %s\n");
	addPair("Exclude  %s\n", L"Ausschließen  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Ausschließen (unerreichte)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formatiert   %s unverändert   ");
	addPair(" seconds   ", L" sekunden   ");
	addPair("%d min %d sec   ", L"%d min %d sek   ");
	addPair("%s lines\n", L"%s linien\n");
	addPair("Using default options file %s\n", L"Mit Standard-Optionen Dat %s\n");
	addPair("Opening HTML documentation %s\n", L"Öffnen HTML-Dokumentation %s\n");
	addPair("Invalid option file options:", L"Ungültige Option Datei-Optionen:");
	addPair("Invalid command line options:", L"Ungültige Kommandozeilen-Optionen:");
	addPair("For help on options type 'astyle -h'", L"Für Hilfe zu den Optionen geben Sie 'astyle -h'");
	addPair("Cannot open options file", L"Kann nicht geöffnet werden Optionsdatei");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style ist beendet");
}

Hindi::Hindi()	// हिन्दी
// build the translation vector in the Translation base class
{
	// NOTE: Scintilla based editors (CodeBlocks) cannot always edit Hindi.
	//       Use Visual Studio instead.
	addPair("Formatted  %s\n", L"स्वरूपित किया  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"अपरिवर्तित     %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"निर्देशिका  %s\n");
	addPair("Exclude  %s\n", L"निकालना  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"अपवर्जित (बेजोड़)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s स्वरूपित किया   %s अपरिवर्तित   ");
	addPair(" seconds   ", L" सेकंड   ");
	addPair("%d min %d sec   ", L"%d मिनट %d सेकंड   ");
	addPair("%s lines\n", L"%s लाइनों\n");
	addPair("Using default options file %s\n", L"डिफ़ॉल्ट विकल्प का उपयोग कर फ़ाइल %s\n");
	addPair("Opening HTML documentation %s\n", L"एचटीएमएल प्रलेखन खोलना %s\n");
	addPair("Invalid option file options:", L"अवैध विकल्प फ़ाइल विकल्प हैं:");
	addPair("Invalid command line options:", L"कमांड लाइन विकल्प अवैध:");
	addPair("For help on options type 'astyle -h'", L"विकल्पों पर मदद के लिए प्रकार 'astyle -h'");
	addPair("Cannot open options file", L"विकल्प फ़ाइल नहीं खोल सकता है");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style समाप्त किया है");
}

Italian::Italian()	// Italiano
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formattata  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Immutato    %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Elenco  %s\n");
	addPair("Exclude  %s\n", L"Escludere  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Escludere (senza pari)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s ormattata   %s immutato   ");
	addPair(" seconds   ", L" secondo   ");
	addPair("%d min %d sec   ", L"%d min %d seg   ");
	addPair("%s lines\n", L"%s linee\n");
	addPair("Using default options file %s\n", L"Utilizzando file delle opzioni di default %s\n");
	addPair("Opening HTML documentation %s\n", L"Apertura di documenti HTML %s\n");
	addPair("Invalid option file options:", L"Opzione non valida file delle opzioni:");
	addPair("Invalid command line options:", L"Opzioni della riga di comando non valido:");
	addPair("For help on options type 'astyle -h'", L"Per informazioni sulle opzioni di tipo 'astyle-h'");
	addPair("Cannot open options file", L"Impossibile aprire il file opzioni");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style ha terminato");
}

Japanese::Japanese()	// 日本
{
	addPair("Formatted  %s\n", L"フォーマット  %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"変更          %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"ディレクトリ  %s\n");
	addPair("Exclude  %s\n", L"除外する  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"除外（マッチせず） %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %sフォーマット   %s 変更   ");
	addPair(" seconds   ", L" 秒   ");
	addPair("%d min %d sec   ", L"%d 分 %d 秒   ");
	addPair("%s lines\n", L"%s の行\n");
	addPair("Using default options file %s\n", L"デフォルトの設定ファイルを使用してください %s\n");
	addPair("Opening HTML documentation %s\n", L"HTML文書を開く %s\n");
	addPair("Invalid option file options:", L"無効なコンフィギュレーションファイルオプション：");
	addPair("Invalid command line options:", L"無効なコマンドラインオプション：");
	addPair("For help on options type 'astyle -h'", L"コマンドラインについてのヘルプは'astyle- h'を入力してください");
	addPair("Cannot open options file", L"コンフィギュレーションファイルを開くことができません");
	addPair("Cannot open directory", L"ディレクトリのオープンに失敗しました");
	addPair("Cannot open HTML file %s\n", L"HTMLファイルを開くことができません %s\n");
	addPair("Command execute failure", L"コマンドの失敗を実行");
	addPair("Command is not installed", L"コマンドがインストールされていません");
	addPair("Missing filename in %s\n", L"%s はファイル名で欠落しています\n");
	addPair("Recursive option with no wildcard", L"再帰的なオプションではワイルドカードではない");
	addPair("Did you intend quote the filename", L"あなたは、ファイル名を参照するつもり");
	addPair("No file to process %s\n", L"いいえファイルは処理できません %s\n");
	addPair("Did you intend to use --recursive", L"あなたが使用する予定 --recursive");
	addPair("Cannot process UTF-32 encoding", L"UTF- 32エンコーディングを処理できない");
	addPair("\nArtistic Style has terminated", L"\nArtistic Style 実行が終了しました");
}

Korean::Korean()	// 한국의
{
	addPair("Formatted  %s\n", L"수정됨    %s\n");		// should align with unchanged
	addPair("Unchanged  %s\n", L"변경없음  %s\n");		// should align with formatted
	addPair("Directory  %s\n", L"디렉토리  %s\n");
	addPair("Exclude  %s\n", L"제외됨   %s\n");
	addPair("Exclude (unmatched)  %s\n", L"제외 (NO 일치) %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s 수정됨   %s 변경없음   ");
	addPair(" seconds   ", L" 초   ");
	addPair("%d min %d sec   ", L"%d 분 %d 초   ");
	addPair("%s lines\n", L"%s 라인\n");
	addPair("Using default options file %s\n", L"기본 구성 파일을 사용 %s\n");
	addPair("Opening HTML documentation %s\n", L"HTML 문서를 열기 %s\n");
	addPair("Invalid option file options:", L"잘못된 구성 파일 옵션 :");
	addPair("Invalid command line options:", L"잘못된 명령줄 옵션 :");
	addPair("For help on options type 'astyle -h'", L"도움말을 보려면 옵션 유형 'astyle - H'를 사용합니다");
	addPair("Cannot open options file", L"구성 파일을 열 수 없습니다");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style를 종료합니다");
}

Polish::Polish()	// Polski
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Sformatowany  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Niezmienione  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Katalog  %s\n");
	addPair("Exclude  %s\n", L"Wykluczać  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Wyklucz (niezrównany)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s sformatowany   %s niezmienione   ");
	addPair(" seconds   ", L" sekund   ");
	addPair("%d min %d sec   ", L"%d min %d sek   ");
	addPair("%s lines\n", L"%s linii\n");
	addPair("Using default options file %s\n", L"Korzystanie z domyślnej opcji %s plik\n");
	addPair("Opening HTML documentation %s\n", L"Otwarcie dokumentacji HTML %s\n");
	addPair("Invalid option file options:", L"Nieprawidłowy opcji pliku opcji:");
	addPair("Invalid command line options:", L"Nieprawidłowe opcje wiersza polecenia:");
	addPair("For help on options type 'astyle -h'", L"Aby uzyskać pomoc od rodzaju opcji 'astyle -h'");
	addPair("Cannot open options file", L"Nie można otworzyć pliku opcji");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style został zakończony");
}

Portuguese::Portuguese()	// Português
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formatado   %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Inalterado  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Diretório  %s\n");
	addPair("Exclude  %s\n", L"Excluir  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Excluir (incomparável)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formatado   %s inalterado   ");
	addPair(" seconds   ", L" segundo   ");
	addPair("%d min %d sec   ", L"%d min %d seg   ");
	addPair("%s lines\n", L"%s linhas\n");
	addPair("Using default options file %s\n", L"Usando o arquivo de opções padrão %s\n");
	addPair("Opening HTML documentation %s\n", L"Abrindo a documentação HTML %s\n");
	addPair("Invalid option file options:", L"Opções de arquivo inválido opção:");
	addPair("Invalid command line options:", L"Opções de linha de comando inválida:");
	addPair("For help on options type 'astyle -h'", L"Para obter ajuda sobre as opções de tipo 'astyle -h'");
	addPair("Cannot open options file", L"Não é possível abrir arquivo de opções");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style terminou");
}

Russian::Russian()	// русский
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Форматированный  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"без изменений    %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"каталог  %s\n");
	addPair("Exclude  %s\n", L"исключать  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Исключить (непревзойденный)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s Форматированный   %s без изменений   ");
	addPair(" seconds   ", L" секунды   ");
	addPair("%d min %d sec   ", L"%d мин %d сек   ");
	addPair("%s lines\n", L"%s линий\n");
	addPair("Using default options file %s\n", L"Использование опции по умолчанию файл %s\n");
	addPair("Opening HTML documentation %s\n", L"Открытие HTML документации %s\n");
	addPair("Invalid option file options:", L"Недопустимый файл опций опцию:");
	addPair("Invalid command line options:", L"Недопустимые параметры командной строки:");
	addPair("For help on options type 'astyle -h'", L"Для получения справки по 'astyle -h' опций типа");
	addPair("Cannot open options file", L"Не удается открыть файл параметров");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style прекратил");
}

Spanish::Spanish()	// Español
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formato     %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Inalterado  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Directorio  %s\n");
	addPair("Exclude  %s\n", L"Excluir  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Excluir (incomparable)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formato   %s inalterado   ");
	addPair(" seconds   ", L" segundo   ");
	addPair("%d min %d sec   ", L"%d min %d seg   ");
	addPair("%s lines\n", L"%s líneas\n");
	addPair("Using default options file %s\n", L"Uso de las opciones por defecto del archivo %s\n");
	addPair("Opening HTML documentation %s\n", L"Apertura de documentación HTML %s\n");
	addPair("Invalid option file options:", L"Opción no válida opciones de archivo:");
	addPair("Invalid command line options:", L"No válido opciones de línea de comando:");
	addPair("For help on options type 'astyle -h'", L"Para obtener ayuda sobre las opciones tipo 'astyle -h'");
	addPair("Cannot open options file", L"No se puede abrir el archivo de opciones");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style ha terminado");
}

Swedish::Swedish()	// Svenska
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"Formaterade  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"Oförändrade  %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Katalog  %s\n");
	addPair("Exclude  %s\n", L"Uteslut  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Uteslut (oöverträffad)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s formaterade   %s oförändrade   ");
	addPair(" seconds   ", L" sekunder   ");
	addPair("%d min %d sec   ", L"%d min %d sek   ");
	addPair("%s lines\n", L"%s linjer\n");
	addPair("Using default options file %s\n", L"Använda standardalternativ fil %s\n");
	addPair("Opening HTML documentation %s\n", L"Öppna HTML-dokumentation %s\n");
	addPair("Invalid option file options:", L"Ogiltigt alternativ fil alternativ:");
	addPair("Invalid command line options:", L"Ogiltig kommandoraden alternativ:");
	addPair("For help on options type 'astyle -h'", L"För hjälp om alternativ typ 'astyle -h'");
	addPair("Cannot open options file", L"Kan inte öppna inställningsfilen");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style har upphört");
}

Ukrainian::Ukrainian()	// Український
// build the translation vector in the Translation base class
{
	addPair("Formatted  %s\n", L"форматований  %s\n");	// should align with unchanged
	addPair("Unchanged  %s\n", L"без змін      %s\n");	// should align with formatted
	addPair("Directory  %s\n", L"Каталог  %s\n");
	addPair("Exclude  %s\n", L"Виключити  %s\n");
	addPair("Exclude (unmatched)  %s\n", L"Виключити (неперевершений)  %s\n");
	addPair(" %s formatted   %s unchanged   ", L" %s відформатований   %s без змін   ");
	addPair(" seconds   ", L" секунди   ");
	addPair("%d min %d sec   ", L"%d хви %d cek   ");
	addPair("%s lines\n", L"%s ліній\n");
	addPair("Using default options file %s\n", L"Використання файлів опцій за замовчуванням %s\n");
	addPair("Opening HTML documentation %s\n", L"Відкриття HTML документації %s\n");
	addPair("Invalid option file options:", L"Неприпустимий файл опцій опцію:");
	addPair("Invalid command line options:", L"Неприпустима параметри командного рядка:");
	addPair("For help on options type 'astyle -h'", L"Для отримання довідки по 'astyle -h' опцій типу");
	addPair("Cannot open options file", L"Не вдається відкрити файл параметрів");
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
	addPair("\nArtistic Style has terminated", L"\nArtistic Style припинив");
}


#endif	// ASTYLE_LIB

}   // end of namespace astyle

