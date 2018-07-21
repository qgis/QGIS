/***************************************************************************
                         qgsalgorithmfilterpoints.h
                         --------------------------
    begin                : July 2018
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

#ifndef QGSALGORITHMFILTERPOINTS_H
#define QGSALGORITHMFILTERPOINTS_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsmaptopixelgeometrysimplifier.h"

///@cond PRIVATE

/**
 * Base class for 'filter points' algorithms.
 */
class QgsFilterPointsAlgorithmBase : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QString group() const override final;
    QString groupId() const override final;
    QList<int> inputLayerTypes() const override final;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override final;
    QString shortHelpString() const override final;

  protected:
    QString outputName() const override final;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override final;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &, QgsProcessingFeedback *feedback ) override final;

  private:

    double mMin = 0.0;
    bool mDynamicMin = false;
    QgsProperty mMinProperty;

    double mMax = 0.0;
    bool mDynamicMax = false;
    QgsProperty mMaxProperty;

    virtual QString componentString() const = 0;
    virtual void filter( QgsGeometry &geometry, double min, double max ) const = 0;
};

/**
 * Filter points by M value algorithm.
 */
class QgsFilterPointsByM : public QgsFilterPointsAlgorithmBase
{

  public:

    QgsFilterPointsByM() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QgsFilterPointsByM *createInstance() const override SIP_FACTORY;

  private:

    QString componentString() const override;
    void filter( QgsGeometry &geometry, double min, double max ) const override;
};


/**
 * Filter points by Z value algorithm.
 */
class QgsFilterPointsByZ : public QgsFilterPointsAlgorithmBase
{

  public:

    QgsFilterPointsByZ() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QgsFilterPointsByZ *createInstance() const override SIP_FACTORY;

  private:

    QString componentString() const override;
    void filter( QgsGeometry &geometry, double min, double max ) const override;
};


///@endcond PRIVATE

#endif // QGSALGORITHMFILTERPOINTS_H


