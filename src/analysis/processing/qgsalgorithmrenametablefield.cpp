/***************************************************************************
                         qgsalgorithmrenametablefield.cpp
                         -----------------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsalgorithmrenametablefield.h"

///@cond PRIVATE

QString QgsRenameTableFieldAlgorithm::name() const
{
  return QStringLiteral( "renametablefield" );
}

QString QgsRenameTableFieldAlgorithm::displayName() const
{
  return QObject::tr( "Rename field" );
}

QString QgsRenameTableFieldAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm renames an existing field from a vector layer." );
}

QString QgsRenameTableFieldAlgorithm::shortDescription() const
{
  return QObject::tr( "Renames an existing field from a vector layer." );
}

QStringList QgsRenameTableFieldAlgorithm::tags() const
{
  return QObject::tr( "rename,attribute,fields,table,change" ).split( ',' );
}

QString QgsRenameTableFieldAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsRenameTableFieldAlgorithm::groupId() const
{
  return QStringLiteral( "vectortable" );
}

QString QgsRenameTableFieldAlgorithm::outputName() const
{
  return QObject::tr( "Renamed" );
}

QList<int> QgsRenameTableFieldAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector );
}

Qgis::ProcessingFeatureSourceFlags QgsRenameTableFieldAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

QgsRenameTableFieldAlgorithm *QgsRenameTableFieldAlgorithm::createInstance() const
{
  return new QgsRenameTableFieldAlgorithm();
}

void QgsRenameTableFieldAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Field to rename" ), QVariant(), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "NEW_NAME" ), QObject::tr( "New field name" ) ) );
}

QgsFields QgsRenameTableFieldAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields outFields = inputFields;
  const int index = outFields.lookupField( mOriginalName );
  if ( index < 0 )
    throw QgsProcessingException( QObject::tr( "Field %1 could not be found in input table" ).arg( mOriginalName ) );

  if ( outFields.lookupField( mNewName ) >= 0 )
    throw QgsProcessingException( QObject::tr( "A field already exists with the name %1" ).arg( mNewName ) );

  outFields.rename( index, mNewName );
  return outFields;
}

bool QgsRenameTableFieldAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mOriginalName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  mNewName = parameterAsString( parameters, QStringLiteral( "NEW_NAME" ), context );
  return true;
}

QgsFeatureList QgsRenameTableFieldAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  return QgsFeatureList() << feature;
}

bool QgsRenameTableFieldAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return false;
}

///@endcond
