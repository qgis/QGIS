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
#include <QWidget>
#include <QItemDelegate>

class QgsComposerLegend;

class QgsComposerLegendWidgetStyleDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    QgsComposerLegendWidgetStyleDelegate( QObject *parent = 0 );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

/** \ingroup MapComposer
 * A widget for setting properties relating to a composer legend.
 */
class QgsComposerLegendWidget: public QWidget, private Ui::QgsComposerLegendWidgetBase
{
    Q_OBJECT

  public:
    QgsComposerLegendWidget( QgsComposerLegend* legend );
    ~QgsComposerLegendWidget();

    /**Updates the legend layers and groups*/
    void updateLegend();

  public slots:

    void on_mWrapCharLineEdit_textChanged( const QString& text );
    void on_mTitleLineEdit_textChanged( const QString& text );
    void on_mColumnCountSpinBox_valueChanged( int c );
    void on_mSplitLayerCheckBox_toggled( bool checked );
    void on_mEqualColumnWidthCheckBox_toggled( bool checked );
    void on_mSymbolWidthSpinBox_valueChanged( double d );
    void on_mSymbolHeightSpinBox_valueChanged( double d );
    void on_mGroupSpaceSpinBox_valueChanged( double d );
    void on_mLayerSpaceSpinBox_valueChanged( double d );
    void on_mSymbolSpaceSpinBox_valueChanged( double d );
    void on_mIconLabelSpaceSpinBox_valueChanged( double d );
    void on_mTitleFontButton_clicked();
    void on_mGroupFontButton_clicked();
    void on_mLayerFontButton_clicked();
    void on_mItemFontButton_clicked();
    void on_mFontColorPushButton_clicked();
    void on_mBoxSpaceSpinBox_valueChanged( double d );
    void on_mColumnSpaceSpinBox_valueChanged( double d );
    void on_mCheckBoxAutoUpdate_stateChanged( int state );
    void on_mMapComboBox_currentIndexChanged( int index );

    //item manipulation
    void on_mMoveDownToolButton_clicked();
    void on_mMoveUpToolButton_clicked();
    void on_mRemoveToolButton_clicked();
    void on_mAddToolButton_clicked();
    void on_mEditPushButton_clicked();
    void on_mCountToolButton_clicked( bool checked );
    void on_mUpdatePushButton_clicked();
    void on_mUpdateAllPushButton_clicked();
    void on_mAddGroupToolButton_clicked();

    void selectedChanged( const QModelIndex & current, const QModelIndex & previous );

  protected:
    void showEvent( QShowEvent * event );

  private slots:
    /**Sets GUI according to state of mLegend*/
    void setGuiElements();

  private:
    QgsComposerLegendWidget();
    void blockAllSignals( bool b );
    void refreshMapComboBox();


    QgsComposerLegend* mLegend;
};

#endif
