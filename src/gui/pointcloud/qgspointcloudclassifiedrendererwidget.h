/***************************************************************************
    qgspointcloudclassifiedrendererwidget.h
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDCLASSIFIEDRENDERERWIDGET_H
#define QGSPOINTCLOUDCLASSIFIEDRENDERERWIDGET_H

#include "qgspointcloudrendererwidget.h"
#include "qgspointcloudclassifiedrenderer.h"
#include "ui_qgspointcloudclassifiedrendererwidgetbase.h"
#include "qgis_gui.h"
#include "qgsproxystyle.h"
#include "qgspointcloudattribute.h"

class QgsPointCloudLayer;
class QgsStyle;
class QLineEdit;
class QgsPointCloudClassifiedRenderer;
class QgsPointCloud3DLayer3DRenderer;


#ifndef SIP_RUN
///@cond PRIVATE

class GUI_EXPORT QgsPointCloudClassifiedRendererModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    QgsPointCloudClassifiedRendererModel( QObject *parent = nullptr );
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    Qt::DropActions supportedDropActions() const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    void setRendererCategories( const QgsPointCloudCategoryList &categories );

    void addCategory( const QgsPointCloudCategory &cat );
    QgsPointCloudCategory category( const QModelIndex &index );
    void deleteRows( QList<int> rows );
    void removeAllRows();

    QgsPointCloudCategoryList categories() const { return mCategories; }

    void setCategoryColor( int row, const QColor &color );
    //! Updates the model with percentage of points per category
    void updateCategoriesPercentages( const QMap< int, float > &percentages ) { mPercentages = percentages; };

  signals:
    void categoriesChanged();

  private:
    QgsPointCloudCategoryList mCategories;
    QMap< int, float > mPercentages;
    QString mMimeFormat;
};

/**
 * \ingroup gui
 * \brief View style which shows drop indicator line between items
 */
class QgsPointCloudClassifiedRendererViewStyle: public QgsProxyStyle
{
    Q_OBJECT

  public:
    explicit QgsPointCloudClassifiedRendererViewStyle( QWidget *parent );

    void drawPrimitive( PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr ) const override;
};


class GUI_EXPORT QgsPointCloudClassifiedRendererWidget: public QgsPointCloudRendererWidget, private Ui::QgsPointCloudClassifiedRendererWidgetBase
{
    Q_OBJECT

  public:
    QgsPointCloudClassifiedRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style );
    static QgsPointCloudRendererWidget *create( QgsPointCloudLayer *layer, QgsStyle *style, QgsPointCloudRenderer * );

    QgsPointCloudRenderer *renderer() override;
    QgsPointCloudCategoryList categoriesList();
    QString attribute();

    /**
     * Sets the selected attribute and categories based on a 2D renderer.
     * If the renderer is not a QgsPointCloudClassifiedRenderer, the widget is reinitialized
     */
    void setFromRenderer( const QgsPointCloudRenderer *r );
    void setFromCategories( QgsPointCloudCategoryList categories, const QString &attribute );

  private slots:

    /**
     * Gets the available classes for the selected attribute from the layer and adds any categories that are missing.
     * Categories for the Classification attribute get a default color and name
     */
    void addCategories();
    void emitWidgetChanged();
    void categoriesDoubleClicked( const QModelIndex &idx );
    void addCategory();
    void deleteCategories();
    void deleteAllCategories();
    void attributeChanged();
  private:
    //! Sets default category and available classes
    void initialize();
    void changeCategorySymbol();
    //! Returns a list of indexes for the categories under selection
    QList<int> selectedCategories();
    //! Returns row index for the currently selected category (-1 if on no selection)
    int currentCategoryRow();
    //! Updates the model with percentage of points per category
    void updateCategoriesPercentages();

    QgsPointCloudClassifiedRendererModel *mModel = nullptr;
    bool mBlockChangedSignal = false;
};

///@endcond
#endif

#endif // QGSPOINTCLOUDCLASSIFIEDRENDERERWIDGET_H
