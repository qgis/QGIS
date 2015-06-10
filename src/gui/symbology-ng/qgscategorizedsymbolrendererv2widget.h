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
    Qt::ItemFlags flags( const QModelIndex & index ) const override;
    Qt::DropActions supportedDropActions() const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex & index, const QVariant & value, int role ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    void setRenderer( QgsCategorizedSymbolRendererV2* renderer );

    void addCategory( const QgsRendererCategoryV2 &cat );
    QgsRendererCategoryV2 category( const QModelIndex &index );
    void deleteRows( QList<int> rows );
    void removeAllRows();
    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override;
    void updateSymbology();

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

    void drawPrimitive( PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0 ) const override;
};

class GUI_EXPORT QgsCategorizedSymbolRendererV2Widget : public QgsRendererV2Widget, private Ui::QgsCategorizedSymbolRendererV2Widget
{
    Q_OBJECT
  public:
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    QgsCategorizedSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsCategorizedSymbolRendererV2Widget();

    virtual QgsFeatureRendererV2* renderer() override;

    /** Replaces category symbols with the symbols from a style that have a matching
     * name.
     * @param style style containing symbols to match with
     * @return number of symbols matched
     * @see matchToSymbolsFromLibrary
     * @see matchToSymbolsFromXml
     * @note added in QGIS 2.9
     */
    int matchToSymbols( QgsStyleV2* style );

  public slots:
    void changeCategorizedSymbol();
    void categoryColumnChanged( QString field );
    void categoriesDoubleClicked( const QModelIndex & idx );
    void addCategory();
    void addCategories();
    void applyColorRamp();
    void deleteCategories();
    void deleteAllCategories();

    void rotationFieldChanged( QString fldName );
    void sizeScaleFieldChanged( QString fldName );
    void scaleMethodChanged( QgsSymbolV2::ScaleMethod scaleMethod );

    void showSymbolLevels();

    void rowsMoved();

    /** Replaces category symbols with the symbols from the users' symbol library that have a
     * matching name.
     * @see matchToSymbolsFromXml
     * @see matchToSymbols
     * @note added in QGIS 2.9
     */
    void matchToSymbolsFromLibrary();

    /** Prompts for selection of an xml file, then replaces category symbols with the symbols
     * from the XML file with a matching name.
     * @see matchToSymbolsFromLibrary
     * @see matchToSymbols
     * @note added in QGIS 2.9
     */
    void matchToSymbolsFromXml();

  protected:

    void updateUiFromRenderer();

    void updateCategorizedSymbolIcon();

    // Called by virtual refreshSymbolView()
    void populateCategories();

    //! return row index for the currently selected category (-1 if on no selection)
    int currentCategoryRow();

    //! return a list of indexes for the categories unders selection
    QList<int> selectedCategories();

    //! change the selected symbols alone for the change button, if there is a selection
    void changeSelectedSymbols();

    void changeCategorySymbol();

    QgsVectorColorRampV2* getColorRamp();

    QList<QgsSymbolV2*> selectedSymbols() override;
    QgsCategoryList selectedCategoryList();
    void refreshSymbolView() override { populateCategories(); }
    void keyPressEvent( QKeyEvent* event ) override;

  protected:
    QgsCategorizedSymbolRendererV2* mRenderer;

    QgsSymbolV2* mCategorizedSymbol;

    QgsRendererV2DataDefinedMenus* mDataDefinedMenus;

    QgsCategorizedSymbolRendererV2Model* mModel;

  private:
    QString mOldClassificationAttribute;
    QgsCategoryList mCopyBuffer;
};

#endif // QGSCATEGORIZEDSYMBOLRENDERERV2WIDGET_H
