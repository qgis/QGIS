/***************************************************************************
                         qgsalgorithmpointtolayer.h
                         ---------------------
    begin                : May 2019
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

#ifndef QGSALGORITHMPOINTTOLAYER_H
#define QGSALGORITHMPOINTTOLAYER_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native point to layer algorithm.
 */
class QgsPointToLayerAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsPointToLayerAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override { return QObject::tr( "Create layer from point" ); }
    [[nodiscard]] QStringList tags() const override { return QObject::tr( "point,layer,polygon,create,new" ).split( ',' ); }
    [[nodiscard]] QString group() const override { return QObject::tr( "Vector creation" ); }
    [[nodiscard]] QString groupId() const override { return QStringLiteral( "vectorcreation" ); }
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsPointToLayerAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMPOINTTOLAYER_H
