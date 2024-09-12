/***************************************************************************
  qgscalloutposition.h
  -------------------
   begin                : February 2021
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCALLOUTPOSITION_H
#define QGSCALLOUTPOSITION_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgsfeatureid.h"

#include <QPointF>

/**
 * \ingroup core
 * \class QgsCalloutPosition
 * \brief Represents the calculated placement of a map label callout line.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsCalloutPosition
{
  public:

    /**
     * Constructor for QgsCalloutPosition.
     * \param id associated feature ID
     * \param layer ID of associated map layer
     * \param providerId ID of associated label provider
     */
    QgsCalloutPosition( QgsFeatureId id, const QString &layer, const QString &providerId = QString() )
      : featureId( id )
      , layerID( layer )
      , providerID( providerId )
    {}

    QgsCalloutPosition() = default;

    /**
     * ID of feature associated with this callout.
     */
    QgsFeatureId featureId = FID_NULL;

    /**
     * ID of associated map layer.
     */
    QString layerID;

    /**
     * ID of the associated label provider.
     */
    QString providerID;

    /**
     * Returns the origin of the callout line, in map coordinates.
     *
     * The origin of the callout line is the line point associated with the label text.
     *
     * \see setOrigin()
     * \see destination()
     */
    QPointF origin() const { return mOrigin; }

    /**
     * Sets the \a origin of the callout line, in map coordinates.
     *
     * The origin of the callout line is the line point associated with the label text.
     *
     * \see origin()
     * \see setDestination()
     */
    void setOrigin( const QPointF &origin ) { mOrigin = origin; }

    /**
     * Returns the destination of the callout line, in map coordinates.
     *
     * The destination of the callout line is the line point associated with the feature's geometry.
     *
     * \see setDestination()
     * \see origin()
     */
    QPointF destination() const { return mDestination; }

    /**
     * Sets the \a destination of the callout line, in map coordinates.
     *
     * The destination of the callout line is the line point associated with the feature's geometry.
     *
     * \see destination()
     * \see setOrigin()
     */
    void setDestination( const QPointF &destination ) { mDestination = destination; }

    /**
     * Returns TRUE if the origin of the callout has pinned (manually placed).
     *
     * The origin of the callout line is the line point associated with the label text.
     *
     * \see destinationIsPinned()
     * \see setOriginIsPinned()
     */
    bool originIsPinned() const { return mOriginIsPinned; }

    /**
     * Sets whether the origin of the callout has pinned (manually placed).
     *
     * The origin of the callout line is the line point associated with the label text.
     *
     * \see setDestinationIsPinned()
     * \see originIsPinned()
     */
    void setOriginIsPinned( bool pinned ) { mOriginIsPinned = pinned; }

    /**
     * Returns TRUE if the destination of the callout has pinned (manually placed).
     *
     * The destination of the callout line is the line point associated with the feature's geometry.
     *
     * \see originIsPinned()
     * \see setDestinationIsPinned()
     */
    bool destinationIsPinned() const { return mDestinationIsPinned; }

    /**
     * Sets whether the destination of the callout has pinned (manually placed).
     *
     * The destination of the callout line is the line point associated with the feature's geometry.
     *
     * \see setOriginIsPinned()
     * \see destinationIsPinned()
     */
    void setDestinationIsPinned( bool pinned ) { mDestinationIsPinned = pinned; }

  private:

    QPointF mOrigin;

    QPointF mDestination;

    bool mOriginIsPinned = false;
    bool mDestinationIsPinned = false;
};

#endif // QGSCALLOUTPOSITION_H
