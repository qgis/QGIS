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
#include "qgis_app.h"

class QgsLabelingWidget;
class QgsMapLayer;
class QgsMapCanvas;
class QgsRendererPropertiesDialog;
class QgsRendererRasterPropertiesWidget;
class QgsRendererMeshPropertiesWidget;
class QgsUndoWidget;
class QgsRasterHistogramWidget;
class QgsMapLayerStyleManagerWidget;
class QgsVectorLayer3DRendererWidget;

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
    QgsMapLayerStyleCommand( QgsMapLayer *layer, const QString &text, const QDomNode &current, const QDomNode &last );

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

    QgsLayerStylingWidget( QgsMapCanvas *canvas, const QList<QgsMapLayerConfigWidgetFactory *> &pages, QWidget *parent = nullptr );
    ~QgsLayerStylingWidget() override;
    QgsMapLayer *layer() { return mCurrentLayer; }

    void setPageFactories( const QList<QgsMapLayerConfigWidgetFactory *> &factories );

    /**
     * Sets whether updates of the styling widget are blocked. This can be called to prevent
     * the widget being refreshed multiple times when a batch of layer style changes are
     * about to be applied
     * \param blocked set to true to block updates, or false to re-allow updates
     */
    void blockUpdates( bool blocked );

  signals:
    void styleChanged( QgsMapLayer *layer );

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

  private slots:

    void layerAboutToBeRemoved( QgsMapLayer *layer );
    void liveApplyToggled( bool value );

  private:
    void pushUndoItem( const QString &name );
    int mNotSupportedPage;
    int mLayerPage;
    QTimer *mAutoApplyTimer = nullptr;
    QDomNode mLastStyleXml;
    QgsMapCanvas *mMapCanvas = nullptr;
    bool mBlockAutoApply;
    QgsUndoWidget *mUndoWidget = nullptr;
    QgsMapLayer *mCurrentLayer = nullptr;
    QgsLabelingWidget *mLabelingWidget = nullptr;
#ifdef HAVE_3D
    QgsVectorLayer3DRendererWidget *mVector3DWidget = nullptr;
#endif
    QgsRendererRasterPropertiesWidget *mRasterStyleWidget = nullptr;
    QgsRendererMeshPropertiesWidget *mMeshStyleWidget = nullptr;
    QList<QgsMapLayerConfigWidgetFactory *> mPageFactories;
    QMap<int, QgsMapLayerConfigWidgetFactory *> mUserPages;
    QgsLayerStyleManagerWidgetFactory *mStyleManagerFactory = nullptr;
};

#endif // QGSLAYERSTYLESDOCK_H
