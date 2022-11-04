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

#include "qgscoordinatereferencesystem.h"

class QgsAppGpsConnection;
class QgsMapCanvas;
class QLabel;
class QgsVectorLayer;

class QgsGpsToolBar : public QToolBar
{
    Q_OBJECT

  public:

    QgsGpsToolBar( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    QAction *showInfoAction() { return mShowInfoAction; }

  signals:

    void addVertexClicked();
    void addFeatureClicked();
    void resetFeatureClicked();

  public slots:

    void setAddVertexButtonEnabled( bool enabled );

  private slots:

    void updateLocationLabel( const QgsPoint &point );
    void updateCloseFeatureButton( QgsVectorLayer *lyr );
    void layerEditStateChanged();

  private:

    QgsAppGpsConnection *mConnection = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QAction *mConnectAction = nullptr;
    QAction *mRecenterAction = nullptr;
    QAction *mShowInfoAction = nullptr;
    QAction *mAddTrackVertexAction = nullptr;
    QAction *mAddFeatureAction = nullptr;
    QAction *mResetFeatureAction = nullptr;

    QLabel *mLocationLabel = nullptr;

    QgsCoordinateReferenceSystem mWgs84CRS;
    bool mEnableAddVertexButton = true;

    QPointer< QgsVectorLayer > mLastLayer;
};

#endif // QGSGPSTOOLBAR_H
