/***************************************************************************
                             qgscolorscheme.h
                             -------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORSCHEME_H
#define QGSCOLORSCHEME_H

#include <QString>
#include <QColor>
#include <QPair>

/**List of colors paired with a friendly display name identifying the color
 * \note Added in version 2.5
*/
typedef QList< QPair< QColor, QString > > QgsNamedColorList;

/** \ingroup core
 * \class QgsColorScheme
 * \brief Abstract base class for color schemes
 *
 * A color scheme for display in QgsColorButtonV2. Color schemes return lists
 * of colors with an optional associated color name. The colors returned
 * can be generated using an optional base color.
 * \note Added in version 2.5
 */
class CORE_EXPORT QgsColorScheme
{
  public:

    /** Flags for controlling behaviour of color scheme
     */
    enum SchemeFlag
    {
      ShowInColorDialog = 0x01, /*< show scheme in color picker dialog */
      ShowInColorButtonMenu = 0x02, /*< show scheme in color button drop down menu */
      ShowInAllContexts = ShowInColorDialog | ShowInColorButtonMenu /*< show scheme in all contexts */
    };
    Q_DECLARE_FLAGS( SchemeFlags, SchemeFlag )

    QgsColorScheme();

    virtual ~QgsColorScheme();

    /**Gets the name for the color scheme
     * @returns color scheme name
    */
    virtual QString schemeName() const = 0;

    /**Returns the current flags for the color scheme.
     * @returns current flags
    */
    virtual SchemeFlags flags() const { return ShowInColorDialog; }

    /**Gets a list of colors from the scheme. The colors can optionally
     * be generated using the supplied context and base color.
     * @param context string specifiying an optional context for the returned
     * colors. For instance, a "recent colors" scheme may filter returned colors
     * by context so that colors used only in a "composer" context are returned.
     * @param baseColor base color for the scheme's colors. Some color schemes
     * may take advantage of this to filter or modify their returned colors
     * to colors related to the base color.
     * @returns a list of QPairs of color and color name
    */
    virtual QgsNamedColorList fetchColors( const QString context = QString(),
                                           const QColor baseColor = QColor() ) = 0;

    /**Returns whether the color scheme is editable
     * @returns true if scheme is editable
     * @see setColors
    */
    virtual bool isEditable() const { return false; }

    /**Sets the colors for the scheme. This method is only valid for editable color schemes.
     * @param colors list of colors for the scheme
     * @param context to set colors for
     * @param baseColor base color to set colors for
     * @returns true if colors were set successfully
     * @see isEditable
    */
    virtual bool setColors( const QgsNamedColorList colors, const QString context = QString(), const QColor baseColor = QColor() );

    /**Clones a color scheme
     * @returns copy of color scheme
    */
    virtual QgsColorScheme* clone() const = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsColorScheme::SchemeFlags )

/** \ingroup core
 * \class QgsGplColorScheme
 * \brief A color scheme which stores its colors in a gpl palette file.
 * \note Added in version 2.5
 */
class CORE_EXPORT QgsGplColorScheme : public QgsColorScheme
{
  public:

    QgsGplColorScheme();

    virtual ~QgsGplColorScheme();

    virtual QgsNamedColorList fetchColors( const QString context = QString(),
                                           const QColor baseColor = QColor() );

    virtual bool setColors( const QgsNamedColorList colors, const QString context = QString(), const QColor baseColor = QColor() );

  protected:

    /**Returns the file path for the associated gpl palette file
     * @returns gpl file path
    */
    virtual QString gplFilePath() = 0;

};

/** \ingroup core
 * \class QgsUserColorScheme
 * \brief A color scheme which stores its colors in a gpl palette file within the "palettes"
 * subfolder off the user's QGIS settings folder.
 * \note Added in version 2.5
 */
class CORE_EXPORT QgsUserColorScheme : public QgsGplColorScheme
{
  public:

    /**Constructs a new user color scheme, using a specified gpl palette file
     * @param filename filename of gpl palette file stored in the users "palettes" folder
    */
    QgsUserColorScheme( const QString filename );

    virtual ~QgsUserColorScheme();

    virtual QString schemeName() const;

    virtual QgsColorScheme* clone() const;

    virtual bool isEditable() const { return true; }

    /**Sets the name for the scheme
     * @param name new name
    */
    void setName( const QString name ) { mName = name; }

    /**Erases the associated gpl palette file from the users "palettes" folder
     * @returns true if erase was successful
    */
    bool erase();

  protected:

    QString mName;

    QString mFilename;

    virtual QString gplFilePath();

};

/** \ingroup core
 * \class QgsRecentColorScheme
 * \brief A color scheme which contains the most recently used colors.
 * \note Added in version 2.5
 */
class CORE_EXPORT QgsRecentColorScheme : public QgsColorScheme
{
  public:

    QgsRecentColorScheme();

    virtual ~QgsRecentColorScheme();

    virtual QString schemeName() const { return QT_TR_NOOP( "Recent colors" ); }

    virtual SchemeFlags flags() const { return ShowInAllContexts; }

    virtual QgsNamedColorList fetchColors( const QString context = QString(),
                                           const QColor baseColor = QColor() );

    QgsColorScheme* clone() const;
};

/** \ingroup core
 * \class QgsCustomColorScheme
 * \brief A color scheme which contains custom colors set through QGIS app options dialog.
 * \note Added in version 2.5
 */
class CORE_EXPORT QgsCustomColorScheme : public QgsColorScheme
{
  public:

    QgsCustomColorScheme();

    virtual ~QgsCustomColorScheme();

    virtual QString schemeName() const { return QT_TR_NOOP( "Standard colors" ); }

    virtual SchemeFlags flags() const { return ShowInAllContexts; }

    virtual QgsNamedColorList fetchColors( const QString context = QString(),
                                           const QColor baseColor = QColor() );

    virtual bool isEditable() const { return true; }

    virtual bool setColors( const QgsNamedColorList colors, const QString context = QString(), const QColor baseColor = QColor() );

    QgsColorScheme* clone() const;
};

/** \ingroup core
 * \class QgsProjectColorScheme
 * \brief A color scheme which contains project specific colors set through project properties dialog.
 * \note Added in version 2.5
 */
class CORE_EXPORT QgsProjectColorScheme : public QgsColorScheme
{
  public:

    QgsProjectColorScheme();

    virtual ~QgsProjectColorScheme();

    virtual QString schemeName() const { return QT_TR_NOOP( "Project colors" ); }

    virtual SchemeFlags flags() const { return ShowInAllContexts; }

    virtual QgsNamedColorList fetchColors( const QString context = QString(),
                                           const QColor baseColor = QColor() );

    virtual bool isEditable() const { return true; }

    virtual bool setColors( const QgsNamedColorList colors, const QString context = QString(), const QColor baseColor = QColor() );

    QgsColorScheme* clone() const;
};

#endif
