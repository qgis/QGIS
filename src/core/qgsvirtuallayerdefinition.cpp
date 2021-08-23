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

#include "qgsvirtuallayerdefinition.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "fromencodedcomponenthelper.h"

#include <QUrl>
#include <QRegularExpression>
#include <QStringList>
#include <QUrlQuery>


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

  const QList<QPair<QString, QString> > items = QUrlQuery( url ).queryItems( QUrl::FullyEncoded );
  for ( int i = 0; i < items.size(); i++ )
  {
    const QString key = items.at( i ).first;
    const QString value = items.at( i ).second;
    if ( key == QLatin1String( "layer_ref" ) )
    {
      layerIdx++;
      // layer id, with optional layer_name
      const int pos = value.indexOf( ':' );
      QString layerId, vlayerName;
      if ( pos == -1 )
      {
        layerId = QUrl::fromPercentEncoding( value.toUtf8() );
        vlayerName = QStringLiteral( "vtab%1" ).arg( layerIdx );
      }
      else
      {
        layerId = QUrl::fromPercentEncoding( value.left( pos ).toUtf8() );
        vlayerName = QUrl::fromPercentEncoding( value.mid( pos + 1 ).toUtf8() );
      }
      // add the layer to the list
      def.addSource( vlayerName, layerId );
    }
    else if ( key == QLatin1String( "layer" ) )
    {
      layerIdx++;
      // syntax: layer=provider:url_encoded_source_URI(:name(:encoding)?)?
      const int pos = value.indexOf( ':' );
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
          const int pos3 = value.indexOf( ':', pos2 + 1 );
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
      const QRegularExpression reGeom( "(" + columnNameRx + ")(?::([a-zA-Z0-9]+):(\\d+))?" );
      const QRegularExpressionMatch match = reGeom.match( value );
      if ( match.hasMatch() )
      {
        def.setGeometryField( match.captured( 1 ) );
        if ( match.capturedTexts().size() > 2 )
        {
          // not used by the spatialite provider for now ...
          QgsWkbTypes::Type wkbType = QgsWkbTypes::parseType( match.captured( 2 ) );
          if ( wkbType == QgsWkbTypes::Unknown )
          {
            wkbType = static_cast<QgsWkbTypes::Type>( match.captured( 2 ).toLong() );
          }
          def.setGeometryWkbType( wkbType );
          def.setGeometrySrid( match.captured( 3 ).toLong() );
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
      const QRegularExpression reField( "(" + columnNameRx + "):(int|real|text)" );
      const QRegularExpressionMatch match = reField.match( value );
      if ( match.hasMatch() )
      {
        const QString fieldName( match.captured( 1 ) );
        const QString fieldType( match.captured( 2 ) );
        if ( fieldType == QLatin1String( "int" ) )
        {
          fields.append( QgsField( fieldName, QVariant::LongLong, fieldType ) );
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
    else if ( key == QLatin1String( "subsetstring" ) )
    {
      def.setSubsetString( QUrl::fromPercentEncoding( value.toUtf8() ) );
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

  QUrlQuery urlQuery( url );

  const auto constSourceLayers = sourceLayers();
  for ( const QgsVirtualLayerDefinition::SourceLayer &l : constSourceLayers )
  {
    if ( l.isReferenced() )
      urlQuery.addQueryItem( QStringLiteral( "layer_ref" ), QStringLiteral( "%1:%2" ).arg( l.reference(), l.name() ) );
    else
      // if you can find a way to port this away from fromEncodedComponent_helper without breaking existing projects,
      // please do so... this is GROSS!
      urlQuery.addQueryItem( fromEncodedComponent_helper( "layer" ),
                             fromEncodedComponent_helper( QStringLiteral( "%1:%4:%2:%3" ) // the order is important, since the 4th argument may contain '%2' as well
                                 .arg( l.provider(),
                                       QString( QUrl::toPercentEncoding( l.name() ) ),
                                       l.encoding(),
                                       QString( QUrl::toPercentEncoding( l.source() ) ) ).toUtf8() ) );
  }

  if ( !query().isEmpty() )
  {
    urlQuery.addQueryItem( QStringLiteral( "query" ), query() );
  }

  if ( !uid().isEmpty() )
    urlQuery.addQueryItem( QStringLiteral( "uid" ), uid() );

  if ( geometryWkbType() == QgsWkbTypes::NoGeometry )
    urlQuery.addQueryItem( QStringLiteral( "nogeometry" ), QString() );
  else if ( !geometryField().isEmpty() )
  {
    if ( hasDefinedGeometry() )
      urlQuery.addQueryItem( QStringLiteral( "geometry" ), QStringLiteral( "%1:%2:%3" ).arg( geometryField() ). arg( geometryWkbType() ).arg( geometrySrid() ).toUtf8() );
    else
      urlQuery.addQueryItem( QStringLiteral( "geometry" ), geometryField() );
  }

  const auto constFields = fields();
  for ( const QgsField &f : constFields )
  {
    if ( f.type() == QVariant::Int
         || f.type() == QVariant::UInt
         || f.type() == QVariant::Bool
         || f.type() == QVariant::LongLong )
      urlQuery.addQueryItem( QStringLiteral( "field" ), f.name() + ":int" );
    else if ( f.type() == QVariant::Double )
      urlQuery.addQueryItem( QStringLiteral( "field" ), f.name() + ":real" );
    else if ( f.type() == QVariant::String )
      urlQuery.addQueryItem( QStringLiteral( "field" ), f.name() + ":text" );
  }

  if ( isLazy() )
  {
    urlQuery.addQueryItem( QStringLiteral( "lazy" ), QString() );
  }

  if ( ! subsetString().isEmpty() )
  {
    urlQuery.addQueryItem( QStringLiteral( "subsetstring" ), QUrl::toPercentEncoding( subsetString() ) );
  }

  url.setQuery( urlQuery );

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
  const auto constSourceLayers = sourceLayers();
  for ( const QgsVirtualLayerDefinition::SourceLayer &l : constSourceLayers )
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
  const auto constSourceLayers = sourceLayers();
  for ( const QgsVirtualLayerDefinition::SourceLayer &l : constSourceLayers )
  {
    if ( l.isReferenced() )
    {
      return true;
    }
  }
  return false;
}

QString QgsVirtualLayerDefinition::subsetString() const
{
  return mSubsetString;
}

void QgsVirtualLayerDefinition::setSubsetString( const QString &subsetString )
{
  mSubsetString = subsetString;
}
