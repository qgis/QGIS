/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   ASLocalizer.h
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

#ifndef ASLOCALIZER_H
#define ASLOCALIZER_H

#include <string>
#include <vector>

namespace astyle {

using namespace std;

#ifndef ASTYLE_LIB

//-----------------------------------------------------------------------------
// ASLocalizer class for console build.
// This class encapsulates all language-dependent settings and is a
// generalization of the C locale concept.
//-----------------------------------------------------------------------------
class Translation;

class ASLocalizer
{
	public:		// functions
		ASLocalizer();
		virtual ~ASLocalizer();
		string getLanguageID() const;
		const Translation* getTranslationClass() const;
#ifdef _WIN32
		void setLanguageFromLCID(size_t lcid);
#endif
		void setLanguageFromName(const char* langID);
		const char* settext(const char* textIn) const;

	private:	// functions
		void setTranslationClass();

	private:	// variables
		Translation* m_translation;		// pointer to a polymorphic Translation class
		string m_langID;				// language identifier from the locale
		string m_subLangID;				// sub language identifier, if needed
		string m_localeName;			// name of the current locale (Linux only)
		size_t m_lcid;					// LCID of the user locale (Windows only)
};

//----------------------------------------------------------------------------
// Translation base class.
//----------------------------------------------------------------------------

class Translation
// This base class is inherited by the language translation classes.
// Polymorphism is used to call the correct language translator.
// This class contains the translation vector and settext translation method.
// The language vector is built by the language sub classes.
// NOTE: This class must have virtual methods for typeid() to work.
//       typeid() is used by AStyleTestI18n_Localizer.cpp.
{
	public:
		Translation() {}
		virtual ~Translation() {}
		string convertToMultiByte(const wstring &wideStr) const;
		size_t getTranslationVectorSize() const;
		bool getWideTranslation(const string &stringIn, wstring &wideOut) const;
		string &translate(const string &stringIn) const;

	protected:
		void addPair(const string &english, const wstring &translated);
		// variables
		vector<pair<string, wstring> > m_translation;		// translation vector
};

//----------------------------------------------------------------------------
// Translation classes
// One class for each language.
// These classes have only a constructor which builds the language vector.
//----------------------------------------------------------------------------

class ChineseSimplified : public Translation
{
	public:
		ChineseSimplified();
};

class ChineseTraditional : public Translation
{
	public:
		ChineseTraditional();
};

class Dutch : public Translation
{
	public:
		Dutch();
};

class English : public Translation
{
	public:
		English();
};

class Finnish : public Translation
{
	public:
		Finnish();
};

class French : public Translation
{
	public:
		French();
};

class German : public Translation
{
	public:
		German();
};

class Hindi : public Translation
{
	public:
		Hindi();
};

class Italian : public Translation
{
	public:
		Italian();
};

class Japanese : public Translation
{
	public:
		Japanese();
};

class Korean : public Translation
{
	public:
		Korean();
};

class Polish : public Translation
{
	public:
		Polish();
};

class Portuguese : public Translation
{
	public:
		Portuguese();
};

class Russian : public Translation
{
	public:
		Russian();
};

class Spanish : public Translation
{
	public:
		Spanish();
};

class Swedish : public Translation
{
	public:
		Swedish();
};

class Ukrainian : public Translation
{
	public:
		Ukrainian();
};


#endif	//  ASTYLE_LIB

}	// namespace astyle

#endif	//  ASLOCALIZER_H
