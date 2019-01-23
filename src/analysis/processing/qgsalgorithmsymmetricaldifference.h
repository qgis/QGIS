/***************************************************************************
  qgsalgorithmsymmetricaldifference.h
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

#ifndef QGSALGORITHMSYMMETRICALDIFFERENCE_H
#define QGSALGORITHMSYMMETRICALDIFFERENCE_H

#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

class QgsSymmetricalDifferenceAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsSymmetricalDifferenceAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmSymmetricalDifference.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmSymmetricalDifference.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMSYMMETRICALDIFFERENCE_H
