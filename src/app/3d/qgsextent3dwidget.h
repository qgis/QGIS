/***************************************************************************
  qgsextent3dwidget.h
  --------------------------------------
  Date                 : July 2024
  Copyright            : (C) 2024 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTENT3DWIDGET_H
#define QGSEXTENT3DWIDGET_H

#include "qgscollapsiblegroupbox.h"
#include "qgsextentwidget.h"
#include "qgsmapcanvas.h"
#include "qgsmaptool.h"
#include "qgsrectangle.h"
#include "ui_qgsextent3dwidget.h"

#include <QMenu>

class QgsBookmarkManagerProxyModel;
class QgsMapCanvas;
class QgsMapLayerProxyModel;
class QgsMapToolExtent;

//! A widget for configuration of 3D extent
class QgsExtent3DWidget : public QgsCollapsibleGroupBox, private Ui::QgsExtent3DWidget
{
    Q_OBJECT
  public:
    explicit QgsExtent3DWidget( QWidget *parent = nullptr );

  private:

    QMenu *mMenu = nullptr;
    QMenu *mLayerMenu = nullptr;
    QMenu *mLayoutMenu = nullptr;
    QMenu *mBookmarkMenu = nullptr;

    QgsMapLayerProxyModel *mMapLayerModel = nullptr;
    QgsBookmarkManagerProxyModel *mBookmarkModel = nullptr;

    QList< QAction * > mLayerMenuActions;
    QAction *mUseCanvasExtentAction = nullptr;
    QAction *mUseDefaultExtentAction = nullptr;
    QAction *mDrawOnCanvasAction = nullptr;

    QgsRectangle mDefaultExtent;
    QPointer< const QgsMapLayer > mExtentLayer;

    QgsMapCanvas *mCanvas = nullptr;
    std::unique_ptr< QgsMapToolExtent > mMapToolExtent;
    QPointer< QgsMapTool > mMapToolPrevious = nullptr;

    QgsCoordinateReferenceSystem mCrs;

    void setOutputExtent( const QgsRectangle &rectangle, const QgsCoordinateReferenceSystem &srcCrs, const QgsExtentWidget::ExtentState &state );

    void updateTitle( const QgsExtentWidget::ExtentState &state );

  signals:

    void toggleDialogVisibility( bool visible );
    void extentChanged();
    void rotationChanged();
    void showIn2DViewChanged();

  private slots:

    void layerMenuAboutToShow();
    void layoutMenuAboutToShow();
    void bookmarkMenuAboutToShow();

    void extentDrawn( const QgsRectangle &extent );
    void mapToolDeactivated();

    void onRotationChanged( int rotation );

  public slots:

    /**
     * Sets the output extent to be the same as current extent (may be transformed to output CRS).
     */
    void setOutputExtentFromCurrent();

    /**
     * Sets the output extent by dragging on the canvas.
     */
    void setOutputExtentFromDrawOnCanvas();

  public:

    /**
    * Sets the map canvas to enable dragging of extent on a canvas.
    * \param canvas the map canvas
    * \param drawOnCanvasOption set to false to disable to draw on canvas option
    */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
    * Sets the current extent to show in the widget - should be called as part of initialization (or whenever current extent changes).
    * The current extent is usually set to match the current map canvas extent.
    */
    void setDefaultExtent( const QgsRectangle &defaultExtent, const QgsCoordinateReferenceSystem &crs );

    bool showIn2DView() const { return mPreviewExtentCheckBox->isChecked(); }

    void setShowIn2DView( bool show ) { mPreviewExtentCheckBox->setChecked( show ); }

    QgsRectangle extent() const ;

    void setRotation( double rotation );

    double rotation() const { return mRotationSpinBox->value(); }
};

#endif // QGSEXTENT3DWIDGET_H
