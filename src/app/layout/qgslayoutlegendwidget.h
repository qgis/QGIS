/***************************************************************************
                         qgslayoutlegendwidget.h
                         -----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLAYOUTLEGENDWIDGET_H
#define QGSLAYOUTLEGENDWIDGET_H

#include "ui_qgslayoutlegendwidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemlegend.h"
#include <QWidget>
#include <QItemDelegate>

/**
 * \ingroup app
 * A widget for setting properties relating to a layout legend.
 */
class QgsLayoutLegendWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutLegendWidgetBase
{
    Q_OBJECT

  public:
    explicit QgsLayoutLegendWidget( QgsLayoutItemLegend *legend );

    //! Updates the legend layers and groups
    void updateLegend();

    QgsLayoutItemLegend *legend() { return mLegend; }
    void setReportTypeString( const QString &string ) override;

  protected:

    bool setNewItem( QgsLayoutItem *item ) override;

  public slots:

    void mWrapCharLineEdit_textChanged( const QString &text );
    void mTitleLineEdit_textChanged( const QString &text );
    void mTitleAlignCombo_currentIndexChanged( int index );
    void mColumnCountSpinBox_valueChanged( int c );
    void mSplitLayerCheckBox_toggled( bool checked );
    void mEqualColumnWidthCheckBox_toggled( bool checked );
    void mSymbolWidthSpinBox_valueChanged( double d );
    void mSymbolHeightSpinBox_valueChanged( double d );
    void mWmsLegendWidthSpinBox_valueChanged( double d );
    void mWmsLegendHeightSpinBox_valueChanged( double d );
    void mTitleSpaceBottomSpinBox_valueChanged( double d );
    void mGroupSpaceSpinBox_valueChanged( double d );
    void mLayerSpaceSpinBox_valueChanged( double d );
    void mSymbolSpaceSpinBox_valueChanged( double d );
    void mIconLabelSpaceSpinBox_valueChanged( double d );
    void mFontColorButton_colorChanged( const QColor &newFontColor );
    void mBoxSpaceSpinBox_valueChanged( double d );
    void mColumnSpaceSpinBox_valueChanged( double d );
    void mLineSpacingSpinBox_valueChanged( double d );
    void mCheckBoxAutoUpdate_stateChanged( int state );
    void composerMapChanged( QgsLayoutItem *item );
    void mCheckboxResizeContents_toggled( bool checked );

    void mRasterStrokeGroupBox_toggled( bool state );
    void mRasterStrokeWidthSpinBox_valueChanged( double d );
    void mRasterStrokeColorButton_colorChanged( const QColor &newColor );

    //item manipulation
    void mMoveDownToolButton_clicked();
    void mMoveUpToolButton_clicked();
    void mRemoveToolButton_clicked();
    void mAddToolButton_clicked();
    void mEditPushButton_clicked();
    void mCountToolButton_clicked( bool checked );
    void mExpressionFilterButton_toggled( bool checked );
    void mFilterByMapToolButton_toggled( bool checked );
    void resetLayerNodeToDefaults();
    void mUpdateAllPushButton_clicked();
    void mAddGroupToolButton_clicked();
    void mLayerExpressionButton_clicked();

    void mFilterLegendByAtlasCheckBox_toggled( bool checked );

    void selectedChanged( const QModelIndex &current, const QModelIndex &previous );

    void setCurrentNodeStyleFromAction();

    void setLegendMapViewData();

  private slots:
    //! Sets GUI according to state of mLegend
    void setGuiElements();

    //! Update the enabling state of the filter by atlas button
    void updateFilterLegendByAtlasButton();

    void mItemTreeView_doubleClicked( const QModelIndex &index );
    void titleFontChanged();
    void groupFontChanged();
    void layerFontChanged();
    void itemFontChanged();

  private:
    QgsLayoutLegendWidget() = delete;
    void blockAllSignals( bool b );

    QPointer< QgsLayoutItemLegend > mLegend;

    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;
};


class QgsLayoutLegendMenuProvider : public QgsLayerTreeViewMenuProvider
{

  public:
    QgsLayoutLegendMenuProvider( QgsLayerTreeView *view, QgsLayoutLegendWidget *w );

    QMenu *createContextMenu() override;

  protected:
    QgsLayerTreeView *mView = nullptr;
    QgsLayoutLegendWidget *mWidget = nullptr;
};


#endif //QGSLAYOUTLEGENDWIDGET_H

