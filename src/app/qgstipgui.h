/***************************************************************************
                          qgstipgui.h  -  description
                             -------------------
    begin                : Fri 18 Feb 2011
    copyright            : (C) 2011 by Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTIPGUI_H
#define QGSTIPGUI_H

#include "ui_qgstipguibase.h"
class QgsTip;

class APP_EXPORT QgsTipGui : public QDialog, private Ui::QgsTipGuiBase
{
    Q_OBJECT
  public:
    QgsTipGui( QWidget *parent = nullptr );
    ~QgsTipGui();

  private:
    void init();
    void showTip( QgsTip );

    int mTipPosition;

  private slots:
    void on_cbxDisableTips_toggled( bool theFlag );
    void prevClicked();
    void nextClicked();
};

#endif
