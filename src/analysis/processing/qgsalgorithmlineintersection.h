/***************************************************************************
                         qgsalgorithmlineintersection.h
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

#ifndef QGSALGORITHMLINEINTERSECTION_H
#define QGSALGORITHMLINEINTERSECTION_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native line intersection algorithm.
 */
class QgsLineIntersectionAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsLineIntersectionAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmLineIntersections.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmLineIntersections.svg"_s ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    QgsLineIntersectionAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMLINEINTERSECTION_H
