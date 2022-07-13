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

#include <QDir>

//
// QgsFontOptionsWidget
//

QgsFontOptionsWidget::QgsFontOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  mTableReplacements->setHorizontalHeaderLabels( {tr( "Font Family" ), tr( "Replacement Family" ) } );
  mTableReplacements->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

  mTableUserFonts->setHorizontalHeaderLabels( {tr( "File" ), tr( "Font Families" ) } );
  mTableUserFonts->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

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

  mCheckBoxDownloadFonts->setChecked( QgsFontManager::settingsDownloadMissingFonts.value() );

  const QMap< QString, QStringList > userFonts = QgsApplication::fontManager()->userFontToFamilyMap();
  mTableUserFonts->setRowCount( userFonts.size() );
  mTableUserFonts->setSelectionBehavior( QAbstractItemView::SelectRows );
  row = 0;
  for ( auto it = userFonts.constBegin(); it != userFonts.constEnd(); ++it )
  {
    QTableWidgetItem *item = new QTableWidgetItem( QDir::toNativeSeparators( it.key() ) );
    item->setFlags( item->flags() & ~( Qt::ItemIsEditable ) );
    item->setData( Qt::UserRole, it.key() );
    mTableUserFonts->setItem( row, 0, item );

    item = new QTableWidgetItem( it.value().join( QObject::tr( ", " ) ) );
    item->setFlags( item->flags() & ~( Qt::ItemIsEditable ) );
    mTableUserFonts->setItem( row, 1, item );
    row++;
  }

  connect( mButtonRemoveUserFont, &QToolButton::clicked, this, [ = ]
  {
    const QModelIndexList selection = mTableUserFonts->selectionModel()->selectedRows();
    QList< int > selectedRows;
    for ( const QModelIndex &index : selection )
      selectedRows.append( index.row() );

    std::sort( selectedRows.begin(), selectedRows.end() );
    for ( int i = selectedRows.size() - 1; i >= 0; i-- )
    {
      int row = selectedRows.at( i );
      mTableUserFonts->removeRow( row );
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

  QgsFontManager::settingsDownloadMissingFonts.setValue( mCheckBoxDownloadFonts->isChecked() );

  const QMap< QString, QStringList > userFonts = QgsApplication::fontManager()->userFontToFamilyMap();
  QSet< QString > remainingUserFonts;
  for ( int row = 0; row < mTableUserFonts->rowCount(); ++row )
  {
    const QString fileName = mTableUserFonts->item( row, 0 )->data( Qt::UserRole ).toString();
    remainingUserFonts.insert( fileName );
  }
  for ( auto it = userFonts.constBegin(); it != userFonts.constEnd(); ++it )
  {
    if ( !remainingUserFonts.contains( it.key() ) )
    {
      QgsApplication::fontManager()->removeUserFont( it.key() );
    }
  }
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

