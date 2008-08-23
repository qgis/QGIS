/***************************************************************************
                                qgshelpviewer.h
                              Simple help browser
                             -------------------
    begin                : 2004-01-28
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSHELPVIEWER_H
#define QGSHELPVIEWER_H

#include "ui_qgshelpviewerbase.h"

class QString;

class QgsHelpViewer : public QDialog, private Ui::QgsHelpViewerBase
{
    Q_OBJECT

  public:
    QgsHelpViewer( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsHelpViewer();
    void showContent( QString path, QString doc );


};

#endif // QGSHELPVIEWER_H
