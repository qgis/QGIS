/***************************************************************************
  qgsprocessingparametermeshdataset.cpp
  ---------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingparametermeshdataset.h"

/// @cond PRIVATE
///
QgsProcessingParameterMeshDatasetGroups::QgsProcessingParameterMeshDatasetGroups( const QString &name,
    const QString &description,
    const QString &meshLayerParameterName,
    const QgsMeshDatasetGroupMetadata::DataType dataType,
    bool optional ):
  QgsProcessingParameterDefinition( name, description, QVariant(), optional, QString() ),
  mMeshLayerParameterName( meshLayerParameterName ),
  mDataType( dataType )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterMeshDatasetGroups::clone() const
{
  return new QgsProcessingParameterMeshDatasetGroups( name(), description(), mMeshLayerParameterName, mDataType );
}

QString QgsProcessingParameterMeshDatasetGroups::type() const
{
  return typeName();
}

bool QgsProcessingParameterMeshDatasetGroups::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  Q_UNUSED( context );
  if ( !input.isValid() )
    return false;

  if ( input.type() != QVariant::List )
    return false;
  const QVariantList list = input.toList();
  return list.count() > 0;
}

QString QgsProcessingParameterMeshDatasetGroups::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  Q_UNUSED( context );
  QStringList parts;
  const QVariantList variantDatasetGroupIndexes = value.toList();
  for ( const QVariant &variantIndex : variantDatasetGroupIndexes )
    parts.append( QString::number( variantIndex.toInt() ) );

  return parts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterMeshDatasetGroups::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterMeshDatasetGroup('%1', '%2')" ).arg( name(), description() );
      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterMeshDatasetGroups::dependsOnOtherParameters() const
{
  return QStringList() << mMeshLayerParameterName;
}

QString QgsProcessingParameterMeshDatasetGroups::meshLayerParameterName() const
{
  return mMeshLayerParameterName;
}

QgsProcessingParameterMeshDatasetTime::QgsProcessingParameterMeshDatasetTime(
  const QString &name,
  const QString &description,
  const QString &meshLayerParameterName,
  const QString &datasetGroupParameterName,
  bool optional )
  : QgsProcessingParameterDefinition( name, description, QVariant(), optional )
  , mMeshLayerParameterName( meshLayerParameterName )
  , mDatasetGroupParameterName( datasetGroupParameterName )
{

}

QgsProcessingParameterDefinition *QgsProcessingParameterMeshDatasetTime::clone() const
{
  return new QgsProcessingParameterMeshDatasetTime( name(), description(), mMeshLayerParameterName, mDatasetGroupParameterName );
}

QString QgsProcessingParameterMeshDatasetTime::type() const
{
  return typeName();
}

bool QgsProcessingParameterMeshDatasetTime::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  Q_UNUSED( context );
  if ( !input.isValid() )
    return false;

  if ( input.type() != QVariant::Map )
    return false;
  const QVariantMap map = input.toMap();
  if ( ! map.contains( QStringLiteral( "type" ) ) )
    return false;

  QString type = map.value( QStringLiteral( "type" ) ).toString();
  QVariant value = map.value( QStringLiteral( "value" ) );

  if ( type == QStringLiteral( "static" ) )
    return true;

  if ( type == QStringLiteral( "dataset-time-step" ) )
  {
    if ( value.type() != QVariant::List )
      return false;
    if ( value.toList().count() != 2 )
      return false;
  }
  else if ( type == QStringLiteral( "current-canvas-time" ) || type == QStringLiteral( "defined-date-time" ) )
  {
    if ( value.type() != QVariant::DateTime )
      return false;
  }
  else
    return false;

  return true;
}

QString QgsProcessingParameterMeshDatasetTime::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  Q_UNUSED( context );
  QStringList parts;
  const QVariantMap variantTimeDataset = value.toMap();
  parts << QStringLiteral( "'type': " ) +  QgsProcessingUtils::variantToPythonLiteral( variantTimeDataset.value( QStringLiteral( "type" ) ).toString() );

  if ( variantTimeDataset.value( QStringLiteral( "type" ) ) == QStringLiteral( "dataset-time-step" ) )
  {
    QVariantList datasetIndex = variantTimeDataset.value( QStringLiteral( "value" ) ).toList();
    parts << QStringLiteral( "'value': " ) + QString( "QgsMeshDatasetIndex(%1,%2)" ).arg( datasetIndex.at( 0 ).toString() ).arg( datasetIndex.at( 1 ).toString() );
  }
  else if ( variantTimeDataset.value( QStringLiteral( "type" ) ) != QStringLiteral( "static" ) )
  {
    QDateTime dateTime = variantTimeDataset.value( QStringLiteral( "value" ) ).toDateTime();
    parts << QStringLiteral( "'value': " ) + QStringLiteral( "QDateTime(QDate(%1, %2, %3), QTime(%4, %5, %6))" )
          .arg( dateTime.date().year() )
          .arg( dateTime.date().month() )
          .arg( dateTime.date().day() )
          .arg( dateTime.time().hour() )
          .arg( dateTime.time().minute() )
          .arg( dateTime.time().second() );
  }

  return parts.join( ',' ).prepend( '{' ).append( '}' );
}

QString QgsProcessingParameterMeshDatasetTime::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterMeshDatasetTime('%1', '%2')" ).arg( name(), description() );
      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterMeshDatasetTime::dependsOnOtherParameters() const
{
  return QStringList() << mMeshLayerParameterName << mDatasetGroupParameterName;
}

QString QgsProcessingParameterMeshDatasetTime::meshLayerParameterName() const
{
  return mMeshLayerParameterName;
}

QString QgsProcessingParameterMeshDatasetTime::datasetGroupParameterName() const
{
  return mDatasetGroupParameterName;
}

/// @endcond PRIVATE
