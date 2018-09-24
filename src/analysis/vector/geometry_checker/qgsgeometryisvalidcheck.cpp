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

#include "qgsgeometryisvalidcheck.h"
#include "qgsfeature.h"
#include "qgssettings.h"
#include "qgsgeos.h"
#include "qgsgeometryvalidator.h"

QgsGeometryIsValidCheck::QgsGeometryIsValidCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration )
  : QgsSingleGeometryCheck( context, configuration )
{}

QList<QgsWkbTypes::GeometryType> QgsGeometryIsValidCheck::compatibleGeometryTypes() const
{
  return factoryCompatibleGeometryTypes();
}

QList<QgsSingleGeometryCheckError *> QgsGeometryIsValidCheck::processGeometry( const QgsGeometry &geometry ) const
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

  // We are already on a thread here normally, no reason to start yet another one. Run synchronously.
  validator.run();

  QList<QgsSingleGeometryCheckError *> result;
  for ( const auto &error : qgis::as_const( errors ) )
  {
    result << new QgsSingleGeometryCheckError( this, geometry, QgsGeometry( qgis::make_unique<QgsPoint>( error.where() ) ) );
  }
  return result;
}

QStringList QgsGeometryIsValidCheck::resolutionMethods() const
{
  return QStringList();
}
///@cond private
QList<QgsWkbTypes::GeometryType> QgsGeometryIsValidCheck::factoryCompatibleGeometryTypes()
{
  return {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry};
}

bool QgsGeometryIsValidCheck::factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP
{
  return factoryCompatibleGeometryTypes().contains( layer->geometryType() );
}

QString QgsGeometryIsValidCheck::factoryDescription()
{
  return tr( "Is Valid" );
}

QString QgsGeometryIsValidCheck::factoryId()
{
  return QStringLiteral( "QgsIsValidCheck" );
}

QgsGeometryCheck::Flags QgsGeometryIsValidCheck::factoryFlags()
{
  return AvailableInValidation;
}

QgsGeometryCheck::CheckType QgsGeometryIsValidCheck::factoryCheckType()
{
  return QgsGeometryCheck::FeatureNodeCheck;
}
///@endcond
