/***************************************************************************
    qgscommentinputdialog.cpp
    ---------------------
    begin                : September 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QMessageBox>
#include <QGridLayout>
#include <QLabel>

#include "qgscommentinputdialog.h"
#include "moc_qgscommentinputdialog.cpp"

QgsCommentInputDialog::QgsCommentInputDialog( const QString &title, const QString &comment, QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( title );

  QGridLayout *layout = new QGridLayout( this );

  mCommentEdit = new QPlainTextEdit( this );
  mCommentEdit->setPlainText( comment );
  layout->addWidget( new QLabel( tr( "Comment" ) ), 0, 0 );
  layout->addWidget( mCommentEdit, 0, 1 );

  mButtonBox = new QDialogButtonBox( this );
  mButtonBox->setOrientation( Qt::Horizontal );
  mButtonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsCommentInputDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsCommentInputDialog::accept );

  layout->addWidget( mButtonBox, 1, 0, 2, 0 );
}

QString QgsCommentInputDialog::comment() const
{
  return mCommentEdit->toPlainText();
}
