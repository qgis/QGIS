/***************************************************************************
  qgsalgorithmwritevectortiles.h
  ---------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWRITEVECTORTILESALGORITHM_H
#define QGSWRITEVECTORTILESALGORITHM_H


#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

class QgsVectorTileWriter;


class QgsWriteVectorTilesBaseAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    void addBaseParameters();

    virtual void prepareWriter( QgsVectorTileWriter &writer, const QVariantMap &parameters, QgsProcessingContext &context, QVariantMap &outputs ) = 0;
};


class QgsWriteVectorTilesXyzAlgorithm : public QgsWriteVectorTilesBaseAlgorithm
{
  public:
    QString name() const override;
    QString displayName() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

    void prepareWriter( QgsVectorTileWriter &writer, const QVariantMap &parameters, QgsProcessingContext &context, QVariantMap &outputs ) override;
};


class QgsWriteVectorTilesMbtilesAlgorithm : public QgsWriteVectorTilesBaseAlgorithm
{
  public:
    QString name() const override;
    QString displayName() const override;

  protected:
    QgsProcessingAlgorithm *createInstance() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

    void prepareWriter( QgsVectorTileWriter &writer, const QVariantMap &parameters, QgsProcessingContext &context, QVariantMap &outputs ) override;
};


///@endcond PRIVATE

#endif // QGSWRITEVECTORTILESALGORITHM_H
