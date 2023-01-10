/***************************************************************************
    qgsgpstoolbar.h
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

#ifndef QGSGPSTOOLBAR_H
#define QGSGPSTOOLBAR_H

#include <QToolBar>
#include <QPointer>
#include <QElapsedTimer>

#include "qgscoordinatereferencesystem.h"
#include "qgssettingsentryenumflag.h"

class QgsAppGpsConnection;
class QgsMapCanvas;
class QLabel;
class QgsVectorLayer;
class QgsMapLayerProxyModel;
class QToolButton;
class QgsAppGpsDigitizing;


class QgsGpsToolBar : public QToolBar
{
    Q_OBJECT

  public:

    static const inline QgsSettingsEntryEnumFlag<Qgis::GpsInformationComponents> settingShowInToolbar = QgsSettingsEntryEnumFlag<Qgis::GpsInformationComponents>( QStringLiteral( "show-in-toolbar" ), QgsSettings::Prefix::GPS, Qgis::GpsInformationComponent::Location, QStringLiteral( "GPS information components to show in GPS toolbar" ) );

    QgsGpsToolBar( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    QAction *showInfoAction() { return mShowInfoAction; }

    void setGpsDigitizing( QgsAppGpsDigitizing *digitizing );

  signals:

    void addVertexClicked();
    void addFeatureClicked();
    void resetFeatureClicked();

  public slots:

    void setAddVertexButtonEnabled( bool enabled );
    void setResetTrackButtonEnabled( bool enabled );

  private slots:

    void updateLocationLabel();
    void destinationLayerChanged( QgsVectorLayer *lyr );
    void destinationMenuAboutToShow();

  private:

    void createLocationWidget();
    void adjustSize();

    QgsAppGpsConnection *mConnection = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QPointer< QgsAppGpsDigitizing > mDigitizing;

    QAction *mConnectAction = nullptr;
    QAction *mRecenterAction = nullptr;
    QAction *mShowInfoAction = nullptr;
    QAction *mAddTrackVertexAction = nullptr;
    QAction *mCreateFeatureAction = nullptr;
    QAction *mResetFeatureAction = nullptr;
    QAction *mSettingsMenuAction = nullptr;

    QToolButton *mDestinationLayerButton = nullptr;

    QMenu *mDestinationLayerMenu = nullptr;

    QPointer< QToolButton > mInformationButton;

    QgsCoordinateReferenceSystem mWgs84CRS;
    bool mEnableAddVertexButton = true;

    QgsMapLayerProxyModel *mDestinationLayerModel = nullptr;

    bool mIsFirstSizeChange = true;
    QElapsedTimer mLastLabelSizeChangeTimer;
};

#endif // QGSGPSTOOLBAR_H
