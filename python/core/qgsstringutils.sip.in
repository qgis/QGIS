/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsstringutils.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsStringReplacement
{
%Docstring
 A representation of a single string replacement.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsstringutils.h"
%End
  public:

    QgsStringReplacement( const QString &match,
                          const QString &replacement,
                          bool caseSensitive = false,
                          bool wholeWordOnly = false );
%Docstring
Constructor for QgsStringReplacement.

:param match: string to match
:param replacement: string to replace match with
:param caseSensitive: set to true for a case sensitive match
:param wholeWordOnly: set to true to match complete words only, or false to allow partial word matches
%End

    QString match() const;
%Docstring
Returns the string matched by this object
%End

    QString replacement() const;
%Docstring
Returns the string to replace matches with
%End

    bool caseSensitive() const;
%Docstring
Returns true if match is case sensitive
%End

    bool wholeWordOnly() const;
%Docstring
Returns true if match only applies to whole words, or false if partial word matches are permitted
%End

    QString process( const QString &input ) const;
%Docstring
Processes a given input string, applying any valid replacements which should be made.

:param input: input string

:return: input string with any matches replaced by replacement string
%End

    bool operator==( const QgsStringReplacement &other );

    QgsStringMap properties() const;
%Docstring
Returns a map of the replacement properties.

.. seealso:: :py:func:`fromProperties`
%End

    static QgsStringReplacement fromProperties( const QgsStringMap &properties );
%Docstring
Creates a new QgsStringReplacement from an encoded properties map.

.. seealso:: :py:func:`properties`
%End

};



class QgsStringReplacementCollection
{
%Docstring
 A collection of string replacements (specified using QgsStringReplacement objects).

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsstringutils.h"
%End
  public:

    QgsStringReplacementCollection( const QList< QgsStringReplacement > &replacements = QList< QgsStringReplacement >() );
%Docstring
Constructor for QgsStringReplacementCollection

:param replacements: initial list of string replacements
%End

    QList< QgsStringReplacement > replacements() const;
%Docstring
Returns the list of string replacements in this collection.

.. seealso:: :py:func:`setReplacements`
%End

    void setReplacements( const QList< QgsStringReplacement > &replacements );
%Docstring
Sets the list of string replacements in this collection.

:param replacements: list of string replacements to apply. Replacements are applied in the
order they are specified here.

.. seealso:: :py:func:`replacements`
%End

    QString process( const QString &input ) const;
%Docstring
Processes a given input string, applying any valid replacements which should be made
using QgsStringReplacement objects contained by this collection. Replacements
are made in order of the QgsStringReplacement objects contained in the collection.

:param input: input string

:return: input string with any matches replaced by replacement string
%End

    void writeXml( QDomElement &elem, QDomDocument &doc ) const;
%Docstring
Writes the collection state to an XML element.

:param elem: target DOM element
:param doc: DOM document

.. seealso:: :py:func:`readXml`
%End

    void readXml( const QDomElement &elem );
%Docstring
Reads the collection state from an XML element.

:param elem: DOM element

.. seealso:: :py:func:`writeXml`
%End

};


class QgsStringUtils
{
%Docstring
 Utility functions for working with strings.

.. versionadded:: 2.11
%End

%TypeHeaderCode
#include "qgsstringutils.h"
%End
  public:

    enum Capitalization
    {
      MixedCase,
      AllUppercase,
      AllLowercase,
      ForceFirstLetterToCapital,
    };

    static QString capitalize( const QString &string, Capitalization capitalization );
%Docstring
Converts a string by applying capitalization rules to the string.

:param string: input string
:param capitalization: capitalization type to apply

:return: capitalized string

.. versionadded:: 3.0
%End

    static int levenshteinDistance( const QString &string1, const QString &string2, bool caseSensitive = false );
%Docstring
Returns the Levenshtein edit distance between two strings. This equates to the minimum
number of character edits (insertions, deletions or substitutions) required to change
one string to another.

:param string1: first string
:param string2: second string
:param caseSensitive: set to true for case sensitive comparison

:return: edit distance. Lower distances indicate more similar strings.
%End

    static QString longestCommonSubstring( const QString &string1, const QString &string2, bool caseSensitive = false );
%Docstring
Returns the longest common substring between two strings. This substring is the longest
string that is a substring of the two input strings. For example, the longest common substring
of "ABABC" and "BABCA" is "ABC".

:param string1: first string
:param string2: second string
:param caseSensitive: set to true for case sensitive comparison

:return: longest common substring
%End

    static int hammingDistance( const QString &string1, const QString &string2, bool caseSensitive = false );
%Docstring
Returns the Hamming distance between two strings. This equates to the number of characters at
corresponding positions within the input strings where the characters are different. The input
strings must be the same length.

:param string1: first string
:param string2: second string
:param caseSensitive: set to true for case sensitive comparison

:return: Hamming distance between strings, or -1 if strings are different lengths.
%End

    static QString soundex( const QString &string );
%Docstring
Returns the Soundex representation of a string. Soundex is a phonetic matching algorithm,
so strings with similar sounds should be represented by the same Soundex code.

:param string: input string

:return: 4 letter Soundex code
%End

    static QString insertLinks( const QString &string, bool *foundLinks = 0 );
%Docstring
Returns a string with any URL (e.g., http(s)/ftp) and mailto: text converted to valid HTML <a ...>
links.

:param string: string to insert links into
:param foundLinks: if specified, will be set to true if any links were inserted into the string

:return: string with inserted links

.. versionadded:: 3.0
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsstringutils.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
