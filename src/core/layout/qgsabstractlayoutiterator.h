/***************************************************************************
                             qgsabstractlayoutiterator.h
                             ---------------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
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
#ifndef QGSABSTRACTLAYOUTITERATOR_H
#define QGSABSTRACTLAYOUTITERATOR_H

#include "qgis_core.h"
#include <QString>

class QgsLayout;

/**
 * \ingroup core
 * \class QgsAbstractLayoutIterator
 * \brief An abstract base class for QgsLayout based classes which can be exported by QgsLayoutExporter.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAbstractLayoutIterator
{

  public:

    virtual ~QgsAbstractLayoutIterator() = default;

    /**
     * Returns the layout associated with the iterator.
     */
    virtual QgsLayout *layout() = 0;

    /**
     * Called when rendering begins, before iteration commences. Returns TRUE if successful, FALSE if no iteration
     * is available or required.
     * \see endRender()
    */
    virtual bool beginRender() = 0;

    /**
     * Ends the render, performing any required cleanup tasks.
     */
    virtual bool endRender() = 0;

    /**
     * Returns the number of features to iterate over.
     */
    virtual int count() = 0;

    /**
     * Iterates to next feature, returning FALSE if no more features exist to iterate over.
     */
    virtual bool next() = 0;

    /**
     * Returns the file path for the current feature, based on a
     * specified base file path and extension.
     */
    virtual QString filePath( const QString &baseFilePath, const QString &extension ) = 0;

};

#endif //QGSABSTRACTLAYOUTITERATOR_H



