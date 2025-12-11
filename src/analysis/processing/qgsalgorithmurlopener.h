/***************************************************************************
                         QGSALGORITHMURLOPENER.h
                         ---------------------
    begin                : August 2024
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

#ifndef QGSALGORITHMURLOPENER_H
#define QGSALGORITHMURLOPENER_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

class QgsProcessingFeedback;

///@cond PRIVATE

/**
 * Native url and file opener algorithm.
 */
class QgsOpenUrlAlgorithm : public QObject, public QgsProcessingAlgorithm
{
    Q_OBJECT

  public:
    QgsOpenUrlAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QgsOpenUrlAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMURLOPENER_H
