/***************************************************************************
                             qgsprocessingscripteditordialog.cpp
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

#include "qgsprocessingscripteditordialog.h"

#include "qgsgui.h"

#include "moc_qgsprocessingscripteditordialog.cpp"

using namespace Qt::StringLiterals;

///@cond NOT_STABLE


///@endcond

QgsProcessingScriptEditorDialog::QgsProcessingScriptEditorDialog( QWidget *parent, Qt::WindowFlags flags )
  : QMainWindow( parent, flags )
{
  setupUi( this );

  setObjectName( "QgsProcessingScriptEditorDialog" );
  QgsGui::enableAutoGeometryRestore( this );

  setStyleSheet( QgsGui::applicationStyleSheet() );
  connect( QgsGui::instance(), &QgsGui::applicationStyleSheetChanged, this, &QgsProcessingScriptEditorDialog::setStyleSheet );

  mToolBar->setIconSize( QgsGui::iconSize() );
}

QgsProcessingScriptEditorDialog::~QgsProcessingScriptEditorDialog() = default;

QWidget *QgsProcessingScriptEditorDialog::editorContainer()
{
  return mEditorContainer;
}

QAction *QgsProcessingScriptEditorDialog::actionToggleComment()
{
  return mActionToggleComment;
}

QAction *QgsProcessingScriptEditorDialog::actionOpenScript()
{
  return mActionOpenScript;
}

QAction *QgsProcessingScriptEditorDialog::actionSaveScript()
{
  return mActionSaveScript;
}

QAction *QgsProcessingScriptEditorDialog::actionSaveScriptAs()
{
  return mActionSaveScriptAs;
}

QAction *QgsProcessingScriptEditorDialog::actionRunScript()
{
  return mActionRunScript;
}

QAction *QgsProcessingScriptEditorDialog::actionCut()
{
  return mActionCut;
}

QAction *QgsProcessingScriptEditorDialog::actionCopy()
{
  return mActionCopy;
}

QAction *QgsProcessingScriptEditorDialog::actionPaste()
{
  return mActionPaste;
}

QAction *QgsProcessingScriptEditorDialog::actionUndo()
{
  return mActionUndo;
}

QAction *QgsProcessingScriptEditorDialog::actionRedo()
{
  return mActionRedo;
}

QAction *QgsProcessingScriptEditorDialog::actionFindReplace()
{
  return mActionFindReplace;
}

QAction *QgsProcessingScriptEditorDialog::actionIncreaseFontSize()
{
  return mActionIncreaseFontSize;
}

QAction *QgsProcessingScriptEditorDialog::actionDecreaseFontSize()
{
  return mActionDecreaseFontSize;
}
