/***************************************************************************
                qgsvirtuallayerdefinition.cpp
begin                : December 2015
copyright            : (C) 2015 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QUrl>
#include <QRegExp>
#include <QStringList>

#include "qgsvirtuallayerdefinition.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

QgsVirtualLayerDefinition::QgsVirtualLayerDefinition( const QString &filePath )
  : mFilePath( filePath )
{
}

QgsVirtualLayerDefinition QgsVirtualLayerDefinition::fromUrl( const QUrl &url )
{
  QgsVirtualLayerDefinition def;

  def.setFilePath( url.toLocalFile() );

  // regexp for column name
  const QString columnNameRx( QStringLiteral( "[a-zA-Z_\x80-\xFF][a-zA-Z0-9_\x80-\xFF]*" ) );

  QgsFields fields;

  int layerIdx = 0;
  QList<QPair<QByteArray, QByteArray> > items = url.encodedQueryItems();
  for ( int i = 0; i < items.size(); i++ )
  {
    QString key = items.at( i ).first;
    QString value = items.at( i ).second;
    if ( key == QLatin1String( "layer_ref" ) )
    {
      layerIdx++;
      // layer id, with optional layer_name
      int pos = value.indexOf( ':' );
      QString layerId, vlayerName;
      if ( pos == -1 )
      {
        layerId = value;
        vlayerName = QStringLiteral( "vtab%1" ).arg( layerIdx );
      }
      else
      {
        layerId = value.left( pos );
        vlayerName = QUrl::fromPercentEncoding( value.mid( pos + 1 ).toUtf8() );
      }
      // add the layer to the list
      def.addSource( vlayerName, layerId );
    }
    else if ( key == QLatin1String( "layer" ) )
    {
      layerIdx++;
      // syntax: layer=provider:url_encoded_source_URI(:name(:encoding)?)?
      int pos = value.indexOf( ':' );
      if ( pos != -1 )
      {
        QString providerKey, source, vlayerName, encoding = QStringLiteral( "UTF-8" );

        providerKey = value.left( pos );
        int pos2 = value.indexOf( ':', pos + 1 );
        if ( pos2 - pos == 2 )
          pos2 = value.indexOf( ':', pos + 3 );
        if ( pos2 != -1 )
        {
          source = QUrl::fromPercentEncoding( value.mid( pos + 1, pos2 - pos - 1 ).toUtf8() );
          int pos3 = value.indexOf( ':', pos2 + 1 );
          if ( pos3 != -1 )
          {
            vlayerName = QUrl::fromPercentEncoding( value.mid( pos2 + 1, pos3 - pos2 - 1 ).toUtf8() );
            encoding = value.mid( pos3 + 1 );
          }
          else
          {
            vlayerName = QUrl::fromPercentEncoding( value.mid( pos2 + 1 ).toUtf8() );
          }
        }
        else
        {
          source = QUrl::fromPercentEncoding( value.mid( pos + 1 ).toUtf8() );
          vlayerName = QStringLiteral( "vtab%1" ).arg( layerIdx );
        }

        def.addSource( vlayerName, source, providerKey, encoding );
      }
    }
    else if ( key == QLatin1String( "geometry" ) )
    {
      // geometry field definition, optional
      // geometry_column(:wkb_type:srid)?
      QRegExp reGeom( "(" + columnNameRx + ")(?::([a-zA-Z0-9]+):(\\d+))?" );
      int pos = reGeom.indexIn( value );
      if ( pos >= 0 )
      {
        def.setGeometryField( reGeom.cap( 1 ) );
        if ( reGeom.captureCount() > 1 )
        {
          // not used by the spatialite provider for now ...
          QgsWkbTypes::Type wkbType = QgsWkbTypes::parseType( reGeom.cap( 2 ) );
          if ( wkbType == QgsWkbTypes::Unknown )
          {
            wkbType = static_cast<QgsWkbTypes::Type>( reGeom.cap( 2 ).toLong() );
          }
          def.setGeometryWkbType( wkbType );
          def.setGeometrySrid( reGeom.cap( 3 ).toLong() );
        }
      }
    }
    else if ( key == QLatin1String( "nogeometry" ) )
    {
      def.setGeometryWkbType( QgsWkbTypes::NoGeometry );
    }
    else if ( key == QLatin1String( "uid" ) )
    {
      def.setUid( value );
    }
    else if ( key == QLatin1String( "query" ) )
    {
      // url encoded query
      def.setQuery( QUrl::fromPercentEncoding( value.toUtf8() ) );
    }
    else if ( key == QLatin1String( "field" ) )
    {
      // field_name:type (int, real, text)
      QRegExp reField( "(" + columnNameRx + "):(int|real|text)" );
      int pos = reField.indexIn( value );
      if ( pos >= 0 )
      {
        QString fieldName( reField.cap( 1 ) );
        QString fieldType( reField.cap( 2 ) );
        if ( fieldType == QLatin1String( "int" ) )
        {
          fields.append( QgsField( fieldName, QVariant::Int, fieldType ) );
        }
        else if ( fieldType == QLatin1String( "real" ) )
        {
          fields.append( QgsField( fieldName, QVariant::Double, fieldType ) );
        }
        if ( fieldType == QLatin1String( "text" ) )
        {
          fields.append( QgsField( fieldName, QVariant::String, fieldType ) );
        }
      }
    }
    else if ( key == QLatin1String( "lazy" ) )
    {
      def.setLazy( true );
    }
  }
  def.setFields( fields );

  return def;
}

QUrl QgsVirtualLayerDefinition::toUrl() const
{
  QUrl url;
  if ( !filePath().isEmpty() )
    url = QUrl::fromLocalFile( filePath() );

  Q_FOREACH ( const QgsVirtualLayerDefinition::SourceLayer &l, sourceLayers() )
  {
    if ( l.isReferenced() )
      url.addQueryItem( QStringLiteral( "layer_ref" ), QStringLiteral( "%1:%2" ).arg( l.reference(), l.name() ) );
    else
      url.addEncodedQueryItem( "layer", QStringLiteral( "%1:%4:%2:%3" ) // the order is important, since the 4th argument may contain '%2' as well
                               .arg( l.provider(),
                                     QString( QUrl::toPercentEncoding( l.name() ) ),
                                     l.encoding(),
                                     QString( QUrl::toPercentEncoding( l.source() ) ) ).toUtf8() );
  }

  if ( !query().isEmpty() )
  {
    url.addQueryItem( QStringLiteral( "query" ), query() );
  }

  if ( !uid().isEmpty() )
    url.addQueryItem( QStringLiteral( "uid" ), uid() );

  if ( geometryWkbType() == QgsWkbTypes::NoGeometry )
    url.addQueryItem( QStringLiteral( "nogeometry" ), QString() );
  else if ( !geometryField().isEmpty() )
  {
    if ( hasDefinedGeometry() )
      url.addQueryItem( QStringLiteral( "geometry" ), QStringLiteral( "%1:%2:%3" ).arg( geometryField() ). arg( geometryWkbType() ).arg( geometrySrid() ).toUtf8() );
    else
      url.addQueryItem( QStringLiteral( "geometry" ), geometryField() );
  }

  Q_FOREACH ( const QgsField &f, fields() )
  {
    if ( f.type() == QVariant::Int )
      url.addQueryItem( QStringLiteral( "field" ), f.name() + ":int" );
    else if ( f.type() == QVariant::Double )
      url.addQueryItem( QStringLiteral( "field" ), f.name() + ":real" );
    else if ( f.type() == QVariant::String )
      url.addQueryItem( QStringLiteral( "field" ), f.name() + ":text" );
  }

  if ( isLazy() )
  {
    url.addQueryItem( QStringLiteral( "lazy" ), QString() );
  }

  return url;
}

QString QgsVirtualLayerDefinition::toString() const
{
  return QString( toUrl().toEncoded() );
}

void QgsVirtualLayerDefinition::addSource( const QString &name, const QString &ref )
{
  mSourceLayers.append( SourceLayer( name, ref ) );
}

void QgsVirtualLayerDefinition::addSource( const QString &name, const QString &source, const QString &provider, const QString &encoding )
{
  mSourceLayers.append( SourceLayer( name, source, provider, encoding ) );
}

bool QgsVirtualLayerDefinition::hasSourceLayer( const QString &name ) const
{
  Q_FOREACH ( const QgsVirtualLayerDefinition::SourceLayer &l, sourceLayers() )
  {
    if ( l.name() == name )
    {
      return true;
    }
  }
  return false;
}

bool QgsVirtualLayerDefinition::hasReferencedLayers() const
{
  Q_FOREACH ( const QgsVirtualLayerDefinition::SourceLayer &l, sourceLayers() )
  {
    if ( l.isReferenced() )
    {
      return true;
    }
  }
  return false;
}
