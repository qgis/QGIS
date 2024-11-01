/***************************************************************************
  qgsmediawidget.h

 ---------------------
 begin                : 2023.01.24
 copyright            : (C) 2023 by Mathieu Pellerin
 email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMEDIAWIDGET_H
#define QGSMEDIAWIDGET_H

#include <QWidget>
#include <QMediaPlayer>

#include "qgis_gui.h"
#include "qgis_sip.h"

class QLabel;
class QVBoxLayout;
class QVideoWidget;
class QPushButton;
class QSlider;

/**
 * \ingroup gui
 * \brief The QgsMediaWidget class creates a widget for playing back audio and video media files.
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsMediaWidget : public QWidget
{
    Q_OBJECT

  public:
    /**
     * The mode determines the user interface elements visible within the widget.
    */
    enum Mode
    {
      Audio, //!< Audio-centric user interface
      Video, //!< Video-centric user interface
    };
    Q_ENUM( Mode )

    //! Constructor
    explicit QgsMediaWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the media path.
     */
    QString mediaPath() const { return mMediaPath; }

    /**
     * Sets the media \a path.
     */
    void setMediaPath( const QString &path );

    /**
     * Returns the media widget mode.
     */
    Mode mode() const { return mMode; }

    /**
     * Sets the media widget \a mode.
     */
    void setMode( Mode mode );

    /**
     * Returns the video frame height.
     */
    int videoHeight() const;

    /**
     * Sets the video frame height.
     * \note setting the height to 0 is interpreted as a video frame that will
     * expand to fill available height in the widget's parent layout.
     */
    void setVideoHeight( int height );

    /**
     * Returns the QMediaPlayer object.
     */
    QMediaPlayer *mediaPlayer() { return &mMediaPlayer; }

  private slots:

    void mediaStatusChanged( QMediaPlayer::MediaStatus status );

  private:
    void adjustControls();
    void setControlsEnabled( bool enabled );

    Mode mMode = Audio;

    QVBoxLayout *mLayout = nullptr;
    QVideoWidget *mVideoWidget = nullptr;
    QPushButton *mPlayButton = nullptr;
    QSlider *mPositionSlider = nullptr;
    QLabel *mDurationLabel = nullptr;

    QMediaPlayer mMediaPlayer;

    QString mMediaPath;
};

#endif // QGSMEDIAWIDGET_H
