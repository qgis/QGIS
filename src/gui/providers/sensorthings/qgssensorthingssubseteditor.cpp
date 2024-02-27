/***************************************************************************
    qgssensorthingssubseteditor.cpp
     --------------------------------------
    Date                 : February 2024
    Copyright            : (C) 2024 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorthingssubseteditor.h"
#include "qgsvectorlayer.h"
#include "qgscodeeditor.h"
#include "qgsfieldproxymodel.h"
#include "qgsfieldmodel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QPushButton>

///@cond PRIVATE

QgsSensorThingsSubsetEditor::QgsSensorThingsSubsetEditor( QgsVectorLayer *layer, const QgsFields &fields, QWidget *parent, Qt::WindowFlags fl )
  : QgsSubsetStringEditorInterface( parent, fl )
  , mLayer( layer )
  , mFields( ( layer && fields.isEmpty() ) ? layer->fields() : fields )
{
  setupUi( this );

  mSubsetEditor = new QgsCodeEditor();
  mSubsetEditor->setWrapMode( QsciScintilla::WrapWord );

  QVBoxLayout *vl = new QVBoxLayout();
  vl->addWidget( mSubsetEditor );
  mEditorGroupBox->setLayout( vl );

  mModelFields = new QgsFieldProxyModel();
  mModelFields->setFilters( QgsFieldProxyModel::Filter::AllTypes | QgsFieldProxyModel::Filter::OriginProvider );
  mModelFields->sourceFieldModel()->setFields( mFields );
  lstFields->setModel( mModelFields );
  lstFields->setViewMode( QListView::ListMode );
  lstFields->setUniformItemSizes( true );
  lstFields->setAlternatingRowColors( true );
  lstFields->setSelectionBehavior( QAbstractItemView::SelectRows );

  QFont boldFont = font();
  boldFont.setBold( true );
  mLabelComparisons->setFont( boldFont );
  mLabelLogical->setFont( boldFont );
  mLabelArithmetic->setFont( boldFont );

  mButtonEq->setToolTip( tr( "Equal" ) );
  mButtonEq->setProperty( "expression", " eq " );
  mButtonNe->setToolTip( tr( "Not equal" ) );
  mButtonNe->setProperty( "expression", " ne " );
  mButtonGt->setToolTip( tr( "Greater than" ) );
  mButtonGt->setProperty( "expression", " gt " );
  mButtonGe->setToolTip( tr( "Greater than or equal" ) );
  mButtonGe->setProperty( "expression", " ge " );
  mButtonLt->setToolTip( tr( "Less than" ) );
  mButtonLt->setProperty( "expression", " lt " );
  mButtonLe->setToolTip( tr( "Less than or equal" ) );
  mButtonLe->setProperty( "expression", " le " );
  mButtonAnd->setToolTip( tr( "Logical and" ) );
  mButtonAnd->setProperty( "expression", " and " );
  mButtonOr->setToolTip( tr( "Logical or" ) );
  mButtonOr->setProperty( "expression", " or " );
  mButtonNot->setToolTip( tr( "Logical negation" ) );
  mButtonNot->setProperty( "expression", " not " );
  mButtonAdd->setToolTip( tr( "Addition" ) );
  mButtonAdd->setProperty( "expression", " add " );
  mButtonSub->setToolTip( tr( "Subtraction" ) );
  mButtonSub->setProperty( "expression", " sub " );
  mButtonMul->setToolTip( tr( "Multiplication" ) );
  mButtonMul->setProperty( "expression", " mul " );
  mButtonDiv->setToolTip( tr( "Division" ) );
  mButtonDiv->setProperty( "expression", " div " );
  mButtonMod->setToolTip( tr( "Modulo" ) );
  mButtonMod->setProperty( "expression", " mod " );

  if ( mLayer )
    lblDataUri->setText( tr( "Set filter on %1" ).arg( mLayer->name() ) );
  else
    lblDataUri->setText( tr( "Set filter for layer" ) );

  connect( mButtonBox->button( QDialogButtonBox::Reset ), &QPushButton::clicked, this, &QgsSensorThingsSubsetEditor::reset );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsSensorThingsSubsetEditor::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsSensorThingsSubsetEditor::reject );

  connect( lstFields, &QListView::doubleClicked, this, &QgsSensorThingsSubsetEditor::lstFieldsDoubleClicked );

  for ( QPushButton *button :
        {
          mButtonEq,
          mButtonNe,
          mButtonGt,
          mButtonGe,
          mButtonLt,
          mButtonLe,
          mButtonAnd,
          mButtonOr,
          mButtonNot,
          mButtonAdd,
          mButtonSub,
          mButtonMul,
          mButtonDiv,
          mButtonMod
        } )
  {
    connect( button, &QPushButton::clicked, this, [this, button]
    {
      mSubsetEditor->insertText( button->property( "expression" ).toString() );
      mSubsetEditor->setFocus();
    } );

  }
}

QString QgsSensorThingsSubsetEditor::subsetString() const
{
  return mSubsetEditor->text();
}

void QgsSensorThingsSubsetEditor::setSubsetString( const QString &subsetString )
{
  mSubsetEditor->setText( subsetString );
}

void QgsSensorThingsSubsetEditor::accept()
{
  if ( mLayer )
  {
    mLayer->setSubsetString( subsetString() );
  }
  QDialog::accept();
}

void QgsSensorThingsSubsetEditor::reset()
{
  mSubsetEditor->clear();
}

void QgsSensorThingsSubsetEditor::lstFieldsDoubleClicked( const QModelIndex &index )
{
  mSubsetEditor->insertText( mModelFields->data( index, static_cast< int >( QgsFieldModel::CustomRole::FieldName ) ).toString() );
  mSubsetEditor->setFocus();
}


///@endcond
