/***************************************************************************
                         qgsalgorithmdrape.h
                         -------------------------
    begin                : July 2018
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMDRAPE_H
#define QGSALGORITHMDRAPE_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native drape algorithm.
 */
class QgsDrapeAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsDrapeAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QgsDrapeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    std::unique_ptr< QgsRasterDataProvider > mRasterDataProvider;
    int mRasterBand;
    QgsRectangle mRasterExtent;
    QgsCoordinateReferenceSystem mRasterCrs;
    QgsCoordinateTransformContext mTransformContext;
};

///@endcond PRIVATE

#endif // QGSALGORITHMDRAPE_H


