/***************************************************************************
                         qgsitempositiondialog.h
                         -------------------------
    begin                : October 2008
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

#ifndef QGSITEMPOSITIONDIALOG_H
#define QGSITEMPOSITIONDIALOG_H

#include "ui_qgsitempositiondialogbase.h"
#include "qgscomposeritem.h"
class QgsPoint;

/**A dialog to set the position of upper/middle/lower left/middle/lower point of an item*/
class QgsItemPositionDialog: public QDialog, private Ui::QgsItemPositionDialogBase
{
    Q_OBJECT
  public:
    QgsItemPositionDialog( QgsComposerItem* item, QWidget* parent = 0 );
    ~QgsItemPositionDialog();

    /**Get selected x- and y-coordinate as point. Returns 0 in case of success*/
    int position( QgsPoint& point ) const;
    /**A combination of upper/middle/lower and left/middle/right*/
    QgsComposerItem::ItemPositionMode positionMode() const;

  public slots:

    void on_mCloseButton_clicked();
    void on_mSetPositionButton_clicked();

    //adjust coordinates in line edits
    void on_mUpperLeftCheckBox_stateChanged( int state );
    void on_mUpperMiddleCheckBox_stateChanged( int state );
    void on_mUpperRightCheckBox_stateChanged( int state );
    void on_mMiddleLeftCheckBox_stateChanged( int state );
    void on_mMiddleCheckBox_stateChanged( int state );
    void on_mMiddleRightCheckBox_stateChanged( int state );
    void on_mLowerLeftCheckBox_stateChanged( int state );
    void on_mLowerMiddleCheckBox_stateChanged( int state );
    void on_mLowerRightCheckBox_stateChanged( int state );

  private:
    QgsComposerItem* mItem;

    //default constructor forbidden
    QgsItemPositionDialog();
};

#endif
