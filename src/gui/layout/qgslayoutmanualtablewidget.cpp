/***************************************************************************
                         qgslayoutmanualtablewidget.h
                         ---------------------------------
    begin                : January 2020
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

#include "qgslayoutmanualtablewidget.h"
#include "qgslayoutatlas.h"
#include "qgslayout.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemmanualtable.h"
#include "qgslayouttablecolumn.h"
#include "qgsguiutils.h"
#include "qgslayouttablebackgroundcolorsdialog.h"

QgsLayoutManualTableWidget::QgsLayoutManualTableWidget( QgsLayoutFrame *frame )
  : QgsLayoutItemBaseWidget( nullptr, frame ? qobject_cast< QgsLayoutItemManualTable* >( frame->multiFrame() ) : nullptr )
  , mTable( frame ? qobject_cast< QgsLayoutItemManualTable* >( frame->multiFrame() ) : nullptr )
  , mFrame( frame )
{
  setupUi( this );

  connect( mSetContentsButton, &QPushButton::clicked, this, &QgsLayoutManualTableWidget::setTableContents );
  connect( mMarginSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutManualTableWidget::mMarginSpinBox_valueChanged );
  connect( mGridStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutManualTableWidget::mGridStrokeWidthSpinBox_valueChanged );
  connect( mGridColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutManualTableWidget::mGridColorButton_colorChanged );
  connect( mBackgroundColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutManualTableWidget::mBackgroundColorButton_colorChanged );
  connect( mDrawHorizontalGrid, &QCheckBox::toggled, this, &QgsLayoutManualTableWidget::mDrawHorizontalGrid_toggled );
  connect( mDrawVerticalGrid, &QCheckBox::toggled, this, &QgsLayoutManualTableWidget::mDrawVerticalGrid_toggled );
  connect( mShowGridGroupCheckBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutManualTableWidget::mShowGridGroupCheckBox_toggled );
  connect( mHeaderHAlignmentComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutManualTableWidget::mHeaderHAlignmentComboBox_currentIndexChanged );
  connect( mHeaderModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutManualTableWidget::mHeaderModeComboBox_currentIndexChanged );
  connect( mAddFramePushButton, &QPushButton::clicked, this, &QgsLayoutManualTableWidget::mAddFramePushButton_clicked );
  connect( mResizeModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutManualTableWidget::mResizeModeComboBox_currentIndexChanged );
  connect( mDrawEmptyCheckBox, &QCheckBox::toggled, this, &QgsLayoutManualTableWidget::mDrawEmptyCheckBox_toggled );
  connect( mEmptyFrameCheckBox, &QCheckBox::toggled, this, &QgsLayoutManualTableWidget::mEmptyFrameCheckBox_toggled );
  connect( mHideEmptyBgCheckBox, &QCheckBox::toggled, this, &QgsLayoutManualTableWidget::mHideEmptyBgCheckBox_toggled );
  connect( mWrapBehaviorComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutManualTableWidget::mWrapBehaviorComboBox_currentIndexChanged );
  connect( mAdvancedCustomizationButton, &QPushButton::clicked, this, &QgsLayoutManualTableWidget::mAdvancedCustomizationButton_clicked );
  setPanelTitle( tr( "Table Properties" ) );

  mContentFontToolButton->setMode( QgsFontButton::ModeTextRenderer );
  mHeaderFontToolButton->setMode( QgsFontButton::ModeTextRenderer );

  mContentFontToolButton->registerExpressionContextGenerator( this );
  mHeaderFontToolButton->registerExpressionContextGenerator( this );

  blockAllSignals( true );

  mResizeModeComboBox->addItem( tr( "Use Existing Frames" ), QgsLayoutMultiFrame::UseExistingFrames );
  mResizeModeComboBox->addItem( tr( "Extend to Next Page" ), QgsLayoutMultiFrame::ExtendToNextPage );
  mResizeModeComboBox->addItem( tr( "Repeat Until Finished" ), QgsLayoutMultiFrame::RepeatUntilFinished );

  mWrapBehaviorComboBox->addItem( tr( "Truncate Text" ), QgsLayoutTable::TruncateText );
  mWrapBehaviorComboBox->addItem( tr( "Wrap Text" ), QgsLayoutTable::WrapText );

  mHeaderModeComboBox->addItem( tr( "On First Frame" ), QgsLayoutTable::FirstFrame );
  mHeaderModeComboBox->addItem( tr( "On All Frames" ), QgsLayoutTable::AllFrames );
  mHeaderModeComboBox->addItem( tr( "No Header" ), QgsLayoutTable::NoHeaders );

  mHeaderHAlignmentComboBox->addItem( tr( "Follow Column Alignment" ), QgsLayoutTable::FollowColumn );
  mHeaderHAlignmentComboBox->addItem( tr( "Left" ), QgsLayoutTable::HeaderLeft );
  mHeaderHAlignmentComboBox->addItem( tr( "Center" ), QgsLayoutTable::HeaderCenter );
  mHeaderHAlignmentComboBox->addItem( tr( "Right" ), QgsLayoutTable::HeaderRight );

  mGridColorButton->setColorDialogTitle( tr( "Select Grid Color" ) );
  mGridColorButton->setAllowOpacity( true );
  mGridColorButton->setContext( QStringLiteral( "composer" ) );
  mGridColorButton->setDefaultColor( Qt::black );
  mBackgroundColorButton->setColorDialogTitle( tr( "Select Background Color" ) );
  mBackgroundColorButton->setAllowOpacity( true );
  mBackgroundColorButton->setContext( QStringLiteral( "composer" ) );
  mBackgroundColorButton->setShowNoColor( true );
  mBackgroundColorButton->setNoColorString( tr( "No Background" ) );

  updateGuiElements();

  if ( mTable )
  {
    connect( mTable, &QgsLayoutMultiFrame::changed, this, &QgsLayoutManualTableWidget::updateGuiElements );
  }

  //embed widget for general options
  if ( mFrame )
  {
    //add widget for general composer item properties
    mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, mFrame );
    mainLayout->addWidget( mItemPropertiesWidget );
  }

  connect( mHeaderFontToolButton, &QgsFontButton::changed, this, &QgsLayoutManualTableWidget::headerFontChanged );
  connect( mContentFontToolButton, &QgsFontButton::changed, this, &QgsLayoutManualTableWidget::contentFontChanged );
}

void QgsLayoutManualTableWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

QgsExpressionContext QgsLayoutManualTableWidget::createExpressionContext() const
{
  QgsExpressionContext context;

  // frames include their parent multiframe's context, so prefer that if possible
  if ( mFrame )
    context = mFrame->createExpressionContext();
  else if ( mTable )
    context = mTable->createExpressionContext();

  std::unique_ptr< QgsExpressionContextScope > cellScope = std::make_unique< QgsExpressionContextScope >();
  cellScope->setVariable( QStringLiteral( "row_number" ), 1, true );
  cellScope->setVariable( QStringLiteral( "column_number" ), 1, true );
  context.appendScope( cellScope.release() );

  context.setHighlightedVariables( { QStringLiteral( "row_number" ),
                                     QStringLiteral( "column_number" )} );

  return context;
}

bool QgsLayoutManualTableWidget::setNewItem( QgsLayoutItem *item )
{
  QgsLayoutFrame *frame = qobject_cast< QgsLayoutFrame * >( item );
  if ( !frame )
    return false;

  QgsLayoutMultiFrame *multiFrame = frame->multiFrame();
  if ( !multiFrame )
    return false;

  if ( multiFrame->type() != QgsLayoutItemRegistry::LayoutManualTable )
    return false;

  if ( mTable )
  {
    disconnect( mTable, &QgsLayoutObject::changed, this, &QgsLayoutManualTableWidget::updateGuiElements );
  }
  if ( mEditorDialog )
  {
    mEditorDialog->close();
  }

  mTable = qobject_cast< QgsLayoutItemManualTable * >( multiFrame );
  mFrame = frame;
  mItemPropertiesWidget->setItem( frame );

  if ( mTable )
  {
    connect( mTable, &QgsLayoutObject::changed, this, &QgsLayoutManualTableWidget::updateGuiElements );
  }

  updateGuiElements();

  return true;
}

void QgsLayoutManualTableWidget::setTableContents()
{
  if ( !mTable )
  {
    return;
  }

  if ( mEditorDialog )
  {
    // the unholy quadfecta
    mEditorDialog->show();
    mEditorDialog->raise();
    mEditorDialog->setWindowState( windowState() & ~Qt::WindowMinimized );
    mEditorDialog->activateWindow();
  }
  else
  {
    mEditorDialog = new QgsTableEditorDialog( this );
    mEditorDialog->registerExpressionContextGenerator( mTable );
    connect( this, &QWidget::destroyed, mEditorDialog, &QMainWindow::close );

    mEditorDialog->setIncludeTableHeader( mTable->includeTableHeader() );
    mEditorDialog->setTableContents( mTable->tableContents() );

    int row = 0;
    const QList< double > rowHeights = mTable->rowHeights();
    for ( const double height : rowHeights )
    {
      mEditorDialog->setTableRowHeight( row, height );
      row++;
    }
    int col = 0;
    const QList< double > columnWidths = mTable->columnWidths();
    QVariantList headers;
    headers.reserve( columnWidths.size() );
    for ( const double width : columnWidths )
    {
      mEditorDialog->setTableColumnWidth( col, width );
      headers << ( col < mTable->headers().count() ? mTable->headers().value( col ).heading() : QVariant() );
      col++;
    }
    mEditorDialog->setTableHeaders( headers );

    connect( mEditorDialog, &QgsTableEditorDialog::tableChanged, this, [ = ]
    {
      if ( mTable )
      {
        mTable->beginCommand( tr( "Change Table Contents" ) );
        mTable->setTableContents( mEditorDialog->tableContents() );

        const QVariantList headerText = mEditorDialog->tableHeaders();
        if ( mEditorDialog->includeTableHeader() )
        {
          QgsLayoutTableColumns headers;
          for ( const QVariant &h : headerText )
          {
            headers << QgsLayoutTableColumn( h.toString() );
          }
          mTable->setHeaders( headers );
        }

        const int rowCount = mTable->tableContents().size();
        QList< double > rowHeights;
        rowHeights.reserve( rowCount );
        for ( int row = 0; row < rowCount; ++row )
        {
          rowHeights << mEditorDialog->tableRowHeight( row );
        }
        mTable->setRowHeights( rowHeights );

        if ( !mTable->tableContents().empty() )
        {
          const int columnCount = mTable->tableContents().at( 0 ).size();
          QList< double > columnWidths;
          columnWidths.reserve( columnCount );
          for ( int col = 0; col < columnCount; ++col )
          {
            columnWidths << mEditorDialog->tableColumnWidth( col );
          }
          mTable->setColumnWidths( columnWidths );
        }

        mTable->endCommand();
      }
    } );

    connect( mEditorDialog, &QgsTableEditorDialog::includeHeaderChanged, this, [ = ]( bool included )
    {
      if ( mTable )
      {
        mTable->beginCommand( tr( "Change Table Header" ) );
        mTable->setIncludeTableHeader( included );
        mTable->endCommand();
      }
    } );
    mEditorDialog->show();
  }
}

void QgsLayoutManualTableWidget::mMarginSpinBox_valueChanged( double d )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Margin" ), QgsLayoutMultiFrame::UndoTableMargin );
  mTable->setCellMargin( d );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::contentFontChanged()
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Font" ) );
  mTable->setContentTextFormat( mContentFontToolButton->textFormat() );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::mGridStrokeWidthSpinBox_valueChanged( double d )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Line Width" ), QgsLayoutMultiFrame::UndoTableGridStrokeWidth );
  mTable->setGridStrokeWidth( d );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::mGridColorButton_colorChanged( const QColor &newColor )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Grid Color" ), QgsLayoutMultiFrame::UndoTableGridColor );
  mTable->setGridColor( newColor );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::mDrawHorizontalGrid_toggled( bool state )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Toggle Table Grid" ) );
  mTable->setHorizontalGrid( state );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::mDrawVerticalGrid_toggled( bool state )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Toggled Table Grid" ) );
  mTable->setVerticalGrid( state );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::mShowGridGroupCheckBox_toggled( bool state )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Toggle Table Grid" ) );
  mTable->setShowGrid( state );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::mHeaderHAlignmentComboBox_currentIndexChanged( int )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Alignment" ) );
  mTable->setHeaderHAlignment( static_cast<  QgsLayoutTable::HeaderHAlignment >( mHeaderHAlignmentComboBox->currentData().toInt() ) );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::mHeaderModeComboBox_currentIndexChanged( int )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Header Mode" ) );
  mTable->setHeaderMode( static_cast< QgsLayoutTable::HeaderMode >( mHeaderModeComboBox->currentData().toInt() ) );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::mBackgroundColorButton_colorChanged( const QColor &newColor )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Color" ), QgsLayoutMultiFrame::UndoTableBackgroundColor );
  mTable->setBackgroundColor( newColor );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::headerFontChanged()
{
  if ( !mTable )
    return;

  mTable->beginCommand( tr( "Change Table Font" ) );
  mTable->setHeaderTextFormat( mHeaderFontToolButton->textFormat() );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::updateGuiElements()
{
  if ( !mTable || !mFrame )
  {
    return;
  }

  blockAllSignals( true );

  mMarginSpinBox->setValue( mTable->cellMargin() );
  mGridStrokeWidthSpinBox->setValue( mTable->gridStrokeWidth() );
  mGridColorButton->setColor( mTable->gridColor() );
  mDrawHorizontalGrid->setChecked( mTable->horizontalGrid() );
  mDrawVerticalGrid->setChecked( mTable->verticalGrid() );
  if ( mTable->showGrid() )
  {
    mShowGridGroupCheckBox->setChecked( true );
  }
  else
  {
    mShowGridGroupCheckBox->setChecked( false );
  }
  mBackgroundColorButton->setColor( mTable->backgroundColor() );

  mHeaderFontToolButton->setTextFormat( mTable->headerTextFormat() );
  mContentFontToolButton->setTextFormat( mTable->contentTextFormat() );

  mDrawEmptyCheckBox->setChecked( mTable->showEmptyRows() );
  mWrapBehaviorComboBox->setCurrentIndex( mWrapBehaviorComboBox->findData( mTable->wrapBehavior() ) );

  mResizeModeComboBox->setCurrentIndex( mResizeModeComboBox->findData( mTable->resizeMode() ) );
  mAddFramePushButton->setEnabled( mTable->resizeMode() == QgsLayoutMultiFrame::UseExistingFrames );

  mHeaderHAlignmentComboBox->setCurrentIndex( mHeaderHAlignmentComboBox->findData( mTable->headerHAlignment() ) );
  mHeaderModeComboBox->setCurrentIndex( mHeaderModeComboBox->findData( mTable->headerMode() ) );

  mEmptyFrameCheckBox->setChecked( mFrame->hidePageIfEmpty() );
  mHideEmptyBgCheckBox->setChecked( mFrame->hideBackgroundIfEmpty() );

  blockAllSignals( false );
}

void QgsLayoutManualTableWidget::blockAllSignals( bool b )
{
  mMarginSpinBox->blockSignals( b );
  mGridColorButton->blockSignals( b );
  mGridStrokeWidthSpinBox->blockSignals( b );
  mBackgroundColorButton->blockSignals( b );
  mDrawHorizontalGrid->blockSignals( b );
  mDrawVerticalGrid->blockSignals( b );
  mShowGridGroupCheckBox->blockSignals( b );
  mResizeModeComboBox->blockSignals( b );
  mEmptyFrameCheckBox->blockSignals( b );
  mHideEmptyBgCheckBox->blockSignals( b );
  mDrawEmptyCheckBox->blockSignals( b );
  mWrapBehaviorComboBox->blockSignals( b );
  mContentFontToolButton->blockSignals( b );
  mHeaderHAlignmentComboBox->blockSignals( b );
  mHeaderModeComboBox->blockSignals( b );
  mHeaderFontToolButton->blockSignals( b );
}

void QgsLayoutManualTableWidget::mEmptyFrameCheckBox_toggled( bool checked )
{
  if ( !mFrame )
  {
    return;
  }

  mFrame->beginCommand( tr( "Toggle Empty Frame Mode" ) );
  mFrame->setHidePageIfEmpty( checked );
  mFrame->endCommand();
}

void QgsLayoutManualTableWidget::mHideEmptyBgCheckBox_toggled( bool checked )
{
  if ( !mFrame )
  {
    return;
  }

  mFrame->beginCommand( tr( "Toggle Background Display" ) );
  mFrame->setHideBackgroundIfEmpty( checked );
  mFrame->endCommand();
}

void QgsLayoutManualTableWidget::mAddFramePushButton_clicked()
{
  if ( !mTable || !mFrame )
  {
    return;
  }

  //create a new frame based on the current frame
  QPointF pos = mFrame->pos();
  //shift new frame so that it sits 10 units below current frame
  pos.ry() += mFrame->rect().height() + 10;

  QgsLayoutFrame *newFrame = mTable->createNewFrame( mFrame, pos, mFrame->rect().size() );
  mTable->recalculateFrameSizes();

  //set new frame as selection
  if ( QgsLayout *layout = mTable->layout() )
  {
    layout->setSelectedItem( newFrame );
  }
}

void QgsLayoutManualTableWidget::mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Resize Mode" ) );
  mTable->setResizeMode( static_cast< QgsLayoutMultiFrame::ResizeMode >( mResizeModeComboBox->itemData( index ).toInt() ) );
  mTable->endCommand();

  mAddFramePushButton->setEnabled( mTable->resizeMode() == QgsLayoutMultiFrame::UseExistingFrames );
}

void QgsLayoutManualTableWidget::mWrapBehaviorComboBox_currentIndexChanged( int index )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Wrap Mode" ) );
  mTable->setWrapBehavior( static_cast< QgsLayoutTable::WrapBehavior >( mWrapBehaviorComboBox->itemData( index ).toInt() ) );
  mTable->endCommand();
}

void QgsLayoutManualTableWidget::mAdvancedCustomizationButton_clicked()
{
  if ( !mTable )
  {
    return;
  }

  QgsLayoutTableBackgroundColorsDialog d( mTable, this );

  d.exec();
}

void QgsLayoutManualTableWidget::mDrawEmptyCheckBox_toggled( bool checked )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Show Empty Rows" ) );
  mTable->setShowEmptyRows( checked );
  mTable->endCommand();
}

