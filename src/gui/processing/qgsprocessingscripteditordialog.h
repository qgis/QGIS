/***************************************************************************
                             qgsprocessingscripteditordialog.h
                             ------------------------
    Date                 : September 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGSCRIPTEDITORDIALOG_H
#define QGSPROCESSINGSCRIPTEDITORDIALOG_H

#include "ui_qgsprocessingscripteditordialogbase.h"

#include "qgis.h"
#include "qgis_gui.h"

class QgsCodeEditorPython;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief Processing script editor dialog base class.
 * \warning Not stable API
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsProcessingScriptEditorDialog : public QMainWindow, private Ui::QgsProcessingScriptEditorDialogBase
{
    Q_OBJECT
  public:
    QgsProcessingScriptEditorDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
    ~QgsProcessingScriptEditorDialog() override;

    /**
     * Returns the code editor shown in the dialog.
     */
    virtual QgsCodeEditorPython *codeEditor() = 0;

    QWidget *editorContainer();
    QAction *actionToggleComment();
    QAction *actionOpenScript();
    QAction *actionSaveScript();
    QAction *actionSaveScriptAs();
    QAction *actionRunScript();
    QAction *actionCut();
    QAction *actionCopy();
    QAction *actionPaste();
    QAction *actionUndo();
    QAction *actionRedo();
    QAction *actionFindReplace();
    QAction *actionIncreaseFontSize();
    QAction *actionDecreaseFontSize();
};

///@endcond

#endif // QGSPROCESSINGSCRIPTEDITORDIALOG_H
