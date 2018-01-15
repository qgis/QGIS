/***************************************************************************
                         qgsalgorithmdissolve.h
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

#ifndef QGSALGORITHMDISSOLVE_H
#define QGSALGORITHMDISSOLVE_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Base class for dissolve/collect type algorithms.
 */
class QgsCollectorAlgorithm : public QgsProcessingAlgorithm
{
  protected:

    Flags flags() const override;
    QVariantMap processCollection( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback,
                                   const std::function<QgsGeometry( const QVector<QgsGeometry>& )> &collector, int maxQueueLength = 0 );
};

/**
 * Native dissolve algorithm.
 */
class QgsDissolveAlgorithm : public QgsCollectorAlgorithm
{

  public:

    QgsDissolveAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsDissolveAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native collect geometries algorithm.
 */
class QgsCollectAlgorithm : public QgsCollectorAlgorithm
{

  public:

    QgsCollectAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsCollectAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMDISSOLVE_H


