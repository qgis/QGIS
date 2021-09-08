/***************************************************************************
    qgsannotationitempropertieswidget.h
    ---------------------
    begin                : September 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONITEMPROPERTIESWIDGET_H
#define QGSANNOTATIONITEMPROPERTIESWIDGET_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include <QPointer>

class QgsAnnotationLayer;
class QgsAnnotationItemBaseWidget;
class QStackedWidget;

class QgsAnnotationItemPropertiesWidget : public QgsMapLayerConfigWidget
{
    Q_OBJECT
  public:

    QgsAnnotationItemPropertiesWidget( QgsAnnotationLayer *layer, QgsMapCanvas *canvas, QWidget *parent );

    void syncToLayer( QgsMapLayer *layer ) override;
    void setMapLayerConfigWidgetContext( const QgsMapLayerConfigWidgetContext &context ) override;
    void setDockMode( bool dockMode ) override;

  public slots:
    void apply() override;
    void focusDefaultWidget() override;

  private slots:

    void onChanged();
  private:

    void setItemId( const QString &itemId );

    QStackedWidget *mStack = nullptr;
    QPointer< QgsAnnotationLayer > mLayer;
    QPointer< QgsAnnotationItemBaseWidget > mItemWidget;
    QWidget *mPageNoItem = nullptr;

};


class QgsAnnotationItemPropertiesWidgetFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsAnnotationItemPropertiesWidgetFactory( QObject *parent = nullptr );

    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;
    bool supportLayerPropertiesDialog() const override;
    bool supportsStyleDock() const override;
    bool supportsLayer( QgsMapLayer *layer ) const override;
};



#endif // QGSANNOTATIONITEMPROPERTIESWIDGET_H
