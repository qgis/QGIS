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

    //! Constructor for QgsCalloutPosition
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

  private:

    QPointF mOrigin;

    QPointF mDestination;
};

#endif // QGSCALLOUTPOSITION_H
