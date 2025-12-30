/***************************************************************************
                         qgsalgorithmmultidifference.cpp
                         ------------------
    begin                : December 2021
    copyright            : (C) 2021 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMMULTIDIFFERENCE_H
#define QGSALGORITHMMULTIDIFFERENCE_H

#define SIP_NO_FILE

#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

class QgsMultiDifferenceAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsMultiDifferenceAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmDifference.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmDifference.svg"_s ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMMULTIDIFFERENCE_H
