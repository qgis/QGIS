/***************************************************************************
                         qgscomposerarrowwidget.h
                         ------------------------
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

#ifndef QGSCOMPOSERARROWWIDGET_H
#define QGSCOMPOSERARROWWIDGET_H

#include "ui_qgscomposerarrowwidgetbase.h"

class QgsComposerArrow;

class QgsComposerArrowWidget: public QWidget, private Ui::QgsComposerArrowWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerArrowWidget( QgsComposerArrow* arrow );
    ~QgsComposerArrowWidget();

  private:
    QgsComposerArrow* mArrow;

    void blockAllSignals( bool block );
    void setGuiElementValues();

  private slots:
    void on_mOutlineWidthSpinBox_valueChanged( double d );
    void on_mArrowHeadWidthSpinBox_valueChanged( double d );
    void on_mShowArrowHeadCheckBox_stateChanged( int state );
    void on_mArrowColorButton_clicked();
};

#endif // QGSCOMPOSERARROWWIDGET_H
