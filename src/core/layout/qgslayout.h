/***************************************************************************
                              qgslayout.h
                             -------------------
    begin                : June 2017
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
#ifndef QGSLAYOUT_H
#define QGSLAYOUT_H

#include "qgis_core.h"
#include <QGraphicsScene>
#include "qgslayoutcontext.h"

/**
 * \ingroup core
 * \class QgsLayout
 * \brief Base class for layouts, which can contain items such as maps, labels, scalebars, etc.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayout : public QGraphicsScene
{
    Q_OBJECT

  public:

    //! Preset item z-values, to ensure correct stacking
    enum ZValues
    {
      ZMapTool = 10000, //!< Z-Value for temporary map tool items
    };

    QgsLayout();

    /**
     * Sets the native measurement \a units for the layout. These also form the default unit
     * for measurements for the layout.
     * \see units()
     * \see convertToLayoutUnits()
    */
    void setUnits( QgsUnitTypes::LayoutUnit units ) { mUnits = units; }

    /**
     * Returns the native units for the layout.
     * \see setUnits()
     * \see convertToLayoutUnits()
    */
    QgsUnitTypes::LayoutUnit units() const { return mUnits; }

    /**
     * Converts a measurement into the layout's native units.
     * \returns length of measurement in layout units
     * \see convertFromLayoutUnits()
     * \see units()
    */
    double convertToLayoutUnits( const QgsLayoutMeasurement &measurement ) const;

    /**
     * Converts a size into the layout's native units.
     * \returns size of measurement in layout units
     * \see convertFromLayoutUnits()
     * \see units()
    */
    QSizeF convertToLayoutUnits( const QgsLayoutSize &size ) const;

    /**
     * Converts a \a point into the layout's native units.
     * \returns point in layout units
     * \see convertFromLayoutUnits()
     * \see units()
     */
    QPointF convertToLayoutUnits( const QgsLayoutPoint &point ) const;

    /**
     * Converts a \a length measurement from the layout's native units to a specified target \a unit.
     * \returns length of measurement in specified units
     * \see convertToLayoutUnits()
     * \see units()
    */
    QgsLayoutMeasurement convertFromLayoutUnits( const double length, const QgsUnitTypes::LayoutUnit unit ) const;

    /**
     * Converts a \a size from the layout's native units to a specified target \a unit.
     * \returns size of measurement in specified units
     * \see convertToLayoutUnits()
     * \see units()
    */
    QgsLayoutSize convertFromLayoutUnits( const QSizeF &size, const QgsUnitTypes::LayoutUnit unit ) const;

    /**
     * Converts a \a point from the layout's native units to a specified target \a unit.
     * \returns point in specified units
     * \see convertToLayoutUnits()
     * \see units()
    */
    QgsLayoutPoint convertFromLayoutUnits( const QPointF &point, const QgsUnitTypes::LayoutUnit unit ) const;

    /**
     * Returns a reference to the layout's context, which stores information relating to the
     * current context and rendering settings for the layout.
     */
    QgsLayoutContext &context() { return mContext; }

    /**
     * Returns a reference to the layout's context, which stores information relating to the
     * current context and rendering settings for the layout.
     */
    SIP_SKIP const QgsLayoutContext &context() const { return mContext; }

  private:

    QgsUnitTypes::LayoutUnit mUnits = QgsUnitTypes::LayoutMillimeters;
    QgsLayoutContext mContext;

};

#endif //QGSLAYOUT_H



