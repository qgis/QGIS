

/** \ingroup core
 * \class QgsStringReplacement
 * \brief A representation of a single string replacement.
 * \note Added in version 3.0
 */

class QgsStringReplacement
{
%TypeHeaderCode
#include <qgsstringutils.h>
%End
  public:

    /** Constructor for QgsStringReplacement.
     * @param match string to match
     * @param replacement string to replace match with
     * @param caseSensitive set to true for a case sensitive match
     * @param wholeWordOnly set to true to match complete words only, or false to allow partial word matches
     */
    QgsStringReplacement( const QString& match,
                          const QString& replacement,
                          bool caseSensitive = false,
                          bool wholeWordOnly = false );

    //! Returns the string matched by this object
    QString match() const;

    //! Returns the string to replace matches with
    QString replacement() const;

    //! Returns true if match is case sensitive
    bool caseSensitive() const;

    //! Returns true if match only applies to whole words, or false if partial word matches are permitted
    bool wholeWordOnly() const;

    /** Processes a given input string, applying any valid replacements which should be made.
     * @param input input string
     * @returns input string with any matches replaced by replacement string
     */
    QString process( const QString& input ) const;

    bool operator==( const QgsStringReplacement& other );

    /** Returns a map of the replacement properties.
     * @see fromProperties()
     */
    QgsStringMap properties() const;

    /** Creates a new QgsStringReplacement from an encoded properties map.
     * @see properties()
     */
    static QgsStringReplacement fromProperties( const QgsStringMap& properties );

};


/** \ingroup core
 * \class QgsStringReplacementCollection
 * \brief A collection of string replacements (specified using QgsStringReplacement objects).
 * \note Added in version 3.0
 */

class QgsStringReplacementCollection
{
%TypeHeaderCode
#include <qgsstringutils.h>
%End
  public:

    /** Constructor for QgsStringReplacementCollection
     * @param replacements initial list of string replacements
     */
    QgsStringReplacementCollection( const QList< QgsStringReplacement >& replacements = QList< QgsStringReplacement >() );

    /** Returns the list of string replacements in this collection.
     * @see setReplacements()
     */
    QList< QgsStringReplacement > replacements() const;

    /** Sets the list of string replacements in this collection.
     * @param replacements list of string replacements to apply. Replacements are applied in the
     * order they are specified here.
     * @see replacements()
     */
    void setReplacements( const QList< QgsStringReplacement >& replacements );

    /** Processes a given input string, applying any valid replacements which should be made
     * using QgsStringReplacement objects contained by this collection. Replacements
     * are made in order of the QgsStringReplacement objects contained in the collection.
     * @param input input string
     * @returns input string with any matches replaced by replacement string
     */
    QString process( const QString& input ) const;

    /** Writes the collection state to an XML element.
     * @param elem target DOM element
     * @param doc DOM document
     * @see readXml()
     */
    void writeXml( QDomElement& elem, QDomDocument& doc ) const;

    /** Reads the collection state from an XML element.
     * @param elem DOM element
     * @see writeXml()
     */
    void readXml( const QDomElement& elem );
};


/** \ingroup core
 * \class QgsStringUtils
 * \brief Utility functions for working with strings.
 * \note Added in version 2.11
 */

class QgsStringUtils
{
%TypeHeaderCode
#include <qgsstringutils.h>
%End
  public:


    //! Capitalization options
    enum Capitalization
    {
      MixedCase, //!< Mixed case, ie no change
      AllUppercase, //!< Convert all characters to uppercase
      AllLowercase,  //!< Convert all characters to lowercase
      ForceFirstLetterToCapital, //!< Convert just the first letter of each word to uppercase, leave the rest untouched
    };

    /** Converts a string by applying capitalization rules to the string.
     * @param string input string
     * @param capitalization capitalization type to apply
     * @return capitalized string
     * @note added in QGIS 3.0
     */
    static QString capitalize( const QString& string, Capitalization capitalization );

    /** Returns the Levenshtein edit distance between two strings. This equates to the minimum
     * number of character edits (insertions, deletions or substitutions) required to change
     * one string to another.
     * @param string1 first string
     * @param string2 second string
     * @param caseSensitive set to true for case sensitive comparison
     * @returns edit distance. Lower distances indicate more similar strings.
     */
    static int levenshteinDistance( const QString &string1, const QString &string2, bool caseSensitive = false );

    /** Returns the longest common substring between two strings. This substring is the longest
     * string that is a substring of the two input strings. Eg, the longest common substring
     * of "ABABC" and "BABCA" is "ABC".
     * @param string1 first string
     * @param string2 second string
     * @param caseSensitive set to true for case sensitive comparison
     * @returns longest common substring
     */
    static QString longestCommonSubstring( const QString &string1, const QString &string2, bool caseSensitive = false );

    /** Returns the Hamming distance between two strings. This equates to the number of characters at
     * corresponding positions within the input strings where the characters are different. The input
     * strings must be the same length.
     * @param string1 first string
     * @param string2 second string
     * @param caseSensitive set to true for case sensitive comparison
     * @returns Hamming distance between strings, or -1 if strings are different lengths.
     */
    static int hammingDistance( const QString &string1, const QString &string2, bool caseSensitive = false );

    /** Returns the Soundex representation of a string. Soundex is a phonetic matching algorithm,
     * so strings with similar sounds should be represented by the same Soundex code.
     * @param string input string
     * @returns 4 letter Soundex code
     */
    static QString soundex( const QString &string );

    /** Returns a string with any URL (eg http(s)/ftp) and mailto: text converted to valid HTML <a ...>
     * links.
     * @param string string to insert links into
     * @param foundLinks if specified, will be set to true if any links were inserted into the string
     * @returns string with inserted links
     * @note added in QGIS 3.0
     */
    static QString insertLinks( const QString& string, bool* foundLinks = nullptr );
};
