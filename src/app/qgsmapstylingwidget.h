/***************************************************************************
    qgsmapstylingwidget.h
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
#ifndef QGSMAPSTYLESDOCK_H
#define QGSMAPSTYLESDOCK_H

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
class QgsMapStylePanelFactory;
class QgsMapLayerStyleManagerWidget;

class APP_EXPORT QgsMapLayerStyleManagerWidgetFactory : public QgsMapStylePanelFactory
{
  public:
    QIcon icon() override;
    QString title() override;
    QgsMapStylePanel *createPanel( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent ) override;
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

class APP_EXPORT QgsMapStylingWidget : public QWidget, private Ui::QgsMapStylingWidget
{
    Q_OBJECT
  public:
    QgsMapStylingWidget( QgsMapCanvas *canvas, QList<QgsMapStylePanelFactory *> pages, QWidget *parent = 0 );
    ~QgsMapStylingWidget();
    QgsMapLayer* layer() { return mCurrentLayer; }

    void setPageFactories( QList<QgsMapStylePanelFactory*> factories );

  signals:
    void styleChanged( QgsMapLayer* layer );

  public slots:
    void setLayer( QgsMapLayer* layer );
    void apply();
    void autoApply();
    void undo();
    void redo();

  private slots:
    void updateCurrentWidgetLayer();
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
    QgsRendererV2PropertiesDialog* mVectorStyleWidget;
    QgsRendererRasterPropertiesWidget* mRasterStyleWidget;
    QList<QgsMapStylePanelFactory*> mPageFactories;
    QMap<int, QgsMapStylePanelFactory*> mUserPages;
    QgsMapLayerStyleManagerWidgetFactory* mStyleManagerFactory;
};

#endif // QGSMAPSTYLESDOCK_H
