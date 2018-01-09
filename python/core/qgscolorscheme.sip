/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscolorscheme.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





typedef QList< QPair< QColor, QString > > QgsNamedColorList;

class QgsColorScheme
{
%Docstring
 Abstract base class for color schemes

A color scheme for display in QgsColorButton. Color schemes return lists
of colors with an optional associated color name. The colors returned
can be generated using an optional base color.

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorscheme.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsUserColorScheme *>( sipCpp ) != NULL )
      sipType = sipType_QgsUserColorScheme;
    else if ( dynamic_cast<QgsRecentColorScheme *>( sipCpp ) != NULL )
      sipType = sipType_QgsRecentColorScheme;
    else if ( dynamic_cast<QgsCustomColorScheme *>( sipCpp ) != NULL )
      sipType = sipType_QgsCustomColorScheme;
    else if ( dynamic_cast<QgsProjectColorScheme *>( sipCpp ) != NULL )
      sipType = sipType_QgsProjectColorScheme;
    else if ( dynamic_cast<QgsGplColorScheme *>( sipCpp ) != NULL )
      sipType = sipType_QgsGplColorScheme;
    else
      sipType = sipType_QgsColorScheme;
%End
  public:

    enum SchemeFlag
    {
      ShowInColorDialog,
      ShowInColorButtonMenu,
      ShowInAllContexts
    };
    typedef QFlags<QgsColorScheme::SchemeFlag> SchemeFlags;


    QgsColorScheme();
%Docstring
Constructor for QgsColorScheme.
%End

    virtual ~QgsColorScheme();

    virtual QString schemeName() const = 0;
%Docstring
Gets the name for the color scheme

:return: color scheme name
%End

    virtual SchemeFlags flags() const;
%Docstring
Returns the current flags for the color scheme.

:return: current flags
%End

    virtual QgsNamedColorList fetchColors( const QString &context = QString(),
                                           const QColor &baseColor = QColor() ) = 0;
%Docstring
Gets a list of colors from the scheme. The colors can optionally
be generated using the supplied context and base color.

:param context: string specifying an optional context for the returned
colors. For instance, a "recent colors" scheme may filter returned colors
by context so that colors used only in a "composer" context are returned.
:param baseColor: base color for the scheme's colors. Some color schemes
may take advantage of this to filter or modify their returned colors
to colors related to the base color.

:return: a list of QPairs of color and color name
%End

    virtual bool isEditable() const;
%Docstring
Returns whether the color scheme is editable

:return: true if scheme is editable

.. seealso:: :py:func:`setColors`
%End

    virtual bool setColors( const QgsNamedColorList &colors, const QString &context = QString(), const QColor &baseColor = QColor() );
%Docstring
Sets the colors for the scheme. This method is only valid for editable color schemes.

:param colors: list of colors for the scheme
:param context: to set colors for
:param baseColor: base color to set colors for

:return: true if colors were set successfully

.. seealso:: :py:func:`isEditable`
%End

    virtual QgsColorScheme *clone() const = 0 /Factory/;
%Docstring
Clones a color scheme

:return: copy of color scheme
%End
};

QFlags<QgsColorScheme::SchemeFlag> operator|(QgsColorScheme::SchemeFlag f1, QFlags<QgsColorScheme::SchemeFlag> f2);


class QgsGplColorScheme : QgsColorScheme
{
%Docstring
 A color scheme which stores its colors in a gpl palette file.

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorscheme.h"
%End
  public:

    QgsGplColorScheme();
%Docstring
Constructor for QgsGplColorScheme.
%End

     virtual QgsNamedColorList fetchColors( const QString &context = QString(),
                                   const QColor &baseColor = QColor() );

    virtual bool setColors( const QgsNamedColorList &colors, const QString &context = QString(), const QColor &baseColor = QColor() );


  protected:

    virtual QString gplFilePath() = 0;
%Docstring
Returns the file path for the associated gpl palette file

:return: gpl file path
%End

};

class QgsUserColorScheme : QgsGplColorScheme
{
%Docstring
 A color scheme which stores its colors in a gpl palette file within the "palettes"
subfolder off the user's QGIS settings folder.

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorscheme.h"
%End
  public:

    QgsUserColorScheme( const QString &filename );
%Docstring
Constructs a new user color scheme, using a specified gpl palette file

:param filename: filename of gpl palette file stored in the users "palettes" folder
%End

    virtual QString schemeName() const;


    virtual QgsUserColorScheme *clone() const /Factory/;


    virtual bool isEditable() const;

    virtual QgsColorScheme::SchemeFlags flags() const;


    void setName( const QString &name );
%Docstring
Sets the name for the scheme

:param name: new name
%End

    bool erase();
%Docstring
Erases the associated gpl palette file from the users "palettes" folder

:return: true if erase was successful
%End

    void setShowSchemeInMenu( bool show );
%Docstring
Sets whether a this scheme should be shown in color button menus.

:param show: set to true to show in color button menus, or false to hide from menus

.. versionadded:: 3.0
%End

  protected:



    virtual QString gplFilePath();


};

class QgsRecentColorScheme : QgsColorScheme
{
%Docstring
 A color scheme which contains the most recently used colors.

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorscheme.h"
%End
  public:

    QgsRecentColorScheme();
%Docstring
Constructor for QgsRecentColorScheme.
%End

    virtual QString schemeName() const;

    virtual SchemeFlags flags() const;

     virtual QgsNamedColorList fetchColors( const QString &context = QString(),
                                   const QColor &baseColor = QColor() );

    virtual QgsRecentColorScheme *clone() const /Factory/;


    static void addRecentColor( const QColor &color );
%Docstring
Adds a color to the list of recent colors.

:param color: color to add

.. versionadded:: 2.14

.. seealso:: :py:func:`lastUsedColor`
%End

    static QColor lastUsedColor();
%Docstring
Returns the most recently used color.

.. versionadded:: 3.0

.. seealso:: :py:func:`addRecentColor`
%End
};

class QgsCustomColorScheme : QgsColorScheme
{
%Docstring
 A color scheme which contains custom colors set through QGIS app options dialog.

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorscheme.h"
%End
  public:

    QgsCustomColorScheme();
%Docstring
Constructor for QgsCustomColorScheme.
%End

    virtual QString schemeName() const;

    virtual SchemeFlags flags() const;

     virtual QgsNamedColorList fetchColors( const QString &context = QString(),
                                   const QColor &baseColor = QColor() );

    virtual bool isEditable() const;

    virtual bool setColors( const QgsNamedColorList &colors, const QString &context = QString(), const QColor &baseColor = QColor() );


    virtual QgsCustomColorScheme *clone() const /Factory/;

};

class QgsProjectColorScheme : QgsColorScheme
{
%Docstring
 A color scheme which contains project specific colors set through project properties dialog.

.. versionadded:: 2.5
%End

%TypeHeaderCode
#include "qgscolorscheme.h"
%End
  public:

    QgsProjectColorScheme();
%Docstring
Constructor for QgsProjectColorScheme.
%End

    virtual QString schemeName() const;

    virtual SchemeFlags flags() const;

     virtual QgsNamedColorList fetchColors( const QString &context = QString(),
                                   const QColor &baseColor = QColor() );

    virtual bool isEditable() const;

    virtual bool setColors( const QgsNamedColorList &colors, const QString &context = QString(), const QColor &baseColor = QColor() );


    virtual QgsProjectColorScheme *clone() const /Factory/;

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscolorscheme.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
