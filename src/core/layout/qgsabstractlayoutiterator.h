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


class CORE_EXPORT QgsAbstractLayoutIterator
{

  public:

    virtual ~QgsAbstractLayoutIterator() = default;

    /**
     * Called when rendering begins, before iteration commences. Returns true if successful, false if no iteration
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
    virtual int count() const = 0;

    /**
     * Iterates to next feature, returning false if no more features exist to iterate over.
     * \see previous()
     * \see last()
     * \see first()
     */
    virtual bool next() = 0;

    /**
     * Iterates to the previous feature, returning false if no previous feature exists.
     * \see next()
     * \see last()
     * \see first()
     */
    virtual bool previous() = 0;

    /**
     * Seeks to the last feature, returning false if no feature was found.
     * \see next()
     * \see previous()
     * \see first()
     */
    virtual bool last() = 0;

    /**
     * Seeks to the first feature, returning false if no feature was found.
     * \see next()
     * \see previous()
     * \see last()
     */
    virtual bool first() = 0;
};

#endif //QGSABSTRACTLAYOUTITERATOR_H



