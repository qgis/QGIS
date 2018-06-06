/***************************************************************************
  qgsalgorithmsnapgeometries.h
  ---------------------
  Date                 : May 2018
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

#ifndef QGSALGORITHMSNAPGEOMETRIES_H
#define QGSALGORITHMSNAPGEOMETRIES_H

#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

class QgsSnapGeometriesAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    static void run( const QgsFeatureSource &source, QgsFeatureSink &sink, double thresh, QgsProcessingFeedback *feedback );
};

///@endcond PRIVATE

#endif // QGSALGORITHMSNAPGEOMETRIES_H
