/***************************************************************************
                         QGSALGORITHMHTTPREQUEST.h
                         ---------------------
    begin                : September 2024
    copyright            : (C) 2024 by Dave Signer
    email                : david at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMHTTPREQUEST_H
#define QGSALGORITHMHTTPREQUEST_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include <QNetworkReply>

class QgsProcessingFeedback;

///@cond PRIVATE

/**
 * Native http(s) request tool (post/get)
 */
class QgsHttpRequestAlgorithm : public QObject, public QgsProcessingAlgorithm
{
    Q_OBJECT

  public:
    QgsHttpRequestAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QString shortDescription() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsHttpRequestAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMHTTPREQUEST_H
