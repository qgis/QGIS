/***************************************************************************
    qgsappgpssettingsmenu.h
    -------------------
    begin                : October 2022
    copyright            : (C) 2022 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAPPGPSSETTINGSMENU_H
#define QGSAPPGPSSETTINGSMENU_H

#include <QMenu>
#include <QWidgetAction>
#include "qgis_app.h"
#include "qgssettingsentryimpl.h"
#include "qgsgpscanvasbridge.h"

class QRadioButton;
class QgsFieldProxyModel;

class QgsGpsMapRotationAction: public QWidgetAction
{
    Q_OBJECT

  public:

    QgsGpsMapRotationAction( QWidget *parent = nullptr );

    QRadioButton *radioAlwaysRecenter() { return mRadioAlwaysRecenter; }
    QRadioButton *radioRecenterWhenOutside() { return mRadioRecenterWhenOutside; }
    QRadioButton *radioNeverRecenter() { return mRadioNeverRecenter; }

  private:
    QRadioButton *mRadioAlwaysRecenter = nullptr;
    QRadioButton *mRadioRecenterWhenOutside = nullptr;
    QRadioButton *mRadioNeverRecenter = nullptr;

};

class APP_EXPORT QgsAppGpsSettingsMenu : public QMenu
{
    Q_OBJECT

  public:

    QgsAppGpsSettingsMenu( QWidget *parent );

    bool locationMarkerVisible() const;
    bool bearingLineVisible() const;
    bool rotateMap() const;
    Qgis::MapRecenteringMode mapCenteringMode() const;

  public slots:

    void onGpkgLoggingFailed();

  signals:

    void locationMarkerToggled( bool visible );
    void bearingLineToggled( bool visible );
    void rotateMapToggled( bool enabled );
    void mapCenteringModeChanged( Qgis::MapRecenteringMode mode );
    void enableNmeaLog( bool enabled );
    void gpkgLogDestinationChanged( const QString &path );
    void nmeaLogFileChanged( const QString &filename );

  private slots:

    void timeStampMenuAboutToShow();

  private:

    QAction *mShowLocationMarkerAction = nullptr;
    QAction *mShowBearingLineAction = nullptr;
    QAction *mRotateMapAction = nullptr;
    QAction *mAutoAddTrackVerticesAction = nullptr;
    QAction *mAutoSaveAddedFeatureAction = nullptr;
    QAction *mActionGpkgLog = nullptr;
    QAction *mActionNmeaLog = nullptr;

    QRadioButton *mRadioAlwaysRecenter = nullptr;
    QRadioButton *mRadioRecenterWhenOutside = nullptr;
    QRadioButton *mRadioNeverRecenter = nullptr;

    QgsFieldProxyModel *mFieldProxyModel = nullptr;
    QMenu *mTimeStampDestinationFieldMenu = nullptr;

    friend class TestQgsGpsIntegration;

};

#endif // QGSAPPGPSSETTINGSMANAGER_H
