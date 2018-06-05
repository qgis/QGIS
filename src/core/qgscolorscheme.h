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
#include <QObject>

#include "qgis_core.h"
#include "qgis.h"

/**
 * \ingroup core
 * List of colors paired with a friendly display name identifying the color
 * \since QGIS 2.5
*/
typedef QList< QPair< QColor, QString > > QgsNamedColorList;

/**
 * \ingroup core
 * \class QgsColorScheme
 * \brief Abstract base class for color schemes
 *
 * A color scheme for display in QgsColorButton. Color schemes return lists
 * of colors with an optional associated color name. The colors returned
 * can be generated using an optional base color.
 * \since QGIS 2.5
 */
class CORE_EXPORT QgsColorScheme
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
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
    SIP_END
#endif

  public:

    /**
     * Flags for controlling behavior of color scheme
     */
    enum SchemeFlag
    {
      ShowInColorDialog = 0x01, //!< Show scheme in color picker dialog
      ShowInColorButtonMenu = 0x02, //!< Show scheme in color button drop-down menu
      ShowInAllContexts = ShowInColorDialog | ShowInColorButtonMenu //!< Show scheme in all contexts
    };
    Q_DECLARE_FLAGS( SchemeFlags, SchemeFlag )

    /**
     * Constructor for QgsColorScheme.
     */
    QgsColorScheme() = default;

    virtual ~QgsColorScheme() = default;

    /**
     * Gets the name for the color scheme
     * \returns color scheme name
     */
    virtual QString schemeName() const = 0;

    /**
     * Returns the current flags for the color scheme.
     * \returns current flags
     */
    virtual SchemeFlags flags() const { return ShowInColorDialog; }

    /**
     * Gets a list of colors from the scheme. The colors can optionally
     * be generated using the supplied context and base color.
     * \param context string specifying an optional context for the returned
     * colors. For instance, a "recent colors" scheme may filter returned colors
     * by context so that colors used only in a "composer" context are returned.
     * \param baseColor base color for the scheme's colors. Some color schemes
     * may take advantage of this to filter or modify their returned colors
     * to colors related to the base color.
     * \returns a list of QPairs of color and color name
     */
    virtual QgsNamedColorList fetchColors( const QString &context = QString(),
                                           const QColor &baseColor = QColor() ) = 0;

    /**
     * Returns whether the color scheme is editable
     * \returns true if scheme is editable
     * \see setColors
     */
    virtual bool isEditable() const { return false; }

    /**
     * Sets the colors for the scheme. This method is only valid for editable color schemes.
     * \param colors list of colors for the scheme
     * \param context to set colors for
     * \param baseColor base color to set colors for
     * \returns true if colors were set successfully
     * \see isEditable
     */
    virtual bool setColors( const QgsNamedColorList &colors, const QString &context = QString(), const QColor &baseColor = QColor() );

    /**
     * Clones a color scheme
     * \returns copy of color scheme
     */
    virtual QgsColorScheme *clone() const = 0 SIP_FACTORY;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsColorScheme::SchemeFlags )

/**
 * \ingroup core
 * \class QgsGplColorScheme
 * \brief A color scheme which stores its colors in a gpl palette file.
 * \since QGIS 2.5
 */
class CORE_EXPORT QgsGplColorScheme : public QgsColorScheme
{
  public:

    /**
     * Constructor for QgsGplColorScheme.
     */
    QgsGplColorScheme() = default;

    QgsNamedColorList fetchColors( const QString &context = QString(),
                                   const QColor &baseColor = QColor() ) override;

    bool setColors( const QgsNamedColorList &colors, const QString &context = QString(), const QColor &baseColor = QColor() ) override;

  protected:

    /**
     * Returns the file path for the associated gpl palette file
     * \returns gpl file path
     */
    virtual QString gplFilePath() = 0;

};

/**
 * \ingroup core
 * \class QgsUserColorScheme
 * \brief A color scheme which stores its colors in a gpl palette file within the "palettes"
 * subfolder off the user's QGIS settings folder.
 * \since QGIS 2.5
 */
class CORE_EXPORT QgsUserColorScheme : public QgsGplColorScheme
{
  public:

    /**
     * Constructs a new user color scheme, using a specified gpl palette file
     * \param filename filename of gpl palette file stored in the users "palettes" folder
     */
    QgsUserColorScheme( const QString &filename );

    QString schemeName() const override;

    QgsUserColorScheme *clone() const override SIP_FACTORY;

    bool isEditable() const override { return mEditable; }

    QgsColorScheme::SchemeFlags flags() const override;

    /**
     * Sets the name for the scheme
     * \param name new name
     */
    void setName( const QString &name ) { mName = name; }

    /**
     * Erases the associated gpl palette file from the users "palettes" folder
     * \returns true if erase was successful
     */
    bool erase();

    /**
     * Sets whether a this scheme should be shown in color button menus.
     * \param show set to true to show in color button menus, or false to hide from menus
     * \since QGIS 3.0
     */
    void setShowSchemeInMenu( bool show );

  protected:

    QString mName;

    QString mFilename;

    bool mEditable = false;

    QString gplFilePath() override;

};

/**
 * \ingroup core
 * \class QgsRecentColorScheme
 * \brief A color scheme which contains the most recently used colors.
 * \since QGIS 2.5
 */
class CORE_EXPORT QgsRecentColorScheme : public QgsColorScheme
{
  public:

    /**
     * Constructor for QgsRecentColorScheme.
     */
    QgsRecentColorScheme() = default;

    QString schemeName() const override { return QObject::tr( "Recent colors" ); }

    SchemeFlags flags() const override { return ShowInAllContexts; }

    QgsNamedColorList fetchColors( const QString &context = QString(),
                                   const QColor &baseColor = QColor() ) override;

    QgsRecentColorScheme *clone() const override SIP_FACTORY;

    /**
     * Adds a color to the list of recent colors.
     * \param color color to add
     * \see lastUsedColor()
     * \since QGIS 2.14
     */
    static void addRecentColor( const QColor &color );

    /**
     * Returns the most recently used color.
     * \see addRecentColor()
     * \since QGIS 3.0
     */
    static QColor lastUsedColor();
};

/**
 * \ingroup core
 * \class QgsCustomColorScheme
 * \brief A color scheme which contains custom colors set through QGIS app options dialog.
 * \since QGIS 2.5
 */
class CORE_EXPORT QgsCustomColorScheme : public QgsColorScheme
{
  public:

    /**
     * Constructor for QgsCustomColorScheme.
     */
    QgsCustomColorScheme() = default;

    QString schemeName() const override { return QObject::tr( "Standard colors" ); }

    SchemeFlags flags() const override { return ShowInAllContexts; }

    QgsNamedColorList fetchColors( const QString &context = QString(),
                                   const QColor &baseColor = QColor() ) override;

    bool isEditable() const override { return true; }

    bool setColors( const QgsNamedColorList &colors, const QString &context = QString(), const QColor &baseColor = QColor() ) override;

    QgsCustomColorScheme *clone() const override SIP_FACTORY;
};

/**
 * \ingroup core
 * \class QgsProjectColorScheme
 * \brief A color scheme which contains project specific colors set through project properties dialog.
 * \since QGIS 2.5
 */
class CORE_EXPORT QgsProjectColorScheme : public QgsColorScheme
{
  public:

    /**
     * Constructor for QgsProjectColorScheme.
     */
    QgsProjectColorScheme() = default;

    QString schemeName() const override { return QObject::tr( "Project colors" ); }

    SchemeFlags flags() const override { return ShowInAllContexts; }

    QgsNamedColorList fetchColors( const QString &context = QString(),
                                   const QColor &baseColor = QColor() ) override;

    bool isEditable() const override { return true; }

    bool setColors( const QgsNamedColorList &colors, const QString &context = QString(), const QColor &baseColor = QColor() ) override;

    QgsProjectColorScheme *clone() const override SIP_FACTORY;
};

#endif
