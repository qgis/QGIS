/***************************************************************************
                         qgsalgorithmcategorizeusingstyle.h
                         ---------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#ifndef QGSALGORITHMCATEGORIZEUSINGSTYLE_H
#define QGSALGORITHMCATEGORIZEUSINGSTYLE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

class QgsCategorizedSymbolRenderer;

///@cond PRIVATE

/**
 * Native create categorized renderer from style algorithm
 */
class QgsCategorizeUsingStyleAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsCategorizeUsingStyleAlgorithm();
    ~QgsCategorizeUsingStyleAlgorithm() override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsCategorizeUsingStyleAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    QString mField;
    QString mLayerId;
    QString mLayerName;
    QgsWkbTypes::GeometryType mLayerGeometryType = QgsWkbTypes::UnknownGeometry;
    QgsFields mLayerFields;
    QgsExpression mExpression;
    QgsExpressionContext mExpressionContext;
    QgsFeatureIterator mIterator;
    std::unique_ptr<QgsCategorizedSymbolRenderer> mRenderer;
};

///@endcond PRIVATE

#endif // QGSALGORITHMCATEGORIZEUSINGSTYLE_H


