/***************************************************************************
                         qgsalgorithmpdalcreatecopc.h
                         ---------------------
    begin                : May 2023
    copyright            : (C) 2023 by Alexander Bruy
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

#ifndef QGSALGORITHMPDALCREATECOPC_H
#define QGSALGORITHMPDALCREATECOPC_H


#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

#include <QString>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;


///@cond PRIVATE

/**
 * Native point cloud create COPC algorithm.
 */
class QgsPdalCreateCopcAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsPdalCreateCopcAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QString group() const override;
    QString groupId() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/mIconPointCloudLayer.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/mIconPointCloudLayer.svg"_s ); }
    QgsPdalCreateCopcAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    friend class TestQgsProcessingPdalAlgs;
};

///@endcond PRIVATE

#endif // QGSALGORITHMPDALCREATECOPC_H
