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

#include <atomic>
#include <memory>

#include "qgis_sip.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgpkgtiles.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmbtiles.h"
#include "qgsprocessingalgorithm.h"
#include "qgsrectangle.h"
#include "qgstiles.h"
#include "qobjectuniqueptr.h"

#include <QString>
#include <QThreadPool>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;


///@cond PRIVATE

template<typename T> class PendingTilesToWriteQueue;

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
    MetaTile() = default;

    void addTile( const int row, const int col, Tile tileToAdd, const QgsRectangle &extent );

    QMap<QPair<int, int>, Tile> tiles;
    int rows = 0;
    int cols = 0;
    QgsRectangle mExtent;
};

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

    /**
     * Creates tile matrix and CRS parameters (target CRS, zoom 0 extent, zoom 0 matrix width/height)
     */
    void createTileMatrixParameters();

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    void checkLayersUsagePolicy( QgsProcessingFeedback *feedback );

    void startJobs( QgsProcessingFeedback *feedback );
    void checkPipelineFinished( QgsProcessingFeedback *feedback );
    virtual void processMetaTile( const MetaTile &metaTile, const QImage &renderedImg, QgsProcessingFeedback *feedback ) = 0;

    std::optional<QgsMapSettings> mapSettingsForTile( const MetaTile &metaTile ) const;

    static constexpr double MERC_MAX = 20037508.342789244;

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
    bool mSkipEmptyTiles = false;
    QgsExpressionContext mExpressionContext;
    QString mTileFormat;
    QList<QgsMapLayer *> mLayers;
    QgsRectangle mWgs84Extent;
    QObjectUniquePtr<QObject> mJobOwner = nullptr;

    QgsCoordinateReferenceSystem mTargetCrs = QgsCoordinateReferenceSystem( u"EPSG:3857"_s );
    QgsTileMatrix mZ0matrix;
    QgsRectangle mTileGenerationRegion;

    long long mTotalMetaTiles = 0;
    std::atomic<long long> mProcessedMetaTiles { 0 };
    std::atomic<long long> mTilesWritten { 0 };
    std::atomic<long long> mEmptyTiles { 0 };
    std::atomic<int> mActivePostProcessingTasks { 0 };
    QgsCoordinateTransformContext mTransformContext;
    QString mEllipsoid;
    QPointer<QEventLoop> mEventLoop;
    QList<MetaTile> mMetaTiles;
    QMap<QgsMapRendererSequentialJob *, MetaTile> mRendererJobs;
    Qgis::ScaleCalculationMethod mScaleMethod = Qgis::ScaleCalculationMethod::HorizontalMiddle;
    std::unique_ptr<QThreadPool> mPostProcessingPool;
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
    QString shortDescription() const override;
    QgsXyzTilesDirectoryAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    void processMetaTile( const MetaTile &metaTile, const QImage &renderedImg, QgsProcessingFeedback *feedback ) override;

  private:
    bool mTms = false;
    QString mOutputDir;
    void doExport( QgsProcessingFeedback *feedback );
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
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsXyzTilesMbtilesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void processMetaTile( const MetaTile &metaTile, const QImage &renderedImg, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsMbTiles> mMbtilesWriter;
    PendingTilesToWriteQueue<QgsMbTiles::TileData> *mWriteQueue = nullptr;
    void doExport( QgsProcessingFeedback *feedback );
};

/**
 * Native GeoPackage raster tiles algorithm.
 */
class QgsXyzTilesGpkgAlgorithm : public QgsXyzTilesBaseAlgorithm
{
  public:
    QgsXyzTilesGpkgAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsXyzTilesGpkgAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    void processMetaTile( const MetaTile &metaTile, const QImage &renderedImg, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsGeoPackageTiles> mGpkgWriter;
    PendingTilesToWriteQueue<QgsGeoPackageTiles::TileData> *mWriteQueue = nullptr;
    void doExport( QgsProcessingFeedback *feedback );
};

///@endcond PRIVATE

#endif // QGSALGORITHMXYZTILES_H
