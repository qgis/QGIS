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

class QgsComposerAttributeTable;

class QgsComposerTableWidget: public QWidget, private Ui::QgsComposerTableWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerTableWidget( QgsComposerAttributeTable* table );
    ~QgsComposerTableWidget();

  private:
    QgsComposerAttributeTable* mComposerTable;

    /**Sets the GUI elements to the values of mComposerTable*/
    void updateGuiElements();
    /**Blocks / unblocks the signals of all GUI elements*/
    void blockAllSignals( bool b );

  private slots:
    void on_mLayerComboBox_currentIndexChanged( int index );
    void on_mAttributesPushButton_clicked();
    void on_mComposerMapComboBox_activated( int index );
    void on_mMaximumColumnsSpinBox_valueChanged( int i );
    void on_mMarginSpinBox_valueChanged( double d );
    void on_mGridStrokeWidthSpinBox_valueChanged( double d );
    void on_mGridColorButton_clicked();
    void on_mHeaderFontPushButton_clicked();
    void on_mContentFontPushButton_clicked();
    void on_mShowGridCheckBox_stateChanged( int state );
    void on_mShowOnlyVisibleFeaturesCheckBox_stateChanged( int state );

    /**Inserts a new maximum number of features into the spin box (without the spinbox emitting a signal)*/
    void setMaximumNumberOfFeatures( int n );
};

#endif // QGSCOMPOSERTABLEWIDGET_H
