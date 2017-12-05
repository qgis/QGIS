/***************************************************************************
                         qgsalgorithmfiledownloader.h
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

#ifndef QGSALGORITHMFILEDOWNLOADER_H
#define QGSALGORITHMFILEDOWNLOADER_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

class QgsProcessingFeedback;

///@cond PRIVATE

/**
 * Native file downloader algorithm.
 */
class QgsFileDownloaderAlgorithm : public QgsProcessingAlgorithm, public QObject
{
  public:
    QgsFileDownloaderAlgorithm() = default;
    Flags flags() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsFileDownloaderAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback * ) override;

  private:
    QString mTotal;
    QString mReceived;
    QgsProcessingFeedback *mFeedback = nullptr;
    QString mLastReport;
    void reportErrors( QStringList errors );
    void receiveProgressFromDownloader( qint64 bytesReceived, qint64 bytesTotal );
    void sendProgressFeedback();
};

///@endcond PRIVATE

#endif // QGSALGORITHMFILEDOWNLOADER_H
