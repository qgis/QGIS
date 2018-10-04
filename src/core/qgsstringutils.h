/***************************************************************************
    qgsstringutils.h
    ----------------
    begin                : June 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis_core.h"
#include <QString>
#include <QRegExp>
#include <QList>
#include <QDomDocument>
#include <QFont> // for enum values
#include "qgis.h"

#ifndef QGSSTRINGUTILS_H
#define QGSSTRINGUTILS_H


/**
 * \ingroup core
 * \class QgsStringReplacement
 * \brief A representation of a single string replacement.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsStringReplacement
{

  public:

    /**
     * Constructor for QgsStringReplacement.
     * \param match string to match
     * \param replacement string to replace match with
     * \param caseSensitive set to true for a case sensitive match
     * \param wholeWordOnly set to true to match complete words only, or false to allow partial word matches
     */
    QgsStringReplacement( const QString &match,
                          const QString &replacement,
                          bool caseSensitive = false,
                          bool wholeWordOnly = false );

    //! Returns the string matched by this object
    QString match() const { return mMatch; }

    //! Returns the string to replace matches with
    QString replacement() const { return mReplacement; }

    //! Returns true if match is case sensitive
    bool caseSensitive() const { return mCaseSensitive; }

    //! Returns true if match only applies to whole words, or false if partial word matches are permitted
    bool wholeWordOnly() const { return mWholeWordOnly; }

    /**
     * Processes a given input string, applying any valid replacements which should be made.
     * \param input input string
     * \returns input string with any matches replaced by replacement string
     */
    QString process( const QString &input ) const;

    bool operator==( const QgsStringReplacement &other )
    {
      return mMatch == other.mMatch
             && mReplacement == other.mReplacement
             && mCaseSensitive == other.mCaseSensitive
             && mWholeWordOnly == other.mWholeWordOnly;
    }

    /**
     * Returns a map of the replacement properties.
     * \see fromProperties()
     */
    QgsStringMap properties() const;

    /**
     * Creates a new QgsStringReplacement from an encoded properties map.
     * \see properties()
     */
    static QgsStringReplacement fromProperties( const QgsStringMap &properties );

  private:

    QString mMatch;

    QString mReplacement;

    bool mCaseSensitive;

    bool mWholeWordOnly;

    QRegExp mRx;
};


/**
 * \ingroup core
 * \class QgsStringReplacementCollection
 * \brief A collection of string replacements (specified using QgsStringReplacement objects).
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsStringReplacementCollection
{

  public:

    /**
     * Constructor for QgsStringReplacementCollection
     * \param replacements initial list of string replacements
     */
    QgsStringReplacementCollection( const QList< QgsStringReplacement > &replacements = QList< QgsStringReplacement >() )
      : mReplacements( replacements )
    {}

    /**
     * Returns the list of string replacements in this collection.
     * \see setReplacements()
     */
    QList< QgsStringReplacement > replacements() const { return mReplacements; }

    /**
     * Sets the list of string replacements in this collection.
     * \param replacements list of string replacements to apply. Replacements are applied in the
     * order they are specified here.
     * \see replacements()
     */
    void setReplacements( const QList< QgsStringReplacement > &replacements )
    {
      mReplacements = replacements;
    }

    /**
     * Processes a given input string, applying any valid replacements which should be made
     * using QgsStringReplacement objects contained by this collection. Replacements
     * are made in order of the QgsStringReplacement objects contained in the collection.
     * \param input input string
     * \returns input string with any matches replaced by replacement string
     */
    QString process( const QString &input ) const;

    /**
     * Writes the collection state to an XML element.
     * \param elem target DOM element
     * \param doc DOM document
     * \see readXml()
     */
    void writeXml( QDomElement &elem, QDomDocument &doc ) const;

    /**
     * Reads the collection state from an XML element.
     * \param elem DOM element
     * \see writeXml()
     */
    void readXml( const QDomElement &elem );

  private:

    QList< QgsStringReplacement > mReplacements;


};

/**
 * \ingroup core
 * \class QgsStringUtils
 * \brief Utility functions for working with strings.
 * \since QGIS 2.11
 */

class CORE_EXPORT QgsStringUtils
{
  public:

    //! Capitalization options
    enum Capitalization
    {
      MixedCase = QFont::MixedCase, //!< Mixed case, ie no change
      AllUppercase = QFont::AllUppercase, //!< Convert all characters to uppercase
      AllLowercase = QFont::AllLowercase,  //!< Convert all characters to lowercase
      ForceFirstLetterToCapital = QFont::Capitalize, //!< Convert just the first letter of each word to uppercase, leave the rest untouched
      TitleCase = QFont::Capitalize + 1000, //!< Simple title case conversion - does not fully grammatically parse the text and uses simple rules only. Note that this method does not convert any characters to lowercase, it only uppercases required letters. Callers must ensure that input strings are already lowercased.
    };

    /**
     * Converts a string by applying capitalization rules to the string.
     * \param string input string
     * \param capitalization capitalization type to apply
     * \returns capitalized string
     * \since QGIS 3.0
     */
    static QString capitalize( const QString &string, Capitalization capitalization );

    /**
     * Makes a raw string safe for inclusion as a HTML/XML string literal.
     *
     * This includes replacing '<' with '&lt;', '>' with '&gt;', '&' with '&amp', and
     * any extended unicode characters with the XML style &#233; encoded versions
     * of these characters.
     * \since QGIS 3.2
     */
    static QString ampersandEncode( const QString &string );

    /**
     * Returns the Levenshtein edit distance between two strings. This equates to the minimum
     * number of character edits (insertions, deletions or substitutions) required to change
     * one string to another.
     * \param string1 first string
     * \param string2 second string
     * \param caseSensitive set to true for case sensitive comparison
     * \returns edit distance. Lower distances indicate more similar strings.
     */
    static int levenshteinDistance( const QString &string1, const QString &string2, bool caseSensitive = false );

    /**
     * Returns the longest common substring between two strings. This substring is the longest
     * string that is a substring of the two input strings. For example, the longest common substring
     * of "ABABC" and "BABCA" is "ABC".
     * \param string1 first string
     * \param string2 second string
     * \param caseSensitive set to true for case sensitive comparison
     * \returns longest common substring
     */
    static QString longestCommonSubstring( const QString &string1, const QString &string2, bool caseSensitive = false );

    /**
     * Returns the Hamming distance between two strings. This equates to the number of characters at
     * corresponding positions within the input strings where the characters are different. The input
     * strings must be the same length.
     * \param string1 first string
     * \param string2 second string
     * \param caseSensitive set to true for case sensitive comparison
     * \returns Hamming distance between strings, or -1 if strings are different lengths.
     */
    static int hammingDistance( const QString &string1, const QString &string2, bool caseSensitive = false );

    /**
     * Returns the Soundex representation of a string. Soundex is a phonetic matching algorithm,
     * so strings with similar sounds should be represented by the same Soundex code.
     * \param string input string
     * \returns 4 letter Soundex code
     */
    static QString soundex( const QString &string );

    /**
     * Returns a string with any URL (e.g., http(s)/ftp) and mailto: text converted to valid HTML <a ...>
     * links.
     * \param string string to insert links into
     * \param foundLinks if specified, will be set to true if any links were inserted into the string
     * \returns string with inserted links
     * \since QGIS 3.0
     */
    static QString insertLinks( const QString &string, bool *foundLinks = nullptr );

    /**
     * Automatically wraps a \a string by inserting new line characters at appropriate locations in the string.
     *
     * The \a length argument specifies either the minimum or maximum length of lines desired, depending
     * on whether \a useMaxLineLength is true. If \a useMaxLineLength is true, then the string will be wrapped
     * so that each line ideally will not exceed \a length of characters. If \a useMaxLineLength is false, then
     * the string will be wrapped so that each line will ideally exceed \a length of characters.
     *
     * A custom delimiter can be specified to use instead of space characters.
     *
     * \since QGIS 3.4
     */
    static QString wordWrap( const QString &string, int length, bool useMaxLineLength = true, const QString &customDelimiter = QString() );
};

#endif //QGSSTRINGUTILS_H
