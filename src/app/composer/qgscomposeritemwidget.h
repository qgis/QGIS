/***************************************************************************
                         qgscomposeritemwidget.h
                         -------------------------
    begin                : August 2008
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

#ifndef QGSCOMPOSERITEMWIDGET_H
#define QGSCOMPOSERITEMWIDGET_H

#include "ui_qgscomposeritemwidgetbase.h"
#include "qgscomposeritem.h"

class QgsComposerItem;

/**A class to enter generic properties for composer items (e.g. background, outline, frame).
 This widget can be embedded into other item widgets*/
class QgsComposerItemWidget: public QWidget, private Ui::QgsComposerItemWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerItemWidget( QWidget* parent, QgsComposerItem* item );
    ~QgsComposerItemWidget();

    /**A combination of upper/middle/lower and left/middle/right*/
    QgsComposerItem::ItemPositionMode positionMode() const;


  public slots:
    void on_mFrameColorButton_clicked();
    /** Set the frame color
     * @note added in 1.9
     */
    void on_mFrameColorButton_colorChanged( const QColor& newFrameColor );
    void on_mBackgroundColorButton_clicked();
    /** Set the background color
     * @note added in 1.9
     */
    void on_mBackgroundColorButton_colorChanged( const QColor& newBackgroundColor );
//    void on_mTransparencySlider_valueChanged( int value );
//    void on_mTransparencySpinBox_valueChanged( int value );
    void on_mOutlineWidthSpinBox_valueChanged( double d );
    void on_mFrameGroupBox_toggled( bool state );
    void on_mBackgroundGroupBox_toggled( bool state );
    void on_mItemIdLineEdit_editingFinished();

    //adjust coordinates in line edits
    void on_mXLineEdit_editingFinished() { changeItemPosition(); }
    void on_mYLineEdit_editingFinished() { changeItemPosition(); }
    void on_mWidthLineEdit_editingFinished() { changeItemPosition(); }
    void on_mHeightLineEdit_editingFinished() { changeItemPosition(); }

    void on_mUpperLeftCheckBox_stateChanged( int state ) { Q_UNUSED( state ); changeItemPosition(); }
    void on_mUpperMiddleCheckBox_stateChanged( int state ) { Q_UNUSED( state ); changeItemPosition(); }
    void on_mUpperRightCheckBox_stateChanged( int state ) { Q_UNUSED( state ); changeItemPosition(); }
    void on_mMiddleLeftCheckBox_stateChanged( int state ) { Q_UNUSED( state ); changeItemPosition(); }
    void on_mMiddleCheckBox_stateChanged( int state ) { Q_UNUSED( state ); changeItemPosition(); }
    void on_mMiddleRightCheckBox_stateChanged( int state ) { Q_UNUSED( state ); changeItemPosition(); }
    void on_mLowerLeftCheckBox_stateChanged( int state ) { Q_UNUSED( state ); changeItemPosition(); }
    void on_mLowerMiddleCheckBox_stateChanged( int state ) { Q_UNUSED( state ); changeItemPosition(); }
    void on_mLowerRightCheckBox_stateChanged( int state ) { Q_UNUSED( state ); changeItemPosition(); }

    void on_mBlendModeCombo_currentIndexChanged( int index );
    void on_mTransparencySlider_valueChanged( int value );

    void setValuesForGuiElements();
    void setValuesForGuiPositionElements();

  private:
    QgsComposerItemWidget();
//    void changeItemTransparency( int value );
    void changeItemPosition();

    QgsComposerItem* mItem;
};

#endif //QGSCOMPOSERITEMWIDGET_H
