/***************************************************************************
                         qgsalgorithmaddincrementalfield.h
                         ---------------------------------
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

#ifndef QGSALGORITHMADDINCREMENTALFIELD_H
#define QGSALGORITHMADDINCREMENTALFIELD_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native add incremental field algorithm.
 */
class QgsAddIncrementalFieldAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsAddIncrementalFieldAlgorithm() = default;
    Flags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsAddIncrementalFieldAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeature processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    long long mStartValue = 0;
    long long mValue = 0;
    QString mFieldName;
    QHash< QgsAttributes, long long > mGroupedValues;
    mutable QgsFields mFields;
    QStringList mGroupedFieldNames;
    QgsAttributeList mGroupedFields;
};

///@endcond PRIVATE

#endif // QGSNATIVEALGORITHMS_H


