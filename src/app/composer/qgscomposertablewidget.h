/***************************************************************************
                         qgscomposertablewidget.h
                         ------------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERTABLEWIDGET_H
#define QGSCOMPOSERTABLEWIDGET_H

#include "ui_qgscomposertablewidgetbase.h"
#include "qgscomposeritemwidget.h"

class QgsComposerAttributeTable;

class QgsComposerTableWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerTableWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerTableWidget( QgsComposerAttributeTable* table );
    ~QgsComposerTableWidget();

  protected:
    void showEvent( QShowEvent * event ) override;

  private:
    QgsComposerAttributeTable* mComposerTable;

    /**Blocks / unblocks the signals of all GUI elements*/
    void blockAllSignals( bool b );
    void refreshMapComboBox();

  private slots:
    void on_mRefreshPushButton_clicked();
    void on_mAttributesPushButton_clicked();
    void on_mComposerMapComboBox_activated( int index );
    void on_mMaximumColumnsSpinBox_valueChanged( int i );
    void on_mMarginSpinBox_valueChanged( double d );
    void on_mGridStrokeWidthSpinBox_valueChanged( double d );
    void on_mGridColorButton_colorChanged( const QColor& newColor );
    void on_mHeaderFontPushButton_clicked();
    void on_mHeaderFontColorButton_colorChanged( const QColor& newColor );
    void on_mContentFontPushButton_clicked();
    void on_mContentFontColorButton_colorChanged( const QColor& newColor );
    void on_mShowGridGroupCheckBox_toggled( bool state );
    void on_mShowOnlyVisibleFeaturesCheckBox_stateChanged( int state );
    void on_mFeatureFilterCheckBox_stateChanged( int state );
    void on_mFeatureFilterEdit_editingFinished();
    void on_mFeatureFilterButton_clicked();
    void on_mHeaderHAlignmentComboBox_currentIndexChanged( int index );
    void changeLayer( QgsMapLayer* layer );

    /**Inserts a new maximum number of features into the spin box (without the spinbox emitting a signal)*/
    void setMaximumNumberOfFeatures( int n );

    /**Sets the GUI elements to the values of mComposerTable*/
    void updateGuiElements();

};

#endif // QGSCOMPOSERTABLEWIDGET_H
