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
#include "qgssettingsregistrycore.h"
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

  Qgis::GeometryValidationEngine method = Qgis::GeometryValidationEngine::QgisInternal;
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries.value() == 2 )
    method = Qgis::GeometryValidationEngine::Geos;

  QgsGeometryValidator validator( geometry, &errors, method );

  QObject::connect( &validator, &QgsGeometryValidator::errorFound, &validator, [ &errors ]( const QgsGeometry::Error & error )
  {
    errors.append( error );
  } );

  // We are already on a thread here normally, no reason to start yet another one. Run synchronously.
  validator.run();

  QList<QgsSingleGeometryCheckError *> result;
  for ( const auto &error : std::as_const( errors ) )
  {
    QgsGeometry errorGeometry;
    if ( error.hasWhere() )
      errorGeometry = QgsGeometry( std::make_unique<QgsPoint>( error.where() ) );

    result << new QgsGeometryIsValidCheckError( this, geometry, errorGeometry, error.what() );
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

QgsGeometryIsValidCheckError::QgsGeometryIsValidCheckError( const QgsSingleGeometryCheck *check, const QgsGeometry &geometry, const QgsGeometry &errorLocation, const QString &errorDescription )
  : QgsSingleGeometryCheckError( check, geometry, errorLocation )
  , mDescription( errorDescription )
{

}

QString QgsGeometryIsValidCheckError::description() const
{
  return mDescription;
}
