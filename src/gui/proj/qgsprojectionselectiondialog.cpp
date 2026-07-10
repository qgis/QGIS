/***************************************************************************
                          qgsgenericprojectionselector.cpp
                    Set user defined CRS using projection selector widget
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprojectionselectiondialog.h"

#include "qgsapplication.h"
#include "qgscoordinatereferencesystemmodel.h"
#include "qgsdoublespinbox.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsprojectionselectionwidget.h"
#include "qgsrectangle.h"
#include "qgssettings.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QString>

#include "moc_qgsprojectionselectiondialog.cpp"

using namespace Qt::StringLiterals;

//
// QgsCrsSelectionWidget
//
QgsCrsSelectionWidget::QgsCrsSelectionWidget( QWidget *parent, QgsCoordinateReferenceSystemProxyModel::Filters filters )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  projectionSelector->setFilters( filters );

  //we will show this only when a message is set
  textEdit->hide();

  mNotSetText = tr( "No CRS (or unknown)" );
  mLabelNoCrs->setText( tr( "Use this option to treat all coordinates as Cartesian coordinates in an unknown reference system." ) );

  mComboCrsType->addItem( tr( "Predefined CRS" ), static_cast<int>( CrsType::Predefined ) );
  mComboCrsType->addItem( tr( "Custom CRS" ), static_cast<int>( CrsType::Custom ) );
  mComboCrsType->addItem( tr( "Topocentric CRS" ), static_cast<int>( CrsType::Topocentric ) );

  mTopocentricBaseSelector->setFilters( QgsCoordinateReferenceSystemProxyModel::FilterTopocentricCompatible );
  mTopocentricBaseSelector->setAllowTopocentricCrs( false );

  mStackedWidget->setCurrentWidget( mPageDatabase );
  mComboCrsType->setCurrentIndex( mComboCrsType->findData( static_cast<int>( CrsType::Predefined ) ) );

  connect( mComboCrsType, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) {
    if ( !mComboCrsType->currentData().isValid() )
      mStackedWidget->setCurrentWidget( mPageNoCrs );
    else
    {
      switch ( static_cast<CrsType>( mComboCrsType->currentData().toInt() ) )
      {
        case QgsCrsSelectionWidget::CrsType::Predefined:
          mStackedWidget->setCurrentWidget( mPageDatabase );
          break;
        case QgsCrsSelectionWidget::CrsType::Custom:
          mStackedWidget->setCurrentWidget( mPageCustom );
          break;
        case QgsCrsSelectionWidget::CrsType::Topocentric:
          mStackedWidget->setCurrentWidget( mPageTopocentric );
          if ( !mBlockSignals )
          {
            const QgsRectangle b = mTopocentricBaseSelector->crs().bounds();
            if ( !b.isNull() )
            {
              mSpinBoxTopoLat->setClearValue( ( b.yMinimum() + b.yMaximum() ) / 2.0 );
              mSpinBoxTopoLon->setClearValue( ( b.xMinimum() + b.xMaximum() ) / 2.0 );
              mSpinBoxTopoLat->clear();
              mSpinBoxTopoLon->clear();
            }
          }
          break;
      }
    }

    if ( !mBlockSignals )
    {
      emit crsChanged();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  } );

  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::projectionDoubleClicked, this, [this] { emit crsDoubleClicked( projectionSelector->crs() ); } );

  connect( mCrsDefinitionWidget, &QgsCrsDefinitionWidget::crsChanged, this, [this]() {
    if ( !mBlockSignals )
    {
      emit crsChanged();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  } );

  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::crsSelected, this, [this]() {
    if ( !mBlockSignals )
    {
      mDeferredInvalidCrsSet = false;
      emit crsChanged();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  } );

  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::hasValidSelectionChanged, this, [this]() {
    if ( !mBlockSignals )
    {
      emit crsChanged();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  } );

  connect( mTopocentricBaseSelector, &QgsProjectionSelectionWidget::crsChanged, this, [this]( const QgsCoordinateReferenceSystem &newBase ) {
    const QgsRectangle b = newBase.bounds();
    if ( !b.isNull() )
    {
      mSpinBoxTopoLat->setClearValue( ( b.yMinimum() + b.yMaximum() ) / 2.0 );
      mSpinBoxTopoLon->setClearValue( ( b.xMinimum() + b.xMaximum() ) / 2.0 );
    }
    if ( !mBlockSignals )
    {
      mBlockSignals++;
      mSpinBoxTopoLat->clear();
      mSpinBoxTopoLon->clear();
      mBlockSignals--;
      emit crsChanged();
      emit hasValidSelectionChanged( hasValidSelection() );
    }
  } );

  connect( mSpinBoxTopoLat, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( !mBlockSignals )
      emit crsChanged();
  } );

  connect( mSpinBoxTopoLon, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( double ) {
    if ( !mBlockSignals )
      emit crsChanged();
  } );

  const QgsSettings settings;
  mSplitter->restoreState( settings.value( u"Windows/ProjectionSelectorDialog/splitterState"_s ).toByteArray() );
}

QgsCrsSelectionWidget::~QgsCrsSelectionWidget()
{
  QgsSettings settings;
  settings.setValue( u"Windows/ProjectionSelectorDialog/splitterState"_s, mSplitter->saveState() );
}

void QgsCrsSelectionWidget::setMessage( const QString &message )
{
  textEdit->setHtml( u"<head><style>%1</style></head><body>%2</body>"_s.arg( QgsApplication::reportStyleSheet(), message ) );
  textEdit->show();
}

void QgsCrsSelectionWidget::setShowNoCrs( bool show )
{
  if ( mShowNoCrsOption == show )
    return;

  mBlockSignals++;
  mShowNoCrsOption = show;
  if ( mShowNoCrsOption )
  {
    mComboCrsType->insertItem( 0, mNotSetText );
  }
  else
  {
    mComboCrsType->removeItem( 0 );
  }

  if ( show && mDeferredInvalidCrsSet )
  {
    mComboCrsType->setCurrentIndex( 0 );
  }

  mBlockSignals--;

  if ( mDeferredInvalidCrsSet )
    emit crsChanged();

  mDeferredInvalidCrsSet = false;

  emit hasValidSelectionChanged( hasValidSelection() );
}

bool QgsCrsSelectionWidget::showNoCrs() const
{
  return mShowNoCrsOption;
}

void QgsCrsSelectionWidget::setNotSetText( const QString &text, const QString &description )
{
  mNotSetText = text;

  if ( mShowNoCrsOption )
  {
    mComboCrsType->setItemText( 0, mNotSetText );
  }

  mLabelNoCrs->setText( description.isEmpty() ? text : description );
}

bool QgsCrsSelectionWidget::hasValidSelection() const
{
  if ( !mComboCrsType->currentData().isValid() )
    return true;
  else if ( mDeferredInvalidCrsSet )
    return false;
  else
  {
    switch ( static_cast<CrsType>( mComboCrsType->currentData().toInt() ) )
    {
      case QgsCrsSelectionWidget::CrsType::Predefined:
        return projectionSelector->hasValidSelection();
      case QgsCrsSelectionWidget::CrsType::Custom:
        return mCrsDefinitionWidget->crs().isValid();
      case QgsCrsSelectionWidget::CrsType::Topocentric:
      {
        const QgsCoordinateReferenceSystem baseCrs = mTopocentricBaseSelector->crs();
        return baseCrs.isValid();
      }
    }
    BUILTIN_UNREACHABLE
  }
}

QgsCoordinateReferenceSystemProxyModel::Filters QgsCrsSelectionWidget::filters() const
{
  return projectionSelector->filters();
}

void QgsCrsSelectionWidget::setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters )
{
  projectionSelector->setFilters( filters );
}

void QgsCrsSelectionWidget::setAllowTopocentricCrs( bool allow )
{
  if ( allow == mAllowTopocentricCrs )
    return;
  mAllowTopocentricCrs = allow;

  const int idx = mComboCrsType->findData( static_cast<int>( CrsType::Topocentric ) );
  if ( !allow && idx >= 0 )
    mComboCrsType->removeItem( idx );
  else if ( allow && idx < 0 )
    mComboCrsType->addItem( tr( "Topocentric CRS" ), static_cast<int>( CrsType::Topocentric ) );
}

QgsCoordinateReferenceSystem QgsCrsSelectionWidget::crs() const
{
  if ( !mComboCrsType->currentData().isValid() )
    return QgsCoordinateReferenceSystem();
  else
  {
    switch ( static_cast<CrsType>( mComboCrsType->currentData().toInt() ) )
    {
      case QgsCrsSelectionWidget::CrsType::Predefined:
        return projectionSelector->crs();
      case QgsCrsSelectionWidget::CrsType::Custom:
        return mCrsDefinitionWidget->crs();
      case QgsCrsSelectionWidget::CrsType::Topocentric:
      {
        const QgsCoordinateReferenceSystem base = mTopocentricBaseSelector->crs();
        return base.toTopocentricCrs( mSpinBoxTopoLat->value(), mSpinBoxTopoLon->value() );
      }
    }
    BUILTIN_UNREACHABLE
  }
}

void QgsCrsSelectionWidget::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mBlockSignals++;
  if ( !crs.isValid() )
  {
    if ( mShowNoCrsOption )
      mComboCrsType->setCurrentIndex( 0 );
    else
      mDeferredInvalidCrsSet = true;
  }
  else
  {
    double topoLat = 0.0, topoLon = 0.0;
    if ( crs.topocentricOrigin( topoLat, topoLon ) )
    {
      const QgsCoordinateReferenceSystem baseCrs = crs.topocentricBaseCrs();
      mTopocentricBaseSelector->setCrs( baseCrs );

      const QgsRectangle b = baseCrs.bounds();
      if ( !b.isNull() )
      {
        mSpinBoxTopoLat->setClearValue( ( b.yMinimum() + b.yMaximum() ) / 2.0 );
        mSpinBoxTopoLon->setClearValue( ( b.xMinimum() + b.xMaximum() ) / 2.0 );
      }
      mSpinBoxTopoLat->clear();
      mSpinBoxTopoLon->clear();
      mComboCrsType->setCurrentIndex( mComboCrsType->findData( static_cast<int>( CrsType::Topocentric ) ) );
      mStackedWidget->setCurrentWidget( mPageTopocentric );
    }
    else if ( crs.authid().isEmpty() )
    {
      projectionSelector->setCrs( crs );
      mCrsDefinitionWidget->setCrs( crs );
      mComboCrsType->setCurrentIndex( mComboCrsType->findData( static_cast<int>( CrsType::Custom ) ) );
      mStackedWidget->setCurrentWidget( mPageCustom );
    }
    else
    {
      projectionSelector->setCrs( crs );
      mCrsDefinitionWidget->setCrs( crs );
      mComboCrsType->setCurrentIndex( mComboCrsType->findData( static_cast<int>( CrsType::Predefined ) ) );
      mStackedWidget->setCurrentWidget( mPageDatabase );
    }
  }
  mBlockSignals--;

  emit crsChanged();
  emit hasValidSelectionChanged( hasValidSelection() );
}

void QgsCrsSelectionWidget::setOgcWmsCrsFilter( const QSet<QString> &crsFilter )
{
  projectionSelector->setOgcWmsCrsFilter( crsFilter );
}

void QgsCrsSelectionWidget::setPreviewRect( const QgsRectangle &rect )
{
  projectionSelector->setPreviewRect( rect );
}


//
// QgsProjectionSelectionDialog
//

QgsProjectionSelectionDialog::QgsProjectionSelectionDialog( QWidget *parent, Qt::WindowFlags fl, QgsCoordinateReferenceSystemProxyModel::Filters filters )
  : QDialog( parent, fl )
{
  QVBoxLayout *vlayout = new QVBoxLayout();

  mCrsWidget = new QgsCrsSelectionWidget( nullptr, filters );
  vlayout->addWidget( mCrsWidget, 1 );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsProjectionSelectionDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsProjectionSelectionDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsProjectionSelectionDialog::showHelp );

  vlayout->addWidget( mButtonBox );

  setLayout( vlayout );

  QgsGui::enableAutoGeometryRestore( this );

  //apply selected projection upon double-click on item
  connect( mCrsWidget, &QgsCrsSelectionWidget::crsDoubleClicked, this, &QgsProjectionSelectionDialog::accept );
}

void QgsProjectionSelectionDialog::setMessage( const QString &message )
{
  mCrsWidget->setMessage( message );
}

void QgsProjectionSelectionDialog::showNoCrsForLayerMessage()
{
  setMessage(
    tr( "This layer appears to have no projection specification." )
    + ' '
    + tr(
      "By default, this layer will now have its projection set to that of the project, "
      "but you may override this by selecting a different projection below."
    )
  );
}

void QgsProjectionSelectionDialog::setShowNoProjection( bool show )
{
  mCrsWidget->setShowNoCrs( show );
}

bool QgsProjectionSelectionDialog::showNoProjection() const
{
  return mCrsWidget->showNoCrs();
}

void QgsProjectionSelectionDialog::setNotSetText( const QString &text, const QString &description )
{
  mCrsWidget->setNotSetText( text, description );
}

void QgsProjectionSelectionDialog::setRequireValidSelection()
{
  mRequireValidSelection = true;
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );

  connect( mCrsWidget, &QgsCrsSelectionWidget::hasValidSelectionChanged, this, [this]( bool isValid ) { mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( isValid ); } );
}

bool QgsProjectionSelectionDialog::hasValidSelection() const
{
  return mCrsWidget->hasValidSelection();
}

QgsCoordinateReferenceSystemProxyModel::Filters QgsProjectionSelectionDialog::filters() const
{
  return mCrsWidget->filters();
}

void QgsProjectionSelectionDialog::setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters )
{
  mCrsWidget->setFilters( filters );
}

void QgsProjectionSelectionDialog::setAllowTopocentricCrs( bool allow )
{
  mCrsWidget->setAllowTopocentricCrs( allow );
}

QgsCoordinateReferenceSystem QgsProjectionSelectionDialog::crs() const
{
  return mCrsWidget->crs();
}

void QgsProjectionSelectionDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrsWidget->setCrs( crs );

  if ( mRequireValidSelection )
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( hasValidSelection() );
}

void QgsProjectionSelectionDialog::setOgcWmsCrsFilter( const QSet<QString> &crsFilter )
{
  mCrsWidget->setOgcWmsCrsFilter( crsFilter );
}

void QgsProjectionSelectionDialog::showHelp()
{
  QgsHelp::openHelp( u"working_with_projections/working_with_projections.html"_s );
}
