/***************************************************************************
    qgscategorized3drendererwidget.h
    ---------------------
    begin                : November 2025
    copyright            : (C) 2025 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCATEGORIZED3DRENDERERWIDGET_H
#define QGSCATEGORIZED3DRENDERERWIDGET_H

#include "ui_qgscategorized3drendererwidget.h"

#include "qgis_gui.h"
#include "qgscategorized3drenderer.h"
#include "qgspanelwidget.h"
#include "qgsrendererwidget.h"
#include "qgstemplatedcategorizedrendererwidget_p.h"

class Qgs3DSymbolWidget;
class QgsSingleSymbol3DRendererWidget;

///@cond PRIVATE

class QgsCategorized3DRendererModel : public QgsTemplatedCategorizedRendererModel<QgsCategorized3DRenderer>
{
    Q_OBJECT
  public:
    QgsCategorized3DRendererModel( QObject *parent = nullptr, QScreen *screen = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override;

  signals:
    void rowsMoved();

  protected:
    void onRowsMoved() override;
};

///@endcond

/**
 * \class QgsCategorized3DRendererWidget
 * \brief A widget for configuring a QgsCategorized3DRenderer.
 */
class QgsCategorized3DRendererWidget : public QgsPanelWidget, private Ui::QgsCategorized3DRendererWidget
{
    Q_OBJECT
  public:
    QgsCategorized3DRendererWidget( QWidget *parent = nullptr );
    ~QgsCategorized3DRendererWidget() override;

    void setLayer( QgsVectorLayer *layer );

    QgsCategorized3DRenderer *renderer() const { return mRenderer.get(); }

  public slots:
    void categoryColumnChanged( const QString &field );
    void categoriesDoubleClicked( const QModelIndex &idx );
    void addCategory();
    void addCategories();

    /**
     * Applies the color ramp passed on by the color ramp button
     */
    void applyColorRamp();

    void deleteCategories();
    void deleteAllCategories();

    /**
     * Deletes unused categories from the widget which are not used by the layer renderer.
     */
    void deleteUnusedCategories();

    void rowsMoved();

  private slots:

    void updateSymbolsFromWidget( QgsSingleSymbol3DRendererWidget *widget );
    void updateSymbolsFromButton();

    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

  protected:
    void updateUiFromRenderer();

    //! Returns row index for the currently selected category (-1 if on no selection)
    int currentCategoryRow();

    //! Returns a list of indexes for the categories under selection
    QList<int> selectedCategories();

    void changeCategorySymbol();
    //! Applies current symbol to selected categories, or to all categories if none is selected
    void applyChangeToSymbol();

    Qgs3DCategoryList selectedCategoryList() const;
    void keyPressEvent( QKeyEvent *event ) override;

  protected:
    std::unique_ptr<QgsCategorized3DRenderer> mRenderer;
    std::unique_ptr<QgsAbstract3DSymbol> mCategorizedSymbol;

    QgsCategorized3DRendererModel *mModel = nullptr;

  private:
    QgsVectorLayer *mLayer = nullptr;
    QString mOldClassificationAttribute;
    Qgs3DCategoryList mCopyBuffer;
    bool mUpdatingSymbolButton = false;
};

#endif // QGSCATEGORIZED3DRENDERERWIDGET_H
