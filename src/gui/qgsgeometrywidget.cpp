/***************************************************************************
   qgsgeometrywidget.cpp
    --------------------------------------
   Date                 : March 2015
   Copyright            : (C) 2015 Nyall Dawson
   Email                : nyall.dawson@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsgeometrywidget.h"
#include "moc_qgsgeometrywidget.cpp"
#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgsjsonutils.h"
#include "qgsguiutils.h"
#include "qgsprojectionselectiondialog.h"
#include <QLineEdit>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QClipboard>

QgsGeometryWidget::QgsGeometryWidget( QWidget *parent )
  : QWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  mLineEdit->setReadOnly( true );
  mLineEdit->setStyleSheet( QStringLiteral( "font-style: italic;" ) );

  // make text appear in disabled text color, as it's not directly editable
  QPalette palette = mLineEdit->palette();
  palette.setColor( QPalette::Text, palette.color( QPalette::Disabled, QPalette::Text ) );
  mLineEdit->setPalette( palette );

  mLineEdit->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );

  mButton = new QToolButton();
  mButton->setStyleSheet( QStringLiteral( "QToolButton{ background: none; border: 1px solid rgba(0, 0, 0, 0%);} QToolButton:focus { border: 1px solid palette(highlight); }" ) );
  const int iconSize = QgsGuiUtils::scaleIconSize( 24 );
  mButton->setIconSize( QSize( iconSize, iconSize ) );
  // button width is 1.25 * icon size, height 1.1 * icon size. But we round to ensure even pixel sizes for equal margins
  mButton->setFixedSize( 2 * static_cast<int>( 1.25 * iconSize / 2.0 ), 2 * static_cast<int>( iconSize * 1.1 / 2.0 ) );

  mButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOffsetCurve.svg" ) ) );

  layout->addWidget( mLineEdit, 1 );
  layout->addWidget( mButton );

  setLayout( layout );

  setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );

  setFocusProxy( mLineEdit );

  mMenu = new QMenu( this );

  mCopyWktAction = new QAction( mMenu );
  mCopyWktAction->setText( tr( "Copy as WKT" ) );
  connect( mCopyWktAction, &QAction::triggered, this, &QgsGeometryWidget::copyAsWkt );

  mCopyGeoJsonAction = new QAction( mMenu );
  mCopyGeoJsonAction->setText( tr( "Copy as GeoJSON" ) );
  connect( mCopyGeoJsonAction, &QAction::triggered, this, &QgsGeometryWidget::copyAsGeoJson );

  mPasteAction = new QAction( mMenu );
  mPasteAction->setText( tr( "Paste Geometry" ) );
  connect( mPasteAction, &QAction::triggered, this, &QgsGeometryWidget::pasteTriggered );

  mClearAction = new QAction( mMenu );
  mClearAction->setText( tr( "Clear" ) );
  connect( mClearAction, &QAction::triggered, this, &QgsGeometryWidget::clearGeometry );

  mMenu->addAction( mCopyWktAction );
  mMenu->addAction( mCopyGeoJsonAction );
  mMenu->addSeparator();
  mMenu->addAction( mPasteAction );
  mMenu->addAction( mClearAction );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsGeometryWidget::prepareMenu );

  mButton->setMenu( mMenu );
  mButton->setPopupMode( QToolButton::InstantPopup );

  updateLineEdit();
}

void QgsGeometryWidget::setGeometryValue( const QgsReferencedGeometry &geometry )
{
  if ( geometry == mGeometry )
    return;

  if ( !typeIsAcceptable( geometry.wkbType() ) )
  {
    return;
  }

  mGeometry = geometry;
  updateLineEdit();
  emit geometryValueChanged( mGeometry );
}

QgsReferencedGeometry QgsGeometryWidget::geometryValue() const
{
  return mGeometry;
}

void QgsGeometryWidget::setAcceptedWkbTypes( const QList<Qgis::WkbType> &types )
{
  mAcceptedTypes = types;
}

QList<Qgis::WkbType> QgsGeometryWidget::acceptedWkbTypes() const
{
  return mAcceptedTypes;
}

bool QgsGeometryWidget::isReadOnly() const
{
  return mReadOnly;
}

void QgsGeometryWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
}

void QgsGeometryWidget::clearGeometry()
{
  if ( mGeometry.isNull() )
    return;

  setGeometryValue( QgsReferencedGeometry() );
}

void QgsGeometryWidget::copyAsWkt()
{
  if ( mGeometry.isNull() )
    return;

  QApplication::clipboard()->setText( mGeometry.asWkt() );
}

void QgsGeometryWidget::copyAsGeoJson()
{
  if ( mGeometry.isNull() )
    return;

  QApplication::clipboard()->setText( mGeometry.asJson() );
}

void QgsGeometryWidget::pasteTriggered()
{
  if ( !mPastedGeom.isNull() )
  {
    QgsCoordinateReferenceSystem defaultCrs = mPastedGeom.crs();

    // default to CRS of current geometry, if we have no better guesses as to what the clipboard CRS is
    if ( !defaultCrs.isValid() )
      defaultCrs = mGeometry.crs();

    QgsProjectionSelectionDialog crsSelector( this );
    crsSelector.setWindowTitle( tr( "Paste Geometry" ) );
    crsSelector.setMessage( tr( "Please specify the Coordinate Reference System (CRS) for the pasted geometry." ) );
    crsSelector.setCrs( defaultCrs );
    if ( crsSelector.exec() )
    {
      mPastedGeom.setCrs( crsSelector.crs() );
      setGeometryValue( mPastedGeom );
      mPastedGeom = QgsReferencedGeometry();
    }
  }
}

void QgsGeometryWidget::fetchGeomFromClipboard()
{
  mPastedGeom = QgsReferencedGeometry();
  if ( mReadOnly )
    return;

  const QString text = QApplication::clipboard()->text();
  if ( text.isEmpty() )
    return;

  //try reading as a single wkt string
  mPastedGeom = QgsReferencedGeometry( QgsGeometry::fromWkt( text ), QgsCoordinateReferenceSystem() );
  if ( !mPastedGeom.isNull() && typeIsAcceptable( mPastedGeom.wkbType() ) )
  {
    return;
  }
  mPastedGeom = QgsReferencedGeometry();

  //try reading as a list
  const QStringList lines = text.split( "\n", Qt::SkipEmptyParts );
  if ( !lines.isEmpty() )
  {
    for ( const QString &line : lines )
    {
      const QgsGeometry geometry = QgsGeometry::fromWkt( line );
      if ( !geometry.isNull() && typeIsAcceptable( geometry.wkbType() ) )
      {
        mPastedGeom = QgsReferencedGeometry( geometry, QgsCoordinateReferenceSystem() );
        return;
      }
    }
  }

  // try reading a GeoJSON
  const QgsFeatureList features = QgsJsonUtils::stringToFeatureList( text );
  if ( !features.isEmpty() && features.at( 0 ).hasGeometry() )
  {
    // assume EPSG:4326 for GeoJSON
    mPastedGeom = QgsReferencedGeometry( features.at( 0 ).geometry(), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
    return;
  }
}

bool QgsGeometryWidget::typeIsAcceptable( Qgis::WkbType type ) const
{
  if ( mAcceptedTypes.isEmpty() )
  {
    return true;
  }

  return mAcceptedTypes.contains( type );
}

void QgsGeometryWidget::prepareMenu()
{
  fetchGeomFromClipboard();

  mCopyGeoJsonAction->setEnabled( !mGeometry.isNull() );
  mCopyWktAction->setEnabled( !mGeometry.isNull() );
  mClearAction->setEnabled( !mGeometry.isNull() && !mReadOnly );
  mPasteAction->setEnabled( !mPastedGeom.isNull() && !mReadOnly );
}

void QgsGeometryWidget::updateLineEdit()
{
  if ( mGeometry.isNull() )
  {
    mLineEdit->setText( QgsApplication::nullRepresentation() );
  }
  else
  {
    QString wkt = mGeometry.asWkt();
    if ( wkt.length() >= 1050 )
    {
      wkt = wkt.left( QgsField::MAX_WKT_LENGTH ) + QChar( 0x2026 );
    }

    if ( mGeometry.crs().isValid() )
    {
      mLineEdit->setText( QStringLiteral( "%1 [%2]" ).arg( wkt, mGeometry.crs().userFriendlyIdentifier() ) );
    }
    else
    {
      mLineEdit->setText( wkt );
    }
  }
}
