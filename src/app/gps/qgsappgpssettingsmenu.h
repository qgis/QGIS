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

class QgsAppGpsSettingsMenu : public QMenu
{
    Q_OBJECT

  public:

    enum class MapCenteringMode
    {
      Always,
      WhenLeavingExtent,
      Never
    };
    Q_ENUM( MapCenteringMode )

    QgsAppGpsSettingsMenu( QWidget *parent );

    bool locationMarkerVisible() const;
    bool bearingLineVisible() const;
    bool rotateMap() const;
    MapCenteringMode mapCenteringMode() const;
    bool autoAddTrackPoints() const;
    bool autoAddFeature() const;

  public slots:

    void setCurrentTimeStampField( const QString &fieldName );

  signals:

    void locationMarkerToggled( bool visible );
    void bearingLineToggled( bool visible );
    void rotateMapToggled( bool enabled );
    void mapCenteringModeChanged( MapCenteringMode mode );
    void autoAddTrackPointsChanged( bool enabled );
    void autoAddFeatureChanged( bool enabled );
    void timeStampDestinationChanged( const QString &fieldName );

  private slots:

    void timeStampMenuAboutToShow();

  private:

    QAction *mShowLocationMarkerAction = nullptr;
    QAction *mShowBearingLineAction = nullptr;
    QAction *mRotateMapAction = nullptr;
    QAction *mAutoAddTrackPointAction = nullptr;
    QAction *mAutoSaveAddedFeatureAction = nullptr;

    QRadioButton *mRadioAlwaysRecenter = nullptr;
    QRadioButton *mRadioRecenterWhenOutside = nullptr;
    QRadioButton *mRadioNeverRecenter = nullptr;

    QgsFieldProxyModel *mFieldProxyModel = nullptr;
    QMenu *mTimeStampFieldMenu = nullptr;
    QString mCurrentTimeStampField;

};

#endif // QGSAPPGPSSETTINGSMANAGER_H
