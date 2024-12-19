/***************************************************************************
                         qgscoordinateoperationwidget.cpp
                         ---------------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgscoordinateoperationwidget.h"
#include "moc_qgscoordinateoperationwidget.cpp"
#include "qgscoordinatetransform.h"
#include "qgsprojectionselectiondialog.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsguiutils.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsinstallgridshiftdialog.h"

#include <QDir>
#include <QPushButton>
#include <QRegularExpression>

#include "qgsprojutils.h"
#include <proj.h>

QgsCoordinateOperationWidget::QgsCoordinateOperationWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mLabelSrcDescription->setTextInteractionFlags( Qt::TextBrowserInteraction );
  mLabelSrcDescription->setOpenExternalLinks( true );
  mInstallGridButton->hide();

  connect( mInstallGridButton, &QPushButton::clicked, this, &QgsCoordinateOperationWidget::installGrid );
  connect( mAllowFallbackCheckBox, &QCheckBox::toggled, this, [=] {
    if ( !mBlockSignals )
      emit operationChanged();
  } );
  mCoordinateOperationTableWidget->setColumnCount( 3 );

  QStringList headers;
  headers << tr( "Transformation" ) << tr( "Accuracy (meters)" ) << tr( "Area of Use" );
  mCoordinateOperationTableWidget->setHorizontalHeaderLabels( headers );

  mHideDeprecatedCheckBox->setVisible( false );
  mShowSupersededCheckBox->setVisible( true );
  mLabelDstDescription->hide();

  connect( mHideDeprecatedCheckBox, &QCheckBox::stateChanged, this, [=] { loadAvailableOperations(); } );
  connect( mShowSupersededCheckBox, &QCheckBox::toggled, this, &QgsCoordinateOperationWidget::showSupersededToggled );
  connect( mCoordinateOperationTableWidget, &QTableWidget::currentItemChanged, this, &QgsCoordinateOperationWidget::tableCurrentItemChanged );
  connect( mCoordinateOperationTableWidget, &QTableWidget::itemDoubleClicked, this, &QgsCoordinateOperationWidget::operationDoubleClicked );

  mLabelSrcDescription->clear();
  mLabelDstDescription->clear();
}

void QgsCoordinateOperationWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  if ( canvas )
  {
    // show canvas extent in preview widget
    QPolygonF mainCanvasPoly = canvas->mapSettings().visiblePolygon();
    QgsGeometry g = QgsGeometry::fromQPolygonF( mainCanvasPoly );
    // close polygon
    mainCanvasPoly << mainCanvasPoly.at( 0 );
    if ( QgsProject::instance()->crs() != QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) )
    {
      // reproject extent
      QgsCoordinateTransform ct( QgsProject::instance()->crs(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsProject::instance() );
      ct.setBallparkTransformsAreAppropriate( true );
      g = g.densifyByCount( 5 );
      try
      {
        g.transform( ct );
      }
      catch ( QgsCsException & )
      {
      }
    }
    mAreaCanvas->setCanvasRect( g.boundingBox() );
  }
}

void QgsCoordinateOperationWidget::setShowMakeDefault( bool show )
{
  mMakeDefaultCheckBox->setVisible( show );
}

bool QgsCoordinateOperationWidget::makeDefaultSelected() const
{
  return mMakeDefaultCheckBox->isChecked();
}

bool QgsCoordinateOperationWidget::hasSelection() const
{
  return !mCoordinateOperationTableWidget->selectedItems().isEmpty();
}

QList<QgsCoordinateOperationWidget::OperationDetails> QgsCoordinateOperationWidget::availableOperations() const
{
  QList<QgsCoordinateOperationWidget::OperationDetails> res;
  res.reserve( mDatumTransforms.size() );
  for ( const QgsDatumTransform::TransformDetails &details : mDatumTransforms )
  {
    OperationDetails op;
    op.proj = details.proj;
    op.sourceTransformId = -1;
    op.destinationTransformId = -1;
    op.isAvailable = details.isAvailable;
    res << op;
  }
  return res;
}

void QgsCoordinateOperationWidget::loadAvailableOperations()
{
  mCoordinateOperationTableWidget->setRowCount( 0 );

  int row = 0;
  int preferredInitialRow = -1;

  for ( const QgsDatumTransform::TransformDetails &transform : std::as_const( mDatumTransforms ) )
  {
    std::unique_ptr<QTableWidgetItem> item = std::make_unique<QTableWidgetItem>();
    item->setData( ProjRole, transform.proj );
    item->setData( AvailableRole, transform.isAvailable );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );

    QString name = transform.name;
    if ( !transform.authority.isEmpty() && !transform.code.isEmpty() )
      name += QStringLiteral( " %1 %2:%3" ).arg( QString( QChar( 0x2013 ) ), transform.authority, transform.code );
    item->setText( name );

    if ( row == 0 ) // highlight first (preferred) operation
    {
      QFont f = item->font();
      f.setBold( true );
      item->setFont( f );
      item->setForeground( QBrush( QColor( 0, 120, 0 ) ) );
    }

    if ( !transform.isAvailable )
    {
      item->setForeground( QBrush( palette().color( QPalette::Disabled, QPalette::Text ) ) );
    }

    if ( preferredInitialRow < 0 && transform.isAvailable )
    {
      // try to select a "preferred" entry by default
      preferredInitialRow = row;
    }

    QString missingMessage;
    if ( !transform.isAvailable )
    {
      QStringList gridMessages;
      QStringList missingGrids;
      QStringList missingGridPackages;
      QStringList missingGridUrls;

      for ( const QgsDatumTransform::GridDetails &grid : transform.grids )
      {
        if ( !grid.isAvailable )
        {
          missingGrids << grid.shortName;
          missingGridPackages << grid.packageName;
          missingGridUrls << grid.url;
          QString m = tr( "This transformation requires the grid file “%1”, which is not available for use on the system." ).arg( grid.shortName );
          if ( !grid.url.isEmpty() )
          {
            if ( !grid.packageName.isEmpty() )
            {
              m += ' ' + tr( "This grid is part of the <i>%1</i> package, available for download from <a href=\"%2\">%2</a>." ).arg( grid.packageName, grid.url );
            }
            else
            {
              m += ' ' + tr( "This grid is available for download from <a href=\"%1\">%1</a>." ).arg( grid.url );
            }
          }
          gridMessages << m;
        }
      }

      item->setData( MissingGridsRole, missingGrids );
      item->setData( MissingGridPackageNamesRole, missingGridPackages );
      item->setData( MissingGridUrlsRole, missingGridUrls );

      if ( gridMessages.count() > 1 )
      {
        for ( int k = 0; k < gridMessages.count(); ++k )
          gridMessages[k] = QStringLiteral( "<li>%1</li>" ).arg( gridMessages.at( k ) );

        missingMessage = QStringLiteral( "<ul>%1</ul" ).arg( gridMessages.join( QString() ) );
      }
      else if ( !gridMessages.empty() )
      {
        missingMessage = gridMessages.constFirst();
      }
    }

    QStringList areasOfUse;
    QStringList authorityCodes;

    QStringList opText;
    QString lastSingleOpScope;
    QString lastSingleOpRemarks;
    for ( const QgsDatumTransform::SingleOperationDetails &singleOpDetails : transform.operationDetails )
    {
      QString text;
      if ( !singleOpDetails.scope.isEmpty() )
      {
        text += QStringLiteral( "<b>%1</b>: %2" ).arg( tr( "Scope" ), formatScope( singleOpDetails.scope ) );
        lastSingleOpScope = singleOpDetails.scope;
      }
      if ( !singleOpDetails.remarks.isEmpty() )
      {
        if ( !text.isEmpty() )
          text += QLatin1String( "<br>" );
        text += QStringLiteral( "<b>%1</b>: %2" ).arg( tr( "Remarks" ), singleOpDetails.remarks );
        lastSingleOpRemarks = singleOpDetails.remarks;
      }
      if ( !singleOpDetails.areaOfUse.isEmpty() )
      {
        if ( !areasOfUse.contains( singleOpDetails.areaOfUse ) )
          areasOfUse << singleOpDetails.areaOfUse;
      }
      if ( !singleOpDetails.authority.isEmpty() && !singleOpDetails.code.isEmpty() )
      {
        const QString identifier = QStringLiteral( "%1:%2" ).arg( singleOpDetails.authority, singleOpDetails.code );
        if ( !authorityCodes.contains( identifier ) )
          authorityCodes << identifier;
      }

      if ( !text.isEmpty() )
      {
        opText.append( text );
      }
    }

    QString text;
    if ( !transform.scope.isEmpty() && transform.scope != lastSingleOpScope )
    {
      text += QStringLiteral( "<b>%1</b>: %2" ).arg( tr( "Scope" ), transform.scope );
    }
    if ( !transform.remarks.isEmpty() && transform.remarks != lastSingleOpRemarks )
    {
      if ( !text.isEmpty() )
        text += QLatin1String( "<br>" );
      text += QStringLiteral( "<b>%1</b>: %2" ).arg( tr( "Remarks" ), transform.remarks );
    }
    if ( !text.isEmpty() )
    {
      opText.append( text );
    }

    if ( opText.count() > 1 )
    {
      for ( int k = 0; k < opText.count(); ++k )
        opText[k] = QStringLiteral( "<li>%1</li>" ).arg( opText.at( k ) );
    }

    if ( !transform.areaOfUse.isEmpty() && !areasOfUse.contains( transform.areaOfUse ) )
      areasOfUse << transform.areaOfUse;
    item->setData( BoundsRole, transform.bounds );

    const QString id = !transform.authority.isEmpty() && !transform.code.isEmpty() ? QStringLiteral( "%1:%2" ).arg( transform.authority, transform.code ) : QString();
    if ( !id.isEmpty() && !authorityCodes.contains( id ) )
      authorityCodes << id;

    const QColor disabled = palette().color( QPalette::Disabled, QPalette::Text );
    const QColor active = palette().color( QPalette::Active, QPalette::Text );

    const QColor codeColor( static_cast<int>( active.red() * 0.6 + disabled.red() * 0.4 ), static_cast<int>( active.green() * 0.6 + disabled.green() * 0.4 ), static_cast<int>( active.blue() * 0.6 + disabled.blue() * 0.4 ) );
    const QString toolTipString = QStringLiteral( "<b>%1</b>" ).arg( transform.name )
                                  + ( !opText.empty() ? ( opText.count() == 1 ? QStringLiteral( "<p>%1</p>" ).arg( opText.at( 0 ) ) : QStringLiteral( "<ul>%1</ul>" ).arg( opText.join( QString() ) ) ) : QString() )
                                  + ( !areasOfUse.empty() ? QStringLiteral( "<p><b>%1</b>: %2</p>" ).arg( tr( "Area of use" ), areasOfUse.join( QLatin1String( ", " ) ) ) : QString() )
                                  + ( !authorityCodes.empty() ? QStringLiteral( "<p><b>%1</b>: %2</p>" ).arg( tr( "Identifiers" ), authorityCodes.join( QLatin1String( ", " ) ) ) : QString() )
                                  + ( !missingMessage.isEmpty() ? QStringLiteral( "<p><b style=\"color: red\">%1</b></p>" ).arg( missingMessage ) : QString() )
                                  + QStringLiteral( "<p><code style=\"color: %1\">%2</code></p>" ).arg( codeColor.name(), transform.proj );

    item->setToolTip( toolTipString );
    mCoordinateOperationTableWidget->setRowCount( row + 1 );
    mCoordinateOperationTableWidget->setItem( row, 0, item.release() );

    item = std::make_unique<QTableWidgetItem>();
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    item->setText( transform.accuracy >= 0 ? QLocale().toString( transform.accuracy ) : tr( "Unknown" ) );
    item->setToolTip( toolTipString );
    if ( !transform.isAvailable )
    {
      item->setForeground( QBrush( palette().color( QPalette::Disabled, QPalette::Text ) ) );
    }
    mCoordinateOperationTableWidget->setItem( row, 1, item.release() );

    // area of use column
    item = std::make_unique<QTableWidgetItem>();
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    item->setText( areasOfUse.join( QLatin1String( ", " ) ) );
    item->setToolTip( toolTipString );
    if ( !transform.isAvailable )
    {
      item->setForeground( QBrush( palette().color( QPalette::Disabled, QPalette::Text ) ) );
    }
    mCoordinateOperationTableWidget->setItem( row, 2, item.release() );

    row++;
  }

  if ( mCoordinateOperationTableWidget->currentRow() < 0 )
    mCoordinateOperationTableWidget->selectRow( preferredInitialRow >= 0 ? preferredInitialRow : 0 );

  mCoordinateOperationTableWidget->resizeColumnsToContents();

  tableCurrentItemChanged( nullptr, nullptr );
}

QgsCoordinateOperationWidget::~QgsCoordinateOperationWidget()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/DatumTransformDialog/hideDeprecated" ), mHideDeprecatedCheckBox->isChecked() );

  for ( int i = 0; i < 2; i++ )
  {
    settings.setValue( QStringLiteral( "Windows/DatumTransformDialog/columnWidths/%1" ).arg( i ), mCoordinateOperationTableWidget->columnWidth( i ) );
  }
}

QgsCoordinateOperationWidget::OperationDetails QgsCoordinateOperationWidget::defaultOperation() const
{
  OperationDetails preferred;

  // for proj 6, return the first available transform -- they are sorted by preference by proj already
  for ( const QgsDatumTransform::TransformDetails &transform : std::as_const( mDatumTransforms ) )
  {
    if ( transform.isAvailable )
    {
      preferred.proj = transform.proj;
      preferred.isAvailable = transform.isAvailable;
      break;
    }
  }
  return preferred;
}

QString QgsCoordinateOperationWidget::formatScope( const QString &s )
{
  QString scope = s;

  const thread_local QRegularExpression reGNSS( QStringLiteral( "\\bGNSS\\b" ) );
  scope.replace( reGNSS, QObject::tr( "GNSS (Global Navigation Satellite System)" ) );

  const thread_local QRegularExpression reCORS( QStringLiteral( "\\bCORS\\b" ) );
  scope.replace( reCORS, QObject::tr( "CORS (Continually Operating Reference Station)" ) );

  return scope;
}

QgsCoordinateOperationWidget::OperationDetails QgsCoordinateOperationWidget::selectedOperation() const
{
  int row = mCoordinateOperationTableWidget->currentRow();
  OperationDetails op;

  if ( row >= 0 )
  {
    QTableWidgetItem *srcItem = mCoordinateOperationTableWidget->item( row, 0 );
    op.sourceTransformId = srcItem ? srcItem->data( TransformIdRole ).toInt() : -1;
    QTableWidgetItem *destItem = mCoordinateOperationTableWidget->item( row, 1 );
    op.destinationTransformId = destItem ? destItem->data( TransformIdRole ).toInt() : -1;
    op.proj = srcItem ? srcItem->data( ProjRole ).toString() : QString();
    op.isAvailable = srcItem ? srcItem->data( AvailableRole ).toBool() : true;
    op.allowFallback = mAllowFallbackCheckBox->isChecked();
  }
  else
  {
    op.sourceTransformId = -1;
    op.destinationTransformId = -1;
    op.proj = QString();
  }
  return op;
}

void QgsCoordinateOperationWidget::setSelectedOperation( const QgsCoordinateOperationWidget::OperationDetails &operation )
{
  int prevRow = mCoordinateOperationTableWidget->currentRow();
  mBlockSignals++;
  for ( int row = 0; row < mCoordinateOperationTableWidget->rowCount(); ++row )
  {
    QTableWidgetItem *srcItem = mCoordinateOperationTableWidget->item( row, 0 );
    if ( srcItem && srcItem->data( ProjRole ).toString() == operation.proj )
    {
      mCoordinateOperationTableWidget->selectRow( row );
      break;
    }
  }

  bool fallbackChanged = mAllowFallbackCheckBox->isChecked() != operation.allowFallback;
  mAllowFallbackCheckBox->setChecked( operation.allowFallback );
  mBlockSignals--;

  if ( mCoordinateOperationTableWidget->currentRow() != prevRow || fallbackChanged )
    emit operationChanged();
}

void QgsCoordinateOperationWidget::setSelectedOperationUsingContext( const QgsCoordinateTransformContext &context )
{
  const QString op = context.calculateCoordinateOperation( mSourceCrs, mDestinationCrs );
  if ( !op.isEmpty() )
  {
    OperationDetails deets;
    deets.proj = op;
    deets.allowFallback = context.allowFallbackTransform( mSourceCrs, mDestinationCrs );
    setSelectedOperation( deets );
  }
  else
  {
    setSelectedOperation( defaultOperation() );
  }
}

void QgsCoordinateOperationWidget::setShowFallbackOption( bool visible )
{
  mAllowFallbackCheckBox->setVisible( visible );
}

bool QgsCoordinateOperationWidget::gridShiftTransformation( const QString &itemText ) const
{
  return !itemText.isEmpty() && !itemText.contains( QLatin1String( "towgs84" ), Qt::CaseInsensitive );
}

bool QgsCoordinateOperationWidget::testGridShiftFileAvailability( QTableWidgetItem *item ) const
{
  if ( !item )
  {
    return true;
  }

  QString itemText = item->text();
  if ( itemText.isEmpty() )
  {
    return true;
  }

  char *projLib = getenv( "PROJ_LIB" );
  if ( !projLib ) //no information about installation directory
  {
    return true;
  }

  QStringList itemEqualSplit = itemText.split( '=' );
  QString filename;
  for ( int i = 1; i < itemEqualSplit.size(); ++i )
  {
    if ( i > 1 )
    {
      filename.append( '=' );
    }
    filename.append( itemEqualSplit.at( i ) );
  }

  QDir projDir( projLib );
  if ( projDir.exists() )
  {
    //look if filename in directory
    QStringList fileList = projDir.entryList();
    QStringList::const_iterator fileIt = fileList.constBegin();
    for ( ; fileIt != fileList.constEnd(); ++fileIt )
    {
#if defined( Q_OS_WIN )
      if ( fileIt->compare( filename, Qt::CaseInsensitive ) == 0 )
#else
      if ( fileIt->compare( filename ) == 0 )
#endif //Q_OS_WIN
      {
        return true;
      }
    }
    item->setToolTip( tr( "File '%1' not found in directory '%2'" ).arg( filename, projDir.absolutePath() ) );
    return false; //not found in PROJ_LIB directory
  }
  return true;
}

void QgsCoordinateOperationWidget::tableCurrentItemChanged( QTableWidgetItem *, QTableWidgetItem * )
{
  int row = mCoordinateOperationTableWidget->currentRow();
  if ( row < 0 )
  {
    mLabelSrcDescription->clear();
    mLabelDstDescription->clear();
    mAreaCanvas->hide();
    mInstallGridButton->hide();
  }
  else
  {
    QTableWidgetItem *srcItem = mCoordinateOperationTableWidget->item( row, 0 );
    mLabelSrcDescription->setText( srcItem ? srcItem->toolTip() : QString() );
    if ( srcItem )
    {
      // find area of intersection of operation, source and dest bounding boxes
      // see https://github.com/OSGeo/PROJ/issues/1549 for justification
      const QgsRectangle operationRect = srcItem->data( BoundsRole ).value<QgsRectangle>();
      const QgsRectangle sourceRect = mSourceCrs.bounds();
      const QgsRectangle destRect = mDestinationCrs.bounds();
      QgsRectangle rect = operationRect.intersect( sourceRect );
      rect = rect.intersect( destRect );

      mAreaCanvas->setPreviewRect( rect );
      mAreaCanvas->show();

      const QStringList missingGrids = srcItem->data( MissingGridsRole ).toStringList();
      mInstallGridButton->setVisible( !missingGrids.empty() );
      if ( !missingGrids.empty() )
      {
        mInstallGridButton->setText( tr( "Install “%1” Grid…" ).arg( missingGrids.at( 0 ) ) );
      }
    }
    else
    {
      mAreaCanvas->setPreviewRect( QgsRectangle() );
      mAreaCanvas->hide();
      mInstallGridButton->hide();
    }
    QTableWidgetItem *destItem = mCoordinateOperationTableWidget->item( row, 1 );
    mLabelDstDescription->setText( destItem ? destItem->toolTip() : QString() );
  }
  OperationDetails newOp = selectedOperation();
  if ( newOp.proj != mPreviousOp.proj && !mBlockSignals )
    emit operationChanged();
  mPreviousOp = newOp;
}

void QgsCoordinateOperationWidget::setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs )
{
  mSourceCrs = sourceCrs;
  mDatumTransforms = QgsDatumTransform::operations( mSourceCrs, mDestinationCrs, mShowSupersededCheckBox->isChecked() );
  loadAvailableOperations();
}

void QgsCoordinateOperationWidget::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  mDestinationCrs = destinationCrs;
  mDatumTransforms = QgsDatumTransform::operations( mSourceCrs, mDestinationCrs, mShowSupersededCheckBox->isChecked() );
  loadAvailableOperations();
}

void QgsCoordinateOperationWidget::showSupersededToggled( bool )
{
  mDatumTransforms = QgsDatumTransform::operations( mSourceCrs, mDestinationCrs, mShowSupersededCheckBox->isChecked() );
  loadAvailableOperations();
}

void QgsCoordinateOperationWidget::installGrid()
{
  int row = mCoordinateOperationTableWidget->currentRow();
  QTableWidgetItem *srcItem = mCoordinateOperationTableWidget->item( row, 0 );
  if ( !srcItem )
    return;

  const QStringList missingGrids = srcItem->data( MissingGridsRole ).toStringList();
  if ( missingGrids.empty() )
    return;

  const QStringList missingGridPackagesNames = srcItem->data( MissingGridPackageNamesRole ).toStringList();
  const QString packageName = missingGridPackagesNames.value( 0 );
  const QStringList missingGridUrls = srcItem->data( MissingGridUrlsRole ).toStringList();
  const QString gridUrl = missingGridUrls.value( 0 );

  QString downloadMessage;
  if ( !packageName.isEmpty() )
  {
    downloadMessage = tr( "This grid is part of the “<i>%1</i>” package, available for download from <a href=\"%2\">%2</a>." ).arg( packageName, gridUrl );
  }
  else if ( !gridUrl.isEmpty() )
  {
    downloadMessage = tr( "This grid is available for download from <a href=\"%1\">%1</a>." ).arg( gridUrl );
  }

  const QString longMessage = tr( "<p>This transformation requires the grid file “%1”, which is not available for use on the system.</p>" ).arg( missingGrids.at( 0 ) );

  QgsInstallGridShiftFileDialog *dlg = new QgsInstallGridShiftFileDialog( missingGrids.at( 0 ), this );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->setWindowTitle( tr( "Install Grid File" ) );
  dlg->setDescription( longMessage );
  dlg->setDownloadMessage( downloadMessage );
  dlg->exec();
}
