/***************************************************************************
    qgssubstitutionlistwidget.cpp
    -----------------------------
    begin                : August 2016
    copyright            : (C) 2016 Nyall Dawson
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

#include "qgssubstitutionlistwidget.h"
#include "qgsgui.h"

#include <QDialogButtonBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

QgsSubstitutionListWidget::QgsSubstitutionListWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mButtonAdd, &QToolButton::clicked, this, &QgsSubstitutionListWidget::mButtonAdd_clicked );
  connect( mButtonRemove, &QToolButton::clicked, this, &QgsSubstitutionListWidget::mButtonRemove_clicked );
  connect( mButtonExport, &QToolButton::clicked, this, &QgsSubstitutionListWidget::mButtonExport_clicked );
  connect( mButtonImport, &QToolButton::clicked, this, &QgsSubstitutionListWidget::mButtonImport_clicked );
  connect( mTableSubstitutions, &QTableWidget::cellChanged, this, &QgsSubstitutionListWidget::tableChanged );
}

void QgsSubstitutionListWidget::setSubstitutions( const QgsStringReplacementCollection &substitutions )
{
  mTableSubstitutions->blockSignals( true );
  mTableSubstitutions->clearContents();
  const auto constReplacements = substitutions.replacements();
  for ( const QgsStringReplacement &replacement : constReplacements )
  {
    addSubstitution( replacement );
  }
  mTableSubstitutions->blockSignals( false );
}

QgsStringReplacementCollection QgsSubstitutionListWidget::substitutions() const
{
  QList< QgsStringReplacement > result;
  for ( int i = 0; i < mTableSubstitutions->rowCount(); ++i )
  {
    if ( !mTableSubstitutions->item( i, 0 ) )
      continue;

    if ( mTableSubstitutions->item( i, 0 )->text().isEmpty() )
      continue;

    QCheckBox *chkCaseSensitive = qobject_cast<QCheckBox *>( mTableSubstitutions->cellWidget( i, 2 ) );
    QCheckBox *chkWholeWord = qobject_cast<QCheckBox *>( mTableSubstitutions->cellWidget( i, 3 ) );

    const QgsStringReplacement replacement( mTableSubstitutions->item( i, 0 )->text(),
                                            mTableSubstitutions->item( i, 1 )->text(),
                                            chkCaseSensitive->isChecked(),
                                            chkWholeWord->isChecked() );
    result << replacement;
  }
  return QgsStringReplacementCollection( result );
}

void QgsSubstitutionListWidget::mButtonAdd_clicked()
{
  addSubstitution( QgsStringReplacement( QString(), QString(), false, true ) );
  mTableSubstitutions->setFocus();
  mTableSubstitutions->setCurrentCell( mTableSubstitutions->rowCount() - 1, 0 );
}

void QgsSubstitutionListWidget::mButtonRemove_clicked()
{
  const int currentRow = mTableSubstitutions->currentRow();
  mTableSubstitutions->removeRow( currentRow );
  tableChanged();
}

void QgsSubstitutionListWidget::tableChanged()
{
  emit substitutionsChanged( substitutions() );
}

void QgsSubstitutionListWidget::mButtonExport_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save Substitutions" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure the user never omitted the extension from the file name
  if ( !fileName.endsWith( QLatin1String( ".xml" ), Qt::CaseInsensitive ) )
  {
    fileName += QLatin1String( ".xml" );
  }

  QDomDocument doc;
  QDomElement root = doc.createElement( QStringLiteral( "substitutions" ) );
  root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0" ) );
  const QgsStringReplacementCollection collection = substitutions();
  collection.writeXml( root, doc );
  doc.appendChild( root );

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
  {
    QMessageBox::warning( nullptr, tr( "Export Substitutions" ),
                          tr( "Cannot write file %1:\n%2" ).arg( fileName, file.errorString() ),
                          QMessageBox::Ok,
                          QMessageBox::Ok );
    return;
  }

  QTextStream out( &file );
  doc.save( out, 4 );
}

void QgsSubstitutionListWidget::mButtonImport_clicked()
{
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Substitutions" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    QMessageBox::warning( nullptr, tr( "Import Substitutions" ),
                          tr( "Cannot read file %1:\n%2" ).arg( fileName, file.errorString() ),
                          QMessageBox::Ok,
                          QMessageBox::Ok );
    return;
  }

  QDomDocument doc;
  QString errorStr;
  int errorLine;
  int errorColumn;

  if ( !doc.setContent( &file, true, &errorStr, &errorLine, &errorColumn ) )
  {
    QMessageBox::warning( nullptr, tr( "Import substitutions" ),
                          tr( "Parse error at line %1, column %2:\n%3" )
                          .arg( errorLine )
                          .arg( errorColumn )
                          .arg( errorStr ),
                          QMessageBox::Ok,
                          QMessageBox::Ok );
    return;
  }

  const QDomElement root = doc.documentElement();
  if ( root.tagName() != QLatin1String( "substitutions" ) )
  {
    QMessageBox::warning( nullptr, tr( "Import Substitutions" ),
                          tr( "The selected file is not a substitution list." ),
                          QMessageBox::Ok,
                          QMessageBox::Ok );
    return;
  }

  QgsStringReplacementCollection collection;
  collection.readXml( root );
  setSubstitutions( collection );
  tableChanged();
}

void QgsSubstitutionListWidget::addSubstitution( const QgsStringReplacement &substitution )
{
  const int row = mTableSubstitutions->rowCount();
  mTableSubstitutions->insertRow( row );

  const Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable
                                  | Qt::ItemIsEditable;

  QTableWidgetItem *matchItem = new QTableWidgetItem( substitution.match() );
  matchItem->setFlags( itemFlags );
  mTableSubstitutions->setItem( row, 0, matchItem );
  QTableWidgetItem *replaceItem = new QTableWidgetItem( substitution.replacement() );
  replaceItem->setFlags( itemFlags );
  mTableSubstitutions->setItem( row, 1, replaceItem );

  QCheckBox *caseSensitiveChk = new QCheckBox( this );
  caseSensitiveChk->setChecked( substitution.caseSensitive() );
  mTableSubstitutions->setCellWidget( row, 2, caseSensitiveChk );
  connect( caseSensitiveChk, &QAbstractButton::toggled, this, &QgsSubstitutionListWidget::tableChanged );

  QCheckBox *wholeWordChk = new QCheckBox( this );
  wholeWordChk->setChecked( substitution.wholeWordOnly() );
  mTableSubstitutions->setCellWidget( row, 3, wholeWordChk );
  connect( wholeWordChk, &QAbstractButton::toggled, this, &QgsSubstitutionListWidget::tableChanged );
}


//
// QgsSubstitutionListDialog
//


QgsSubstitutionListDialog::QgsSubstitutionListDialog( QWidget *parent )
  : QDialog( parent )

{
  setWindowTitle( tr( "Substitutions" ) );
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsSubstitutionListWidget();
  vLayout->addWidget( mWidget );
  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( bbox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
}

void QgsSubstitutionListDialog::setSubstitutions( const QgsStringReplacementCollection &substitutions )
{
  mWidget->setSubstitutions( substitutions );
}

QgsStringReplacementCollection QgsSubstitutionListDialog::substitutions() const
{
  return mWidget->substitutions();
}
