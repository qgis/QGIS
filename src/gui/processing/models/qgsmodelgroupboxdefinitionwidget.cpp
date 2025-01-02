/***************************************************************************
                         qgsmodelgroupboxdefinitionwidget.cpp
                         ------------------------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsmodelgroupboxdefinitionwidget.h"
#include "moc_qgsmodelgroupboxdefinitionwidget.cpp"

#include "qgscolorbutton.h"
#include "qgsgui.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QLabel>

QgsModelGroupBoxDefinitionDialog::QgsModelGroupBoxDefinitionDialog( const QgsProcessingModelGroupBox &box, QWidget *parent )
  : QDialog( parent )
  , mBox( box )
{
  QVBoxLayout *commentLayout = new QVBoxLayout();
  commentLayout->addWidget( new QLabel( tr( "Title" ) ) );
  mCommentEdit = new QTextEdit();
  mCommentEdit->setAcceptRichText( false );
  mCommentEdit->setText( box.description() );
  commentLayout->addWidget( mCommentEdit, 1 );

  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );
  hl->addWidget( new QLabel( tr( "Color" ) ) );
  mCommentColorButton = new QgsColorButton();
  mCommentColorButton->setAllowOpacity( true );
  mCommentColorButton->setWindowTitle( tr( "Comment Color" ) );
  mCommentColorButton->setShowNull( true, tr( "Default" ) );

  if ( box.color().isValid() )
    mCommentColorButton->setColor( box.color() );
  else
    mCommentColorButton->setToNull();

  hl->addWidget( mCommentColorButton );
  commentLayout->addLayout( hl );

  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  connect( bbox, &QDialogButtonBox::accepted, this, &QgsModelGroupBoxDefinitionDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QgsModelGroupBoxDefinitionDialog::reject );

  commentLayout->addWidget( bbox );
  setLayout( commentLayout );
  setWindowTitle( tr( "Group Box Properties" ) );
  setObjectName( QStringLiteral( "QgsModelGroupBoxDefinitionWidget" ) );
  QgsGui::enableAutoGeometryRestore( this );

  mCommentEdit->setFocus();
  mCommentEdit->selectAll();
}

QgsProcessingModelGroupBox QgsModelGroupBoxDefinitionDialog::groupBox() const
{
  QgsProcessingModelGroupBox box = mBox;
  box.setColor( mCommentColorButton->isNull() ? QColor() : mCommentColorButton->color() );
  box.setDescription( mCommentEdit->toPlainText() );
  return box;
}
