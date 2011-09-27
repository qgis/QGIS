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

class QgsComposerItem;

/**A class to enter generic properties for composer items (e.g. background, outline, frame).
 This widget can be embedded into other item widgets*/
class QgsComposerItemWidget: public QWidget, private Ui::QgsComposerItemWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerItemWidget( QWidget* parent, QgsComposerItem* item );
    ~QgsComposerItemWidget();

  public slots:
    void on_mFrameColorButton_clicked();
    void on_mBackgroundColorButton_clicked();
    void on_mOpacitySlider_sliderReleased();
    void on_mOutlineWidthSpinBox_valueChanged( double d );
    void on_mFrameCheckBox_stateChanged( int state );
    void on_mPositionButton_clicked();
    void on_mItemIdLineEdit_textChanged( const QString& text );

  private:
    QgsComposerItemWidget();
    void setValuesForGuiElements();

    QgsComposerItem* mItem;
};

#endif //QGSCOMPOSERITEMWIDGET_H
