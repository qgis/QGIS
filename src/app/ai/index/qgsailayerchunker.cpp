/***************************************************************************
    qgsailayerchunker.cpp
    ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsailayerchunker.h"

#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"

#include <QByteArray>
#include <QStringList>

using namespace Qt::StringLiterals;

namespace
{
  QString serializeFeatureLine( const QgsFeature &feature, const QgsFields &fields, const QString &geometryType )
  {
    QStringList kv;
    kv.reserve( fields.size() );
    for ( const QgsField &f : fields )
    {
      const QVariant val = feature.attribute( f.name() );
      const QString rendered = val.isValid() && !val.isNull() ? val.toString() : QString();
      kv << u"%1=%2"_s.arg( f.name(), rendered );
    }

    QString bbox = u"NULL"_s;
    const QgsGeometry geom = feature.geometry();
    if ( !geom.isNull() )
    {
      const QgsRectangle r = geom.boundingBox();
      bbox = u"(%1,%2,%3,%4)"_s
               .arg( r.xMinimum() )
               .arg( r.yMinimum() )
               .arg( r.xMaximum() )
               .arg( r.yMaximum() );
    }

    return u"[%1] %2; bbox=%3; type=%4"_s.arg( feature.id() ).arg( kv.join( "; "_L1 ), bbox, geometryType );
  }

  QString rasterMetadataText( QgsRasterLayer *layer )
  {
    QString out;
    out += u"Raster layer '%1' (id=%2)\n"_s.arg( layer->name(), layer->id() );
    out += u"crs=%1; size=%2x%3; bands=%4\n"_s.arg( layer->crs().authid() ).arg( layer->width() ).arg( layer->height() ).arg( layer->bandCount() );

    const QgsRectangle ext = layer->extent();
    out += u"extent=(%1,%2,%3,%4)\n"_s.arg( ext.xMinimum() ).arg( ext.yMinimum() ).arg( ext.xMaximum() ).arg( ext.yMaximum() );

    QgsRasterDataProvider *dp = layer->dataProvider();
    if ( !dp )
      return out + "(no data provider)\n"_L1;

    for ( int b = 1; b <= layer->bandCount(); ++b )
    {
      const QgsRasterBandStats stats = dp->bandStatistics( b );
      out += u"band %1: min=%2 max=%3 mean=%4 stddev=%5\n"_s
               .arg( b )
               .arg( stats.minimumValue )
               .arg( stats.maximumValue )
               .arg( stats.mean )
               .arg( stats.stdDev );
    }
    return out;
  }
} // namespace

QList<QgsAiWorkspaceIndex::Chunk> QgsAiLayerChunker::chunkVector( QgsVectorLayer *layer )
{
  QList<QgsAiWorkspaceIndex::Chunk> chunks;
  if ( !layer )
    return chunks;

  const QgsFields fields = layer->fields();
  const QString geometryType = QgsWkbTypes::geometryDisplayString( layer->geometryType() );
  const QString header = u"Vector layer '%1' (id=%2, crs=%3, geometry=%4, fields=%5)\n"_s
                           .arg( layer->name(), layer->id(), layer->crs().authid(), geometryType )
                           .arg( fields.size() );

  QString currentText = header;
  QByteArray currentWkts;
  qint64 firstFid = -1;
  qint64 lastFid = -1;
  int chunkIndex = 0;

  auto flush = [&]() {
    if ( firstFid < 0 )
      return;
    QgsAiWorkspaceIndex::Chunk c;
    c.sourceType = QString::fromLatin1( QgsAiWorkspaceIndex::SOURCE_TYPE_LAYER );
    c.relativePath = layer->name();
    c.layerId = layer->id();
    c.firstFeatureId = firstFid;
    c.lastFeatureId = lastFid;
    c.chunkIndex = chunkIndex++;
    c.text = currentText;
    if ( !currentWkts.isEmpty() )
      c.wktBlob = qCompress( currentWkts );
    chunks.append( c );

    currentText = header;
    currentWkts.clear();
    firstFid = -1;
    lastFid = -1;
  };

  QgsFeatureIterator it = layer->getFeatures();
  QgsFeature feature;
  while ( it.nextFeature( feature ) )
  {
    const QString line = serializeFeatureLine( feature, fields, geometryType );
    const QgsGeometry geom = feature.geometry();
    const QString wkt = geom.isNull() ? u"NULL"_s : geom.asWkt();

    // Flush before adding if appending would exceed the target *and* the chunk
    // already has at least one feature (avoid empty/header-only chunks).
    if ( firstFid >= 0 && currentText.size() + line.size() + 1 > QgsAiWorkspaceIndex::CHUNK_TARGET_CHARS )
      flush();

    if ( firstFid < 0 )
      firstFid = feature.id();
    lastFid = feature.id();

    currentText += line;
    currentText += '\n';

    if ( !currentWkts.isEmpty() )
      currentWkts.append( '\n' );
    currentWkts.append( wkt.toUtf8() );
  }
  flush();

  return chunks;
}

QList<QgsAiWorkspaceIndex::Chunk> QgsAiLayerChunker::chunkRaster( QgsRasterLayer *layer )
{
  QList<QgsAiWorkspaceIndex::Chunk> chunks;
  if ( !layer )
    return chunks;

  QgsAiWorkspaceIndex::Chunk c;
  c.sourceType = QString::fromLatin1( QgsAiWorkspaceIndex::SOURCE_TYPE_LAYER );
  c.relativePath = layer->name();
  c.layerId = layer->id();
  c.chunkIndex = 0;
  c.text = rasterMetadataText( layer );
  chunks.append( c );
  return chunks;
}
