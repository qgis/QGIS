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
    /** Returns the Levenshtein edit distance between two strings. This equates to the minimum
     * number of character edits (insertions, deletions or substitutions) required to change
     * one string to another.
     * @param string1 first string
     * @param string2 second string
     * @param caseSensitive set to true for case sensitive comparison
     * @returns edit distance. Lower distances indicate more similiar strings.
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

};
