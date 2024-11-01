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
#include "qgstextformat.h"
#include "qgsexpressioncontextgenerator.h"
#include <memory>

#include "ui_qgsannotationpointtextwidgetbase.h"
#include "ui_qgsannotationsymbolwidgetbase.h"
#include "ui_qgsannotationlinetextwidgetbase.h"
#include "ui_qgsannotationrectangulartextwidgetbase.h"
#include "ui_qgsannotationpicturewidgetbase.h"

class QgsSymbolSelectorWidget;
class QgsFillSymbol;
class QgsLineSymbol;
class QgsMarkerSymbol;
class QgsAnnotationPolygonItem;
class QgsAnnotationLineItem;
class QgsAnnotationMarkerItem;
class QgsAnnotationPointTextItem;
class QgsAnnotationLineTextItem;
class QgsAnnotationPictureItem;
class QgsAnnotationRectangleTextItem;
class QgsTextFormatWidget;

#define SIP_NO_FILE

///@cond PRIVATE

class QgsAnnotationPolygonItemWidget : public QgsAnnotationItemBaseWidget, private Ui_QgsAnnotationSymbolWidgetBase
{
    Q_OBJECT

  public:
    QgsAnnotationPolygonItemWidget( QWidget *parent );
    ~QgsAnnotationPolygonItemWidget() override;
    QgsAnnotationItem *createItem() override;
    void updateItem( QgsAnnotationItem *item ) override;
    void setDockMode( bool dockMode ) override;
    void setContext( const QgsSymbolWidgetContext &context ) override;

  protected:
    bool setNewItem( QgsAnnotationItem *item ) override;

  private:
    QgsSymbolSelectorWidget *mSelector = nullptr;
    std::unique_ptr<QgsFillSymbol> mSymbol;
    bool mBlockChangedSignal = false;
    std::unique_ptr<QgsAnnotationPolygonItem> mItem;
};

class QgsAnnotationLineItemWidget : public QgsAnnotationItemBaseWidget, private Ui_QgsAnnotationSymbolWidgetBase
{
    Q_OBJECT

  public:
    QgsAnnotationLineItemWidget( QWidget *parent );
    ~QgsAnnotationLineItemWidget() override;
    QgsAnnotationItem *createItem() override;
    void updateItem( QgsAnnotationItem *item ) override;
    void setDockMode( bool dockMode ) override;
    void setContext( const QgsSymbolWidgetContext &context ) override;

  protected:
    bool setNewItem( QgsAnnotationItem *item ) override;

  private:
    QgsSymbolSelectorWidget *mSelector = nullptr;
    std::unique_ptr<QgsLineSymbol> mSymbol;
    bool mBlockChangedSignal = false;
    std::unique_ptr<QgsAnnotationLineItem> mItem;
};

class QgsAnnotationMarkerItemWidget : public QgsAnnotationItemBaseWidget, private Ui_QgsAnnotationSymbolWidgetBase
{
    Q_OBJECT

  public:
    QgsAnnotationMarkerItemWidget( QWidget *parent );
    ~QgsAnnotationMarkerItemWidget() override;
    QgsAnnotationItem *createItem() override;
    void updateItem( QgsAnnotationItem *item ) override;
    void setDockMode( bool dockMode ) override;
    void setContext( const QgsSymbolWidgetContext &context ) override;

  protected:
    bool setNewItem( QgsAnnotationItem *item ) override;

  private:
    QgsSymbolSelectorWidget *mSelector = nullptr;
    std::unique_ptr<QgsMarkerSymbol> mSymbol;
    bool mBlockChangedSignal = false;
    std::unique_ptr<QgsAnnotationMarkerItem> mItem;
};


class QgsAnnotationPointTextItemWidget : public QgsAnnotationItemBaseWidget, private Ui_QgsAnnotationPointTextWidgetBase
{
    Q_OBJECT

  public:
    QgsAnnotationPointTextItemWidget( QWidget *parent );
    ~QgsAnnotationPointTextItemWidget() override;
    QgsAnnotationItem *createItem() override;
    void updateItem( QgsAnnotationItem *item ) override;
    void setDockMode( bool dockMode ) override;
    void setContext( const QgsSymbolWidgetContext &context ) override;

  public slots:

    void focusDefaultWidget() override;

  protected:
    bool setNewItem( QgsAnnotationItem *item ) override;

  private:
    void mInsertExpressionButton_clicked();

    QgsTextFormatWidget *mTextFormatWidget = nullptr;
    bool mBlockChangedSignal = false;
    std::unique_ptr<QgsAnnotationPointTextItem> mItem;
};


class QgsAnnotationRectangleTextItemWidget : public QgsAnnotationItemBaseWidget, private Ui_QgsAnnotationRectangleTextWidgetBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsAnnotationRectangleTextItemWidget( QWidget *parent );
    ~QgsAnnotationRectangleTextItemWidget() override;
    QgsAnnotationItem *createItem() override;
    void updateItem( QgsAnnotationItem *item ) override;
    void setDockMode( bool dockMode ) override;
    void setContext( const QgsSymbolWidgetContext &context ) override;
    QgsExpressionContext createExpressionContext() const override;

  public slots:

    void focusDefaultWidget() override;

  protected:
    bool setNewItem( QgsAnnotationItem *item ) override;

  private slots:

    void onWidgetChanged();
    void sizeModeChanged();
    void setWidth();
    void setHeight();

  private:
    void mInsertExpressionButton_clicked();

    QgsTextFormatWidget *mTextFormatWidget = nullptr;
    bool mBlockChangedSignal = false;
    bool mUpdateItemPosition = false;

    std::unique_ptr<QgsAnnotationRectangleTextItem> mItem;
};

class QgsAnnotationLineTextItemWidget : public QgsAnnotationItemBaseWidget, private Ui_QgsAnnotationLineTextWidgetBase
{
    Q_OBJECT

  public:
    QgsAnnotationLineTextItemWidget( QWidget *parent );
    ~QgsAnnotationLineTextItemWidget() override;
    QgsAnnotationItem *createItem() override;
    void updateItem( QgsAnnotationItem *item ) override;
    void setDockMode( bool dockMode ) override;
    void setContext( const QgsSymbolWidgetContext &context ) override;

  public slots:

    void focusDefaultWidget() override;

  protected:
    bool setNewItem( QgsAnnotationItem *item ) override;

  private:
    void mInsertExpressionButton_clicked();

    QgsTextFormatWidget *mTextFormatWidget = nullptr;
    bool mBlockChangedSignal = false;
    std::unique_ptr<QgsAnnotationLineTextItem> mItem;
};


class QgsAnnotationPictureItemWidget : public QgsAnnotationItemBaseWidget, private Ui_QgsAnnotationPictureWidgetBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsAnnotationPictureItemWidget( QWidget *parent );
    ~QgsAnnotationPictureItemWidget() override;
    QgsAnnotationItem *createItem() override;
    void updateItem( QgsAnnotationItem *item ) override;
    void setDockMode( bool dockMode ) override;
    void setContext( const QgsSymbolWidgetContext &context ) override;
    QgsExpressionContext createExpressionContext() const override;
  public slots:

    void focusDefaultWidget() override;

  protected:
    bool setNewItem( QgsAnnotationItem *item ) override;

  private slots:

    void onWidgetChanged();

    void modeChanged( bool checked );
    void sizeModeChanged();
    void setWidth();
    void setHeight();
    void setLockAspectRatio( bool locked );

  private:
    double pictureAspectRatio() const;

    bool mBlockChangedSignal = false;
    bool mUpdateItemPosition = false;
    std::unique_ptr<QgsAnnotationPictureItem> mItem;
};

///@endcond

#endif // QGSANNOTATIONITEMWIDGETIMPL_H
