/***************************************************************************
  qgsvectortilemvtdecoder.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <string>

#include "qgsvectortileloader.h"
#include "qgsvectortilemvtdecoder.h"

#include "qgsvectortilemvtutils.h"
#include "qgsvectortileutils.h"

#include "qgslogger.h"
#include "qgsmultipoint.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"

#include <QPointer>
#include <QNetworkRequest>


QgsVectorTileMVTDecoder::QgsVectorTileMVTDecoder( const QgsVectorTileMatrixSet &structure )
  : mStructure( structure )
{}

QgsVectorTileMVTDecoder::~QgsVectorTileMVTDecoder() = default;

bool QgsVectorTileMVTDecoder::decode( const QgsVectorTileRawData &rawTileData )
{
  mLayerNameToIndex.clear();

  QMap<QString, QByteArray>::const_iterator it = rawTileData.data.constBegin();
  for ( ; it != rawTileData.data.constEnd(); ++it )
  {
    QString sourceId = it.key();

    vector_tile::Tile tile;
    if ( !tile.ParseFromArray( it.value().constData(), it.value().count() ) )
      return false;

    for ( int layerNum = 0; layerNum < tile.layers_size(); layerNum++ )
    {
      const ::vector_tile::Tile_Layer &layer = tile.layers( layerNum );
      const QString layerName = layer.name().c_str();
      mLayerNameToIndex[sourceId][layerName] = layerNum;
    }

    tiles[sourceId] = tile;

  }

  mTileID = rawTileData.tileGeometryId;

  return true;
}

QStringList QgsVectorTileMVTDecoder::layers() const
{
  QStringList layerNames;
  const int layerSize = std::accumulate( tiles.constBegin(), tiles.constEnd(), 0, []( int count, const vector_tile::Tile & tile ) {return count + tile.layers_size();} );
  layerNames.reserve( layerSize );

  QMap<QString, vector_tile::Tile>::const_iterator it = tiles.constBegin();
  for ( ; it != tiles.constEnd(); ++it )
  {
    for ( int layerNum = 0; layerNum < layerSize; layerNum++ )
    {
      const ::vector_tile::Tile_Layer &layer = it.value().layers( layerNum );
      const QString layerName = layer.name().c_str();
      layerNames << layerName;
    }
  }
  return layerNames;
}

QStringList QgsVectorTileMVTDecoder::layerFieldNames( const QString &layerName ) const
{
  QMap<QString, vector_tile::Tile>::const_iterator it = tiles.constBegin();
  for ( ; it != tiles.constEnd(); ++it )
  {
    if ( !mLayerNameToIndex[it.key()].contains( layerName ) )
      continue;

    const ::vector_tile::Tile_Layer &layer = it.value().layers( mLayerNameToIndex[it.key()][layerName] );
    QStringList fieldNames;
    const int size = layer.keys_size();
    fieldNames.reserve( size );
    for ( int i = 0; i < size; ++i )
    {
      const QString fieldName = layer.keys( i ).c_str();
      fieldNames << fieldName;
    }
    return fieldNames;
  }
  return QStringList();
}

QgsVectorTileFeatures QgsVectorTileMVTDecoder::layerFeatures( const QMap<QString, QgsFields> &perLayerFields, const QgsCoordinateTransform &ct, const QSet<QString> *layerSubset ) const
{
  QgsVectorTileFeatures features;

  const int numTiles = static_cast<int>( pow( 2, mTileID.zoomLevel() ) ); // assuming we won't ever go over 30 zoom levels
  const QgsTileMatrix &rootMatrix = mStructure.rootMatrix();
  const double z0Width = rootMatrix.extent().width();
  const double z0Height = rootMatrix.extent().height();
  const double z0xMinimum = rootMatrix.extent().xMinimum();
  const double z0yMaximum = rootMatrix.extent().yMaximum();

  const double tileDX = z0Width / numTiles;
  const double tileDY = z0Height / numTiles;
  const double tileXMin = z0xMinimum + mTileID.column() * tileDX;
  const double tileYMax = z0yMaximum - mTileID.row() * tileDY;

  QMap<QString, vector_tile::Tile>::const_iterator it = tiles.constBegin();
  for ( ; it != tiles.constEnd(); ++it )
  {
    vector_tile::Tile tile = it.value();

    for ( int layerNum = 0; layerNum < tile.layers_size(); layerNum++ )
    {
      const ::vector_tile::Tile_Layer &layer = tile.layers( layerNum );

      const QString layerName = layer.name().c_str();
      if ( layerSubset && !layerSubset->contains( QString() ) && !layerSubset->contains( layerName ) )
        continue;

      QVector<QgsFeature> layerFeatures;
      QgsFields layerFields = perLayerFields[layerName];

      const auto allLayerFields = perLayerFields.find( QString() );
      if ( allLayerFields != perLayerFields.end() )
      {
        // need to add the fields from any "all layer" rules to every layer
        for ( const QgsField &field : allLayerFields.value() )
        {
          if ( layerFields.lookupField( field.name() ) == -1 )
          {
            layerFields.append( field );
          }
        }
      }

      // figure out how field indexes in MVT encoding map to field indexes in QgsFields (we may not use all available fields)
      QHash<int, int> tagKeyIndexToFieldIndex;
      for ( int i = 0; i < layer.keys_size(); ++i )
      {
        const int fieldIndex = layerFields.indexOf( layer.keys( i ).c_str() );
        if ( fieldIndex != -1 )
          tagKeyIndexToFieldIndex.insert( i, fieldIndex );
      }

      // go through features of a layer
      for ( int featureNum = 0; featureNum < layer.features_size(); featureNum++ )
      {
        const ::vector_tile::Tile_Feature &feature = layer.features( featureNum );

        QgsFeatureId fid;
        {
          // There is no assigned ID, but some parts of QGIS do not work correctly if all IDs are zero
          // (e.g. labeling will not register two features with the same FID within a single layer),
          // so let's generate some pseudo-unique FIDs to keep those bits happy
          fid = featureNum;
          fid |= ( layerNum & 0xff ) << 24;
          fid |= ( static_cast<QgsFeatureId>( mTileID.row() ) & 0xff ) << 32;
          fid |= ( static_cast<QgsFeatureId>( mTileID.column() ) & 0xff ) << 40;
        }

        QgsFeature f( layerFields, fid );

        //
        // parse attributes
        //

        for ( int tagNum = 0; tagNum + 1 < feature.tags_size(); tagNum += 2 )
        {
          const int keyIndex = static_cast<int>( feature.tags( tagNum ) );
          const int fieldIndex = tagKeyIndexToFieldIndex.value( keyIndex, -1 );
          if ( fieldIndex == -1 )
            continue;

          const int valueIndex = static_cast<int>( feature.tags( tagNum + 1 ) );
          if ( valueIndex >= layer.values_size() )
          {
            QgsDebugError( QStringLiteral( "Invalid value index for attribute" ) );
            continue;
          }
          const ::vector_tile::Tile_Value &value = layer.values( valueIndex );

          if ( value.has_string_value() )
            f.setAttribute( fieldIndex, QString::fromStdString( value.string_value() ) );
          else if ( value.has_float_value() )
            f.setAttribute( fieldIndex, static_cast<double>( value.float_value() ) );
          else if ( value.has_double_value() )
            f.setAttribute( fieldIndex, value.double_value() );
          else if ( value.has_int_value() )
            f.setAttribute( fieldIndex, static_cast<int>( value.int_value() ) );
          else if ( value.has_uint_value() )
            f.setAttribute( fieldIndex, static_cast<int>( value.uint_value() ) );
          else if ( value.has_sint_value() )
            f.setAttribute( fieldIndex, static_cast<int>( value.sint_value() ) );
          else if ( value.has_bool_value() )
            f.setAttribute( fieldIndex, static_cast<bool>( value.bool_value() ) );
          else
          {
            QgsDebugError( QStringLiteral( "Unexpected attribute value" ) );
          }
        }

        //
        // parse geometry
        //

        const int extent = static_cast<int>( layer.extent() );
        int cursorx = 0, cursory = 0;

        QVector<QgsPoint *> outputPoints; // for point/multi-point
        QVector<QgsLineString *> outputLinestrings;  // for linestring/multi-linestring
        QVector<QgsPolygon *> outputPolygons;
        QVector<QgsPoint> tmpPoints;

        for ( int i = 0; i < feature.geometry_size(); i ++ )
        {
          const unsigned g = feature.geometry( i );
          const unsigned cmdId = g & 0x7;
          const int cmdCount = static_cast<int>( g >> 3 );
          if ( cmdId == 1 ) // MoveTo
          {
            if ( i + static_cast<int>( cmdCount ) * 2 >= feature.geometry_size() )
            {
              QgsDebugError( QStringLiteral( "Malformed geometry: invalid cmdCount" ) );
              break;
            }

            if ( feature.type() == vector_tile::Tile_GeomType_POINT )
              outputPoints.reserve( static_cast<int>( outputPoints.size() ) + cmdCount );
            else
              tmpPoints.reserve( static_cast<int>( tmpPoints.size() ) + cmdCount );

            for ( int j = 0; j < cmdCount; j++ )
            {
              const int v = static_cast<int>( feature.geometry( i + 1 ) );
              const int w = static_cast<int>( feature.geometry( i + 2 ) );
              const int dx = ( ( v >> 1 ) ^ ( -( v & 1 ) ) );
              const int dy = ( ( w >> 1 ) ^ ( -( w & 1 ) ) );
              cursorx += dx;
              cursory += dy;
              const double px = tileXMin + tileDX * double( cursorx ) / double( extent );
              const double py = tileYMax - tileDY * double( cursory ) / double( extent );

              if ( feature.type() == vector_tile::Tile_GeomType_POINT )
              {
                outputPoints.append( new QgsPoint( px, py ) );
              }
              else if ( feature.type() == vector_tile::Tile_GeomType_LINESTRING )
              {
                if ( tmpPoints.size() > 0 )
                {
                  outputLinestrings.append( new QgsLineString( tmpPoints ) );
                  tmpPoints.clear();
                }
                tmpPoints.append( QgsPoint( px, py ) );
              }
              else if ( feature.type() == vector_tile::Tile_GeomType_POLYGON )
              {
                tmpPoints.append( QgsPoint( px, py ) );
              }
              i += 2;
            }
          }
          else if ( cmdId == 2 ) // LineTo
          {
            if ( i + static_cast<int>( cmdCount ) * 2 >= feature.geometry_size() )
            {
              QgsDebugError( QStringLiteral( "Malformed geometry: invalid cmdCount" ) );
              break;
            }
            tmpPoints.reserve( tmpPoints.size() + cmdCount );
            for ( int j = 0; j < cmdCount; j++ )
            {
              const int v = static_cast<int>( feature.geometry( i + 1 ) );
              const int w = static_cast<int>( feature.geometry( i + 2 ) );
              const int dx = ( v >> 1 ) ^ ( -( v & 1 ) );
              const int dy = ( w >> 1 ) ^ ( -( w & 1 ) );
              cursorx += dx;
              cursory += dy;
              const double px = tileXMin + tileDX * double( cursorx ) / double( extent );
              const double py = tileYMax - tileDY * double( cursory ) / double( extent );

              tmpPoints.push_back( QgsPoint( px, py ) );
              i += 2;
            }
          }
          else if ( cmdId == 7 ) // ClosePath
          {
            if ( feature.type() == vector_tile::Tile_GeomType_POLYGON )
            {
              tmpPoints.append( tmpPoints.first() );  // close the ring

              std::unique_ptr<QgsLineString> ring( new QgsLineString( tmpPoints ) );
              tmpPoints.clear();

              if ( QgsVectorTileMVTUtils::isExteriorRing( ring.get() ) )
              {
                // start a new polygon
                QgsPolygon *p = new QgsPolygon;
                p->setExteriorRing( ring.release() );
                outputPolygons.append( p );
              }
              else
              {
                // interior ring (hole)
                if ( outputPolygons.count() != 0 )
                {
                  outputPolygons[outputPolygons.count() - 1]->addInteriorRing( ring.release() );
                }
                else
                {
                  QgsDebugError( QStringLiteral( "Malformed geometry: first ring of a polygon is interior ring" ) );
                }
              }
            }

          }
          else
          {
            QgsDebugError( QStringLiteral( "Unexpected command ID: %1" ).arg( cmdId ) );
          }
        }

        QString geomType;
        if ( feature.type() == vector_tile::Tile_GeomType_POINT )
        {
          geomType = QStringLiteral( "Point" );
          if ( outputPoints.count() == 1 )
            f.setGeometry( QgsGeometry( outputPoints.at( 0 ) ) );
          else
          {
            QgsMultiPoint *mp = new QgsMultiPoint;
            mp->reserve( outputPoints.count() );
            for ( int k = 0; k < outputPoints.count(); ++k )
              mp->addGeometry( outputPoints[k] );
            f.setGeometry( QgsGeometry( mp ) );
          }
        }
        else if ( feature.type() == vector_tile::Tile_GeomType_LINESTRING )
        {
          geomType = QStringLiteral( "LineString" );

          // finish the linestring we have started
          outputLinestrings.append( new QgsLineString( tmpPoints ) );

          if ( outputLinestrings.count() == 1 )
            f.setGeometry( QgsGeometry( outputLinestrings.at( 0 ) ) );
          else
          {
            QgsMultiLineString *mls = new QgsMultiLineString;
            mls->reserve( outputLinestrings.size() );
            for ( int k = 0; k < outputLinestrings.count(); ++k )
              mls->addGeometry( outputLinestrings[k] );
            f.setGeometry( QgsGeometry( mls ) );
          }
        }
        else if ( feature.type() == vector_tile::Tile_GeomType_POLYGON )
        {
          geomType = QStringLiteral( "Polygon" );

          if ( outputPolygons.count() == 1 )
            f.setGeometry( QgsGeometry( outputPolygons.at( 0 ) ) );
          else
          {
            QgsMultiPolygon *mpl = new QgsMultiPolygon;
            mpl->reserve( outputPolygons.size() );
            for ( int k = 0; k < outputPolygons.count(); ++k )
              mpl->addGeometry( outputPolygons[k] );
            f.setGeometry( QgsGeometry( mpl ) );
          }
        }

        f.setAttribute( QStringLiteral( "_geom_type" ), geomType );
        f.geometry().transform( ct );

        layerFeatures.append( f );
      }

      features[layerName] = layerFeatures;
    }
  }
  return features;
}
