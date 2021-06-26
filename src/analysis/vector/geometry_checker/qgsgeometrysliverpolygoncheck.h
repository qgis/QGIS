/***************************************************************************
    qgsgeometrysliverpolygoncheck.h
    ---------------------
    begin                : September 2015
    copyright            : (C) 2014 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define SIP_NO_FILE

#ifndef QGS_GEOMETRY_SLIVERPOLYGON_CHECK_H
#define QGS_GEOMETRY_SLIVERPOLYGON_CHECK_H

#include "qgsgeometryareacheck.h"

/**
 * \ingroup analysis
 */
class ANALYSIS_EXPORT QgsGeometrySliverPolygonCheck : public QgsGeometryAreaCheck
{
  public:
    QgsGeometrySliverPolygonCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsGeometryAreaCheck( context, configuration )
    {
      mThresholdMapUnits = configurationValue<double>( "threshold" );
      mMaxArea = configurationValue<double>( "maxArea" );
    }
    static QString factoryDescription() { return tr( "Sliver polygon" ); }
    QString description() const override { return factoryDescription(); }
    static QString factoryId() { return QStringLiteral( "QgsGeometrySliverPolygonCheck" ); }
    QString id() const override { return factoryId(); }

  private:
    bool checkThreshold( double layerToMapUnits, const QgsAbstractGeometry *geom, double &value ) const override;

    double mThresholdMapUnits;
    double mMaxArea;

};

#endif // QGS_GEOMETRY_SLIVERPOLYGON_CHECK_H
