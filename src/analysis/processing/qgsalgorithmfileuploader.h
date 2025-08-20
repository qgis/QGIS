/***************************************************************************
                         qgsalgorithmfileuploader.h
                         ---------------------
    begin                : October 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMFILEUPLOADER_H
#define QGSALGORITHMFILEUPLOADER_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

class QgsProcessingFeedback;

///@cond PRIVATE

/**
 * Native file uploader algorithm.
 *
 * \since QGIS 4.0
 */
class QgsFileUploaderAlgorithm : public QObject, public QgsProcessingAlgorithm
{
    Q_OBJECT

  public:
    QgsFileUploaderAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QString shortDescription() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsFileUploaderAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) override;

  private:
    QString mTotal;
    QString mSent;
    QgsProcessingFeedback *mFeedback = nullptr;
    QString mLastReport;
    void receiveProgressFromUploader( qint64 bytesSent, qint64 bytesTotal );
    void sendProgressFeedback();
};

///@endcond PRIVATE

#endif // QGSALGORITHMFILEUPLOADER_H
