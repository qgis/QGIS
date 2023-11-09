/***************************************************************************
                         qgsalgorithmimportphotos.h
                         ------------------
    begin                : March 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#ifndef QGSALGORITHMIMPORTPHOTOS_H
#define QGSALGORITHMIMPORTPHOTOS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgis_analysis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native geotagged photos import.
 */
class ANALYSIS_EXPORT QgsImportPhotosAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsImportPhotosAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsImportPhotosAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    static QVariant parseCoord( const QString &string );
    static QVariantMap parseMetadataList( const QStringList &input );
    static QVariant parseMetadataValue( const QString &value );
    static bool extractGeoTagFromMetadata( const QVariantMap &metadata, QgsPointXY &tag );
    static QVariant extractAltitudeFromMetadata( const QVariantMap &metadata );
    static QVariant extractDirectionFromMetadata( const QVariantMap &metadata );
    static QVariant extractOrientationFromMetadata( const QVariantMap &metadata );
    static QVariant extractTimestampFromMetadata( const QVariantMap &metadata );

    friend class TestQgsProcessingAlgsPt1;
};

///@endcond PRIVATE

#endif // QGSALGORITHMIMPORTPHOTOS_H


