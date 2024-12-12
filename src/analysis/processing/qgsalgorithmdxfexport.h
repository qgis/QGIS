/***************************************************************************
  qgsalgorithmdxfexport.h
  ---------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDXFEXPORTALGORITHM_H
#define QGSDXFEXPORTALGORITHM_H

#define SIP_NO_FILE

#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

class QgsDxfExportAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsDxfExportAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QgsDxfExportAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QMap<QString, QString> mMapThemeStyleOverrides;
};

///@endcond PRIVATE

#endif // QGSDXFEXPORTALGORITHM_H
