/***************************************************************************
                         qgsalgorithmshortestline.cpp
                         ---------------------
    begin                : September 2021
    copyright            : (C) 2020 by Matteo Ghetta, Clemens Raffler
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

//Disclaimer:This feature was originally developed in Python by: Matteo Ghetta, 2021

#ifndef QGSALGORITHMSHORTESTLINE_H
#define QGSALGORITHMSHORTESTLINE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native Shortest Line between layers algorithm.
 */
class QgsShortestLineAlgorithm : public QgsProcessingAlgorithm
{

  public:
    QgsShortestLineAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsShortestLineAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr< QgsFeatureSource > mSource;
    std::unique_ptr< QgsFeatureSource > mDestination;
    int mMethod = 0;
    long long mKNeighbors = 1;
    double mMaxDistance = 0.0;


};

///@endcond PRIVATE

#endif // QGSALGORITHMSHORTESTLINE_H
