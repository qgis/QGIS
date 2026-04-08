/***************************************************************************
  qgsmaterialpreviewwidget.h
  --------------------------------------
  Date                 : March 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMATERIALPREVIEWWIDGET_H
#define QGSMATERIALPREVIEWWIDGET_H

#include "qgsmaterial3dhandler.h"

#include <QWidget>

class QgsAbstractMaterial3DHandler;
class QgsAbstractMaterialSettings;

namespace Qt3DRender
{
  class QCamera;
}

namespace Qt3DExtras
{
  class Qt3DWindow;
}

namespace Qt3DCore
{
  class QEntity;
}

//! Widget for previewing 3D materials
class QgsMaterialPreviewWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsMaterialPreviewWidget( QWidget *parent = nullptr );

    void setMaterialType( const QString &type );

    void updatePreview( const QgsAbstractMaterialSettings *settings );

  protected:
    bool eventFilter( QObject *watched, QEvent *event ) override;

  private:
    void setupCamera( Qt3DRender::QCamera *camera );

    Qt3DExtras::Qt3DWindow *mView = nullptr;
    Qt3DCore::QEntity *mSceneRoot = nullptr;
    QString mPreviewSceneType;
    Qt3DCore::QEntity *mPreviewScene = nullptr;

    QList< QgsAbstractMaterial3DHandler::PreviewMeshType > mMeshTypes;
    std::unique_ptr< QgsAbstractMaterialSettings > mLastPreviewSettings;
};
#endif // QGSMATERIALPREVIEWWIDGET_H
