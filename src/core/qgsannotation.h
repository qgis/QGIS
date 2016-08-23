/***************************************************************************
                             qgsannotation.h
                             ---------------
    begin                : July 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#ifndef QGSANNOTATION_H
#define QGSANNOTATION_H

#include "qgspoint.h"
#include "qgscoordinatereferencesystem.h"

class QPainter;
class QStyleOptionGraphicsItem;

/** \ingroup core
 * \class QgsAnnotation
 * \note added in QGIS 3.0
 *
 * \brief An interface for annotation items which are drawn over a map.
 *
 * QgsAnnotation is an interface class for map annotation items. These annotations can be
 * drawn within a map, and have either a fixed map position (retrieved using mapPosition())
 * or are placed relative to the map's frame (retrieved using relativePosition()).
 * Annotations with a fixed map position also have a corresponding
 * QgsCoordinateReferenceSystem, which can be determined by calling mapPositionCrs().
 */

class CORE_EXPORT QgsAnnotation
{
  public:

    //! Returns true if the annotation should be shown.
    virtual bool showItem() const = 0;

    /** Returns true if the annotation is attached to a fixed map position, or
     * false if the annotation uses a position relative to the current map
     * extent.
     * @see mapPosition()
     * @see relativePositon()
     */
    virtual bool hasFixedMapPosition() const = 0;

    /** Returns the map position of the annotation, if it is attached to a fixed map
     * position.
     * @see mapPositionFixed()
     * @see mapPositionCrs()
     */
    virtual QgsPoint mapPosition() const { return QgsPoint(); }

    /** Returns the CRS of the map position, or an invalid CRS if the annotation does
     * not have a fixed map position.
    */
    virtual QgsCoordinateReferenceSystem mapPositionCrs() const { return QgsCoordinateReferenceSystem(); }

    /** Returns the relative position of the annotation, if it is not attached to a fixed map
     * position. The coordinates in the return point should be between 0 and 1, and represent
     * the relative percentage for the position compared to the map width and height.
     * @see mapPositionFixed()
     */
    virtual QPointF relativePosition() const { return QPointF(); }

    /** Returns a scaling factor which should be applied to painters before rendering
     * the item.
     */
    virtual double scaleFactor() const = 0;

    //! deprecated - do not use
    // TODO QGIS 3.0 - remove
    virtual void setItemData( int role, const QVariant& value ) = 0;

    //! Paint the annotation to a destination painter
    virtual void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr ) = 0;


};

#endif // QGSANNOTATION_H
