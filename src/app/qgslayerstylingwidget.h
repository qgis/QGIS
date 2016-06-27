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
#include <QTimer>

#include "ui_qgsmapstylingwidgetbase.h"
#include "qgsmapstylepanel.h"

class QgsLabelingWidget;
class QgsMapLayer;
class QgsMapCanvas;
class QgsRendererV2PropertiesDialog;
class QgsRendererRasterPropertiesWidget;
class QgsUndoWidget;
class QgsRasterHistogramWidget;
class QgsLayerStylingPanelFactory;
class QgsMapLayerStyleManagerWidget;

class APP_EXPORT QgsLayerStyleManagerWidgetFactory : public QgsLayerStylingPanelFactory
{
  public:
    QIcon icon() override;
    QString title() override;
    QgsLayerStylingPanel *createPanel( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent ) override;
    bool supportsLayer( QgsMapLayer *layer ) override;
};

class APP_EXPORT QgsMapLayerStyleCommand : public QUndoCommand
{
  public:
    QgsMapLayerStyleCommand( QgsMapLayer* layer, const QDomNode& current, const QDomNode& last );

    virtual void undo() override;
    virtual void redo() override;

  private:
    QgsMapLayer* mLayer;
    QDomNode mXml;
    QDomNode mLastState;
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
    };

    QgsLayerStylingWidget( QgsMapCanvas *canvas, QList<QgsLayerStylingPanelFactory *> pages, QWidget *parent = 0 );
    ~QgsLayerStylingWidget();
    QgsMapLayer* layer() { return mCurrentLayer; }

    void setPageFactories( QList<QgsLayerStylingPanelFactory*> factories );

    /** Sets whether updates of the styling widget are blocked. This can be called to prevent
     * the widget being refreshed multiple times when a batch of layer style changes are
     * about to be applied
     * @param blocked set to true to block updates, or false to re-allow updates
     */
    void blockUpdates( bool blocked );

  signals:
    void styleChanged( QgsMapLayer* layer );

  public slots:
    void setLayer( QgsMapLayer* layer );
    void apply();
    void autoApply();
    void undo();
    void redo();
    void updateCurrentWidgetLayer();

    /** Sets the current visible page in the widget.
     * @param page standard page to display
     */
    void setCurrentPage( Page page );

  private slots:

    void layerAboutToBeRemoved( QgsMapLayer* layer );
    void liveApplyToggled( bool value );

  private:
    void pushUndoItem( const QString& name );
    int mNotSupportedPage;
    int mLayerPage;
    QTimer* mAutoApplyTimer;
    QDomNode mLastStyleXml;
    QgsMapCanvas* mMapCanvas;
    bool mBlockAutoApply;
    QgsUndoWidget* mUndoWidget;
    QgsMapLayer* mCurrentLayer;
    QgsLabelingWidget *mLabelingWidget;
    QgsRendererRasterPropertiesWidget* mRasterStyleWidget;
    QList<QgsLayerStylingPanelFactory*> mPageFactories;
    QMap<int, QgsLayerStylingPanelFactory*> mUserPages;
    QgsLayerStyleManagerWidgetFactory* mStyleManagerFactory;
};

#endif // QGSLAYERSTYLESDOCK_H
