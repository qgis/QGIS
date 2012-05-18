/***************************************************************************
    qgscategorizedsymbolrendererv2widget.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
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

class QgsCategorizedSymbolRendererV2;
class QgsRendererCategoryV2;

#include "ui_qgscategorizedsymbolrendererv2widget.h"

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
    void addCategories();
    void deleteCategory();
    void deleteAllCategories();
    void changeCurrentValue( QStandardItem * item );

    void rotationFieldChanged( QString fldName );
    void sizeScaleFieldChanged( QString fldName );

    void showSymbolLevels();

  protected:

    void updateUiFromRenderer();

    void updateCategorizedSymbolIcon();

    //! populate categories view
    void populateCategories();

    //! populate column combo
    void populateColumns();

    void populateColorRamps();

    void addCategory( const QgsRendererCategoryV2& cat );

    //! return row index for the currently selected category (-1 if on no selection)
    int currentCategoryRow();

    //! return key for the currently selected category
    QVariant currentCategory();

    void changeCategorySymbol();

    QList<QgsSymbolV2*> selectedSymbols();
    void refreshSymbolView() { populateCategories(); }

  protected slots:
    void addCategory();

  protected:
    QgsCategorizedSymbolRendererV2* mRenderer;

    QgsSymbolV2* mCategorizedSymbol;

    QgsRendererV2DataDefinedMenus* mDataDefinedMenus;

  private:
    QString mOldClassificationAttribute;
};



#endif // QGSCATEGORIZEDSYMBOLRENDERERV2WIDGET_H
