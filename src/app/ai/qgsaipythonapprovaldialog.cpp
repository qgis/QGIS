/***************************************************************************
    qgsaipythonapprovaldialog.cpp
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

#include "qgsaipythonapprovaldialog.h"
#include "moc_qgsaipythonapprovaldialog.cpp"

#include "qgscodeeditorpython.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

QgsAiPythonApprovalDialog::QgsAiPythonApprovalDialog( const QString &description, const QString &code, QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( tr( "AI: confirm Python execution" ) );
  resize( 900, 600 );
  setSizeGripEnabled( true );
  setModal( true );

  QVBoxLayout *layout = new QVBoxLayout( this );

  QLabel *intro = new QLabel( this );
  intro->setTextFormat( Qt::PlainText );
  intro->setWordWrap( true );
  const QString why = description.trimmed().isEmpty() ? tr( "(no description provided)" ) : description.trimmed();
  intro->setText( tr(
                    "The AI assistant wants to run the following PyQGIS code in this QGIS session.\n"
                    "It runs with the same privileges as the GUI: it can read/write files, modify the project, and call any QGIS API.\n\n"
                    "Reason: %1"
  )
                    .arg( why ) );
  layout->addWidget( intro );

  // Read-only Python editor with syntax highlighting via QScintilla.
  mEditor = new QgsCodeEditorPython( this );
  mEditor->setText( code );
  mEditor->setReadOnly( true );
  layout->addWidget( mEditor, /*stretch=*/1 );

  QLabel *footer = new QLabel( this );
  footer->setTextFormat( Qt::PlainText );
  footer->setWordWrap( true );
  footer->setText( tr( "Click Run only if you understand and trust this code. Click Cancel to refuse." ) );
  layout->addWidget( footer );

  QDialogButtonBox *buttons = new QDialogButtonBox( this );
  QPushButton *runBtn = buttons->addButton( tr( "Run" ), QDialogButtonBox::AcceptRole );
  QPushButton *cancelBtn = buttons->addButton( tr( "Cancel" ), QDialogButtonBox::RejectRole );

  // Cancel is the safe default.
  cancelBtn->setDefault( true );
  cancelBtn->setAutoDefault( true );
  runBtn->setDefault( false );

  connect( runBtn, &QPushButton::clicked, this, &QDialog::accept );
  connect( cancelBtn, &QPushButton::clicked, this, &QDialog::reject );
  layout->addWidget( buttons );
}
