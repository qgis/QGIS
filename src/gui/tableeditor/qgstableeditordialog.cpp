/***************************************************************************
    qgstableeditordialog.cpp
    ------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstableeditordialog.h"
#include "qgstableeditorwidget.h"
#include "qgsmessagebar.h"
#include "qgsgui.h"
#include "qgsdockwidget.h"
#include "qgspanelwidgetstack.h"
#include "qgstableeditorformattingwidget.h"
#include "qgssettings.h"

#include <QClipboard>
#include <QMessageBox>

QgsTableEditorDialog::QgsTableEditorDialog( QWidget *parent )
  : QMainWindow( parent )
{
  setupUi( this );
  setWindowTitle( tr( "Table Designer" ) );

  setAttribute( Qt::WA_DeleteOnClose );
  setDockOptions( dockOptions() | QMainWindow::GroupedDragging );

  QgsGui::enableAutoGeometryRestore( this );

  QGridLayout *viewLayout = new QGridLayout();
  viewLayout->setSpacing( 0 );
  viewLayout->setContentsMargins( 0, 0, 0, 0 );
  centralWidget()->layout()->setSpacing( 0 );
  centralWidget()->layout()->setContentsMargins( 0, 0, 0, 0 );

  mMessageBar = new QgsMessageBar( centralWidget() );
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  static_cast< QGridLayout * >( centralWidget()->layout() )->addWidget( mMessageBar, 0, 0, 1, 1, Qt::AlignTop );

  mTableWidget = new QgsTableEditorWidget();
  mTableWidget->setContentsMargins( 0, 0, 0, 0 );
  viewLayout->addWidget( mTableWidget, 0, 0 );
  mViewFrame->setLayout( viewLayout );
  mViewFrame->setContentsMargins( 0, 0, 0, 0 );

  mTableWidget->setFocus();
  mTableWidget->setTableContents( QgsTableContents() << ( QgsTableRow() << QgsTableCell() ) );

  connect( mTableWidget, &QgsTableEditorWidget::tableChanged, this, [ = ]
  {
    if ( !mBlockSignals )
      emit tableChanged();
  } );

  const int minDockWidth( fontMetrics().boundingRect( QStringLiteral( "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" ) ).width() );

  mPropertiesDock = new QgsDockWidget( tr( "Cell Contents" ), this );
  mPropertiesDock->setObjectName( QStringLiteral( "FormattingDock" ) );
  mPropertiesStack = new QgsPanelWidgetStack();
  mPropertiesDock->setWidget( mPropertiesStack );
  mPropertiesDock->setMinimumWidth( minDockWidth );

  mFormattingWidget = new QgsTableEditorFormattingWidget();
  mFormattingWidget->setDockMode( true );
  mPropertiesStack->setMainPanel( mFormattingWidget );

  mPropertiesDock->setFeatures( QDockWidget::NoDockWidgetFeatures );

  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::backgroundColorChanged, mTableWidget, &QgsTableEditorWidget::setSelectionBackgroundColor );

  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::horizontalAlignmentChanged, mTableWidget, &QgsTableEditorWidget::setSelectionHorizontalAlignment );
  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::verticalAlignmentChanged, mTableWidget, &QgsTableEditorWidget::setSelectionVerticalAlignment );
  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::cellPropertyChanged, mTableWidget, &QgsTableEditorWidget::setSelectionCellProperty );

  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::textFormatChanged, this, [ = ]
  {
    mTableWidget->setSelectionTextFormat( mFormattingWidget->textFormat() );
  } );

  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::numberFormatChanged, this, [ = ]
  {
    mTableWidget->setSelectionNumericFormat( mFormattingWidget->numericFormat() );
  } );
  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::rowHeightChanged, mTableWidget, &QgsTableEditorWidget::setSelectionRowHeight );
  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::columnWidthChanged, mTableWidget, &QgsTableEditorWidget::setSelectionColumnWidth );

  connect( mTableWidget, &QgsTableEditorWidget::activeCellChanged, this, [ = ]
  {
    mFormattingWidget->setBackgroundColor( mTableWidget->selectionBackgroundColor() );
    mFormattingWidget->setNumericFormat( mTableWidget->selectionNumericFormat(), mTableWidget->hasMixedSelectionNumericFormat() );
    mFormattingWidget->setRowHeight( mTableWidget->selectionRowHeight() );
    mFormattingWidget->setColumnWidth( mTableWidget->selectionColumnWidth() );
    mFormattingWidget->setTextFormat( mTableWidget->selectionTextFormat() );
    mFormattingWidget->setHorizontalAlignment( mTableWidget->selectionHorizontalAlignment() );
    mFormattingWidget->setVerticalAlignment( mTableWidget->selectionVerticalAlignment() );
    mFormattingWidget->setCellProperty( mTableWidget->selectionCellProperty() );

    updateActionNamesFromSelection();

    mFormattingWidget->setEnabled( !mTableWidget->isHeaderCellSelected() );
  } );
  updateActionNamesFromSelection();

  addDockWidget( Qt::RightDockWidgetArea, mPropertiesDock );

  mActionImportFromClipboard->setEnabled( !QApplication::clipboard()->text().isEmpty() );
  connect( QApplication::clipboard(), &QClipboard::dataChanged, this, [ = ]() { mActionImportFromClipboard->setEnabled( !QApplication::clipboard()->text().isEmpty() ); } );

  connect( mActionImportFromClipboard, &QAction::triggered, this, &QgsTableEditorDialog::setTableContentsFromClipboard );
  connect( mActionClose, &QAction::triggered, this, &QMainWindow::close );
  connect( mActionInsertRowsAbove, &QAction::triggered, mTableWidget, &QgsTableEditorWidget::insertRowsAbove );
  connect( mActionInsertRowsBelow, &QAction::triggered, mTableWidget, &QgsTableEditorWidget::insertRowsBelow );
  connect( mActionInsertColumnsBefore, &QAction::triggered, mTableWidget, &QgsTableEditorWidget::insertColumnsBefore );
  connect( mActionInsertColumnsAfter, &QAction::triggered, mTableWidget, &QgsTableEditorWidget::insertColumnsAfter );
  connect( mActionDeleteRows, &QAction::triggered, mTableWidget, &QgsTableEditorWidget::deleteRows );
  connect( mActionDeleteColumns, &QAction::triggered, mTableWidget, &QgsTableEditorWidget::deleteColumns );
  connect( mActionSelectRow, &QAction::triggered, mTableWidget, &QgsTableEditorWidget::expandRowSelection );
  connect( mActionSelectColumn, &QAction::triggered, mTableWidget, &QgsTableEditorWidget::expandColumnSelection );
  connect( mActionSelectAll, &QAction::triggered, mTableWidget, &QgsTableEditorWidget::selectAll );
  connect( mActionClear, &QAction::triggered, mTableWidget, &QgsTableEditorWidget::clearSelectedCells );
  connect( mActionIncludeHeader, &QAction::toggled, this, [ = ]( bool checked )
  {
    mTableWidget->setIncludeTableHeader( checked );
    emit includeHeaderChanged( checked );
  } );

  // restore the toolbar and dock widgets positions using Qt settings API
  const QgsSettings settings;

  const QByteArray state = settings.value( QStringLiteral( "LayoutDesigner/tableEditorState" ), QByteArray(), QgsSettings::App ).toByteArray();
  if ( !state.isEmpty() && !restoreState( state ) )
  {
    QgsDebugMsg( QStringLiteral( "restore of table editor dialog UI state failed" ) );
  }
}

void QgsTableEditorDialog::closeEvent( QCloseEvent * )
{
  QgsSettings settings;
  // store the toolbar/dock widget settings using Qt settings API
  settings.setValue( QStringLiteral( "LayoutDesigner/tableEditorState" ), saveState(), QgsSettings::App );
}

bool QgsTableEditorDialog::setTableContentsFromClipboard()
{
  if ( QApplication::clipboard()->text().isEmpty() )
    return false;

  if ( QMessageBox::question( this, tr( "Import Content From Clipboard" ),
                              tr( "Importing content from clipboard will overwrite current table content. Are you sure?" ) ) != QMessageBox::Yes )
    return false;

  QgsTableContents contents;
  const QStringList lines = QApplication::clipboard()->text().split( '\n' );
  for ( const QString &line : lines )
  {
    if ( !line.isEmpty() )
    {
      QgsTableRow row;
      const QStringList cells = line.split( '\t' );
      for ( const QString &text : cells )
      {
        const QgsTableCell cell( text );
        row << cell;
      }
      contents << row;
    }
  }

  if ( !contents.isEmpty() )
  {
    setTableContents( contents );
    emit tableChanged();
    return true;
  }

  return false;
}

void QgsTableEditorDialog::setTableContents( const QgsTableContents &contents )
{
  mBlockSignals = true;
  mTableWidget->setTableContents( contents );
  mTableWidget->resizeRowsToContents();
  mTableWidget->resizeColumnsToContents();
  mBlockSignals = false;
}

QgsTableContents QgsTableEditorDialog::tableContents() const
{
  return mTableWidget->tableContents();
}

double QgsTableEditorDialog::tableRowHeight( int row )
{
  return mTableWidget->tableRowHeight( row );
}

double QgsTableEditorDialog::tableColumnWidth( int column )
{
  return mTableWidget->tableColumnWidth( column );
}

void QgsTableEditorDialog::setTableRowHeight( int row, double height )
{
  mTableWidget->setTableRowHeight( row, height );
}

void QgsTableEditorDialog::setTableColumnWidth( int column, double width )
{
  mTableWidget->setTableColumnWidth( column, width );
}

bool QgsTableEditorDialog::includeTableHeader() const
{
  return mActionIncludeHeader->isChecked();
}

void QgsTableEditorDialog::setIncludeTableHeader( bool included )
{
  mActionIncludeHeader->setChecked( included );
}

QVariantList QgsTableEditorDialog::tableHeaders() const
{
  return mTableWidget->tableHeaders();
}

void QgsTableEditorDialog::setTableHeaders( const QVariantList &headers )
{
  mTableWidget->setTableHeaders( headers );
}

void QgsTableEditorDialog::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mFormattingWidget->registerExpressionContextGenerator( generator );
}

void QgsTableEditorDialog::updateActionNamesFromSelection()
{
  const int rowCount = mTableWidget->rowsAssociatedWithSelection().size();
  const int columnCount = mTableWidget->columnsAssociatedWithSelection().size();

  mActionInsertRowsAbove->setEnabled( rowCount > 0 );
  mActionInsertRowsBelow->setEnabled( rowCount > 0 );
  mActionDeleteRows->setEnabled( rowCount > 0 );
  mActionSelectRow->setEnabled( rowCount > 0 );
  if ( rowCount == 0 )
  {
    mActionInsertRowsAbove->setText( tr( "Rows Above" ) );
    mActionInsertRowsBelow->setText( tr( "Rows Below" ) );
    mActionDeleteRows->setText( tr( "Delete Rows" ) );
    mActionSelectRow->setText( tr( "Select Rows" ) );
  }
  else if ( rowCount == 1 )
  {
    mActionInsertRowsAbove->setText( tr( "Row Above" ) );
    mActionInsertRowsBelow->setText( tr( "Row Below" ) );
    mActionDeleteRows->setText( tr( "Delete Row" ) );
    mActionSelectRow->setText( tr( "Select Row" ) );
  }
  else
  {
    mActionInsertRowsAbove->setText( tr( "%n Row(s) Above", nullptr, rowCount ) );
    mActionInsertRowsBelow->setText( tr( "%n Row(s) Below", nullptr, rowCount ) );
    mActionDeleteRows->setText( tr( "Delete %n Row(s)", nullptr, rowCount ) );
    mActionSelectRow->setText( tr( "Select %n Row(s)", nullptr, rowCount ) );
  }

  mActionInsertColumnsBefore->setEnabled( columnCount > 0 );
  mActionInsertColumnsAfter->setEnabled( columnCount > 0 );
  mActionDeleteColumns->setEnabled( columnCount > 0 );
  mActionSelectColumn->setEnabled( columnCount > 0 );
  if ( columnCount == 0 )
  {
    mActionInsertColumnsBefore->setText( tr( "Columns Before" ) );
    mActionInsertColumnsAfter->setText( tr( "Columns After" ) );
    mActionDeleteColumns->setText( tr( "Delete Columns" ) );
    mActionSelectColumn->setText( tr( "Select Columns" ) );
  }
  else if ( columnCount == 1 )
  {
    mActionInsertColumnsBefore->setText( tr( "Column Before" ) );
    mActionInsertColumnsAfter->setText( tr( "Column After" ) );
    mActionDeleteColumns->setText( tr( "Delete Column" ) );
    mActionSelectColumn->setText( tr( "Select Column" ) );
  }
  else
  {
    mActionInsertColumnsBefore->setText( tr( "%n Column(s) Before", nullptr, columnCount ) );
    mActionInsertColumnsAfter->setText( tr( "%n Column(s) After", nullptr, columnCount ) );
    mActionDeleteColumns->setText( tr( "Delete %n Column(s)", nullptr, columnCount ) );
    mActionSelectColumn->setText( tr( "Select %n Column(s)", nullptr, columnCount ) );
  }
}

#include "qgstableeditordialog.h"
