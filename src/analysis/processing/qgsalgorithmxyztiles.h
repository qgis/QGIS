/***************************************************************************
                         qgsalgorithmxyztiles.h
                         ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMXYZTILES_H
#define QGSALGORITHMXYZTILES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#include "qgsmaprenderersequentialjob.h"
#include "qgsmbtiles.h"

///@cond PRIVATE

int tile2tms( const int y, const int zoom );
int lon2tileX( const double lon, const int z );
int lat2tileY( const double lat, const int z );
double tileX2lon( const int x, const int z );
double tileY2lat( const int y, const int z );
void extent2TileXY( const QgsRectangle extent, const int zoom, int &xMin, int &yMin, int &xMax, int &yMax );

struct Tile
{
    Tile( const int x, const int y, const int z )
      : x( x )
      , y( y )
      , z( z )
    {}

    int x;
    int y;
    int z;
};

struct MetaTile
{
    MetaTile()
      : rows( 0 )
      , cols( 0 )
    {}

    void addTile( const int row, const int col, Tile tileToAdd )
    {
      tiles.insert( QPair<int, int>( row, col ), tileToAdd );
      if ( row >= rows )
      {
        rows = row + 1;
      }
      if ( col >= cols )
      {
        cols = col + 1;
      }
    }

    QgsRectangle extent()
    {
      const Tile first = tiles.first();
      const Tile last = tiles.last();
      return QgsRectangle( tileX2lon( first.x, first.z ), tileY2lat( last.y + 1, last.z ), tileX2lon( last.x + 1, last.z ), tileY2lat( first.y, first.z ) );
    }

    QMap<QPair<int, int>, Tile> tiles;
    int rows;
    int cols;
};
QList<MetaTile> getMetatiles( const QgsRectangle extent, const int zoom, const int tileSize = 4 );


/**
 * Base class for native XYZ tiles algorithms.
 */
class QgsXyzTilesBaseAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QString group() const override;
    QString groupId() const override;
    Qgis::ProcessingAlgorithmFlags flags() const override;

  protected:
    /**
     * Creates common parameters used in all algorithms
     */
    void createCommonParameters();

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    void checkLayersUsagePolicy( QgsProcessingFeedback *feedback );

    void startJobs();
    virtual void processMetaTile( QgsMapRendererSequentialJob *job ) = 0;

    QgsRectangle mExtent;
    QColor mBackgroundColor;
    int mMinZoom = 12;
    int mMaxZoom = 12;
    int mDpi = 96;
    bool mAntialias = true;
    int mJpgQuality = 75;
    int mMetaTileSize = 4;
    int mThreadsNumber = 1;
    int mTileWidth = 256;
    int mTileHeight = 256;
    QString mTileFormat;
    QList<QgsMapLayer *> mLayers;
    QgsCoordinateReferenceSystem mWgs84Crs;
    QgsCoordinateReferenceSystem mMercatorCrs;
    QgsCoordinateTransform mSrc2Wgs;
    QgsCoordinateTransform mWgs2Mercator;
    QgsRectangle mWgs84Extent;
    QgsProcessingFeedback *mFeedback = nullptr;
    long long mTotalTiles = 0;
    long long mProcessedTiles = 0;
    QgsCoordinateTransformContext mTransformContext;
    QPointer<QEventLoop> mEventLoop;
    QList<MetaTile> mMetaTiles;
    QMap<QgsMapRendererSequentialJob *, MetaTile> mRendererJobs;
};


/**
 * Native XYZ tiles (directory) algorithm.
 */
class QgsXyzTilesDirectoryAlgorithm : public QgsXyzTilesBaseAlgorithm
{
  public:
    QgsXyzTilesDirectoryAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsXyzTilesDirectoryAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    void processMetaTile( QgsMapRendererSequentialJob *job ) override;

  private:
    bool mTms = false;
    QString mOutputDir;
};

/**
 * Native XYZ tiles (MBTiles) algorithm.
 */
class QgsXyzTilesMbtilesAlgorithm : public QgsXyzTilesBaseAlgorithm
{
  public:
    QgsXyzTilesMbtilesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsXyzTilesMbtilesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    void processMetaTile( QgsMapRendererSequentialJob *job ) override;

  private:
    std::unique_ptr<QgsMbTiles> mMbtilesWriter;
};

///@endcond PRIVATE

#endif // QGSALGORITHMXYZTILES_H
