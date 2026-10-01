/***************************************************************************
                          qgsdialog.cpp
                             -------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdialog.h"

#include <QInputDialog>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>

#include "moc_qgsdialog.cpp"

QgsDialog::QgsDialog( QWidget *parent, Qt::WindowFlags fl, QDialogButtonBox::StandardButtons buttons, Qt::Orientation orientation )
  : QDialog( parent, fl )
{
  // create buttonbox
  mButtonBox = new QDialogButtonBox( buttons, orientation, this );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  // layout
  QBoxLayout *layout = nullptr;
  if ( orientation == Qt::Horizontal )
    layout = new QVBoxLayout();
  else
    layout = new QHBoxLayout();
  mLayout = new QVBoxLayout();
  layout->addLayout( mLayout );
  layout->addWidget( mButtonBox );
  setLayout( layout );
}

QString QgsDialog::getMultiLineText( QWidget *, const QString &title, const QString &label, const QString &text, bool *ok, Qt::WindowFlags flags, Qt::InputMethodHints inputMethodHints )
{
  auto dialog = std::make_unique<QInputDialog>( nullptr, flags );

  dialog->setOptions( QInputDialog::UsePlainTextEditForTextInput );
  dialog->setWindowTitle( title );
  dialog->setLabelText( label );
  dialog->setTextValue( text );
  dialog->setInputMethodHints( inputMethodHints );

  if ( QPlainTextEdit *textEdit = dialog->findChild<QPlainTextEdit *>() )
  {
    textEdit->setLineWrapMode( QPlainTextEdit::WidgetWidth );
  }

  const bool accepted = dialog->exec() == QDialog::Accepted;

  if ( ok )
    *ok = accepted;

  if ( accepted )
  {
    return dialog->textValue();
  }
  else
  {
    return QString();
  }
}
