/***************************************************************************
                         qgsalgorithmassignprojection.h
                         ------------------------------
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

#ifndef QGSALGORITHMASSIGNPROJECTION_H
#define QGSALGORITHMASSIGNPROJECTION_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native assign projection algorithm.
 */
class QgsAssignProjectionAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsAssignProjectionAlgorithm() = default;
    Flags flags() const override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsAssignProjectionAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QgsCoordinateReferenceSystem outputCrs( const QgsCoordinateReferenceSystem & ) const override { return mDestCrs; }
    QString outputName() const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeature processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    QgsCoordinateReferenceSystem mDestCrs;

};

///@endcond PRIVATE

#endif // QGSALGORITHMASSIGNPROJECTION_H


