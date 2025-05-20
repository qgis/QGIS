/***************************************************************************
                         qgsalgorithmconvertgeometrytype.h
                         ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMCONVERTGEOMETRYTYPE_H
#define QGSALGORITHMCONVERTGEOMETRYTYPE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native convert geometry type algorithm.
 */
class QgsConvertGeometryTypeAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsConvertGeometryTypeAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsConvertGeometryTypeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    const QVector< QgsGeometry > convertGeometry( const QgsGeometry &geom, const int typeIndex, const Qgis::WkbType outputWkbType );
};

///@endcond PRIVATE

#endif // QGSALGORITHMCONVERTGEOMETRYTYPE_H
