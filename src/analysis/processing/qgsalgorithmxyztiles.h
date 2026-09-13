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


#include "qgis_sip.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmbtiles.h"
#include "qgsprocessingalgorithm.h"
#include "qobjectuniqueptr.h"

#define SIP_NO_FILE

///@cond PRIVATE

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
    MetaTile() {}

    void addTile( const int row, const int col, Tile tileToAdd );

    QgsRectangle extent() const;

    QMap<QPair<int, int>, Tile> tiles;
    int rows = 0;
    int cols = 0;
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

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    void checkLayersUsagePolicy( QgsProcessingFeedback *feedback );

    void startJobs();
    virtual void processMetaTile( QgsMapRendererSequentialJob *job ) = 0;

    std::optional<QgsMapSettings> mapSettingsForTile( const MetaTile &metaTile ) const;

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
    bool mSkipEmptyTiles = false;
    QgsExpressionContext mExpressionContext;
    QString mTileFormat;
    QList<QgsMapLayer *> mLayers;
    QgsRectangle mWgs84Extent;
    QObjectUniquePtr<QObject> mJobOwner = nullptr;
    QgsProcessingFeedback *mFeedback = nullptr;
    long long mTotalMetaTiles = 0;
    long long mProcessedMetaTiles = 0;
    long long mTilesWritten = 0;
    long long mEmptyTiles = 0;
    QgsCoordinateTransformContext mTransformContext;
    QString mEllipsoid;
    QPointer<QEventLoop> mEventLoop;
    QList<MetaTile> mMetaTiles;
    QMap<QgsMapRendererSequentialJob *, MetaTile> mRendererJobs;
    Qgis::ScaleCalculationMethod mScaleMethod = Qgis::ScaleCalculationMethod::HorizontalMiddle;
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
