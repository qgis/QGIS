/***************************************************************************
    qgstabpositionwidget.h
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstabpositionwidget.h"
#include "moc_qgstabpositionwidget.cpp"
#include "qgsapplication.h"
#include "qgsdoublevalidator.h"
#include "qgsunittypes.h"

#include <QDialogButtonBox>

QgsTabPositionWidget::QgsTabPositionWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mAddButton->setIcon( QgsApplication::getThemeIcon( "symbologyAdd.svg" ) );
  mRemoveButton->setIcon( QgsApplication::getThemeIcon( "symbologyRemove.svg" ) );

  setUnit( Qgis::RenderUnit::Millimeters );

  connect( mAddButton, &QPushButton::clicked, this, &QgsTabPositionWidget::mAddButton_clicked );
  connect( mRemoveButton, &QPushButton::clicked, this, &QgsTabPositionWidget::mRemoveButton_clicked );
  connect( mTabPositionTreeWidget, &QTreeWidget::itemChanged, this, &QgsTabPositionWidget::emitPositionsChanged );
}

void QgsTabPositionWidget::setPositions( const QList<QgsTextFormat::Tab> &positions )
{
  mTabPositionTreeWidget->clear();
  for ( const QgsTextFormat::Tab &tab : positions )
  {
    QTreeWidgetItem *entry = new QTreeWidgetItem();
    entry->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
    entry->setText( 0, QLocale().toString( tab.position() ) );
    entry->setData( 0, Qt::EditRole, tab.position() );
    mTabPositionTreeWidget->addTopLevelItem( entry );
  }
}

QList<QgsTextFormat::Tab> QgsTabPositionWidget::positions() const
{
  QList<QgsTextFormat::Tab> result;
  const int nTopLevelItems = mTabPositionTreeWidget->topLevelItemCount();
  result.reserve( nTopLevelItems );
  for ( int i = 0; i < nTopLevelItems; ++i )
  {
    if ( QTreeWidgetItem *currentItem = mTabPositionTreeWidget->topLevelItem( i ) )
    {
      result << QgsTextFormat::Tab( QgsDoubleValidator::toDouble( currentItem->text( 0 ) ) );
    }
  }

  std::sort( result.begin(), result.end(), []( const QgsTextFormat::Tab &a, const QgsTextFormat::Tab &b ) {
    return a.position() < b.position();
  } );

  return result;
}

void QgsTabPositionWidget::setUnit( Qgis::RenderUnit unit )
{
  QTreeWidgetItem *headerItem = mTabPositionTreeWidget->headerItem();
  headerItem->setText( 0, QStringLiteral( "%1 (%2)" ).arg( tr( "Position" ), QgsUnitTypes::toAbbreviatedString( unit ) ) );
}

void QgsTabPositionWidget::mAddButton_clicked()
{
  const QList<QgsTextFormat::Tab> currentPositions = positions();
  double newPosition = 6;
  if ( !currentPositions.empty() )
  {
    newPosition = currentPositions.last().position() + 6;
  }

  QTreeWidgetItem *entry = new QTreeWidgetItem();
  entry->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
  entry->setText( 0, QLocale().toString( newPosition ) );
  entry->setData( 0, Qt::EditRole, newPosition );
  mTabPositionTreeWidget->addTopLevelItem( entry );
  emitPositionsChanged();
}

void QgsTabPositionWidget::mRemoveButton_clicked()
{
  if ( QTreeWidgetItem *currentItem = mTabPositionTreeWidget->currentItem() )
  {
    mTabPositionTreeWidget->takeTopLevelItem( mTabPositionTreeWidget->indexOfTopLevelItem( currentItem ) );
  }
  emitPositionsChanged();
}

void QgsTabPositionWidget::emitPositionsChanged()
{
  emit positionsChanged( positions() );
}


QgsTabPositionDialog::QgsTabPositionDialog( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsTabPositionWidget();
  vLayout->addWidget( mWidget );
  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( bbox, &QDialogButtonBox::accepted, this, &QgsTabPositionDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QgsTabPositionDialog::reject );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
  setWindowTitle( tr( "Tab Positions" ) );
}

void QgsTabPositionDialog::setPositions( const QList<QgsTextFormat::Tab> &positions )
{
  mWidget->setPositions( positions );
}

QList<QgsTextFormat::Tab> QgsTabPositionDialog::positions() const
{
  return mWidget->positions();
}

void QgsTabPositionDialog::setUnit( Qgis::RenderUnit unit )
{
  mWidget->setUnit( unit );
}
