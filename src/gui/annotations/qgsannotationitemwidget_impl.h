/***************************************************************************
                             qgsannotationitemwidget_impl.h
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSANNOTATIONITEMWIDGETIMPL_H
#define QGSANNOTATIONITEMWIDGETIMPL_H

#include "qgsannotationitemwidget.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include <memory>

class QgsSymbolSelectorWidget;
class QgsFillSymbol;
class QgsLineSymbol;
class QgsMarkerSymbol;
class QgsAnnotationPolygonItem;
class QgsAnnotationLineItem;
class QgsAnnotationMarkerItem;

class QgsAnnotationPolygonItemWidget : public QgsAnnotationItemBaseWidget
{
    Q_OBJECT

  public:
    QgsAnnotationPolygonItemWidget( QWidget *parent );
    ~QgsAnnotationPolygonItemWidget() override;
    QgsAnnotationItem *createItem() override;
    void setDockMode( bool dockMode ) override;

  protected:
    bool setNewItem( QgsAnnotationItem *item ) override;

  private:

    QgsSymbolSelectorWidget *mSelector = nullptr;
    std::unique_ptr< QgsFillSymbol > mSymbol;
    bool mBlockChangedSignal = false;
    std::unique_ptr< QgsAnnotationPolygonItem> mItem;
};

class QgsAnnotationLineItemWidget : public QgsAnnotationItemBaseWidget
{
    Q_OBJECT

  public:
    QgsAnnotationLineItemWidget( QWidget *parent );
    ~QgsAnnotationLineItemWidget() override;
    QgsAnnotationItem *createItem() override;
    void setDockMode( bool dockMode ) override;

  protected:
    bool setNewItem( QgsAnnotationItem *item ) override;

  private:

    QgsSymbolSelectorWidget *mSelector = nullptr;
    std::unique_ptr< QgsLineSymbol > mSymbol;
    bool mBlockChangedSignal = false;
    std::unique_ptr< QgsAnnotationLineItem> mItem;
};

class QgsAnnotationMarkerItemWidget : public QgsAnnotationItemBaseWidget
{
    Q_OBJECT

  public:
    QgsAnnotationMarkerItemWidget( QWidget *parent );
    ~QgsAnnotationMarkerItemWidget() override;
    QgsAnnotationItem *createItem() override;
    void setDockMode( bool dockMode ) override;

  protected:
    bool setNewItem( QgsAnnotationItem *item ) override;

  private:

    QgsSymbolSelectorWidget *mSelector = nullptr;
    std::unique_ptr< QgsMarkerSymbol > mSymbol;
    bool mBlockChangedSignal = false;
    std::unique_ptr< QgsAnnotationMarkerItem> mItem;
};

#endif // QGSANNOTATIONITEMWIDGETIMPL_H
