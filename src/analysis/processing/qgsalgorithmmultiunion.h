/***************************************************************************
                         qgsalgorithmmultiunion.h
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

#ifndef QGSALGORITHMMULTIUNION_H
#define QGSALGORITHMMULTIUNION_H

#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

class QgsMultiUnionAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsMultiUnionAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmUnion.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmUnion.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    bool makeUnion( const QgsFeatureSource &sourceA, const QgsFeatureSource &sourceB, QgsFeatureSink &sink, QgsProcessingContext &context, QgsProcessingFeedback *feedback );
};

///@endcond PRIVATE

#endif // QGSALGORITHMMULTIUNION_H
