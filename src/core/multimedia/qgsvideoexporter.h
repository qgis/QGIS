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
#include <QPointer>
#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
#include <QtMultimedia/QMediaFormat>
#include <QtMultimedia/QMediaRecorder>
#endif

class QgsFeedback;
class QMediaCaptureSession;
class QVideoFrameInput;

/**
 * \ingroup core
 * \brief Handles exports of sequential image files to video formats.
 *
 * Video export functionality is not available on all systems. The
 * isAvailable() function can be used to test whether video
 * export is available on the current system.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsVideoExporter : public QObject
{
    Q_OBJECT

  public:

    /**
     * Returns TRUE if the video export functionality is available on the current system.
     */
    static bool isAvailable();

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
     * Sets an optional \a feedback object, for progress reports and cancellation support.
     *
     * The object must exist for the lifetime of the export, ownership is not transferred.
     *
     * \see feedback()
     */
    void setFeedback( QgsFeedback *feedback );

    /**
     * Returns the optional feedback object.
     *
     * \see setFeedback()
     */
    QgsFeedback *feedback();

    /**
     * Returns the output video frames per second.
     *
     * \see frameDuration()
     */
    double framesPerSecond() const { return mFramesPerSecond; }

    /**
     * Returns the duration of each frame, in micro-seconds.
     *
     * \see framesPerSecond()
     */
    qint64 frameDuration() const { return mFrameDurationUs; }

    /**
     * Returns the output video frame size.
     */
    QSize size() const { return mSize; }

    /**
     * Sets the list of input image \a files.
     *
     * The list must be an ordered list of existing image file paths, which will form
     * the output video frames.
     *
     * \see setInputFilesByPattern()
     * \see inputFiles()
     */
    void setInputFiles( const QStringList &files );

    /**
     * Sets the input image files by searching a \a directory for files matching a \a pattern.
     *
     * E.g. setting \a pattern to "*.png" will find all PNG files in the \a directory and use them
     * as input frames.
     *
     * The frames will be sorted alphabetically by filename.
     *
     * \see setInputFiles()
     * \see inputFiles()
     */
    void setInputFilesByPattern( const QString &directory, const QString &pattern );

    /**
     * Returns the list of input image \a files.
     *
     * \see setInputFiles()
     */
    QStringList inputFiles() const;

#ifndef SIP_RUN
#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )

    /**
     * Sets the output file \a format.
     *
     * The Qt QMediaFormat::supportedFileFormats() method can be used to retrieve a list of formats supported
     * by the system.
     *
     * \see fileFormat()
     */
    void setFileFormat( QMediaFormat::FileFormat format );

    /**
     * Returns the output file format.
     *
     * \see setFileFormat()
     */
    QMediaFormat::FileFormat fileFormat() const;

    /**
     * Sets the output video \a codec.
     *
     * The Qt QMediaFormat::supportedVideoCodecs() method can be used to retrieve a list of video codecs supported
     * by the system.
     *
     * \see videoCodec()
     */
    void setVideoCodec( QMediaFormat::VideoCodec codec );

    /**
     * Returns the output video codec.
     *
     * \see setVideoCodec()
     */
    QMediaFormat::VideoCodec videoCodec() const;

    /**
     * Returns the last error received while writing the video.
     *
     * \see errorString()
     */
    QMediaRecorder::Error error() const;
#endif
#endif

#ifdef SIP_PYQT6_RUN

    /**
     * Sets the output file \a format.
     *
     * The Qt QMediaFormat::supportedFileFormats() method can be used to retrieve a list of formats supported
     * by the system.
     *
     * \see fileFormat()
     */
    void setFileFormat( QMediaFormat::FileFormat format );

    /**
     * Returns the output file format.
     *
     * \see setFileFormat()
     */
    QMediaFormat::FileFormat fileFormat() const;

    /**
     * Sets the output video \a codec.
     *
     * The Qt QMediaFormat::supportedVideoCodecs() method can be used to retrieve a list of video codecs supported
     * by the system.
     *
     * \see videoCodec()
     */
    void setVideoCodec( QMediaFormat::VideoCodec codec );

    /**
     * Returns the output video codec.
     *
     * \see setVideoCodec()
     */
    QMediaFormat::VideoCodec videoCodec() const;

    /**
     * Returns the last error received while writing the video.
     *
     * \see errorString()
     */
    QMediaRecorder::Error error() const;
#endif

    /**
     * Returns the string describing the last error received while writing the video.
     *
     * \see error()
     */
    QString errorString() const;

  public slots:

    /**
     * Starts the video export operation.
     *
     * The finished() signal will be emitted when the operation is complete.
     *
     * \throws QgsNotSupportedException if writing video is not supported on the current system.
     */
    void writeVideo() SIP_THROW( QgsNotSupportedException );

  signals:

    /**
     * Emitted when the video export finishes.
     */
    void finished();

  private slots:

    void feedFrames();
#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
    void checkStatus( QMediaRecorder::RecorderState state );
    void handleError( QMediaRecorder::Error error, const QString &errorString );
#endif

  private:

    QString mFileName;
    QSize mSize;
    QStringList mInputFiles;
    double mFramesPerSecond = 10;
    qint64 mFrameDurationUs = 100000;
#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
    QMediaFormat::FileFormat mFormat = QMediaFormat::FileFormat::MPEG4;
    QMediaFormat::VideoCodec mCodec = QMediaFormat::VideoCodec::H264;
    QMediaRecorder::Error mError = QMediaRecorder::Error::NoError;
    int mCurrentFrameIndex = 0;
#endif
    QPointer< QgsFeedback > mFeedback;

    QString mErrorString;

#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
    std::unique_ptr< QMediaCaptureSession > mSession;
    std::unique_ptr< QMediaRecorder > mRecorder;
    std::unique_ptr< QVideoFrameInput > mVideoInput;
#endif

};


#endif // QGSVIDEOEXPORTER_H
