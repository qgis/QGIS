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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutlegendwidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemlegend.h"
#include <QWidget>
#include <QItemDelegate>

///@cond PRIVATE

/**
 * \ingroup gui
 * \brief A widget for setting properties relating to a layout legend.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutLegendWidget: public QgsLayoutItemBaseWidget, public QgsExpressionContextGenerator, private Ui::QgsLayoutLegendWidgetBase
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutLegendWidget( QgsLayoutItemLegend *legend, QgsMapCanvas *mapCanvas );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;
    void setDesignerInterface( QgsLayoutDesignerInterface *iface ) override;
    //! Updates the legend layers and groups
    void updateLegend();

    //! Returns the legend item associated to this widget
    QgsLayoutItemLegend *legend() { return mLegend; }
    void setReportTypeString( const QString &string ) override;
    QgsExpressionContext createExpressionContext() const override;
  public slots:
    //! Reset a layer node to the default settings
    void resetLayerNodeToDefaults();

    /**
     * Sets the current node style from the data of the action which invokes this slot
     * \see QgsLayoutLegendMenuProvider::createContextMenu
     */
    void setCurrentNodeStyleFromAction();

  protected:

    bool setNewItem( QgsLayoutItem *item ) override;

  private slots:

    void mWrapCharLineEdit_textChanged( const QString &text );
    void mTitleLineEdit_textChanged( const QString &text );
    void mColumnCountSpinBox_valueChanged( int c );
    void mSplitLayerCheckBox_toggled( bool checked );
    void mEqualColumnWidthCheckBox_toggled( bool checked );
    void mSymbolWidthSpinBox_valueChanged( double d );
    void mSymbolHeightSpinBox_valueChanged( double d );
    void mMaxSymbolSizeSpinBox_valueChanged( double d );
    void mMinSymbolSizeSpinBox_valueChanged( double d );
    void mWmsLegendWidthSpinBox_valueChanged( double d );
    void mWmsLegendHeightSpinBox_valueChanged( double d );
    void mTitleSpaceBottomSpinBox_valueChanged( double d );
    void mGroupSpaceSpinBox_valueChanged( double d );
    void mGroupIndentSpinBox_valueChanged( double d );
    void mSubgroupIndentSpinBox_valueChanged( double d );
    void mLayerSpaceSpinBox_valueChanged( double d );
    void mSymbolSpaceSpinBox_valueChanged( double d );
    void mIconLabelSpaceSpinBox_valueChanged( double d );
    void mBoxSpaceSpinBox_valueChanged( double d );
    void mColumnSpaceSpinBox_valueChanged( double d );
    void mCheckBoxAutoUpdate_stateChanged( int state, bool userTriggered = true );
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
    void mFilterByMapCheckBox_toggled( bool checked );
    void mUpdateAllPushButton_clicked();
    void mAddGroupToolButton_clicked();
    void mLayerExpressionButton_clicked();

    void mFilterLegendByAtlasCheckBox_toggled( bool checked );

    void selectedChanged( const QModelIndex &current, const QModelIndex &previous );

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

    void titleAlignmentChanged();
    void groupAlignmentChanged();
    void subgroupAlignmentChanged();
    void itemAlignmentChanged();
    void arrangementChanged();

    void spaceBelowSubGroupHeadingChanged( double space );
    void spaceBelowGroupHeadingChanged( double space );

    void spaceGroupSideChanged( double space );
    void spaceSubGroupSideChanged( double space );

    void spaceSymbolSideChanged( double space );

  private:
    QgsLayoutLegendWidget() = delete;
    void blockAllSignals( bool b );

    QPointer< QgsLayoutItemLegend > mLegend;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;
};

/**
 * \ingroup gui
 * \brief Layout legend menu provider
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutLegendMenuProvider : public QgsLayerTreeViewMenuProvider
{

  public:
    //! constructor
    QgsLayoutLegendMenuProvider( QgsLayerTreeView *view, QgsLayoutLegendWidget *w );

    QMenu *createContextMenu() override;

  protected:
    QgsLayerTreeView *mView = nullptr;
    QgsLayoutLegendWidget *mWidget = nullptr;
};

#include "ui_qgslayoutlegendnodewidgetbase.h"

/**
 * \ingroup gui
 * \brief A widget for properties relating to a node in a layout legend.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsLayoutLegendNodeWidget: public QgsPanelWidget, private Ui::QgsLayoutLegendNodeWidgetBase
{
    Q_OBJECT

  public:

    QgsLayoutLegendNodeWidget( QgsLayoutItemLegend *legend, QgsLayerTreeNode *node, QgsLayerTreeModelLegendNode *legendNode, int originalLegendNodeIndex, QWidget *parent = nullptr );

    void setDockMode( bool dockMode ) override;

  private slots:

    void labelChanged();
    void patchChanged();
    void insertExpression();
    void sizeChanged( double );
    void customSymbolChanged();
    void colorRampLegendChanged();
    void columnBreakToggled( bool checked );
    void columnSplitChanged();

  private:

    QgsLayoutItemLegend *mLegend = nullptr;
    QgsLayerTreeNode *mNode = nullptr;
    QgsLayerTreeLayer *mLayer = nullptr;
    QgsLayerTreeModelLegendNode *mLegendNode = nullptr;
    int mOriginalLegendNodeIndex = -1;

};

///@endcond

#endif //QGSLAYOUTLEGENDWIDGET_H

