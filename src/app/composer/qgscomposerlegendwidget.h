/***************************************************************************
                         qgscomposerlegendwidget.h
                         -------------------------
    begin                : July 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERLEGENDWIDGET_H
#define QGSCOMPOSERLEGENDWIDGET_H

#include "ui_qgscomposerlegendwidgetbase.h"
#include "qgscomposeritemwidget.h"
#include <QWidget>
#include <QItemDelegate>

class QgsComposerLegend;


/**
 * \ingroup app
 * A widget for setting properties relating to a composer legend.
 */
class QgsComposerLegendWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerLegendWidgetBase
{
    Q_OBJECT

  public:
    explicit QgsComposerLegendWidget( QgsComposerLegend *legend );

    //! Updates the legend layers and groups
    void updateLegend();

    QgsComposerLegend *legend() { return mLegend; }

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
    void composerMapChanged( QgsComposerItem *item );
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

    void mFilterLegendByAtlasCheckBox_toggled( bool checked );

    void selectedChanged( const QModelIndex &current, const QModelIndex &previous );

    void setCurrentNodeStyleFromAction();

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
    QgsComposerLegendWidget();
    void blockAllSignals( bool b );

    QgsComposerLegend *mLegend = nullptr;
};


class QgsComposerLegendMenuProvider : public QgsLayerTreeViewMenuProvider
{

  public:
    QgsComposerLegendMenuProvider( QgsLayerTreeView *view, QgsComposerLegendWidget *w );

    virtual QMenu *createContextMenu() override;

  protected:
    QgsLayerTreeView *mView = nullptr;
    QgsComposerLegendWidget *mWidget = nullptr;
};


#endif

