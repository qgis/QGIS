/***************************************************************************
                         qgscomposerlabelwidget.cpp
                         --------------------------
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

#include "qgscomposerlabelwidget.h"
#include "qgscomposerlabel.h"
#include "qgscomposeritemwidget.h"
#include <QFontDialog>
#include <QWidget>

QgsComposerLabelWidget::QgsComposerLabelWidget( QgsComposerLabel* label ): QWidget(), mComposerLabel( label )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, label );
  gridLayout->addWidget( itemPropertiesWidget, 5, 0, 1, 2 );

  if ( mComposerLabel )
  {
    mTextEdit->setText( mComposerLabel->text() );
    mMarginDoubleSpinBox->setValue( mComposerLabel->margin() );
  }
}

void QgsComposerLabelWidget::on_mTextEdit_textChanged()
{
  if ( mComposerLabel )
  {
    mComposerLabel->setText( mTextEdit->toPlainText() );
    mComposerLabel->update();
  }
}

void QgsComposerLabelWidget::on_mFontButton_clicked()
{
  if ( mComposerLabel )
  {
    bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && !defined(__LP64__) 
    // Native Mac dialog works only for 64 bit Cocoa (observed in Qt 4.5.2, probably a Qt bug) 
    QFont newFont = QFontDialog::getFont( &ok, mComposerLabel->font(), this, QString(), QFontDialog::DontUseNativeDialog ); 
#else
    QFont newFont = QFontDialog::getFont( &ok, mComposerLabel->font(), this );
#endif
    if ( ok )
    {
      mComposerLabel->setFont( newFont );
      mComposerLabel->update();
    }
  }
}

void QgsComposerLabelWidget::on_mMarginDoubleSpinBox_valueChanged( double d )
{
  if ( mComposerLabel )
  {
    mComposerLabel->setMargin( d );
    mComposerLabel->update();
  }
}

