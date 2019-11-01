/***************************************************************************
    qgsvaluemapconfigdlg.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaluemapconfigdlg.h"

#include "qgsattributetypeloaddialog.h"
#include "qgsvaluemapfieldformatter.h"
#include "qgsapplication.h"
#include "qgssettings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QClipboard>
#include <QKeyEvent>

QgsValueMapConfigDlg::QgsValueMapConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  tableWidget->insertRow( 0 );

  tableWidget->horizontalHeader()->setClickable( true );
  tableWidget->setSortingEnabled( true );

  connect( addNullButton, &QAbstractButton::clicked, this, &QgsValueMapConfigDlg::addNullButtonPushed );
  connect( removeSelectedButton, &QAbstractButton::clicked, this, &QgsValueMapConfigDlg::removeSelectedButtonPushed );
  connect( loadFromLayerButton, &QAbstractButton::clicked, this, &QgsValueMapConfigDlg::loadFromLayerButtonPushed );
  connect( loadFromCSVButton, &QAbstractButton::clicked, this, &QgsValueMapConfigDlg::loadFromCSVButtonPushed );
  connect( tableWidget, &QTableWidget::cellChanged, this, &QgsValueMapConfigDlg::vCellChanged );
  tableWidget->installEventFilter( this );
}

QVariantMap QgsValueMapConfigDlg::config()
{
  QList<QVariant> valueList;

  //store data to map
  for ( int i = 0; i < tableWidget->rowCount() - 1; i++ )
  {
    QTableWidgetItem *ki = tableWidget->item( i, 0 );
    QTableWidgetItem *vi = tableWidget->item( i, 1 );

    if ( !ki )
      continue;

    QString ks = ki->text();
    if ( ( ks == QgsApplication::nullRepresentation() ) && !( ki->flags() & Qt::ItemIsEditable ) )
      ks = QgsValueMapFieldFormatter::NULL_VALUE;

    QVariantMap value;

    if ( !vi || vi->text().isNull() )
    {
      value.insert( ks, ks );
    }
    else
    {
      value.insert( vi->text(), ks );
    }
    valueList.append( value );
  }

  QVariantMap cfg;
  cfg.insert( QStringLiteral( "map" ), valueList );
  return cfg;
}

void QgsValueMapConfigDlg::setConfig( const QVariantMap &config )
{
  tableWidget->clearContents();
  for ( int i = tableWidget->rowCount() - 1; i > 0; i-- )
  {
    tableWidget->removeRow( i );
  }

  QList<QVariant> valueList = config.value( QStringLiteral( "map" ) ).toList();

  if ( valueList.count() > 0 )
  {
    for ( int i = 0, row = 0; i < valueList.count(); i++, row++ )
    {
      setRow( row, valueList[i].toMap().constBegin().value().toString(), valueList[i].toMap().constBegin().key() );
    }
  }
  else
  {
    int row = 0;
    QVariantMap values = config.value( QStringLiteral( "map" ) ).toMap();
    for ( QVariantMap::ConstIterator mit = values.constBegin(); mit != values.constEnd(); mit++, row++ )
    {
      if ( mit.value().isNull() )
        setRow( row, mit.key(), QString() );
      else
        setRow( row, mit.value().toString(), mit.key() );
    }
  }
}

void QgsValueMapConfigDlg::vCellChanged( int row, int column )
{
  Q_UNUSED( column )
  if ( row == tableWidget->rowCount() - 1 )
  {
    tableWidget->insertRow( row + 1 );
  } //else check type

  emit changed();
}

void QgsValueMapConfigDlg::removeSelectedButtonPushed()
{
  QList<QTableWidgetItem *> list = tableWidget->selectedItems();
  QSet<int> rowsToRemove;
  int removed = 0;
  int i;
  for ( i = 0; i < list.size(); i++ )
  {
    if ( list[i]->column() == 0 )
    {
      int row = list[i]->row();
      if ( !rowsToRemove.contains( row ) )
      {
        rowsToRemove.insert( row );
      }
    }
  }
  for ( i = 0; i < rowsToRemove.size(); i++ )
  {
    tableWidget->removeRow( rowsToRemove.values().at( i ) - removed );
    removed++;
  }
  emit changed();
}

void QgsValueMapConfigDlg::updateMap( const QMap<QString, QVariant> &map, bool insertNull )
{
  tableWidget->clearContents();
  for ( int i = tableWidget->rowCount() - 1; i > 0; i-- )
  {
    tableWidget->removeRow( i );
  }
  int row = 0;

  if ( insertNull )
  {
    setRow( row, QgsValueMapFieldFormatter::NULL_VALUE, QStringLiteral( "<NULL>" ) );
    ++row;
  }

  for ( QMap<QString, QVariant>::const_iterator mit = map.begin(); mit != map.end(); ++mit, ++row )
  {
    if ( mit.value().isNull() )
      setRow( row, mit.key(), QString() );
    else
      setRow( row, mit.key(), mit.value().toString() );
  }
}

void QgsValueMapConfigDlg::populateComboBox( QComboBox *comboBox, const QVariantMap &config, bool skipNull )
{
  const QList<QVariant> valueList = config.value( QStringLiteral( "map" ) ).toList();

  if ( !valueList.empty() )
  {
    for ( const QVariant &value : valueList )
    {
      const QVariantMap valueMap = value.toMap();

      if ( skipNull && valueMap.constBegin().value() == QgsValueMapFieldFormatter::NULL_VALUE )
        continue;

      comboBox->addItem( valueMap.constBegin().key(), valueMap.constBegin().value() );
    }
  }
  else
  {
    const QVariantMap map = config.value( QStringLiteral( "map" ) ).toMap();
    for ( auto it = map.constBegin(); it != map.constEnd(); ++it )
    {
      if ( skipNull && it.value() == QgsValueMapFieldFormatter::NULL_VALUE )
        continue;

      comboBox->addItem( it.key(), it.value() );
    }
  }
}

bool QgsValueMapConfigDlg::eventFilter( QObject *watched, QEvent *event )
{
  Q_UNUSED( watched )
  if ( event->type() == QEvent::KeyPress )
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
    if ( keyEvent->matches( QKeySequence::Copy ) )
    {
      copySelectionToClipboard();
      event->accept();
      return true;
    }
  }
  return false;
}

void QgsValueMapConfigDlg::setRow( int row, const QString &value, const QString &description )
{
  QgsSettings settings;
  QTableWidgetItem *valueCell = nullptr;
  QTableWidgetItem *descriptionCell = new QTableWidgetItem( description );
  tableWidget->insertRow( row );
  if ( value == QgsValueMapFieldFormatter::NULL_VALUE )
  {
    QFont cellFont;
    cellFont.setItalic( true );
    valueCell = new QTableWidgetItem( QgsApplication::nullRepresentation() );
    valueCell->setFont( cellFont );
    valueCell->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    descriptionCell->setFont( cellFont );
  }
  else
  {
    valueCell = new QTableWidgetItem( value );
  }
  tableWidget->setItem( row, 0, valueCell );
  tableWidget->setItem( row, 1, descriptionCell );
}

void QgsValueMapConfigDlg::copySelectionToClipboard()
{
  QAbstractItemModel *model = tableWidget->model();
  QItemSelectionModel *selection = tableWidget->selectionModel();
  const QModelIndexList indexes = selection->selectedIndexes();

  QString clipboardText;
  QModelIndex previous = indexes.first();
  std::unique_ptr<QMimeData> mimeData = qgis::make_unique<QMimeData>();
  for ( const QModelIndex &current : indexes )
  {
    const QString text = model->data( current ).toString();
    if ( current.row() != previous.row() )
    {
      clipboardText.append( '\n' );
    }
    else if ( current.column() != previous.column() )
    {
      clipboardText.append( '\t' );
    }
    clipboardText.append( text );
    previous = current;
  }
  mimeData->setData( QStringLiteral( "text/plain" ), clipboardText.toUtf8() );
  QApplication::clipboard()->setMimeData( mimeData.release() );
}

void QgsValueMapConfigDlg::addNullButtonPushed()
{
  setRow( tableWidget->rowCount() - 1, QgsValueMapFieldFormatter::NULL_VALUE, QStringLiteral( "<NULL>" ) );
}

void QgsValueMapConfigDlg::loadFromLayerButtonPushed()
{
  QgsAttributeTypeLoadDialog layerDialog( layer() );
  if ( !layerDialog.exec() )
    return;

  updateMap( layerDialog.valueMap(), layerDialog.insertNull() );
}

void QgsValueMapConfigDlg::loadFromCSVButtonPushed()
{
  QgsSettings settings;

  QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Select a File" ), QDir::homePath() );
  if ( fileName.isNull() )
    return;

  QFile f( fileName );

  if ( !f.open( QIODevice::ReadOnly ) )
  {
    QMessageBox::information( nullptr,
                              tr( "Load Value Map from File" ),
                              tr( "Could not open file %1\nError was: %2" ).arg( fileName, f.errorString() ),
                              QMessageBox::Cancel );
    return;
  }

  QTextStream s( &f );
  s.setAutoDetectUnicode( true );

  QRegExp re0( "^([^;]*);(.*)$" );
  re0.setMinimal( true );
  QRegExp re1( "^([^,]*),(.*)$" );
  re1.setMinimal( true );
  QMap<QString, QVariant> map;

  s.readLine();

  while ( !s.atEnd() )
  {
    QString l = s.readLine().trimmed();

    QString key, val;
    if ( re0.indexIn( l ) >= 0 && re0.captureCount() == 2 )
    {
      key = re0.cap( 1 ).trimmed();
      val = re0.cap( 2 ).trimmed();
    }
    else if ( re1.indexIn( l ) >= 0 && re1.captureCount() == 2 )
    {
      key = re1.cap( 1 ).trimmed();
      val = re1.cap( 2 ).trimmed();
    }
    else
      continue;

    if ( ( key.startsWith( '\"' ) && key.endsWith( '\"' ) ) ||
         ( key.startsWith( '\'' ) && key.endsWith( '\'' ) ) )
    {
      key = key.mid( 1, key.length() - 2 );
    }

    if ( ( val.startsWith( '\"' ) && val.endsWith( '\"' ) ) ||
         ( val.startsWith( '\'' ) && val.endsWith( '\'' ) ) )
    {
      val = val.mid( 1, val.length() - 2 );
    }

    if ( key == QgsApplication::nullRepresentation() )
      key = QgsValueMapFieldFormatter::NULL_VALUE;

    map[ key ] = val;
  }

  updateMap( map, false );
}
