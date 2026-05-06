/***************************************************************************
    qgsaipythonapprovaldialog.h
    ---------------------
    begin                : April 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAIPYTHONAPPROVALDIALOG_H
#define QGSAIPYTHONAPPROVALDIALOG_H

#include "qgis_app.h"

#include <QDialog>

class QgsCodeEditorPython;

/**
 * Modal dialog used to confirm a `run_python` request from the AI assistant.
 * Shows the description (why the model wants to run code), the actual Python
 * source (read-only, syntax-highlighted via QgsCodeEditorPython), and Run/Cancel
 * buttons. The dialog never auto-confirms; the user must explicitly click Run.
 */
class APP_EXPORT QgsAiPythonApprovalDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * \param description  Free-form description from the model explaining the intent.
     * \param code         The Python source the model wants executed.
     * \param parent       Parent widget for modal positioning.
     */
    QgsAiPythonApprovalDialog( const QString &description, const QString &code, QWidget *parent = nullptr );

  private:
    QgsCodeEditorPython *mEditor = nullptr;
};

#endif // QGSAIPYTHONAPPROVALDIALOG_H
