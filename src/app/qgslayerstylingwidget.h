/***************************************************************************
    qgslayerstylingwidget.h
    ---------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERSTYLESDOCK_H
#define QGSLAYERSTYLESDOCK_H

#include <QToolButton>
#include <QWidget>
#include <QLabel>
#include <QTabWidget>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QUndoCommand>
#include <QDomNode>
#include <QTime>
#include <QTimer>

#include "ui_qgsmapstylingwidgetbase.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgis_app.h"

class QgsLabelingWidget;
class QgsMaskingWidget;
class QgsMapLayer;
class QgsMapCanvas;
class QgsRendererPropertiesDialog;
class QgsRendererRasterPropertiesWidget;
class QgsRendererMeshPropertiesWidget;
class QgsUndoWidget;
class QgsRasterHistogramWidget;
class QgsMapLayerStyleManagerWidget;
class QgsVectorLayer3DRendererWidget;
class QgsMeshLayer3DRendererWidget;
class QgsPointCloudLayer3DRendererWidget;
class QgsMessageBar;
class QgsVectorTileBasicRendererWidget;
class QgsVectorTileBasicLabelingWidget;
class QgsAnnotationLayer;
class QgsLayerTreeGroup;

class APP_EXPORT QgsLayerStyleManagerWidgetFactory : public QgsMapLayerConfigWidgetFactory
{
  public:
    QgsLayerStyleManagerWidgetFactory();
    bool supportsStyleDock() const override { return true; }
    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockMode, QWidget *parent ) const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
};

class APP_EXPORT QgsMapLayerStyleCommand : public QUndoCommand
{
  public:
    QgsMapLayerStyleCommand( QgsMapLayer *layer, const QString &text, const QDomNode &current, const QDomNode &last, bool triggerRepaint = true );

    /**
     * Returns unique ID for this kind of undo command.
     * Currently we do not have a central registry of undo command IDs, so it is a random magic number.
     */
    int id() const override { return 0xbeef; }

    void undo() override;
    void redo() override;

    //! Try to merge with other commands of this type when they are created in small time interval
    bool mergeWith( const QUndoCommand *other ) override;

  private:
    QgsMapLayer *mLayer = nullptr;
    QDomNode mXml;
    QDomNode mLastState;
    QTime mTime;
    bool mTriggerRepaint = true;
};

class APP_EXPORT QgsLayerStylingWidget : public QWidget, private Ui::QgsLayerStylingWidgetBase
{
    Q_OBJECT
  public:

    enum Page
    {
      Symbology = 1,
      VectorLabeling,
      RasterTransparency,
      RasterHistogram,
      History,
      Symbology3D,
    };

    QgsLayerStylingWidget( QgsMapCanvas *canvas, QgsMessageBar *messageBar, const QList<const QgsMapLayerConfigWidgetFactory *> &pages, QWidget *parent = nullptr );
    ~QgsLayerStylingWidget() override;
    QgsMapLayer *layer() { return mCurrentLayer; }

    void setPageFactories( const QList<const QgsMapLayerConfigWidgetFactory *> &factories );

    /**
     * Sets whether updates of the styling widget are blocked. This can be called to prevent
     * the widget being refreshed multiple times when a batch of layer style changes are
     * about to be applied
     * \param blocked set to TRUE to block updates, or FALSE to re-allow updates
     */
    void blockUpdates( bool blocked );

  signals:
    void styleChanged( QgsMapLayer *layer );
    void layerStyleChanged( const QString &currentStyleName );

  public slots:
    void setLayer( QgsMapLayer *layer );
    void apply();
    void autoApply();
    void undo();
    void redo();
    void updateCurrentWidgetLayer();

    /**
     * Sets the current visible page in the widget.
     * \param page standard page to display
     */
    void setCurrentPage( QgsLayerStylingWidget::Page page );

    /**
     * Sets an annotation item to show in the widget.
     */
    void setAnnotationItem( QgsAnnotationLayer *layer, const QString &itemId );

    /**
     * Sets a layer tree group to show in the widget.
     */
    void setLayerTreeGroup( QgsLayerTreeGroup *group );

    /**
     * Focuses the default widget for the current page.
     */
    void focusDefaultWidget();

  private slots:

    void layerAboutToBeRemoved( QgsMapLayer *layer );
    void liveApplyToggled( bool value );

  private:
    void pushUndoItem( const QString &name, bool triggerRepaint = true );
    void emitLayerStyleChanged( const QString &currentStyleName ) {emit layerStyleChanged( currentStyleName );};
    void emitLayerStyleRenamed();
    int mNotSupportedPage;
    int mLayerPage;
    QTimer *mAutoApplyTimer = nullptr;
    QDomNode mLastStyleXml;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    bool mBlockAutoApply;
    QgsUndoWidget *mUndoWidget = nullptr;
    QgsMapLayer *mCurrentLayer = nullptr;
    QgsLabelingWidget *mLabelingWidget = nullptr;
    QgsMaskingWidget *mMaskingWidget = nullptr;
#ifdef HAVE_3D
    QgsVectorLayer3DRendererWidget *mVector3DWidget = nullptr;
    QgsMeshLayer3DRendererWidget *mMesh3DWidget = nullptr;
#endif
    QgsRendererRasterPropertiesWidget *mRasterStyleWidget = nullptr;
    QgsRendererMeshPropertiesWidget *mMeshStyleWidget = nullptr;
    QgsVectorTileBasicRendererWidget *mVectorTileStyleWidget = nullptr;
    QgsVectorTileBasicLabelingWidget *mVectorTileLabelingWidget = nullptr;
    QList<const QgsMapLayerConfigWidgetFactory *> mPageFactories;
    QMap<int, const QgsMapLayerConfigWidgetFactory *> mUserPages;
    QgsLayerStyleManagerWidgetFactory *mStyleManagerFactory = nullptr;
    QgsMapLayerConfigWidgetContext mContext;
};

#endif // QGSLAYERSTYLESDOCK_H
