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

class ANALYSIS_EXPORT QgsGeometrySliverPolygonCheck : public QgsGeometryAreaCheck
{
  public:
    QgsGeometrySliverPolygonCheck( QgsGeometryCheckContext *context, double threshold, double maxAreaMapUnits )
      : QgsGeometryAreaCheck( context, threshold )
      , mMaxAreaMapUnits( maxAreaMapUnits )
    {}
    QString errorDescription() const override { return tr( "Sliver polygon" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometrySliverPolygonCheck" ); }

  private:
    double mMaxAreaMapUnits;

    bool checkThreshold( double layerToMapUnits, const QgsAbstractGeometry *geom, double &value ) const override;
};

#endif // QGS_GEOMETRY_SLIVERPOLYGON_CHECK_H
