/***************************************************************************
                         qgsalgorithmbuffer.h
                         ---------------------
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

#ifndef QGSALGORITHMBUFFER_H
#define QGSALGORITHMBUFFER_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Native buffer algorithm.
 */
class QgsBufferAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsBufferAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmBuffer.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmBuffer.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    QgsBufferAlgorithm *createInstance() const override SIP_FACTORY;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;
    Qgis::ProcessingAlgorithmFlags flags() const override;

    QgsProcessingAlgorithm::VectorProperties sinkProperties( const QString &sink,
        const QVariantMap &parameters,
        QgsProcessingContext &context,
        const QMap< QString, QgsProcessingAlgorithm::VectorProperties > &sourceProperties ) const override;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;


};

///@endcond PRIVATE

#endif // QGSALGORITHMBUFFER_H
