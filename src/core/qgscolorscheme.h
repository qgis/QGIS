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

/** \ingroup core
 * \class QgsColorScheme
 * \brief Abstract base class for color schemes
 *
 * A color scheme for display in QgsColorButtonV2. Color schemes return lists
 * of colors with an optional associated color name. The colors returned
 * can be generated using an optional base color.
 */
class CORE_EXPORT QgsColorScheme
{
  public:

    QgsColorScheme();

    virtual ~QgsColorScheme();

    /**Gets the name for the color scheme
     * @returns color scheme name
     * @note added in QGIS 2.5
    */
    virtual QString schemeName() const = 0;

    /**Gets a list of colors from the scheme. The colors can optionally
     * be generated using the supplied context and base color.
     * @param context string specifiying an optional context for the returned
     * colors. For instance, a "recent colors" scheme may filter returned colors
     * by context so that colors used only in a "composer" context are returned.
     * @param baseColor base color for the scheme's colors. Some color schemes
     * may take advantage of this to filter or modify their returned colors
     * to colors related to the base color.
     * @returns a list of QPairs of color and color name
     * @note added in QGIS 2.5
    */
    virtual QList< QPair< QColor, QString > > fetchColors( const QString context = QString(),
        const QColor baseColor = QColor() ) = 0;

    /**Clones a color scheme
     * @returns copy of color scheme
     * @note added in QGIS 2.5
    */
    virtual QgsColorScheme* clone() const = 0;

};

/** \ingroup core
 * \class QgsRecentColorScheme
 * \brief A color scheme which contains the most recently used colors.
 */
class CORE_EXPORT QgsRecentColorScheme : public QgsColorScheme
{
  public:

    QgsRecentColorScheme();

    virtual ~QgsRecentColorScheme();

    virtual QString schemeName() const { return QT_TR_NOOP( "Recent colors" ); }

    virtual QList< QPair< QColor, QString > > fetchColors( const QString context = QString(),
        const QColor baseColor = QColor() );

    QgsColorScheme* clone() const;
};

#endif
