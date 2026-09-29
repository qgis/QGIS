/***************************************************************************
                         qgsalgorithmupslopearea.h
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

#ifndef QGSALGORITHMUPSLOPEAREA_H
#define QGSALGORITHMUPSLOPEAREA_H


#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * \ingroup processing
 * \brief Base class for Upslope Area hydrologic flow routing processing algorithms.
 *
 * Based on the SAGA "Upslope Area" tool.
 */
class QgsUpslopeAreaAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    QgsUpslopeAreaAlgorithmBase() = default;

    QString group() const override;
    QString groupId() const override;
    QStringList tags() const override;
    QList<QgsAcademicReference> academicReferences() const override;
    QList<QgsProcessingAlgorithm::ExternalLink> externalLinks() const override;

  protected:
    enum class Method : int
    {
      D8 = 0,    //!< Deterministic 8
      DInf = 1,  //!< Deterministic Infinity
      MFD = 2,   //!< Multiple Flow Direction
      MDInf = 3, //!< Multiple Triangular Flow Direction
      MMDGFD = 4 //!< Multiple Maximum Downslope Gradient Based Flow Direction
    };

    void addCommonParameters();
    bool prepareBase( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    // Core flow accumulation engine
    bool calculateUpslopeArea( const std::vector<QgsPointXY> &targetPoints, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    void computeCellValue( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY, Method method, double converge, bool contour );
    void computeD8( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY );
    void computeDInf( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY );
    void computeMFD( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY, double converge, bool contour );
    void computeMMDGFD( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY, bool contour );
    void computeMDInf( const QgsRasterBlock *demBlock, int col, int row, int cols, int rows, double cellSizeX, double cellSizeY, double converge );

    std::unique_ptr<QgsRasterInterface> mDemProvider;
    QgsCoordinateReferenceSystem mDemCrs;
    std::unique_ptr<QgsRasterInterface> mRouteProvider;

    int mCols = 0;
    int mRows = 0;
    double mCellSizeX = 0.0;
    double mCellSizeY = 0.0;
    double mDemNoData = -9999.0;
    double mRouteNoData = -9999.0;
    QgsRectangle mExtent;

    std::vector<double> mRouteData;
    std::vector<double> mFlowData;

    Method mMethod = Method::MFD;
    double mConvergence = 1.1;
    bool mMfdContour = false;
    QString mOutputPath;
    QString mOutputFormat;
    QString mCreationOptions;
    double mOutputNoData = -9999;
};

/**
 * \ingroup processing
 * \brief Upslope area algorithm targeting a single point coordinate.
 */
class QgsUpslopeAreaPointAlgorithm : public QgsUpslopeAreaAlgorithmBase
{
  public:
    QgsUpslopeAreaPointAlgorithm();

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
 * \ingroup processing
 * \brief Upslope area algorithm targeting points from an input vector layer.
 */
class QgsUpslopeAreaLayerAlgorithm : public QgsUpslopeAreaAlgorithmBase
{
  public:
    QgsUpslopeAreaLayerAlgorithm();

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

///@endcond PRIVATE

#endif // QGSALGORITHMUPSLOPEAREA_H
