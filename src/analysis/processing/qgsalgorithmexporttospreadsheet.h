/***************************************************************************
                         qgsalgorithmexporttospreadsheet.h
                         ------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSALGORITHMEXPORTTOSPREADSHEET_H
#define QGSALGORITHMEXPORTTOSPREADSHEET_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsvectorfilewriter.h"

///@cond PRIVATE

class QgsVectorLayer;

/**
 * Native export to spreadsheet algorithm.
 */
class QgsExportToSpreadsheetAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsExportToSpreadsheetAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsExportToSpreadsheetAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    bool exportVectorLayer( QgsVectorLayer *layer, const QString &path, QgsProcessingContext &context, QgsProcessingFeedback *feedback, const QString &driverName, bool createNew, bool preferAlias, QgsVectorFileWriter::FieldValueConverter *converter );

    std::vector<std::unique_ptr<QgsMapLayer>> mLayers;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXPORTTOSPREADSHEET_H
