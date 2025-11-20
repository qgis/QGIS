/***************************************************************************
                             qgsvideoexporter.h
                             --------------------------
    begin                : November 2025
    copyright            : (C) 2025 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVIDEOEXPORTER_H
#define QGSVIDEOEXPORTER_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QSize>
#include <QObject>
#include <QtMultimedia/QMediaFormat>
#include <QtMultimedia/QMediaRecorder>

class QMediaCaptureSession;
class QVideoFrameInput;

/**
 * \ingroup core
 * \brief Handles exports of sequential image files to video formats.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsVideoExporter : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsVideoExporter.
     *
     * \param filename destination video file name
     * \param size output video frame size
     * \param framesPerSecond output video frames per second
     */
    QgsVideoExporter( const QString &filename, QSize size, double framesPerSecond );
    ~QgsVideoExporter() override;

    /**
     * Sets the list of input image \a files.
     *
     * The list must be an ordered list of existing image file paths, which will form
     * the output video frames.
     *
     * \see inputFiles()
     */
    void setInputFiles( const QStringList &files );

    /**
     * Returns the list of input image \a files.
     *
     * \see setInputFiles()
     */
    QStringList inputFiles() const;

    void setFileFormat( QMediaFormat::FileFormat format );
    QMediaFormat::FileFormat fileFormat() const;

    void setVideoCodec( QMediaFormat::VideoCodec codec );
    QMediaFormat::VideoCodec videoCodec() const;


    bool writeVideo();

  signals:

    void finished();

  private slots:

    void feedFrames();
    void checkStatus( QMediaRecorder::RecorderState state );
    void handleError( QMediaRecorder::Error error, const QString &errorString );

  private:

    QString mFileName;
    QSize mSize;
    double mFramesPerSecond = 10;
    qint64 mFrameDurationUs = 100000;
    QStringList mInputFiles;
    QMediaFormat::FileFormat mFormat = QMediaFormat::FileFormat::MPEG4;
    QMediaFormat::VideoCodec mCodec = QMediaFormat::VideoCodec::H264;
    int mCurrentFrameIndex = 0;

    std::unique_ptr< QMediaCaptureSession > mSession;
    std::unique_ptr< QMediaRecorder > mRecorder;
    std::unique_ptr< QVideoFrameInput > mVideoInput;

};


#endif // QGSVIDEOEXPORTER_H
