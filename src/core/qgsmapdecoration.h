/***************************************************************************
                          qgsmapdecoration.h
                          ----------------
    begin                : April 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPDECORATION_H
#define QGSMAPDECORATION_H

#include "qgis_core.h"
#include <QString>

class QgsMapSettings;
class QgsRenderContext;

/**
 * \ingroup core
 * \class QgsMapDecoration
 * \brief Interface for map decorations.
 *
 */
class CORE_EXPORT QgsMapDecoration
{

  public:

    QgsMapDecoration() = default;

    virtual ~QgsMapDecoration() = default;

    /**
     * Renders a map decoration.
     */
    virtual void render( const QgsMapSettings &mapSettings, QgsRenderContext &context ) = 0;

    /**
     * Returns the map decoration display name.
     * \since QGIS 3.14
     */
    const QString displayName() const { return mDisplayName; }

    /**
     * Returns TRUE if the decoration is attached to a fixed map position (e.g grid, layout extent), or
     * FALSE if the annotation uses a position relative to the map canvas (e.g. title, copyright...)
     * \since QGIS 3.34
     */
    virtual bool hasFixedMapPosition() const { return false; }

  protected:

    /**
     * Sets the map decoration display \a name.
     * \since QGIS 3.14
     */
    void setDisplayName( const QString &name ) { mDisplayName = name; }

  private:

    QString mDisplayName;

};

#endif //QGSMAPDECORATION_H
