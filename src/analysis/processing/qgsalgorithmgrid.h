/***************************************************************************
                         qgsalgorithmgrid.h
                         ---------------------
    begin                : August 2019
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


//Disclaimer:This feature was developed by: Michael Minn, 201

#ifndef QGSALGORITHMGRID_H
#define QGSALGORITHMGRID_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Native Grid creation algorithm.
 */
class QgsGridAlgorithm : public QgsProcessingAlgorithm
{
  public:

    QgsGridAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmCreateGrid.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmCreateGrid.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsGridAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context,
                                  QgsProcessingFeedback *feedback ) override;


  private:
    int mIdx;
    QgsRectangle mGridExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mHSpacing;
    double mVSpacing;
    double mHOverlay;
    double mVOverlay;

    //define grid creation methods
    void createPointGrid( std::unique_ptr< QgsFeatureSink > &sink, QgsProcessingFeedback *feedback );
    void createLineGrid( std::unique_ptr< QgsFeatureSink > &sink, QgsProcessingFeedback *feedback );
    void createRectangleGrid( std::unique_ptr< QgsFeatureSink > &sink, QgsProcessingFeedback *feedback );
    void createDiamondGrid( std::unique_ptr< QgsFeatureSink> &sink, QgsProcessingFeedback *feedback );
    void createHexagonGrid( std::unique_ptr< QgsFeatureSink> &sink, QgsProcessingFeedback *feedback );
};


///@endcond PRIVATE

#endif // QGISALGORITHMGRID_H
