/***************************************************************************
                         qgsalgorithmpackage.h
                         ------------------
    begin                : November 2017
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

#ifndef QGSALGORITHMPACKAGE_H
#define QGSALGORITHMPACKAGE_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

class QgsVectorLayer;

/**
 * Native package layers algorithm.
 */
class QgsPackageAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsPackageAlgorithm() = default;
    Flags flags() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsPackageAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    bool packageVectorLayer( QgsVectorLayer *layer, const QString &path, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

};

///@endcond PRIVATE

#endif // QGSALGORITHMPACKAGE_H


