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

#include "qgscoordinatereferencesystem.h"

class QgsAppGpsConnection;
class QgsMapCanvas;
class QLabel;

class QgsGpsToolBar : public QToolBar
{
    Q_OBJECT

  public:

    QgsGpsToolBar( QgsAppGpsConnection *connection, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    QAction *showInfoAction() { return mShowInfoAction; }

  private slots:

    void updateLocationLabel( const QgsPoint &point );

  private:

    QgsAppGpsConnection *mConnection = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QAction *mConnectAction = nullptr;
    QAction *mRecenterAction = nullptr;
    QAction *mShowInfoAction = nullptr;

    QLabel *mLocationLabel = nullptr;

    QgsCoordinateReferenceSystem mWgs84CRS;
};

#endif // QGSGPSTOOLBAR_H
