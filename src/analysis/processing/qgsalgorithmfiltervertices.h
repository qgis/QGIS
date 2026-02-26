/***************************************************************************
                         qgsalgorithmfiltervertices.h
                         ----------------------------
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

#ifndef QGSALGORITHMFILTERVERTICES_H
#define QGSALGORITHMFILTERVERTICES_H


#include "qgis_sip.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * Base class for 'filter vertices' algorithms.
 */
class QgsFilterVerticesAlgorithmBase : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QString group() const final;
    QString groupId() const final;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) final;
    QString shortHelpString() const final;
    QString shortDescription() const override;

  protected:
    QString outputName() const final;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback ) final;

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
 * Filter vertices by M value algorithm.
 */
class QgsFilterVerticesByM : public QgsFilterVerticesAlgorithmBase
{
  public:
    QgsFilterVerticesByM() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QgsFilterVerticesByM *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  private:
    QString componentString() const override;
    void filter( QgsGeometry &geometry, double min, double max ) const override;
};

/**
 * Filter vertices by Z value algorithm.
 */
class QgsFilterVerticesByZ : public QgsFilterVerticesAlgorithmBase
{
  public:
    QgsFilterVerticesByZ() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QgsFilterVerticesByZ *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  private:
    QString componentString() const override;
    void filter( QgsGeometry &geometry, double min, double max ) const override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMFILTERVERTICES_H
