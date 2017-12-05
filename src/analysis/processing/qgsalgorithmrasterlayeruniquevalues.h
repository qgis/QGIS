/***************************************************************************
                         qgsalgorithmrasterlayeruniquevalues.h
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSALGORITHMRASTERLAYERUNIQUEVALUES_H
#define QGSALGORITHMRASTERLAYERUNIQUEVALUES_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native raster layer unique values report algorithm.
 */
class QgsRasterLayerUniqueValuesReportAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsRasterLayerUniqueValuesReportAlgorithm() = default;
    Flags flags() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsRasterLayerUniqueValuesReportAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    std::unique_ptr< QgsRasterInterface > mInterface;
    bool mHasNoDataValue = false;
    int mLayerWidth;
    int mLayerHeight;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX;
    double mRasterUnitsPerPixelY;
    QString mSource;

};

///@endcond PRIVATE

#endif // QGSALGORITHMRASTERLAYERUNIQUEVALUES_H


