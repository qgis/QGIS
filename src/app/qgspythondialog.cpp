/***************************************************************************
    qgspythondialog.h - dialog with embedded python console
    ---------------------
    begin                : October 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgspythondialog.h"
#include "qgspythonutils.h"

#include <QShowEvent>
#include <QCloseEvent>

QgsPythonDialog::QgsPythonDialog(QgisInterface* pIface, QgsPythonUtils* pythonUtils, QWidget *parent)
  : QDialog(parent)
{
  setupUi(this);
  mIface = pIface;
  mPythonUtils = pythonUtils;
}

QgsPythonDialog::~QgsPythonDialog()
{
}

QString QgsPythonDialog::escapeHtml(QString text)
{
    return text.replace("<","&lt;").replace(">","&gt;");
}

void QgsPythonDialog::on_edtCmdLine_returnPressed()
{
  QString command = edtCmdLine->text();
  QString output;
  
  
  // when using Py_single_input the return value will be always null
  // we're using custom hooks for output and exceptions to show output in console
  if (mPythonUtils->runStringUnsafe(command))
  {
    mPythonUtils->evalString("sys.stdout.get_and_clean_data()", output);
    QString result = mPythonUtils->getResult();
    // escape the result so python objects display properly and
    // we can still use html output to get nicely formatted display
    output = escapeHtml(output) + escapeHtml(result);
    
    if (!output.isEmpty())
      output += "<br>";
  }
  else
  {
    QString className, errorText;
    mPythonUtils->getError(className, errorText);
    
    output = "<font color=\"red\">" + escapeHtml(className) + ": " + escapeHtml(errorText) + "</font><br>";
  }
   
  QString str = "<b><font color=\"green\">&gt;&gt;&gt;</font> " + escapeHtml(command) + "</b><br>" + output;
  
  edtCmdLine->setText("");
  
  txtHistory->moveCursor(QTextCursor::End);
  txtHistory->insertHtml(str);
  txtHistory->moveCursor(QTextCursor::End);
  txtHistory->ensureCursorVisible();
}

void QgsPythonDialog::showEvent(QShowEvent* event)
{
  QDialog::showEvent(event);
  
  mPythonUtils->installConsoleHooks();
}

void QgsPythonDialog::closeEvent(QCloseEvent* event)
{
  mPythonUtils->uninstallConsoleHooks();
  
  QDialog::closeEvent(event);
}
