/***************************************************************************
  qgsalgorithmdifference.h
  ---------------------
  Date                 : April 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMDIFFERENCE_H
#define QGSALGORITHMDIFFERENCE_H

#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

class QgsDifferenceAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsDifferenceAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmDifference.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmDifference.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;
    QgsProcessingAlgorithm::Flags flags() const override;
  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMDIFFERENCE_H
