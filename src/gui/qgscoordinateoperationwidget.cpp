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
#include "qgscoordinatetransform.h"
#include "qgsprojectionselectiondialog.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsguiutils.h"
#include "qgsgui.h"
#include "qgshelp.h"

#include <QDir>
#include <QPushButton>

#if PROJ_VERSION_MAJOR>=6
#include "qgsprojutils.h"
#include <proj.h>
#endif

QgsCoordinateOperationWidget::QgsCoordinateOperationWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mLabelSrcDescription->setTextInteractionFlags( Qt::TextBrowserInteraction );
  mLabelSrcDescription->setOpenExternalLinks( true );

#if PROJ_VERSION_MAJOR>=6
  mCoordinateOperationTableWidget->setColumnCount( 3 );
#else
  mCoordinateOperationTableWidget->setColumnCount( 2 );
#endif

  QStringList headers;
#if PROJ_VERSION_MAJOR>=6
  headers << tr( "Transformation" ) << tr( "Accuracy (meters)" ) << tr( "Area of Use" );
#else
  headers << tr( "Source Transform" ) << tr( "Destination Transform" ) ;
#endif
  mCoordinateOperationTableWidget->setHorizontalHeaderLabels( headers );

#if PROJ_VERSION_MAJOR<6
  mAreaCanvas->hide();
#endif

#if PROJ_VERSION_MAJOR>=6
  // proj 6 doesn't provide deprecated operations
  mHideDeprecatedCheckBox->setVisible( false );

#if PROJ_VERSION_MAJOR>6 || PROJ_VERSION_MINOR>=2
  mShowSupersededCheckBox->setVisible( true );
#else
  mShowSupersededCheckBox->setVisible( false );
#endif

  mLabelDstDescription->hide();
#else
  mShowSupersededCheckBox->setVisible( false );
  QgsSettings settings;
  mHideDeprecatedCheckBox->setChecked( settings.value( QStringLiteral( "Windows/DatumTransformDialog/hideDeprecated" ), true ).toBool() );
#endif

  connect( mHideDeprecatedCheckBox, &QCheckBox::stateChanged, this, [ = ] { loadAvailableOperations(); } );
  connect( mShowSupersededCheckBox, &QCheckBox::toggled, this, &QgsCoordinateOperationWidget::showSupersededToggled );
  connect( mCoordinateOperationTableWidget, &QTableWidget::currentItemChanged, this, &QgsCoordinateOperationWidget::tableCurrentItemChanged );
  connect( mCoordinateOperationTableWidget, &QTableWidget::itemDoubleClicked, this, &QgsCoordinateOperationWidget::operationDoubleClicked );

  mLabelSrcDescription->clear();
  mLabelDstDescription->clear();
}

void QgsCoordinateOperationWidget::setMapCanvas( QgsMapCanvas *canvas )
{
#if PROJ_VERSION_MAJOR<6
  ( void )canvas;
#else
  if ( canvas )
  {
    // show canvas extent in preview widget
    QPolygonF mainCanvasPoly = canvas->mapSettings().visiblePolygon();
    QgsGeometry g = QgsGeometry::fromQPolygonF( mainCanvasPoly );
    // close polygon
    mainCanvasPoly << mainCanvasPoly.at( 0 );
    if ( QgsProject::instance()->crs() !=
         QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) )
    {
      // reproject extent
      QgsCoordinateTransform ct( QgsProject::instance()->crs(),
                                 QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsProject::instance() );

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
#endif
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
#if PROJ_VERSION_MAJOR>=6
  for ( const QgsDatumTransform::TransformDetails &details : mDatumTransforms )
  {
    OperationDetails op;
    op.proj = details.proj;
    op.sourceTransformId = -1;
    op.destinationTransformId = -1;
    op.isAvailable = details.isAvailable;
    res << op;
  }
#else
  for ( const QgsDatumTransform::TransformPair &details : mDatumTransforms )
  {
    OperationDetails op;
    op.sourceTransformId = details.sourceTransformId;
    op.destinationTransformId = details.destinationTransformId;
    res << op;
  }
#endif
  return res;
}

void QgsCoordinateOperationWidget::loadAvailableOperations()
{
  mCoordinateOperationTableWidget->setRowCount( 0 );

  int row = 0;
  int preferredInitialRow = -1;
#if PROJ_VERSION_MAJOR>=6
  for ( const QgsDatumTransform::TransformDetails &transform : qgis::as_const( mDatumTransforms ) )
  {
    std::unique_ptr< QTableWidgetItem > item = qgis::make_unique< QTableWidgetItem >();
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
      for ( const QgsDatumTransform::GridDetails &grid : transform.grids )
      {
        if ( !grid.isAvailable )
        {
          QString m = tr( "This transformation requires the grid file “%1”, which is not available for use on the system." ).arg( grid.shortName );
          if ( !grid.url.isEmpty() )
          {
            if ( !grid.packageName.isEmpty() )
            {
              m += ' ' +  tr( "This grid is part of the <i>%1</i> package, available for download from <a href=\"%2\">%2</a>." ).arg( grid.packageName, grid.url );
            }
            else
            {
              m += ' ' + tr( "This grid is available for download from <a href=\"%1\">%1</a>." ).arg( grid.url );
            }
          }
          gridMessages << m;
        }
      }

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

#if PROJ_VERSION_MAJOR > 6 || PROJ_VERSION_MINOR >= 2
    QStringList opText;
    for ( const QgsDatumTransform::SingleOperationDetails &singleOpDetails : transform.operationDetails )
    {
      QString text;
      if ( !singleOpDetails.scope.isEmpty() )
      {
        text += QStringLiteral( "<b>%1</b>: %2" ).arg( tr( "Scope" ), formatScope( singleOpDetails.scope ) );
      }
      if ( !singleOpDetails.remarks.isEmpty() )
      {
        if ( !text.isEmpty() )
          text += QStringLiteral( "<br>" );
        text += QStringLiteral( "<b>%1</b>: %2" ).arg( tr( "Remarks" ), singleOpDetails.remarks );
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
    if ( !transform.scope.isEmpty() )
    {
      text += QStringLiteral( "<b>%1</b>: %2" ).arg( tr( "Scope" ), transform.scope );
    }
    if ( !transform.remarks.isEmpty() )
    {
      if ( !text.isEmpty() )
        text += QStringLiteral( "<br>" );
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
#endif

    if ( !transform.areaOfUse.isEmpty() && !areasOfUse.contains( transform.areaOfUse ) )
      areasOfUse << transform.areaOfUse;
    item->setData( BoundsRole, transform.bounds );

    const QString id = !transform.authority.isEmpty() && !transform.code.isEmpty() ? QStringLiteral( "%1:%2" ).arg( transform.authority, transform.code ) : QString();
    if ( !id.isEmpty() && !authorityCodes.contains( id ) )
      authorityCodes << id;

#if PROJ_VERSION_MAJOR > 6 || PROJ_VERSION_MINOR >= 2
    const QColor disabled = palette().color( QPalette::Disabled, QPalette::Text );
    const QColor active = palette().color( QPalette::Active, QPalette::Text );

    const QColor codeColor( static_cast< int >( active.red() * 0.6 + disabled.red() * 0.4 ),
                            static_cast< int >( active.green() * 0.6 + disabled.green() * 0.4 ),
                            static_cast< int >( active.blue() * 0.6 + disabled.blue() * 0.4 ) );
    const QString toolTipString = QStringLiteral( "<b>%1</b>" ).arg( transform.name )
                                  + ( !opText.empty() ? ( opText.count() == 1 ? QStringLiteral( "<p>%1</p>" ).arg( opText.at( 0 ) ) : QStringLiteral( "<ul>%1</ul>" ).arg( opText.join( QString() ) ) ) : QString() )
                                  + ( !areasOfUse.empty() ? QStringLiteral( "<p><b>%1</b>: %2</p>" ).arg( tr( "Area of use" ), areasOfUse.join( QStringLiteral( ", " ) ) ) : QString() )
                                  + ( !authorityCodes.empty() ? QStringLiteral( "<p><b>%1</b>: %2</p>" ).arg( tr( "Identifiers" ), authorityCodes.join( QStringLiteral( ", " ) ) ) : QString() )
                                  + ( !missingMessage.isEmpty() ? QStringLiteral( "<p><b style=\"color: red\">%1</b></p>" ).arg( missingMessage ) : QString() )
                                  + QStringLiteral( "<p><code style=\"color: %1\">%2</code></p>" ).arg( codeColor.name(), transform.proj );
#else
    const QString toolTipString = QStringLiteral( "<b>%1</b>%2%3%4<p><code>%5</code></p>" ).arg( transform.name,
                                  ( !transform.areaOfUse.isEmpty() ? QStringLiteral( "<p><b>%1</b>: %2</p>" ).arg( tr( "Area of use" ), transform.areaOfUse ) : QString() ),
                                  ( !id.isEmpty() ? QStringLiteral( "<p><b>%1</b>: %2</p>" ).arg( tr( "Identifier" ), id ) : QString() ),
                                  ( !missingMessage.isEmpty() ? QStringLiteral( "<p><b style=\"color: red\">%1</b></p>" ).arg( missingMessage ) : QString() ),
                                  transform.proj );
#endif
    item->setToolTip( toolTipString );
    mCoordinateOperationTableWidget->setRowCount( row + 1 );
    mCoordinateOperationTableWidget->setItem( row, 0, item.release() );

    item = qgis::make_unique< QTableWidgetItem >();
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    item->setText( transform.accuracy >= 0 ? QString::number( transform.accuracy ) : tr( "Unknown" ) );
    item->setToolTip( toolTipString );
    if ( !transform.isAvailable )
    {
      item->setForeground( QBrush( palette().color( QPalette::Disabled, QPalette::Text ) ) );
    }
    mCoordinateOperationTableWidget->setItem( row, 1, item.release() );

#if PROJ_VERSION_MAJOR>=6
    // area of use column
    item = qgis::make_unique< QTableWidgetItem >();
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    item->setText( areasOfUse.join( QStringLiteral( ", " ) ) );
    item->setToolTip( toolTipString );
    if ( !transform.isAvailable )
    {
      item->setForeground( QBrush( palette().color( QPalette::Disabled, QPalette::Text ) ) );
    }
    mCoordinateOperationTableWidget->setItem( row, 2, item.release() );
#endif

    row++;
  }
#else
  Q_NOWARN_DEPRECATED_PUSH
  for ( const QgsDatumTransform::TransformPair &transform : qgis::as_const( mDatumTransforms ) )
  {
    bool itemDisabled = false;
    bool itemHidden = false;

    if ( transform.sourceTransformId == -1 && transform.destinationTransformId == -1 )
      continue;

    QgsDatumTransform::TransformInfo srcInfo = QgsDatumTransform::datumTransformInfo( transform.sourceTransformId );
    QgsDatumTransform::TransformInfo destInfo = QgsDatumTransform::datumTransformInfo( transform.destinationTransformId );
    for ( int i = 0; i < 2; ++i )
    {
      std::unique_ptr< QTableWidgetItem > item = qgis::make_unique< QTableWidgetItem >();
      int nr = i == 0 ? transform.sourceTransformId : transform.destinationTransformId;
      item->setData( TransformIdRole, nr );
      item->setFlags( item->flags() & ~Qt::ItemIsEditable );

      item->setText( QgsDatumTransform::datumTransformToProj( nr ) );

      //Describe datums in a tooltip
      QgsDatumTransform::TransformInfo info = i == 0 ? srcInfo : destInfo;
      if ( info.datumTransformId == -1 )
        continue;

      if ( info.deprecated )
      {
        itemHidden = mHideDeprecatedCheckBox->isChecked();
        item->setForeground( QBrush( QColor( 255, 0, 0 ) ) );
      }

      if ( ( srcInfo.preferred && !srcInfo.deprecated ) || ( destInfo.preferred && !destInfo.deprecated ) )
      {
        QFont f = item->font();
        f.setBold( true );
        item->setFont( f );
        item->setForeground( QBrush( QColor( 0, 120, 0 ) ) );
      }

      if ( info.preferred && !info.deprecated && preferredInitialRow < 0 )
      {
        // try to select a "preferred" entry by default
        preferredInitialRow = row;
      }

      QString toolTipString;
      if ( gridShiftTransformation( item->text() ) )
      {
        toolTipString.append( QStringLiteral( "<p><b>NTv2</b></p>" ) );
      }

      if ( info.epsgCode > 0 )
        toolTipString.append( QStringLiteral( "<p><b>EPSG Transformations Code:</b> %1</p>" ).arg( info.epsgCode ) );

      toolTipString.append( QStringLiteral( "<p><b>Source CRS:</b> %1</p><p><b>Destination CRS:</b> %2</p>" ).arg( info.sourceCrsDescription, info.destinationCrsDescription ) );

      if ( !info.remarks.isEmpty() )
        toolTipString.append( QStringLiteral( "<p><b>Remarks:</b> %1</p>" ).arg( info.remarks ) );
      if ( !info.scope.isEmpty() )
        toolTipString.append( QStringLiteral( "<p><b>Scope:</b> %1</p>" ).arg( info.scope ) );
      if ( info.preferred )
        toolTipString.append( "<p><b>Preferred transformation</b></p>" );
      if ( info.deprecated )
        toolTipString.append( "<p><b>Deprecated transformation</b></p>" );

      item->setToolTip( toolTipString );

      if ( gridShiftTransformation( item->text() ) && !testGridShiftFileAvailability( item.get() ) )
      {
        itemDisabled = true;
      }

      if ( !itemHidden )
      {
        if ( itemDisabled )
        {
          item->setFlags( Qt::NoItemFlags );
        }
        mCoordinateOperationTableWidget->setRowCount( row + 1 );
        mCoordinateOperationTableWidget->setItem( row, i, item.release() );
      }
    }
    row++;
  }
  Q_NOWARN_DEPRECATED_POP
#endif

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

#if PROJ_VERSION_MAJOR>=6
  // for proj 6, return the first available transform -- they are sorted by preference by proj already
  for ( const QgsDatumTransform::TransformDetails &transform : qgis::as_const( mDatumTransforms ) )
  {
    if ( transform.isAvailable )
    {
      preferred.proj = transform.proj;
      preferred.isAvailable = transform.isAvailable;
      break;
    }
  }
  return preferred;
#else
  OperationDetails preferredNonDeprecated;
  bool foundPreferredNonDeprecated = false;
  bool foundPreferred  = false;
  OperationDetails nonDeprecated;
  bool foundNonDeprecated = false;
  OperationDetails fallback;
  bool foundFallback = false;

  Q_NOWARN_DEPRECATED_PUSH
  for ( const QgsDatumTransform::TransformPair &transform : qgis::as_const( mDatumTransforms ) )
  {
    if ( transform.sourceTransformId == -1 && transform.destinationTransformId == -1 )
      continue;

    const QgsDatumTransform::TransformInfo srcInfo = QgsDatumTransform::datumTransformInfo( transform.sourceTransformId );
    const QgsDatumTransform::TransformInfo destInfo = QgsDatumTransform::datumTransformInfo( transform.destinationTransformId );
    if ( !foundPreferredNonDeprecated && ( ( srcInfo.preferred && !srcInfo.deprecated ) || transform.sourceTransformId == -1 )
         && ( ( destInfo.preferred && !destInfo.deprecated ) || transform.destinationTransformId == -1 ) )
    {
      preferredNonDeprecated.sourceTransformId = transform.sourceTransformId;
      preferredNonDeprecated.destinationTransformId = transform.destinationTransformId;
      foundPreferredNonDeprecated = true;
    }
    else if ( !foundPreferred && ( srcInfo.preferred || transform.sourceTransformId == -1 ) &&
              ( destInfo.preferred || transform.destinationTransformId == -1 ) )
    {
      preferred.sourceTransformId = transform.sourceTransformId;
      preferred.destinationTransformId = transform.destinationTransformId;
      foundPreferred = true;
    }
    else if ( !foundNonDeprecated && ( !srcInfo.deprecated || transform.sourceTransformId == -1 )
              && ( !destInfo.deprecated || transform.destinationTransformId == -1 ) )
    {
      nonDeprecated.sourceTransformId = transform.sourceTransformId;
      nonDeprecated.destinationTransformId = transform.destinationTransformId;
      foundNonDeprecated = true;
    }
    else if ( !foundFallback )
    {
      fallback.sourceTransformId = transform.sourceTransformId;
      fallback.destinationTransformId = transform.destinationTransformId;
      foundFallback = true;
    }
  }
  Q_NOWARN_DEPRECATED_POP
  if ( foundPreferredNonDeprecated )
    return preferredNonDeprecated;
  else if ( foundPreferred )
    return preferred;
  else if ( foundNonDeprecated )
    return nonDeprecated;
  else
    return fallback;
#endif
}

QString QgsCoordinateOperationWidget::formatScope( const QString &s )
{
  QString scope = s;

  QRegularExpression reGNSS( QStringLiteral( "\\bGNSS\\b" ) );
  scope.replace( reGNSS, QObject::tr( "GNSS (Global Navigation Satellite System)" ) );

  QRegularExpression reCORS( QStringLiteral( "\\bCORS\\b" ) );
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
  }
  else
  {
    op.sourceTransformId = -1;
    op.destinationTransformId = -1;
    op.proj = QString();
  }
  return op;
}

void QgsCoordinateOperationWidget::setSelectedOperation( const QgsCoordinateOperationWidget::OperationDetails &operation ) const
{
  for ( int row = 0; row < mCoordinateOperationTableWidget->rowCount(); ++row )
  {
    QTableWidgetItem *srcItem = mCoordinateOperationTableWidget->item( row, 0 );
#if PROJ_VERSION_MAJOR>=6
    if ( srcItem && srcItem->data( ProjRole ).toString() == operation.proj )
    {
      mCoordinateOperationTableWidget->selectRow( row );
      break;
    }
#else
    QTableWidgetItem *destItem = mCoordinateOperationTableWidget->item( row, 1 );
    if ( !srcItem || !destItem )
      continue;

    if ( ( operation.sourceTransformId == srcItem->data( TransformIdRole ).toInt() &&
           operation.destinationTransformId == destItem->data( TransformIdRole ).toInt() ) ||
         ( operation.destinationTransformId == srcItem->data( TransformIdRole ).toInt() &&
           operation.sourceTransformId == destItem->data( TransformIdRole ).toInt() ) )
    {
      mCoordinateOperationTableWidget->selectRow( row );
      break;
    }
#endif
  }
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
#if defined(Q_OS_WIN)
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
#if PROJ_VERSION_MAJOR>=6
    mAreaCanvas->hide();
#endif
  }
  else
  {
    QTableWidgetItem *srcItem = mCoordinateOperationTableWidget->item( row, 0 );
    mLabelSrcDescription->setText( srcItem ? srcItem->toolTip() : QString() );
    if ( srcItem )
    {
      // find area of intersection of operation, source and dest bounding boxes
      // see https://github.com/OSGeo/PROJ/issues/1549 for justification
      const QgsRectangle operationRect = srcItem->data( BoundsRole ).value< QgsRectangle >();
      const QgsRectangle sourceRect = mSourceCrs.bounds();
      const QgsRectangle destRect = mDestinationCrs.bounds();
      QgsRectangle rect = operationRect.intersect( sourceRect );
      rect = rect.intersect( destRect );

      mAreaCanvas->setPreviewRect( rect );
#if PROJ_VERSION_MAJOR>=6
      mAreaCanvas->show();
#endif
    }
    else
    {
      mAreaCanvas->setPreviewRect( QgsRectangle() );
#if PROJ_VERSION_MAJOR>=6
      mAreaCanvas->hide();
#endif
    }
    QTableWidgetItem *destItem = mCoordinateOperationTableWidget->item( row, 1 );
    mLabelDstDescription->setText( destItem ? destItem->toolTip() : QString() );
  }
  OperationDetails newOp = selectedOperation();
#if PROJ_VERSION_MAJOR>=6
  if ( newOp.proj != mPreviousOp.proj )
    emit operationChanged();
#else
  if ( newOp.sourceTransformId != mPreviousOp.sourceTransformId ||
       newOp.destinationTransformId != mPreviousOp.destinationTransformId )
    emit operationChanged();
#endif
  mPreviousOp = newOp;
}

void QgsCoordinateOperationWidget::setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs )
{
  mSourceCrs = sourceCrs;
#if PROJ_VERSION_MAJOR>=6
  mDatumTransforms = QgsDatumTransform::operations( mSourceCrs, mDestinationCrs, mShowSupersededCheckBox->isChecked() );
#else
  Q_NOWARN_DEPRECATED_PUSH
  mDatumTransforms = QgsDatumTransform::datumTransformations( mSourceCrs, mDestinationCrs );
  Q_NOWARN_DEPRECATED_POP
#endif
  loadAvailableOperations();
}

void QgsCoordinateOperationWidget::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  mDestinationCrs = destinationCrs;
#if PROJ_VERSION_MAJOR>=6
  mDatumTransforms = QgsDatumTransform::operations( mSourceCrs, mDestinationCrs, mShowSupersededCheckBox->isChecked() );
#else
  Q_NOWARN_DEPRECATED_PUSH
  mDatumTransforms = QgsDatumTransform::datumTransformations( mSourceCrs, mDestinationCrs );
  Q_NOWARN_DEPRECATED_POP
#endif
  loadAvailableOperations();
}

void QgsCoordinateOperationWidget::showSupersededToggled( bool )
{
#if PROJ_VERSION_MAJOR>=6
  mDatumTransforms = QgsDatumTransform::operations( mSourceCrs, mDestinationCrs, mShowSupersededCheckBox->isChecked() );
#else
  Q_NOWARN_DEPRECATED_PUSH
  mDatumTransforms = QgsDatumTransform::datumTransformations( mSourceCrs, mDestinationCrs );
  Q_NOWARN_DEPRECATED_POP
#endif
  loadAvailableOperations();
}
