/***************************************************************************
                         qgsalgorithmdrape.h
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

#ifndef QGSALGORITHMDRAPE_H
#define QGSALGORITHMDRAPE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Base class for drape algorithms.
 */
class QgsDrapeAlgorithmBase : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QString group() const override;
    QString groupId() const override;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    virtual void prepareGeometry( QgsGeometry &geometry, double defaultVal ) const = 0;
    virtual QgsPoint drapeVertex( const QgsPoint &vertex, double rasterVal ) const = 0;

    double mNoData = 0.0;
    bool mDynamicNoData = false;
    QgsProperty mNoDataProperty;

    double mScale = 1.0;
    bool mDynamicScale = false;
    QgsProperty mScaleProperty;

    std::unique_ptr< QgsRasterDataProvider > mRasterProvider;
    int mBand = 1;
    QgsRectangle mRasterExtent;
    bool mCreatedTransform = false;
    QgsCoordinateTransform mTransform;

};

class QgsDrapeToZAlgorithm : public QgsDrapeAlgorithmBase
{
  public:

    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsDrapeToZAlgorithm *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:

    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;

  private:
    void prepareGeometry( QgsGeometry &geometry, double defaultVal ) const override;
    QgsPoint drapeVertex( const QgsPoint &vertex, double rasterVal ) const override;


};


class QgsDrapeToMAlgorithm : public QgsDrapeAlgorithmBase
{
  public:

    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsDrapeToMAlgorithm *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:

    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;

  private:
    void prepareGeometry( QgsGeometry &geometry, double defaultVal ) const override;
    QgsPoint drapeVertex( const QgsPoint &vertex, double rasterVal ) const override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMDRAPE_H


