/***************************************************************************
                         qgsalgorithmchannelnetwork.h
                         ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMCHANNELNETWORK_H
#define QGSALGORITHMCHANNELNETWORK_H


#include "qgis_sip.h"
#include "qgsalgorithmd8base.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * \brief Common base class for channel network processing algorithms.
 */
class QgsChannelNetworkAlgorithmBase : public QgsD8AnalysisAlgorithmBase
{
  public:
    QgsChannelNetworkAlgorithmBase() = default;

    QStringList tags() const override;

  protected:
    void addCommonParameters();

    enum class NodeType
    {
      Spring = 1,
      Junction = 2,
      Outlet = 3,
      Mouth = 4,
    };

    void extractChannelNetwork(
      const QgsRasterBlock *demBlock,
      const std::vector<int8_t> &d8Directions,
      const std::vector<int16_t> &strahlerOrders,
      int width,
      int height,
      const QgsRectangle &extent,
      const QgsCoordinateReferenceSystem &,
      int threshold,
      bool subbasins,
      QgsFeatureSink *channelSink,
      QgsFeatureSink *basinSink,
      QgsFeatureSink *junctionSink,
      QgsProcessingFeedback *feedback,
      const QVariantMap &parameters
    );

    static QgsFields channelFields();
    static QgsFields basinFields();
    static QgsFields junctionFields();

    std::unique_ptr<QgsRasterInterface> mDemInterface;
    int mLayerWidth = 0;
    int mLayerHeight = 0;
    double mCellSizeX = 0.0;
    double mCellSizeY = 0.0;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
};

/**
 * \brief Native processing algorithm for calculating channel networks, drainage basins and junction nodes directly from a DEM raster.
 */
class QgsChannelNetworkFromDemAlgorithm : public QgsChannelNetworkAlgorithmBase
{
  public:
    QgsChannelNetworkFromDemAlgorithm() = default;

    QString name() const override;
    QString displayName() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsProcessingAlgorithm *createInstance() const override;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

/**
 * \brief Native processing algorithm for calculating channel networks, drainage basins and junction nodes directly from a DEM, flow direction, and Strahler order raster.
 */
class QgsChannelNetworkFromFlowDirAndOrderAlgorithm : public QgsChannelNetworkAlgorithmBase
{
  public:
    QgsChannelNetworkFromFlowDirAndOrderAlgorithm() = default;

    QString name() const override;
    QString displayName() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsProcessingAlgorithm *createInstance() const override;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsRasterInterface> mFlowDirInterface;
    std::unique_ptr<QgsRasterInterface> mStrahlerInterface;
};

///@endcond PRIVATE

#endif // QGSALGORITHMCHANNELNETWORK_H
