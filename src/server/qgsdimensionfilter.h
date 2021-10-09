/***************************************************************************
                              qgsdimensionfilter.h
                              -------------------
  begin                : September 2021
  copyright            : (C) 2021 Matthias Kuhn
  email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDIMENSIONFILTER_H
#define QGSDIMENSIONFILTER_H

#define SIP_NO_FILE

#include "qgsfeaturefilterprovider.h"
#include "qgis_server.h"

/**
 * \ingroup server
 * \class QgsDimensionFilter
 * \brief A server filter to apply a dimension filter to a request
 * \since QGIS 3.22
 */
class SERVER_EXPORT QgsDimensionFilter : public QgsFeatureFilterProvider
{
  public:

    /**
     * Creates a new dimension filter object with a list of filters to be applied to
     * vector layers.
     */
    QgsDimensionFilter( const QMap<const QgsVectorLayer *, QStringList> dimensionFilter );

    void filterFeatures( const QgsVectorLayer *layer, QgsFeatureRequest &filterFeatures ) const override;
    QStringList layerAttributes( const QgsVectorLayer *layer, const QStringList &attributes ) const override;
    QgsDimensionFilter *clone() const override;
  private:
    QMap<const QgsVectorLayer *, QStringList> mDimensionFilter;
};

#endif // QGSDIMENSIONFILTER_H
