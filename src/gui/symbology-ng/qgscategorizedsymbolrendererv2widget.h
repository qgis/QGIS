/***************************************************************************
    qgscategorizedsymbolrendererv2widget.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCATEGORIZEDSYMBOLRENDERERV2WIDGET_H
#define QGSCATEGORIZEDSYMBOLRENDERERV2WIDGET_H

#include "qgscategorizedsymbolrendererv2.h"
#include "qgsrendererv2widget.h"
#include <QStandardItem>
#include <QProxyStyle>

class QgsCategorizedSymbolRendererV2;
class QgsRendererCategoryV2;

#include "ui_qgscategorizedsymbolrendererv2widget.h"

class GUI_EXPORT QgsCategorizedSymbolRendererV2Model : public QAbstractItemModel
{
    Q_OBJECT
  public:
    QgsCategorizedSymbolRendererV2Model( QObject * parent = 0 );
    Qt::ItemFlags flags( const QModelIndex & index ) const;
    Qt::DropActions supportedDropActions() const;
    QVariant data( const QModelIndex &index, int role ) const;
    bool setData( const QModelIndex & index, const QVariant & value, int role );
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex & = QModelIndex() ) const;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &index ) const;
    QStringList mimeTypes() const;
    QMimeData *mimeData( const QModelIndexList &indexes ) const;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    void setRenderer( QgsCategorizedSymbolRendererV2* renderer );

    void addCategory( const QgsRendererCategoryV2 &cat );
    void deleteRows( QList<int> rows );
    void removeAllRows( );
    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );

  signals:
    void rowsMoved();

  private:
    QgsCategorizedSymbolRendererV2* mRenderer;
    QString mMimeFormat;
};

// View style which shows drop indicator line between items
class QgsCategorizedSymbolRendererV2ViewStyle: public QProxyStyle
{
  public:
    QgsCategorizedSymbolRendererV2ViewStyle( QStyle* style = 0 );

    void drawPrimitive( PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0 ) const;
};

class GUI_EXPORT QgsCategorizedSymbolRendererV2Widget : public QgsRendererV2Widget, private Ui::QgsCategorizedSymbolRendererV2Widget
{
    Q_OBJECT
  public:
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    QgsCategorizedSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsCategorizedSymbolRendererV2Widget();

    virtual QgsFeatureRendererV2* renderer();

  public slots:
    void changeCategorizedSymbol();
    void categoryColumnChanged();
    void categoriesDoubleClicked( const QModelIndex & idx );
    void addCategory();
    void addCategories();
    void deleteCategories();
    void deleteAllCategories();

    void rotationFieldChanged( QString fldName );
    void sizeScaleFieldChanged( QString fldName );
    void scaleMethodChanged( QgsSymbolV2::ScaleMethod scaleMethod );

    void showSymbolLevels();

    void rowsMoved();

  protected:

    void updateUiFromRenderer();

    void updateCategorizedSymbolIcon();

    // Called by virtual refreshSymbolView()
    void populateCategories();

    //! populate column combo
    void populateColumns();

    //! return row index for the currently selected category (-1 if on no selection)
    int currentCategoryRow();

    //! return a list of indexes for the categories unders selection
    QList<int> selectedCategories();

    //! change the selected symbols alone for the change button, if there is a selection
    void changeSelectedSymbols();

    void changeCategorySymbol();

    QList<QgsSymbolV2*> selectedSymbols();
    void refreshSymbolView() { populateCategories(); }

  protected:
    QgsCategorizedSymbolRendererV2* mRenderer;

    QgsSymbolV2* mCategorizedSymbol;

    QgsRendererV2DataDefinedMenus* mDataDefinedMenus;

    QgsCategorizedSymbolRendererV2Model* mModel;

  private:
    QString mOldClassificationAttribute;
};

#endif // QGSCATEGORIZEDSYMBOLRENDERERV2WIDGET_H
