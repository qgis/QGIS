/***************************************************************************
                         qgscomposerarrowwidget.cpp
                         --------------------------
    begin                : November 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco@hugis.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerarrowwidget.h"
#include "qgscomposerarrow.h"
#include "qgscomposeritemwidget.h"
#include <QColorDialog>

QgsComposerArrowWidget::QgsComposerArrowWidget( QgsComposerArrow* arrow ): QWidget( 0 ), mArrow( arrow )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, mArrow );
  toolBox->addItem( itemPropertiesWidget, tr( "General options" ) );

  setGuiElementValues();
}

QgsComposerArrowWidget::~QgsComposerArrowWidget()
{

}

void QgsComposerArrowWidget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
    if(!mArrow)
    {
        return;
    }

    mArrow->setOutlineWidth( d );
    mArrow->update();
}

void QgsComposerArrowWidget::on_mArrowHeadWidthSpinBox_valueChanged( double d )
{
    if(!mArrow)
    {
        return;
    }

    mArrow->setArrowHeadWidth( d );
    mArrow->update();
}

void QgsComposerArrowWidget::on_mShowArrowHeadCheckBox_stateChanged ( int state )
{
    if(!mArrow)
    {
        return;
    }

    if(state == Qt::Checked)
    {
        mArrow->setShowArrowMarker(true);
    }
    else
    {
        mArrow->setShowArrowMarker(false);
    }
    mArrow->update();
}

void QgsComposerArrowWidget::on_mArrowColorButton_clicked()
{
    if(!mArrow)
    {
        return;
    }

    #if QT_VERSION >= 0x040500
    QColor newColor = QColorDialog::getColor(mArrow->arrowColor(), 0, tr("Arrow color"), QColorDialog::ShowAlphaChannel);
    #else
    QColor newColor = QColorDialog::getColor(mArrow->arrowColor());
    #endif
    if(newColor.isValid())
    {
        mArrow->setArrowColor(newColor);
        mArrow->update();
    }
}

void QgsComposerArrowWidget::blockAllSignals(bool block)
{
    mArrowColorButton->blockSignals(block);
    mShowArrowHeadCheckBox->blockSignals(block);
    mOutlineWidthSpinBox->blockSignals(block);
    mArrowHeadWidthSpinBox->blockSignals(block);
}

void QgsComposerArrowWidget::setGuiElementValues()
{
    if(!mArrow)
    {
        return;
    }

    blockAllSignals(true);
    mOutlineWidthSpinBox->setValue(mArrow->outlineWidth());
    mArrowHeadWidthSpinBox->setValue(mArrow->arrowHeadWidth());
    if( mArrow->showArrowMarker())
    {
        mShowArrowHeadCheckBox->setCheckState(Qt::Checked);
    }
    else
    {
        mShowArrowHeadCheckBox->setCheckState(Qt::Unchecked);
    }
    blockAllSignals(false);
}
