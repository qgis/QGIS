/***************************************************************************
    qgsglobetilesource.h
    ---------------------
    begin                : August 2010
    copyright            : (C) 2010 by Pirmin Kalberer
                           (C) 2015 Sandro Mani
    email                : pka at sourcepole dot ch
                           smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGLOBETILESOURCE_H
#define QGSGLOBETILESOURCE_H

#include <osgEarth/TileSource>
#include <osgEarth/Version>
#include <osg/ImageStream>
#include <QImage>
#include <QStringList>
#include <QLabel>
#include <QMutex>
#include "qgsrectangle.h"

//#define GLOBE_SHOW_TILE_STATS

class QgsCoordinateTransform;
class QgsMapCanvas;
class QgsMapLayer;
class QgsMapRenderer;
class QgsMapSettings;
class QgsGlobeTileSource;
class QgsMapRendererParallelJob;

class QgsGlobeTileStatistics : public QObject
{
    Q_OBJECT
  public:
    QgsGlobeTileStatistics();
    ~QgsGlobeTileStatistics() { s_instance = 0; }
    static QgsGlobeTileStatistics *instance() { return s_instance; }
    void updateTileCount( int change );
    void updateQueueTileCount( int change );
  signals:
    void changed( int queued, int tot );
  private:
    static QgsGlobeTileStatistics *s_instance;
    QMutex mMutex;
    int mTileCount;
    int mQueueTileCount;
};

int getTileCount();

class QgsGlobeTileImage : public osg::Image
{
  public:
    QgsGlobeTileImage( QgsGlobeTileSource *tileSource, const QgsRectangle &tileExtent, int tileSize, int tileLod );
    ~QgsGlobeTileImage();
    bool requiresUpdateCall() const { return !mUpdatedImage.isNull(); }
    QgsMapSettings createSettings( int dpi, const QList<QgsMapLayer *> &layers ) const;
    void setUpdatedImage( const QImage &image ) { mUpdatedImage = image; }
    int dpi() const { return mDpi; }
    const QgsRectangle &extent() { return mTileExtent; }

    void update( osg::NodeVisitor * );

    static bool lodSort( const QgsGlobeTileImage *lhs, const QgsGlobeTileImage *rhs ) { return lhs->mLod > rhs->mLod; }

  private:
    osg::ref_ptr<QgsGlobeTileSource> mTileSource;
    QgsRectangle mTileExtent;
    int mTileSize;
    unsigned char *mTileData;
    int mLod;
    int mDpi;
    QImage mUpdatedImage;
};

class QgsGlobeTileUpdateManager : public QObject
{
    Q_OBJECT
  public:
    QgsGlobeTileUpdateManager( QObject *parent = nullptr );
    ~QgsGlobeTileUpdateManager();
    void updateLayerSet( const QList<QgsMapLayer *> &layers ) { mLayers = layers; }
    void addTile( QgsGlobeTileImage *tile );
    void removeTile( QgsGlobeTileImage *tile );
    void waitForFinished() const;

  signals:
    void startRendering();
    void cancelRendering();

  private:
    QList<QgsMapLayer *> mLayers;
    QList<QgsGlobeTileImage *> mTileQueue;
    QgsGlobeTileImage *mCurrentTile = nullptr;
    QgsMapRendererParallelJob *mRenderer = nullptr;

  private slots:
    void start();
    void cancel();
    void renderingFinished();
};

class QgsGlobeTileSource : public osgEarth::TileSource
{
  public:
    QgsGlobeTileSource( QgsMapCanvas *canvas, const osgEarth::TileSourceOptions &options = osgEarth::TileSourceOptions() );
#if OSGEARTH_VERSION_GREATER_OR_EQUAL(2, 8, 0)
    osgEarth::Status initialize( const osgDB::Options *dbOptions ) override;
#else
    Status initialize( const osgDB::Options *dbOptions ) override;
#endif
    osg::Image *createImage( const osgEarth::TileKey &key, osgEarth::ProgressCallback *progress ) override;
    osg::HeightField *createHeightField( const osgEarth::TileKey &/*key*/, osgEarth::ProgressCallback * /*progress*/ ) override { return 0; }

    bool isDynamic() const override { return true; }
    osgEarth::CachePolicy getCachePolicyHint( const osgEarth::Profile * /*profile*/ ) const override { return osgEarth::CachePolicy::NO_CACHE; }

    void refresh( const QgsRectangle &dirtyExtent );
    void setLayers( const QList<QgsMapLayer *> &layers ) { mLayers = layers; }
    const QList<QgsMapLayer *> &layers() const { return mLayers; }

    void waitForFinished() const
    {
      mTileUpdateManager.waitForFinished();
    }

  private:
    friend class QgsGlobeTileImage;

    QMutex mTileListLock;
    QList<QgsGlobeTileImage *> mTiles;
    QgsMapCanvas *mCanvas = nullptr;
    QList<QgsMapLayer *> mLayers;
    QgsGlobeTileUpdateManager mTileUpdateManager;

    void addTile( QgsGlobeTileImage *tile );
    void removeTile( QgsGlobeTileImage *tile );
};

#endif // QGSGLOBETILESOURCE_H
