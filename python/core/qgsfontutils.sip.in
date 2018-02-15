/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfontutils.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsFontUtils
{

%TypeHeaderCode
#include "qgsfontutils.h"
%End
  public:

    static bool fontMatchOnSystem( const QFont &f );
%Docstring
Check whether exact font is on system

:param f: The font to test for match
%End

    static bool fontFamilyOnSystem( const QString &family );
%Docstring
Check whether font family is on system in a quick manner, which does not compare [foundry]

:param family: The family to test

:return: Whether family was found on system

.. note::

   This is good for use in loops of large lists, e.g. registering many features for labeling
%End

    static bool fontFamilyHasStyle( const QString &family, const QString &style );
%Docstring
Check whether font family on system has specific style

:param family: The family to test
:param style: The style to test for

:return: Whether family has style

.. versionadded:: 2.1
%End

    static bool fontFamilyMatchOnSystem( const QString &family, QString *chosen = 0, bool *match = 0 );
%Docstring
Check whether font family is on system

:param family: The family to test
:param chosen: The actual family (possibly from different foundry) returned by system
:param match: Whether the family [foundry] returned by system is a match

:return: Whether family was found on system
%End

    static bool updateFontViaStyle( QFont &f, const QString &fontstyle, bool fallback = false );
%Docstring
Updates font with named style and retain all font properties

:param f: The font to update
:param fontstyle: The style to try and switch the font to
:param fallback: If no matching fontstyle found for font, assign most similar or first style found to font

:return: Whether the font was updated (also returns true if the requested style matches font's current style)

.. note::

   This is a more featured replacement for a Qt 4.8+ function: void QFont.setStyleName ( const QString & styleName )
%End

    static QString standardTestFontFamily();
%Docstring
Get standard test font family

.. versionadded:: 2.1
%End

    static bool loadStandardTestFonts( const QStringList &loadstyles );
%Docstring
Loads standard test fonts from filesystem or qrc resource

:param loadstyles: List of styles to load, e.g. All, Roman, Oblique, Bold, Bold Oblique

:return: Whether any font was loaded

.. note::

   Done by default on debug app/server startup to ensure fonts available for unit tests (Roman and Bold)

.. versionadded:: 2.1
%End

    static QFont getStandardTestFont( const QString &style = "Roman", int pointsize = 12 );
%Docstring
Get standard test font with specific style

:param style: Style to load, e.g. Roman, Oblique, Bold, Bold Oblique
:param pointsize: Font point size to set

:return: QFont

.. versionadded:: 2.1
%End

    static QDomElement toXmlElement( const QFont &font, QDomDocument &document, const QString &elementName );
%Docstring
Returns a DOM element containing the properties of the font.

:param font: font
:param document: DOM document
:param elementName: name for DOM element

:return: DOM element containing font settings

.. versionadded:: 2.10

.. seealso:: :py:func:`setFromXmlElement`
%End

    static bool setFromXmlElement( QFont &font, const QDomElement &element );
%Docstring
Sets the properties of a font to match the properties stored in an XML element. Calling
this will overwrite the current properties of the font.

:param font: font to update
:param element: DOM element

:return: true if properties were successfully read from element

.. versionadded:: 2.10

.. seealso:: :py:func:`toXmlElement`

.. seealso:: :py:func:`setFromXmlChildNode`
%End

    static bool setFromXmlChildNode( QFont &font, const QDomElement &element, const QString &childNode );
%Docstring
Sets the properties of a font to match the properties stored in an XML child node. Calling
this will overwrite the current properties of the font.

:param font: font to update
:param element: DOM element
:param childNode: name of child node

:return: true if child node exists and properties were successfully read from node

.. versionadded:: 2.10

.. seealso:: :py:func:`setFromXmlElement`

.. seealso:: :py:func:`toXmlElement`
%End

    static QMimeData *toMimeData( const QFont &font ) /Factory/;
%Docstring
Returns new mime data representing the specified ``font`` settings.
Caller takes responsibility for deleting the returned object.

.. seealso:: :py:func:`fromMimeData`

.. versionadded:: 3.0
%End

    static QFont fromMimeData( const QMimeData *data, bool *ok /Out/ = 0 );
%Docstring
Attempts to parse the provided mime ``data`` as a QFont.
If data can be parsed as a QFont, ``ok`` will be set to true.

.. seealso:: :py:func:`toMimeData`

.. versionadded:: 3.0
%End

    static QString translateNamedStyle( const QString &namedStyle );
%Docstring
Returns the localized named style of a font, if such a translation is available.

:param namedStyle: a named style, i.e. "Bold", "Italic", etc

:return: The localized named style

.. versionadded:: 2.12

.. seealso:: :py:func:`untranslateNamedStyle`
%End

    static QString untranslateNamedStyle( const QString &namedStyle );
%Docstring
Returns the english named style of a font, if possible.

:param namedStyle: a localized named style, i.e. "Fett", "Kursiv", etc

:return: The english named style

.. versionadded:: 2.12

.. seealso:: :py:func:`translateNamedStyle`
%End

    static QString asCSS( const QFont &font, double pointToPixelMultiplier = 1.0 );
%Docstring
Returns a CSS string representing the specified font as closely as possible.

:param font: QFont to convert
:param pointToPixelMultiplier: scaling factor to apply to convert point sizes to pixel font sizes.
The CSS returned by this function will always use pixels for font sizes, so this parameter
should be set to a suitable value to convert point sizes to pixels (e.g., taking into account
destination DPI)

:return: partial CSS string, e.g., "font-family: Comic Sans; font-size: 12px;"

.. versionadded:: 2.16
%End

    static void addRecentFontFamily( const QString &family );
%Docstring
Adds a font ``family`` to the list of recently used font families.

.. versionadded:: 3.0

.. seealso:: :py:func:`recentFontFamilies`
%End

    static QStringList recentFontFamilies();
%Docstring
Returns a list of recently used font families.

.. seealso:: :py:func:`addRecentFontFamily`

.. versionadded:: 3.0
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfontutils.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
