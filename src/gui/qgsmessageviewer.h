/***************************************************************************
                          qgsmessageviewer.h  -  description
                             -------------------
    begin                : Wed Jun 4 2003
    copyright            : (C) 2002 by Gary E.Sherman
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
#ifndef QGSMESSAGEVIEWER_H
#define QGSMESSAGEVIEWER_H

#include <ui_qgsmessageviewer.h>
#include <qgisgui.h>

class QgsMessageViewer: public QDialog, private Ui::QgsMessageViewer
{
  public:
    QgsMessageViewer(QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags);
    ~QgsMessageViewer();
    // Call one of the setMessage...() functions first.
    // Subsequent calls to appendMessage use the format as determined
    // by the call to setMessage...()

    // Treats the given text as html.
    void setMessageAsHtml(const QString& msg);
    // Treats the given text as plain text
    void setMessageAsPlainText(const QString& msg);
    void appendMessage(const QString& msg);
    // A checkbox that can be used for something like 
    // "don't show this message again"
    void setCheckBoxText(const QString& text);
    // Make the check box visible/invisible
    void setCheckBoxVisible(bool visible);
    // Sets the check state
    void setCheckBoxState(Qt::CheckState state);
    // The state of the checkbox
    Qt::CheckState checkBoxState();
};

#endif
