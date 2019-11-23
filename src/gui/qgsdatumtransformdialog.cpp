/***************************************************************************
                         qgsdatumtransformdialog.cpp
                         ---------------------------
    begin                : November 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco.hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatumtransformdialog.h"
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

bool QgsDatumTransformDialog::run( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, QWidget *parent, QgsMapCanvas *mapCanvas, const QString &windowTitle )
{
  if ( sourceCrs == destinationCrs )
    return true;

  QgsCoordinateTransformContext context = QgsProject::instance()->transformContext();
  if ( context.hasTransform( sourceCrs, destinationCrs ) )
  {
    return true;
  }

  QgsDatumTransformDialog dlg( sourceCrs, destinationCrs, false, true, true, qMakePair( -1, -1 ), parent, nullptr, QString(), mapCanvas );
  if ( !windowTitle.isEmpty() )
    dlg.setWindowTitle( windowTitle );

  if ( dlg.shouldAskUserForSelection() )
  {
    if ( dlg.exec() )
    {
      const TransformInfo dt = dlg.selectedDatumTransform();
      QgsCoordinateTransformContext context = QgsProject::instance()->transformContext();
      Q_NOWARN_DEPRECATED_PUSH
      context.addSourceDestinationDatumTransform( dt.sourceCrs, dt.destinationCrs, dt.sourceTransformId, dt.destinationTransformId );
      Q_NOWARN_DEPRECATED_POP
      context.addCoordinateOperation( dt.sourceCrs, dt.destinationCrs, dt.proj );
      QgsProject::instance()->setTransformContext( context );
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    dlg.applyDefaultTransform();
    return true;
  }
}

QgsDatumTransformDialog::QgsDatumTransformDialog( const QgsCoordinateReferenceSystem &sCrs,
    const QgsCoordinateReferenceSystem &dCrs, const bool allowCrsChanges, const bool showMakeDefault, const bool forceChoice,
    QPair<int, int> selectedDatumTransforms,
    QWidget *parent,
    Qt::WindowFlags f, const QString &selectedProj, QgsMapCanvas *mapCanvas )
  : QDialog( parent, f )
  , mPreviousCursorOverride( qgis::make_unique< QgsTemporaryCursorRestoreOverride >() ) // this dialog is often shown while cursor overrides are in place, so temporarily remove them
{
  setupUi( this );

  QgsCoordinateReferenceSystem sourceCrs = sCrs;
  QgsCoordinateReferenceSystem destinationCrs = dCrs;

  QgsGui::enableAutoGeometryRestore( this );

  mLabelSrcDescription->setTextInteractionFlags( Qt::TextBrowserInteraction );
  mLabelSrcDescription->setOpenExternalLinks( true );

  if ( !showMakeDefault )
    mMakeDefaultCheckBox->setVisible( false );

  if ( forceChoice )
  {
    mButtonBox->removeButton( mButtonBox->button( QDialogButtonBox::Cancel ) );
    setWindowFlags( windowFlags() | Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowCloseButtonHint );
  }

#if PROJ_VERSION_MAJOR>=6
  mDatumTransformTableWidget->setColumnCount( 3 );
#else
  mDatumTransformTableWidget->setColumnCount( 2 );
#endif

  QStringList headers;
#if PROJ_VERSION_MAJOR>=6
  headers << tr( "Transformation" ) << tr( "Accuracy (meters)" ) << tr( "Area of Use" );
#else
  headers << tr( "Source Transform" ) << tr( "Destination Transform" ) ;
#endif
  mDatumTransformTableWidget->setHorizontalHeaderLabels( headers );

#if PROJ_VERSION_MAJOR>=6
  if ( !sourceCrs.isValid() )
    sourceCrs = QgsProject::instance()->crs();
  if ( !sourceCrs.isValid() )
    sourceCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
  if ( !destinationCrs.isValid() )
    destinationCrs = QgsProject::instance()->crs();
  if ( !destinationCrs.isValid() )
    destinationCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );

  mSourceProjectionSelectionWidget->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, false );
  mDestinationProjectionSelectionWidget->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, false );
#endif

  mSourceProjectionSelectionWidget->setCrs( sourceCrs );
  mDestinationProjectionSelectionWidget->setCrs( destinationCrs );
  if ( !allowCrsChanges )
  {
    mCrsStackedWidget->setCurrentIndex( 1 );
    mSourceProjectionSelectionWidget->setEnabled( false );
    mDestinationProjectionSelectionWidget->setEnabled( false );
    mSourceCrsLabel->setText( QgsProjectionSelectionWidget::crsOptionText( sourceCrs ) );
    mDestCrsLabel->setText( QgsProjectionSelectionWidget::crsOptionText( destinationCrs ) );
  }

#if PROJ_VERSION_MAJOR<6
  mAreaCanvas->hide();
  ( void )mapCanvas;
#else
  if ( mapCanvas )
  {
    // show canvas extent in preview widget
    QPolygonF mainCanvasPoly = mapCanvas->mapSettings().visiblePolygon();
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

  connect( mHideDeprecatedCheckBox, &QCheckBox::stateChanged, this, [ = ] { load(); } );
  connect( mShowSupersededCheckBox, &QCheckBox::toggled, this, &QgsDatumTransformDialog::showSupersededToggled );
  connect( mDatumTransformTableWidget, &QTableWidget::currentItemChanged, this, &QgsDatumTransformDialog::tableCurrentItemChanged );

  connect( mSourceProjectionSelectionWidget, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDatumTransformDialog::setSourceCrs );
  connect( mDestinationProjectionSelectionWidget, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDatumTransformDialog::setDestinationCrs );

  //get list of datum transforms
  mSourceCrs = sourceCrs;
  mDestinationCrs = destinationCrs;
#if PROJ_VERSION_MAJOR>=6
  mDatumTransforms = QgsDatumTransform::operations( sourceCrs, destinationCrs, mShowSupersededCheckBox->isChecked() );
#else
  Q_NOWARN_DEPRECATED_PUSH
  mDatumTransforms = QgsDatumTransform::datumTransformations( sourceCrs, destinationCrs );
  Q_NOWARN_DEPRECATED_POP
#endif
  mLabelSrcDescription->clear();
  mLabelDstDescription->clear();

  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_projections/working_with_projections.html" ) );
  } );

  load( selectedDatumTransforms, selectedProj );
}

void QgsDatumTransformDialog::load( QPair<int, int> selectedDatumTransforms, const QString &selectedProj )
{
  mDatumTransformTableWidget->setRowCount( 0 );

  int row = 0;
  int preferredInitialRow = -1;
#if PROJ_VERSION_MAJOR>=6
  Q_UNUSED( selectedDatumTransforms )
  for ( const QgsDatumTransform::TransformDetails &transform : qgis::as_const( mDatumTransforms ) )
  {
    std::unique_ptr< QTableWidgetItem > item = qgis::make_unique< QTableWidgetItem >();
    item->setData( ProjRole, transform.proj );
    item->setData( AvailableRole, transform.isAvailable );
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );

    QString name = transform.name;
    if ( !transform.authority.isEmpty() && !transform.code.isEmpty() )
      name += QStringLiteral( " — %1:%2" ).arg( transform.authority, transform.code );
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
    mDatumTransformTableWidget->setRowCount( row + 1 );
    mDatumTransformTableWidget->setItem( row, 0, item.release() );

    item = qgis::make_unique< QTableWidgetItem >();
    item->setFlags( item->flags() & ~Qt::ItemIsEditable );
    item->setText( transform.accuracy >= 0 ? QString::number( transform.accuracy ) : tr( "Unknown" ) );
    item->setToolTip( toolTipString );
    if ( !transform.isAvailable )
    {
      item->setForeground( QBrush( palette().color( QPalette::Disabled, QPalette::Text ) ) );
    }
    mDatumTransformTableWidget->setItem( row, 1, item.release() );

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
    mDatumTransformTableWidget->setItem( row, 2, item.release() );
#endif

    if ( transform.proj == selectedProj )
    {
      mDatumTransformTableWidget->selectRow( row );
    }

    row++;
  }
#else
  Q_UNUSED( selectedProj )

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
        mDatumTransformTableWidget->setRowCount( row + 1 );
        mDatumTransformTableWidget->setItem( row, i, item.release() );
      }
    }

    if ( ( transform.sourceTransformId == selectedDatumTransforms.first &&
           transform.destinationTransformId == selectedDatumTransforms.second ) ||
         ( transform.sourceTransformId == selectedDatumTransforms.second &&
           transform.destinationTransformId == selectedDatumTransforms.first ) )
    {
      mDatumTransformTableWidget->selectRow( row );
    }

    row++;
  }
  Q_NOWARN_DEPRECATED_POP
#endif

  if ( mDatumTransformTableWidget->currentRow() < 0 )
    mDatumTransformTableWidget->selectRow( preferredInitialRow >= 0 ? preferredInitialRow : 0 );

  mDatumTransformTableWidget->resizeColumnsToContents();

  tableCurrentItemChanged( nullptr, nullptr );
}

void QgsDatumTransformDialog::setOKButtonEnabled()
{
  int row = mDatumTransformTableWidget->currentRow();
#if PROJ_VERSION_MAJOR>=6
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( mSourceCrs.isValid() && mDestinationCrs.isValid()
      && mDatumTransformTableWidget->item( row, 0 ) && mDatumTransformTableWidget->item( row, 0 )->data( AvailableRole ).toBool() );
#else
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( mSourceCrs.isValid() && mDestinationCrs.isValid() && row >= 0 );
#endif
}

QgsDatumTransformDialog::~QgsDatumTransformDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/DatumTransformDialog/hideDeprecated" ), mHideDeprecatedCheckBox->isChecked() );

  for ( int i = 0; i < 2; i++ )
  {
    settings.setValue( QStringLiteral( "Windows/DatumTransformDialog/columnWidths/%1" ).arg( i ), mDatumTransformTableWidget->columnWidth( i ) );
  }
}

void QgsDatumTransformDialog::accept()
{
  if ( mMakeDefaultCheckBox->isChecked() && !mDatumTransformTableWidget->selectedItems().isEmpty() )
  {
    QgsSettings settings;
    settings.beginGroup( QStringLiteral( "/Projections" ) );

    const TransformInfo dt = selectedDatumTransform();

    QString srcAuthId = dt.sourceCrs.authid();
    QString destAuthId = dt.destinationCrs.authid();
    int sourceDatumTransform = dt.sourceTransformId;
    QString sourceDatumProj;
    Q_NOWARN_DEPRECATED_PUSH
    if ( sourceDatumTransform >= 0 )
      sourceDatumProj = QgsDatumTransform::datumTransformToProj( sourceDatumTransform );
    int destinationDatumTransform = dt.destinationTransformId;
    QString destinationDatumProj;
    if ( destinationDatumTransform >= 0 )
      destinationDatumProj = QgsDatumTransform::datumTransformToProj( destinationDatumTransform );
    Q_NOWARN_DEPRECATED_POP
    settings.setValue( srcAuthId + QStringLiteral( "//" ) + destAuthId + QStringLiteral( "_srcTransform" ), sourceDatumProj );
    settings.setValue( srcAuthId + QStringLiteral( "//" ) + destAuthId + QStringLiteral( "_destTransform" ), destinationDatumProj );
    settings.setValue( srcAuthId + QStringLiteral( "//" ) + destAuthId + QStringLiteral( "_coordinateOp" ), dt.proj );
  }
  QDialog::accept();
}

void QgsDatumTransformDialog::reject()
{
  if ( !mButtonBox->button( QDialogButtonBox::Cancel ) )
    return; // users HAVE to make a choice, no click on the dialog "x" to avoid this!

  QDialog::reject();
}

bool QgsDatumTransformDialog::shouldAskUserForSelection() const
{
  if ( mDatumTransforms.count() > 1 )
  {
    return QgsSettings().value( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), false, QgsSettings::App ).toBool();
  }
  // TODO: show if transform grids are required, but missing
  return false;
}

QgsDatumTransformDialog::TransformInfo QgsDatumTransformDialog::defaultDatumTransform() const
{
  TransformInfo preferred;
  preferred.sourceCrs = mSourceCrs;
  preferred.destinationCrs = mDestinationCrs;

#if PROJ_VERSION_MAJOR>=6
  // for proj 6, return the first available transform -- they are sorted by preference by proj already
  for ( const QgsDatumTransform::TransformDetails &transform : qgis::as_const( mDatumTransforms ) )
  {
    if ( transform.isAvailable )
    {
      preferred.proj = transform.proj;
      break;
    }
  }
  return preferred;
#else
  TransformInfo preferredNonDeprecated;
  preferredNonDeprecated.sourceCrs = mSourceCrs;
  preferredNonDeprecated.destinationCrs = mDestinationCrs;
  bool foundPreferredNonDeprecated = false;
  bool foundPreferred  = false;
  TransformInfo nonDeprecated;
  nonDeprecated.sourceCrs = mSourceCrs;
  nonDeprecated.destinationCrs = mDestinationCrs;
  bool foundNonDeprecated = false;
  TransformInfo fallback;
  fallback.sourceCrs = mSourceCrs;
  fallback.destinationCrs = mDestinationCrs;
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

void QgsDatumTransformDialog::applyDefaultTransform()
{
  if ( mDatumTransforms.count() > 0 )
  {
    QgsCoordinateTransformContext context = QgsProject::instance()->transformContext();
    const TransformInfo dt = defaultDatumTransform();
    Q_NOWARN_DEPRECATED_PUSH
    context.addSourceDestinationDatumTransform( dt.sourceCrs, dt.destinationCrs, dt.sourceTransformId, dt.destinationTransformId );
    Q_NOWARN_DEPRECATED_POP

#if PROJ_VERSION_MAJOR>=6
    // on proj 6 builds, removing a coordinate operation falls back to default
    context.removeCoordinateOperation( dt.sourceCrs, dt.destinationCrs );
#else
    context.addCoordinateOperation( dt.sourceCrs, dt.destinationCrs, dt.proj );
#endif
    QgsProject::instance()->setTransformContext( context );
  }
}

QString QgsDatumTransformDialog::formatScope( const QString &s )
{
  QString scope = s;

  QRegularExpression reGNSS( QStringLiteral( "\\bGNSS\\b" ) );
  scope.replace( reGNSS, QObject::tr( "GNSS (Global Navigation Satellite System)" ) );

  QRegularExpression reCORS( QStringLiteral( "\\bCORS\\b" ) );
  scope.replace( reCORS, QObject::tr( "CORS (Continually Operating Reference Station)" ) );

  return scope;
}

QgsDatumTransformDialog::TransformInfo QgsDatumTransformDialog::selectedDatumTransform()
{
  int row = mDatumTransformTableWidget->currentRow();
  TransformInfo sdt;
  sdt.sourceCrs = mSourceCrs;
  sdt.destinationCrs = mDestinationCrs;

  if ( row >= 0 )
  {
    QTableWidgetItem *srcItem = mDatumTransformTableWidget->item( row, 0 );
    sdt.sourceTransformId = srcItem ? srcItem->data( TransformIdRole ).toInt() : -1;
    QTableWidgetItem *destItem = mDatumTransformTableWidget->item( row, 1 );
    sdt.destinationTransformId = destItem ? destItem->data( TransformIdRole ).toInt() : -1;
    sdt.proj = srcItem ? srcItem->data( ProjRole ).toString() : QString();
  }
  else
  {
    sdt.sourceTransformId = -1;
    sdt.destinationTransformId = -1;
    sdt.proj = QString();
  }
  return sdt;
}

bool QgsDatumTransformDialog::gridShiftTransformation( const QString &itemText ) const
{
  return !itemText.isEmpty() && !itemText.contains( QLatin1String( "towgs84" ), Qt::CaseInsensitive );
}

bool QgsDatumTransformDialog::testGridShiftFileAvailability( QTableWidgetItem *item ) const
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

void QgsDatumTransformDialog::tableCurrentItemChanged( QTableWidgetItem *, QTableWidgetItem * )
{
  int row = mDatumTransformTableWidget->currentRow();
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
    QTableWidgetItem *srcItem = mDatumTransformTableWidget->item( row, 0 );
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
    QTableWidgetItem *destItem = mDatumTransformTableWidget->item( row, 1 );
    mLabelDstDescription->setText( destItem ? destItem->toolTip() : QString() );
  }

  setOKButtonEnabled();
}

void QgsDatumTransformDialog::setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs )
{
  mSourceCrs = sourceCrs;
#if PROJ_VERSION_MAJOR>=6
  mDatumTransforms = QgsDatumTransform::operations( mSourceCrs, mDestinationCrs, mShowSupersededCheckBox->isChecked() );
#else
  Q_NOWARN_DEPRECATED_PUSH
  mDatumTransforms = QgsDatumTransform::datumTransformations( mSourceCrs, mDestinationCrs );
  Q_NOWARN_DEPRECATED_POP
#endif
  load();
  setOKButtonEnabled();
}

void QgsDatumTransformDialog::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  mDestinationCrs = destinationCrs;
#if PROJ_VERSION_MAJOR>=6
  mDatumTransforms = QgsDatumTransform::operations( mSourceCrs, mDestinationCrs, mShowSupersededCheckBox->isChecked() );
#else
  Q_NOWARN_DEPRECATED_PUSH
  mDatumTransforms = QgsDatumTransform::datumTransformations( mSourceCrs, mDestinationCrs );
  Q_NOWARN_DEPRECATED_POP
#endif
  load();
  setOKButtonEnabled();
}

void QgsDatumTransformDialog::showSupersededToggled( bool )
{
#if PROJ_VERSION_MAJOR>=6
  mDatumTransforms = QgsDatumTransform::operations( mSourceCrs, mDestinationCrs, mShowSupersededCheckBox->isChecked() );
#else
  Q_NOWARN_DEPRECATED_PUSH
  mDatumTransforms = QgsDatumTransform::datumTransformations( mSourceCrs, mDestinationCrs );
  Q_NOWARN_DEPRECATED_POP
#endif
  load();
  setOKButtonEnabled();
}
