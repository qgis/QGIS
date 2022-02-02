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
    const QSet<int> supportedDataType,
    bool optional ):
  QgsProcessingParameterDefinition( name, description, QVariantList(), optional, QString() ),
  mMeshLayerParameterName( meshLayerParameterName ),
  mSupportedDataType( supportedDataType )
{
}

QgsProcessingParameterDefinition *QgsProcessingParameterMeshDatasetGroups::clone() const
{
  return new QgsProcessingParameterMeshDatasetGroups( name(), description(), mMeshLayerParameterName, mSupportedDataType );
}

QString QgsProcessingParameterMeshDatasetGroups::type() const
{
  return typeName();
}

bool QgsProcessingParameterMeshDatasetGroups::checkValueIsAcceptable( const QVariant &input, QgsProcessingContext *context ) const
{
  Q_UNUSED( context );
  return valueIsAcceptable( input, mFlags & FlagOptional );
}

QString QgsProcessingParameterMeshDatasetGroups::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  Q_UNUSED( context );
  QStringList parts;
  const QList<int> groups = valueAsDatasetGroup( value );
  for ( const int g : groups )
    parts.append( QString::number( g ) );

  return parts.join( ',' ).prepend( '[' ).append( ']' );
}

QString QgsProcessingParameterMeshDatasetGroups::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterMeshDatasetGroups('%1', %2" )
                     .arg( name(), QgsProcessingUtils::stringToPythonLiteral( description() ) );
      if ( !mMeshLayerParameterName.isEmpty() )
        code += QStringLiteral( ", meshLayerParameterName=%1" ).arg( QgsProcessingUtils::stringToPythonLiteral( mMeshLayerParameterName ) );

      QStringList dt;
      if ( mSupportedDataType.contains( QgsMeshDatasetGroupMetadata::DataOnFaces ) )
        dt.append( QStringLiteral( "QgsMeshDatasetGroupMetadata.DataOnFaces" ) );
      if ( mSupportedDataType.contains( QgsMeshDatasetGroupMetadata::DataOnVertices ) )
        dt.append( QStringLiteral( "QgsMeshDatasetGroupMetadata.DataOnVertices" ) );
      if ( mSupportedDataType.contains( QgsMeshDatasetGroupMetadata::DataOnVolumes ) )
        dt.append( QStringLiteral( "QgsMeshDatasetGroupMetadata.DataOnVolumes" ) );
      if ( mSupportedDataType.contains( QgsMeshDatasetGroupMetadata::DataOnEdges ) )
        dt.append( QStringLiteral( "QgsMeshDatasetGroupMetadata.DataOnEdges" ) );
      if ( !dt.isEmpty() )
      {
        code += QLatin1String( ", supportedDataType=[" );
        code += dt.join( ',' );
        code += ']';
      }

      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );
      code += ')';
      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterMeshDatasetGroups::dependsOnOtherParameters() const
{
  if ( mMeshLayerParameterName.isEmpty() )
    return QStringList();
  else
    return QStringList() << mMeshLayerParameterName;
}

QString QgsProcessingParameterMeshDatasetGroups::meshLayerParameterName() const
{
  return mMeshLayerParameterName;
}

bool QgsProcessingParameterMeshDatasetGroups::isDataTypeSupported( QgsMeshDatasetGroupMetadata::DataType dataType ) const
{
  return mSupportedDataType.contains( dataType );
}

QList<int> QgsProcessingParameterMeshDatasetGroups::valueAsDatasetGroup( const QVariant &value )
{
  if ( !valueIsAcceptable( value, true ) )
    return QList<int>();

  QList<int> ret;

  // if invalid or empty, return only the group 0
  if ( !value.isValid() )
    ret << 0;
  else
  {
    if ( value.type() == QVariant::List )
    {
      const QVariantList varList = value.toList();
      if ( varList.isEmpty() )
        ret << 0;
      else
        for ( const QVariant &v : varList )
          ret << v.toInt();
    }
    else
    {
      ret << value.toInt();
    }
  }

  return ret;
}

bool QgsProcessingParameterMeshDatasetGroups::valueIsAcceptable( const QVariant &input, bool allowEmpty )
{
  if ( !input.isValid() )
    return allowEmpty;

  if ( input.type() != QVariant::List )
  {
    bool ok = false;
    input.toInt( &ok );
    return ok;
  }
  const QVariantList list = input.toList();

  if ( !allowEmpty && list.isEmpty() )
    return false;

  for ( const QVariant &var : list )
  {
    bool ok = false;
    var.toInt( &ok );
    if ( !ok )
      return false;
  }

  return true;
}

QgsProcessingParameterMeshDatasetTime::QgsProcessingParameterMeshDatasetTime( const QString &name,
    const QString &description,
    const QString &meshLayerParameterName,
    const QString &datasetGroupParameterName )
  : QgsProcessingParameterDefinition( name, description, QVariant() )
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
  return valueIsAcceptable( input, mFlags & FlagOptional );
}

QString QgsProcessingParameterMeshDatasetTime::valueAsPythonString( const QVariant &value, QgsProcessingContext &context ) const
{
  Q_UNUSED( context );
  QStringList parts;
  const QString type = QgsProcessingUtils::variantToPythonLiteral( valueAsTimeType( value ) );
  parts << QStringLiteral( "'type': " ) +  type;

  if ( value.toDateTime().isValid() )
  {
    QDateTime dateTime = value.toDateTime();
    dateTime.setTimeSpec( Qt::UTC );
    parts << QStringLiteral( "'value': " ) + QgsProcessingUtils::variantToPythonLiteral( dateTime );
  }
  else
  {
    const QVariantMap variantTimeDataset = value.toMap();
    if ( variantTimeDataset.value( QStringLiteral( "type" ) ) == QLatin1String( "dataset-time-step" ) )
    {
      const QVariantList datasetIndex = variantTimeDataset.value( QStringLiteral( "value" ) ).toList();
      parts << QStringLiteral( "'value': " ) + QString( "[%1,%2]" ).arg( datasetIndex.at( 0 ).toString(), datasetIndex.at( 1 ).toString() );
    }
    else if ( variantTimeDataset.value( QStringLiteral( "type" ) ) == QLatin1String( "defined-date-time" ) )
    {
      parts << QStringLiteral( "'value': " ) + QgsProcessingUtils::variantToPythonLiteral( variantTimeDataset.value( QStringLiteral( "value" ) ) );
    }
  }

  return parts.join( ',' ).prepend( '{' ).append( '}' );
}

QString QgsProcessingParameterMeshDatasetTime::asPythonString( QgsProcessing::PythonOutputType outputType ) const
{
  switch ( outputType )
  {
    case QgsProcessing::PythonQgsProcessingAlgorithmSubclass:
    {
      QString code = QStringLiteral( "QgsProcessingParameterMeshDatasetTime('%1', '%2'" )
                     .arg( name(), description() );
      if ( !mMeshLayerParameterName.isEmpty() )
        code += QStringLiteral( ", meshLayerParameterName=%1" ).arg( QgsProcessingUtils::stringToPythonLiteral( mMeshLayerParameterName ) );

      if ( !mDatasetGroupParameterName.isEmpty() )
        code += QStringLiteral( ", datasetGroupParameterName=%1" ).arg( QgsProcessingUtils::stringToPythonLiteral( mDatasetGroupParameterName ) );

      if ( mFlags & FlagOptional )
        code += QLatin1String( ", optional=True" );
      code += ')';
      return code;
    }
  }
  return QString();
}

QStringList QgsProcessingParameterMeshDatasetTime::dependsOnOtherParameters() const
{
  QStringList otherParameters;
  if ( !mMeshLayerParameterName.isEmpty() )
    otherParameters << mMeshLayerParameterName;

  if ( !mDatasetGroupParameterName.isEmpty() )
    otherParameters << mMeshLayerParameterName << mDatasetGroupParameterName;

  return otherParameters;
}

QString QgsProcessingParameterMeshDatasetTime::meshLayerParameterName() const
{
  return mMeshLayerParameterName;
}

QString QgsProcessingParameterMeshDatasetTime::datasetGroupParameterName() const
{
  return mDatasetGroupParameterName;
}

QString QgsProcessingParameterMeshDatasetTime::valueAsTimeType( const QVariant &value )
{
  if ( !valueIsAcceptable( value, false ) )
    return QString();

  if ( value.toDateTime().isValid() )
    return QStringLiteral( "defined-date-time" );

  return value.toMap().value( QStringLiteral( "type" ) ).toString();
}

QgsMeshDatasetIndex QgsProcessingParameterMeshDatasetTime::timeValueAsDatasetIndex( const QVariant &value )
{
  if ( !valueIsAcceptable( value, false ) || valueAsTimeType( value ) != QLatin1String( "dataset-time-step" ) )
    return QgsMeshDatasetIndex( -1, -1 );

  const QVariantList list = value.toMap().value( QStringLiteral( "value" ) ).toList();
  return QgsMeshDatasetIndex( list.at( 0 ).toInt(), list.at( 1 ).toInt() );
}

QDateTime QgsProcessingParameterMeshDatasetTime::timeValueAsDefinedDateTime( const QVariant &value )
{
  if ( value.toDateTime().isValid() )
  {
    QDateTime dateTime = value.toDateTime();
    dateTime.setTimeSpec( Qt::UTC );
    return dateTime;
  }

  if ( !valueIsAcceptable( value, false ) && valueAsTimeType( value ) != QLatin1String( "defined-date-time" ) )
    return QDateTime();

  return value.toMap().value( QStringLiteral( "value" ) ).toDateTime();
}

bool QgsProcessingParameterMeshDatasetTime::valueIsAcceptable( const QVariant &input, bool allowEmpty )
{
  if ( !input.isValid() )
    return allowEmpty;

  if ( input.toDateTime().isValid() )
    return true;

  if ( input.type() != QVariant::Map )
    return false;

  const QVariantMap map = input.toMap();

  if ( map.isEmpty() )
    return allowEmpty;

  if ( ! map.contains( QStringLiteral( "type" ) ) )
    return false;

  const QString type = map.value( QStringLiteral( "type" ) ).toString();
  const QVariant value = map.value( QStringLiteral( "value" ) );

  if ( type == QLatin1String( "static" ) || type == QLatin1String( "current-context-time" ) )
    return true;

  if ( type == QLatin1String( "dataset-time-step" ) )
  {
    if ( value.type() != QVariant::List )
      return false;
    const QVariantList list = value.toList();
    if ( value.toList().count() != 2 )
      return false;
    if ( list.at( 0 ).type() != QVariant::Int || list.at( 1 ).type() != QVariant::Int )
      return false;
  }
  else if ( type == QLatin1String( "defined-date-time" ) )
  {
    if ( value.type() != QVariant::DateTime )
      return false;
  }
  else
    return false;

  return true;
}

/// @endcond PRIVATE
