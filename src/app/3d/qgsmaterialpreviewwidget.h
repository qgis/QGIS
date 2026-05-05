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
#include <QWindow>

class QgsAbstractMaterial3DHandler;
class QgsAbstractMaterialSettings;

namespace Qt3DRender
{
  class QCamera;
  class QRenderAspect;
  class QRenderSettings;
} //namespace Qt3DRender

namespace Qt3DExtras
{
  class QForwardRenderer;
}

namespace Qt3DCore
{
  class QEntity;
}

namespace Qt3DInput
{
  class QInputAspect;
  class QInputSettings;
} //namespace Qt3DInput

namespace Qt3DLogic
{
  class QLogicAspect;
}

/**
 * Reimplementation of Qt3DWindow which does not set the default surface when initialized.
 */
class Qgs3DWindow : public QWindow
{
    Q_OBJECT
  public:
    Qgs3DWindow();
    ~Qgs3DWindow() override;
    void setRootEntity( Qt3DCore::QEntity *root );
    Qt3DRender::QCamera *camera() const;
    Qt3DExtras::QForwardRenderer *defaultFrameGraph() const;

  protected:
    void showEvent( QShowEvent *e ) override;
    void resizeEvent( QResizeEvent * ) override;

  private:
    Qt3DCore::QAspectEngine *m_aspectEngine = nullptr;
    Qt3DRender::QRenderAspect *m_renderAspect = nullptr;
    Qt3DInput::QInputAspect *m_inputAspect = nullptr;
    Qt3DLogic::QLogicAspect *m_logicAspect = nullptr;
    Qt3DRender::QRenderSettings *m_renderSettings = nullptr;
    Qt3DExtras::QForwardRenderer *m_forwardRenderer;
    Qt3DRender::QCamera *m_defaultCamera = nullptr;
    Qt3DInput::QInputSettings *m_inputSettings = nullptr;
    Qt3DCore::QEntity *m_root = nullptr;
    Qt3DCore::QEntity *m_userRoot = nullptr;

    bool m_initialized = false;
};

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
    void showEvent( QShowEvent *e ) override;

  private:
    void setupCamera( Qt3DRender::QCamera *camera );

    Qgs3DWindow *mView = nullptr;
    Qt3DCore::QEntity *mSceneRoot = nullptr;
    QString mPreviewSceneType;
    Qt3DCore::QEntity *mPreviewScene = nullptr;

    QList< QgsAbstractMaterial3DHandler::PreviewMeshType > mMeshTypes;
    std::unique_ptr< QgsAbstractMaterialSettings > mLastPreviewSettings;
};
#endif // QGSMATERIALPREVIEWWIDGET_H
