/***************************************************************************
                             qgsemfrenderer.h
                             -----------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#ifndef QGSEMFRENDERER_H
#define QGSEMFRENDERER_H


#include "qgis_core.h"
#include <QString>

class QPainter;
class QSizeF;

/**
 * \class QgsEmfRenderer
 * \ingroup core
 *
 * Utility class for rendering EMF picture content to QPainter devices.
 *
 * \since QGIS 3.10.2
*/
class CORE_EXPORT QgsEmfRenderer
{
  public:

    /**
     * Constructor for QgsEmfRenderer, using the EMF file with the specified \a filename.
     */
    QgsEmfRenderer( const QString &filename );

    /**
     * Constructor for QgsEmfRenderer, using the EMF content from a byte array.
     */
    QgsEmfRenderer( const QByteArray &contents );

    /**
     * Renders the EMF content to the specified \a painter.
     *
     * Returns TRUE if the rendering was successful.
     */
    bool render( QPainter *painter, QSizeF size, bool keepAspectRatio = false );

  private:

    QString mFilename;
    QByteArray mContents;

};

#endif //QGSEMFRENDERER_H
