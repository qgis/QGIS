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

///@cond PRIVATE

class QgsDifferenceAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsDifferenceAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmDifference.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmDifference.svg" ) ); }
    virtual QString name() const override;
    virtual QString displayName() const override;
    virtual QString group() const override;
    virtual QString groupId() const override;
    QString shortHelpString() const override;

  protected:
    virtual QgsProcessingAlgorithm *createInstance() const override;
    virtual void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    virtual QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMDIFFERENCE_H
