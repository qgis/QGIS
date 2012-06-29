/***************************************************************************
                         qgscomposerlabelwidget.h
                         ------------------------
    begin                : June 10, 2008
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

#ifndef QGSCOMPOSERLABELWIDGET
#define QGSCOMPOSERLABELWIDGET

#include "ui_qgscomposerlabelwidgetbase.h"

class QgsComposerLabel;

/** \ingroup MapComposer
  * A widget to enter text, font size, box yes/no for composer labels
  */
class QgsComposerLabelWidget: public QWidget, private Ui::QgsComposerLabelWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerLabelWidget( QgsComposerLabel* label );

  public slots:
    void on_mTextEdit_textChanged();
    void on_mFontButton_clicked();
    void on_mMarginDoubleSpinBox_valueChanged( double d );
    void on_mFontColorButton_clicked();
    void on_mCenterRadioButton_clicked();
    void on_mLeftRadioButton_clicked();
    void on_mRightRadioButton_clicked();
    void on_mTopRadioButton_clicked();
    void on_mBottomRadioButton_clicked();
    void on_mMiddleRadioButton_clicked();
    void on_mLabelIdLineEdit_textChanged( const QString& text );
    void on_mRotationSpinBox_valueChanged( double v );

  private slots:
    void setGuiElementValues();

  private:
    QgsComposerLabel* mComposerLabel;

    void blockAllSignals( bool block );
};

#endif //QGSCOMPOSERLABELWIDGET
