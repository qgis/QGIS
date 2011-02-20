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
/* $Id:$ */
#ifndef QGSTIPGUI_H
#define QGSTIPGUI_H

#include "ui_qgstipguibase.h"

class QgsTipGui : public QDialog, private Ui::QgsTipGuiBase
{
    Q_OBJECT
  public:
    QgsTipGui();
    ~QgsTipGui();

  private:
    void init();

    int mTipPosition;

  private slots:
    void on_cbxDisableTips_toggled( bool theFlag );
    void prevClicked();
    void nextClicked();
};

#endif
