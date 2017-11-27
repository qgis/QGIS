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
#include "qgscomposeritemwidget.h"

class QgsComposerLabel;

/**
 * \ingroup app
  * A widget to enter text, font size, box yes/no for composer labels
  */
class QgsComposerLabelWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerLabelWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsComposerLabelWidget( QgsComposerLabel *label );

  public slots:
    void mHtmlCheckBox_stateChanged( int i );
    void mTextEdit_textChanged();
    void mInsertExpressionButton_clicked();
    void mMarginXDoubleSpinBox_valueChanged( double d );
    void mMarginYDoubleSpinBox_valueChanged( double d );
    void mFontColorButton_colorChanged( const QColor &newLabelColor );
    void mCenterRadioButton_clicked();
    void mLeftRadioButton_clicked();
    void mRightRadioButton_clicked();
    void mTopRadioButton_clicked();
    void mBottomRadioButton_clicked();
    void mMiddleRadioButton_clicked();

  private slots:
    void setGuiElementValues();
    void fontChanged();
    void justifyClicked();

  private:
    QgsComposerLabel *mComposerLabel = nullptr;

    void blockAllSignals( bool block );
};

#endif //QGSCOMPOSERLABELWIDGET
