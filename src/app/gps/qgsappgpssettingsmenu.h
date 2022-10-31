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

class QRadioButton;

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

  signals:

    void locationMarkerToggled( bool visible );
    void bearingLineToggled( bool visible );
    void rotateMapToggled( bool enabled );
    void mapCenteringModeChanged( MapCenteringMode mode );

  private:

    QAction *mShowLocationMarkerAction = nullptr;
    QAction *mShowBearingLineAction = nullptr;
    QAction *mRotateMapAction = nullptr;

    QRadioButton *mRadioAlwaysRecenter = nullptr;
    QRadioButton *mRadioRecenterWhenOutside = nullptr;
    QRadioButton *mRadioNeverRecenter = nullptr;



};

#endif // QGSAPPGPSSETTINGSMANAGER_H
