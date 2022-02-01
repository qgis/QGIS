/***************************************************************************
  qgs3dnavigationwidget.h
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DNAVIGATIONWIDGET_H
#define QGS3DNAVIGATIONWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>

class QwtCompass;

#include "qgs3dmapcanvas.h"
#include "qgscameracontroller.h"

class Qgs3DNavigationWidget : public QWidget
{
    Q_OBJECT
  public:
    Qgs3DNavigationWidget( Qgs3DMapCanvas *parent = nullptr );

  public slots:

    /**
     * Update the state of navigation widget from camera's state
     */
    void updateFromCamera();

  signals:
    void sizeChanged( const QSize &newSize );

  protected:
    void resizeEvent( QResizeEvent *event ) override;
    void hideEvent( QHideEvent *event ) override;
    void showEvent( QShowEvent *event ) override;

  private:
    Qgs3DMapCanvas *mParent3DMapCanvas = nullptr;
    QToolButton *mZoomInButton = nullptr;
    QToolButton *mZoomOutButton = nullptr;
    QToolButton *mTiltUpButton = nullptr;
    QToolButton *mTiltDownButton = nullptr;
    QToolButton *mMoveUpButton = nullptr;
    QToolButton *mMoveRightButton = nullptr;
    QToolButton *mMoveDownButton = nullptr;
    QToolButton *mMoveLeftButton = nullptr;
    QwtCompass *mCompas = nullptr;
    QTableView *mCameraInfo = nullptr;
    QStandardItemModel *mCameraInfoItemModel = nullptr;
};

#endif // QGS3DNAVIGATIONWIDGET_H
