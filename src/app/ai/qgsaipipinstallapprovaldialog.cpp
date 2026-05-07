/***************************************************************************
    qgsaipipinstallapprovaldialog.cpp
    ---------------------
    begin                : May 2026
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

#include "qgsaipipinstallapprovaldialog.h"

#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "moc_qgsaipipinstallapprovaldialog.cpp"

QgsAiPipInstallApprovalDialog::QgsAiPipInstallApprovalDialog( const QStringList &packages, const QString &reason, QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( tr( "AI: confirm Python package install" ) );
  resize( 640, 420 );
  setSizeGripEnabled( true );
  setModal( true );

  QVBoxLayout *layout = new QVBoxLayout( this );

  QLabel *intro = new QLabel( this );
  intro->setTextFormat( Qt::PlainText );
  intro->setWordWrap( true );
  const QString why = reason.trimmed().isEmpty() ? tr( "(no reason provided)" ) : reason.trimmed();
  intro->setText( tr(
                    "The AI assistant wants to install the following Python package(s) into the user-scope "
                    "site-packages of the Python interpreter that runs PyQGIS (pip install --user).\n\n"
                    "Reason: %1"
  )
                    .arg( why ) );
  layout->addWidget( intro );

  // Verbatim, monospace, read-only listing of the pinned specs.
  // This is the line of defense against typosquatting (e.g. "requessts"): the user must
  // visually read each name. Do not pretty-format / sort / deduplicate behind the user's back.
  QPlainTextEdit *list = new QPlainTextEdit( this );
  list->setReadOnly( true );
  list->setLineWrapMode( QPlainTextEdit::NoWrap );
  list->setFont( QFontDatabase::systemFont( QFontDatabase::FixedFont ) );
  list->setPlainText( packages.join( '\n' ) );
  layout->addWidget( list, /*stretch=*/1 );

  QLabel *footer = new QLabel( this );
  footer->setTextFormat( Qt::PlainText );
  footer->setWordWrap( true );
  footer->setText( tr(
    "Read the package names carefully. Click Install only if you trust them. "
    "Cancel to refuse."
  ) );
  layout->addWidget( footer );

  QDialogButtonBox *buttons = new QDialogButtonBox( this );
  QPushButton *installBtn = buttons->addButton( tr( "Install" ), QDialogButtonBox::AcceptRole );
  QPushButton *cancelBtn = buttons->addButton( tr( "Cancel" ), QDialogButtonBox::RejectRole );

  // Cancel is the safe default.
  cancelBtn->setDefault( true );
  cancelBtn->setAutoDefault( true );
  installBtn->setDefault( false );

  connect( installBtn, &QPushButton::clicked, this, &QDialog::accept );
  connect( cancelBtn, &QPushButton::clicked, this, &QDialog::reject );
  layout->addWidget( buttons );
}
