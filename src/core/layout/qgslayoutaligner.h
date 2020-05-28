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
      AlignLeft, //!< Align left edges
      AlignHCenter, //!< Align horizontal centers
      AlignRight, //!< Align right edges
      AlignTop, //!< Align top edges
      AlignVCenter, //!< Align vertical centers
      AlignBottom, //!< Align bottom edges
    };

    //! Distribution options
    enum Distribution
    {
      DistributeLeft, //!< Distribute left edges
      DistributeHCenter, //!< Distribute horizontal centers
      DistributeHSpace, //!< Distribute horizontal equispaced
      DistributeRight, //!< Distribute right edges
      DistributeTop, //!< Distribute top edges
      DistributeVCenter, //!< Distribute vertical centers
      DistributeVSpace, //!< Distribute vertical equispaced
      DistributeBottom, //!< Distribute bottom edges
    };

    //! Resize options
    enum Resize
    {
      ResizeNarrowest, //!< Resize width to match narrowest width
      ResizeWidest, //!< Resize width to match widest width
      ResizeShortest, //!< Resize height to match shortest height
      ResizeTallest, //!< Resize height to match tallest height
      ResizeToSquare, //!< Resize items to square
    };

    /**
     * Aligns a set of \a items from a \a layout in place.
     *
     * The \a alignment argument specifies the method to use when aligning the items.
     */
    static void alignItems( QgsLayout *layout, const QList< QgsLayoutItem * > &items, Alignment alignment );

    /**
     * Distributes a set of \a items from a \a layout in place.
     *
     * The \a distribution argument specifies the method to use when distributing the items.
     */
    static void distributeItems( QgsLayout *layout, const QList< QgsLayoutItem * > &items, Distribution distribution );

    /**
     * Resizes a set of \a items from a \a layout in place.
     *
     * The \a resize argument specifies the method to use when resizing the items.
     */
    static void resizeItems( QgsLayout *layout, const QList< QgsLayoutItem * > &items, Resize resize );

  private:

    /**
     * Returns the bounding rectangle of the selected items in
     * scene coordinates.
    */
    static QRectF boundingRectOfItems( const QList< QgsLayoutItem * > &items );

    static QString undoText( Alignment alignment );
    static QString undoText( Distribution distribution );
    static QString undoText( Resize resize );

    /**
     * Distributes a set of \a items from a \a layout in place for \a DistributeHSpace
     * and \a DistributeVSpace distribution type special cases.
     *
     * The \a distribution argument specifies the method to use when distributing the items.
     */
    static void distributeEquispacedItems( QgsLayout *layout, const QList<QgsLayoutItem *> &items, QgsLayoutAligner::Distribution distribution );


};

#endif //QGSLAYOUTALIGNER_H
