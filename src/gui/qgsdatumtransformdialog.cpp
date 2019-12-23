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

  if ( !showMakeDefault )
    mCoordinateOperationsWidget->setShowMakeDefault( false );

  if ( forceChoice )
  {
    mButtonBox->removeButton( mButtonBox->button( QDialogButtonBox::Cancel ) );
    setWindowFlags( windowFlags() | Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() & ~Qt::WindowCloseButtonHint );
  }

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

  mCoordinateOperationsWidget->setMapCanvas( mapCanvas );

  connect( mSourceProjectionSelectionWidget, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDatumTransformDialog::setSourceCrs );
  connect( mDestinationProjectionSelectionWidget, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDatumTransformDialog::setDestinationCrs );

  mCoordinateOperationsWidget->setSourceCrs( sourceCrs );
  mCoordinateOperationsWidget->setDestinationCrs( destinationCrs );

  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_projections/working_with_projections.html" ) );
  } );

  connect( mCoordinateOperationsWidget, &QgsCoordinateOperationWidget::operationChanged, this, &QgsDatumTransformDialog::operationChanged );
  QgsCoordinateOperationWidget::OperationDetails deets;
  deets.proj = selectedProj;
  deets.sourceTransformId = selectedDatumTransforms.first;
  deets.destinationTransformId = selectedDatumTransforms.second;
  mCoordinateOperationsWidget->setSelectedOperation( deets );

  connect( mCoordinateOperationsWidget, &QgsCoordinateOperationWidget::operationDoubleClicked, this, [ = ]
  {

#if PROJ_VERSION_MAJOR>=6
    if ( mCoordinateOperationsWidget->sourceCrs().isValid() && mCoordinateOperationsWidget->destinationCrs().isValid()
         && mCoordinateOperationsWidget->selectedOperation().isAvailable )
      accept();
#else
    if ( mCoordinateOperationsWidget->sourceCrs().isValid() && mCoordinateOperationsWidget->destinationCrs().isValid() && mCoordinateOperationsWidget->hasSelection() )
      accept();
#endif
  } );
}

void QgsDatumTransformDialog::setOKButtonEnabled()
{
#if PROJ_VERSION_MAJOR>=6
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( mCoordinateOperationsWidget->sourceCrs().isValid() && mCoordinateOperationsWidget->destinationCrs().isValid()
      && mCoordinateOperationsWidget->selectedOperation().isAvailable );
#else
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( mCoordinateOperationsWidget->sourceCrs().isValid() && mCoordinateOperationsWidget->destinationCrs().isValid() && mCoordinateOperationsWidget->hasSelection() );
#endif
}

void QgsDatumTransformDialog::accept()
{
  if ( mCoordinateOperationsWidget->makeDefaultSelected() && mCoordinateOperationsWidget->hasSelection() )
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
  if ( mCoordinateOperationsWidget->availableOperations().count() > 1 )
  {
    return QgsSettings().value( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), false, QgsSettings::App ).toBool();
  }
  // TODO: show if transform grids are required, but missing
  return false;
}

QgsDatumTransformDialog::TransformInfo QgsDatumTransformDialog::defaultDatumTransform() const
{
  TransformInfo preferred;
  preferred.sourceCrs = mCoordinateOperationsWidget->sourceCrs();
  preferred.destinationCrs = mCoordinateOperationsWidget->destinationCrs();
  QgsCoordinateOperationWidget::OperationDetails defaultOp = mCoordinateOperationsWidget->defaultOperation();
  preferred.sourceTransformId = defaultOp.sourceTransformId;
  preferred.destinationTransformId = defaultOp.destinationTransformId;
  preferred.proj = defaultOp.proj;
  return preferred;
}

void QgsDatumTransformDialog::applyDefaultTransform()
{
  if ( mCoordinateOperationsWidget->availableOperations().count() > 0 )
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

QgsDatumTransformDialog::TransformInfo QgsDatumTransformDialog::selectedDatumTransform()
{
  QgsCoordinateOperationWidget::OperationDetails selected = mCoordinateOperationsWidget->selectedOperation();
  TransformInfo sdt;
  sdt.sourceCrs = mCoordinateOperationsWidget->sourceCrs();
  sdt.destinationCrs = mCoordinateOperationsWidget->destinationCrs();
  sdt.sourceTransformId = selected.sourceTransformId;
  sdt.destinationTransformId = selected.destinationTransformId;
  sdt.proj = selected.proj;
  return sdt;
}

bool QgsDatumTransformDialog::gridShiftTransformation( const QString &itemText ) const
{
  return !itemText.isEmpty() && !itemText.contains( QLatin1String( "towgs84" ), Qt::CaseInsensitive );
}

void QgsDatumTransformDialog::operationChanged()
{
  setOKButtonEnabled();
}

void QgsDatumTransformDialog::setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs )
{
  mCoordinateOperationsWidget->setSourceCrs( sourceCrs );
  setOKButtonEnabled();
}

void QgsDatumTransformDialog::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  mCoordinateOperationsWidget->setDestinationCrs( destinationCrs );
  setOKButtonEnabled();
}
