/***************************************************************************
    qgsfontoptions.cpp
    -------------------------
    begin                : June 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfontoptions.h"

#include "qgsbabelgpsdevice.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsfontmanager.h"

//
// QgsFontOptionsWidget
//

QgsFontOptionsWidget::QgsFontOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  mTableReplacements->setHorizontalHeaderLabels( {tr( "Font Family" ), tr( "Replacement Family" ) } );
  mTableReplacements->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

  const QMap< QString, QString > replacements = QgsApplication::fontManager()->fontFamilyReplacements();
  mTableReplacements->setRowCount( replacements.size() );
  int row = 0;
  for ( auto it = replacements.constBegin(); it != replacements.constEnd(); ++it )
  {
    mTableReplacements->setItem( row, 0, new QTableWidgetItem( it.key() ) );
    mTableReplacements->setItem( row, 1, new QTableWidgetItem( it.value() ) );
    row++;
  }

  connect( mButtonAddReplacement, &QToolButton::clicked, this, [ = ]
  {
    mTableReplacements->setRowCount( mTableReplacements->rowCount() + 1 );
    mTableReplacements->setFocus();
    mTableReplacements->setCurrentCell( mTableReplacements->rowCount() - 1, 0 );
  } );

  connect( mButtonRemoveReplacement, &QToolButton::clicked, this, [ = ]
  {
    const QModelIndexList selection = mTableReplacements->selectionModel()->selectedRows();
    QList< int > selectedRows;
    for ( const QModelIndex &index : selection )
      selectedRows.append( index.row() );

    std::sort( selectedRows.begin(), selectedRows.end() );
    for ( int i = selectedRows.size() - 1; i >= 0; i-- )
    {
      int row = selectedRows.at( i );
      mTableReplacements->removeRow( row );
    }
  } );
}

void QgsFontOptionsWidget::apply()
{
  QMap< QString, QString > replacements;
  for ( int row = 0; row < mTableReplacements->rowCount(); ++row )
  {
    const QString original = mTableReplacements->item( row, 0 )->text().trimmed();
    const QString replacement = mTableReplacements->item( row, 1 )->text().trimmed();
    if ( original.isEmpty() || replacement.isEmpty() )
      continue;

    replacements.insert( original, replacement );
  }
  QgsApplication::fontManager()->setFontFamilyReplacements( replacements );
}

//
// QgsFontOptionsFactory
//
QgsFontOptionsFactory::QgsFontOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "Fonts" ), QIcon() )
{

}

QIcon QgsFontOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFonts.svg" ) );
}

QgsOptionsPageWidget *QgsFontOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsFontOptionsWidget( parent );
}

QString QgsFontOptionsFactory::pagePositionHint() const
{
  return QStringLiteral( "mOptionsPageComposer" );
}

