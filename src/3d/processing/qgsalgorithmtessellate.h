/***************************************************************************
                         qgsalgorithmtessellate.h
                         ---------------------
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

#ifndef QGSALGORITHMTESSELLATE_H
#define QGSALGORITHMTESSELLATE_H

#define SIP_NO_FILE

#include "qgis.h"
#include "processing/qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native tessellate algorithm.
 */
class QgsTessellateAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsTessellateAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    Flags flags() const override;
    QStringList tags() const override;
    QString group() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsTessellateAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override;
    QgsProcessing::SourceType outputLayerType() const override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;

    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};


///@endcond PRIVATE

#endif // QGSALGORITHMTESSELLATE_H


