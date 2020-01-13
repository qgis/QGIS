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

QgsTableEditorDialog::QgsTableEditorDialog( QWidget *parent )
  : QMainWindow( parent )
{
  setupUi( this );
  setWindowTitle( tr( "QGIS Table Designer" ) );

  setAttribute( Qt::WA_DeleteOnClose );
  setDockOptions( dockOptions() | QMainWindow::GroupedDragging );

  QgsGui::enableAutoGeometryRestore( this );

  QGridLayout *viewLayout = new QGridLayout();
  viewLayout->setSpacing( 0 );
  viewLayout->setMargin( 0 );
  viewLayout->setContentsMargins( 0, 0, 0, 0 );
  centralWidget()->layout()->setSpacing( 0 );
  centralWidget()->layout()->setMargin( 0 );
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

  QgsTableContents c;
  c << QgsTableRow();
  c.last() << QgsTableCell( "test" ) << QgsTableCell( "test2" );
  c << QgsTableRow();
  QgsTableCell cc( "test3" );
  cc.setBackgroundColor( QColor( 255, 255, 255 ) );
  cc.setForegroundColor( QColor( 255, 0, 255 ) );
  c.last() << cc << QgsTableCell( "test4" );
  mTableWidget->setTableContents( c );

  connect( mTableWidget, &QgsTableEditorWidget::tableChanged, this, [ = ]
  {
    if ( !mBlockSignals )
      emit tableChanged();
  } );

  int minDockWidth( fontMetrics().boundingRect( QStringLiteral( "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" ) ).width() );

  mPropertiesDock = new QgsDockWidget( tr( "Formatting" ), this );
  mPropertiesDock->setObjectName( QStringLiteral( "FormattingDock" ) );
  mPropertiesStack = new QgsPanelWidgetStack();
  mPropertiesDock->setWidget( mPropertiesStack );
  mPropertiesDock->setMinimumWidth( minDockWidth );

  mFormattingWidget = new QgsTableEditorFormattingWidget();
  mFormattingWidget->setDockMode( true );
  mPropertiesStack->setMainPanel( mFormattingWidget );

  mPropertiesDock->setFeatures( QDockWidget::NoDockWidgetFeatures );

  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::foregroundColorChanged, mTableWidget, &QgsTableEditorWidget::setSelectionForegroundColor );
  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::backgroundColorChanged, mTableWidget, &QgsTableEditorWidget::setSelectionBackgroundColor );
  connect( mFormattingWidget, &QgsTableEditorFormattingWidget::numberFormatChanged, this, [ = ]
  {
    mTableWidget->setSelectionNumericFormat( mFormattingWidget->numericFormat() );
  } );

  connect( mTableWidget, &QgsTableEditorWidget::activeCellChanged, this, [ = ]
  {
    mFormattingWidget->setForegroundColor( mTableWidget->selectionForegroundColor() );
    mFormattingWidget->setBackgroundColor( mTableWidget->selectionBackgroundColor() );
    mFormattingWidget->setNumericFormat( mTableWidget->selectionNumericFormat(), mTableWidget->hasMixedSelectionNumericFormat() );
  } );

  addDockWidget( Qt::RightDockWidgetArea, mPropertiesDock );

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
}

void QgsTableEditorDialog::setTableContents( const QgsTableContents &contents )
{
  mBlockSignals = true;
  mTableWidget->setTableContents( contents );
  mBlockSignals = false;
}

QgsTableContents QgsTableEditorDialog::tableContents() const
{
  return mTableWidget->tableContents();
}

#include "qgstableeditordialog.h"
