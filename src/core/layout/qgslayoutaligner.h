/***************************************************************************
                             qgslayoutaligner.h
                             ------------------
    begin                : October 2017
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
#ifndef QGSLAYOUTALIGNER_H
#define QGSLAYOUTALIGNER_H

#include "qgis_core.h"
#include <QList>
#include <QRectF>

class QgsLayoutItem;
class QgsLayout;

/**
 * \ingroup core
 * \class QgsLayoutAligner
 * \brief Handles aligning and distributing sets of layout items.
 *
 * QgsLayoutAligner contains methods for automatically aligning and distributing
 * sets of layout items, e.g. aligning a group of items to top or left sides.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutAligner
{

  public:

    //! Alignment options
    enum Alignment
    {
      Left, //!< Align left edges
      HCenter, //!< Align horizontal centers
      Right, //!< Align right edges
      Top, //!< Align top edges
      VCenter, //!< Align vertical centers
      Bottom, //!< Align bottom edges
    };

    /**
     * Aligns a set of \a items from a \a layout in place.
     *
     * The \a alignment argument specifies the method to use when aligning the items.
     */
    static void alignItems( QgsLayout *layout, const QList< QgsLayoutItem * > &items, Alignment alignment );

  private:

    /**
     * Returns the bounding rectangle of the selected items in
     * scene coordinates.
    */
    static QRectF boundingRectOfItems( const QList< QgsLayoutItem * > &items );



};

#endif //QGSLAYOUTALIGNER_H
