/***************************************************************************
                             qgslayoutguidecollection.h
                             --------------------------
    begin                : July 2017
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
#ifndef QGSLAYOUTGUIDECOLLECTION_H
#define QGSLAYOUTGUIDECOLLECTION_H

#include "qgis_core.h"
#include "qgslayoutmeasurement.h"
#include "qgslayoutpoint.h"
#include <QPen>
#include <memory>

class QgsLayout;
class QGraphicsLineItem;

/**
 * \ingroup core
 * \class QgsLayoutGuide
 * \brief Contains the configuration for a single snap guide used by a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutGuide
{

  public:

    //! Guide orientation
    enum Orientation
    {
      Horizontal, //!< Horizontal guide
      Vertical, //!< Vertical guide
    };

    QgsLayoutGuide( QgsLayout *layout, Orientation orientation, const QgsLayoutMeasurement &position );

    /**
     * Returns the guide's orientation.
     */
    Orientation orientation() const;

    /**
     * Returns the guide's position within the page.
     *
     * The position indicates either the horizontal or vertical position
     * of the guide, depending on the guide's orientation().
     *
     * \see setPosition()
     */
    QgsLayoutMeasurement position() const;

    /**
     * Sets the guide's \a position within the page.
     *
     * The \a position argument indicates either the horizontal or vertical position
     * of the guide, depending on the guide's orientation().
     *
     * \see position()
     */
    void setPosition( const QgsLayoutMeasurement &position );

    /**
     * Returns the page number of the guide.
     *
     * Page numbering begins at 0.
     *
     * \see setPage()
     */
    int page() const;

    /**
     * Sets the \a page number of the guide.
     *
     * Page numbering begins at 0.
     *
     * \see page()
     */
    void setPage( int page );

    /**
     * Updates the position of the guide's line item.
     */
    void update();

    /**
     * Returns the guide's line item.
     */
    QGraphicsLineItem *item();

  private:

    Orientation mOrientation = Vertical;

    //! Horizontal/vertical position of guide on page
    QgsLayoutMeasurement mPosition;

    //! Page number, 0 index based
    int mPage = 0;

    QgsLayout *mLayout = nullptr;

    //! Line item used in scene for guide
    std::shared_ptr< QGraphicsLineItem > mLineItem;
};



#endif //QGSLAYOUTGUIDECOLLECTION_H
