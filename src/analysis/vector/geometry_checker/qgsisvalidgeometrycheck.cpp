/***************************************************************************
                      qgssinglegeometrycheck.cpp
                     --------------------------------------
Date                 : 7.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsisvalidgeometrycheck.h"
#include "qgsfeature.h"
#include "qgssettings.h"
#include "qgsgeos.h"
#include "qgsgeometryvalidator.h"

QList<QgsGeometryCheckError *> QgsIsValidGeometryCheck::processGeometry( const QgsGeometryCheckerUtils::LayerFeature &layerFeature, const QgsGeometry &geometry ) const
{
  QVector<QgsGeometry::Error> errors;

  QgsGeometry::ValidationMethod method = QgsGeometry::ValidatorQgisInternal;
  if ( QgsSettings().value( QStringLiteral( "qgis/digitizing/validate_geometries" ), 1 ).toInt() == 2 )
    method = QgsGeometry::ValidatorGeos;

  QgsGeometryValidator validator( geometry, &errors, method );

  QObject::connect( &validator, &QgsGeometryValidator::errorFound, &validator, [ &errors ]( const QgsGeometry::Error & error )
  {
    errors.append( error );
  } );

  // We are already on a thread here normally, no reason to start yet another one. Run synchroneously.
  validator.run();

  QList<QgsGeometryCheckError *> result;
  for ( const auto &error : qgis::as_const( errors ) )
  {
    result << new QgsGeometryCheckError( this, layerFeature, error.where() );
  }
  return result;
}

void QgsIsValidGeometryCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, QgsGeometryCheck::Changes &changes ) const
{

}

QStringList QgsIsValidGeometryCheck::resolutionMethods() const
{

}
