/***************************************************************************
                         qgsalgorithmrandompointsextent.h
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


//Disclaimer: The algorithm optimizes the original Random points in extent algorithm, (C) Alexander Bruy, 2014

#ifndef QGSALGORITHMRANDOMPOINTSEXTENT_H
#define QGSALGORITHMRANDOMPOINTSEXTENT_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Native random points in extent creation algorithm.
 */
class QgsRandomPointsExtentAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsRandomPointsExtentAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmRandomPointsWithinExtent.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmRandomPointsWithinExtent.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsRandomPointsExtentAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;


  private:
    QgsRectangle mExtent;
    int mNumPoints = 0;
    double mDistance = 0;
    int mMaxAttempts = 0;
    QgsCoordinateReferenceSystem mCrs;
};


///@endcond PRIVATE

#endif // QGSALGORITHMRANDOMPOINTSEXTENT_H
