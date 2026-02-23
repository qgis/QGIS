/***************************************************************************
                         qgsalgorithmstdbscanclustering.h
                         ---------------------
    begin                : July 2021
    copyright            : (C) 2021 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMDSTBSCANCLUSTERING_H
#define QGSALGORITHMDSTBSCANCLUSTERING_H


#include "qgis_analysis.h"
#include "qgis_sip.h"
#include "qgsalgorithmdbscanclustering.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE


/**
 * Native ST-DBSCAN density based scan with noise clustering algorithm.
 */
class ANALYSIS_EXPORT QgsStDbscanClusteringAlgorithm : public QgsDbscanClusteringAlgorithm
{
  public:
    QgsStDbscanClusteringAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QString shortDescription() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsStDbscanClusteringAlgorithm *createInstance() const override SIP_FACTORY;
};

///@endcond PRIVATE

#endif // QGSALGORITHMDSTBSCANCLUSTERING_H
