/***************************************************************************
                         qgsalgorithmexplodehstore.h
                         ---------------------
    begin                : September 2018
    copyright            : (C) 2018 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMEXPLODEHSTORE_H
#define QGSALGORITHMEXPLODEHSTORE_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native explode hstore algorithm.
 */
class QgsExplodeHstoreAlgorithm : public QgsProcessingAlgorithm
{

  public:
    QgsExplodeHstoreAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMEXPLODEHSTORE_H


