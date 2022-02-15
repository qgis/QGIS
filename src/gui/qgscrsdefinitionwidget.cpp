/***************************************************************************
    qgscrsdefinitionwidget.cpp
    ---------------------
    begin                : December 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscrsdefinitionwidget.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsprojutils.h"

#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <proj.h>

QgsCrsDefinitionWidget::QgsCrsDefinitionWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( mButtonCalculate, &QPushButton::clicked, this, &QgsCrsDefinitionWidget::pbnCalculate_clicked );
  connect( mButtonCopyCRS, &QPushButton::clicked, this, &QgsCrsDefinitionWidget::pbnCopyCRS_clicked );
  connect( mButtonValidate, &QPushButton::clicked, this, &QgsCrsDefinitionWidget::validateCurrent );

  mFormatComboBox->addItem( tr( "WKT (Recommended)" ), static_cast< int >( Qgis::CrsDefinitionFormat::Wkt ) );
  mFormatComboBox->addItem( tr( "Proj String (Legacy — Not Recommended)" ), static_cast< int >( Qgis::CrsDefinitionFormat::Proj ) );
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( Qgis::CrsDefinitionFormat::Wkt ) ) );

  connect( mFormatComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsCrsDefinitionWidget::formatChanged );
  connect( mTextEditParameters, &QPlainTextEdit::textChanged, this, &QgsCrsDefinitionWidget::crsChanged );
}

QgsCoordinateReferenceSystem QgsCrsDefinitionWidget::crs() const
{
  QgsCoordinateReferenceSystem crs;
  switch ( static_cast< Qgis::CrsDefinitionFormat >( mFormatComboBox->currentData().toInt() ) )
  {
    case Qgis::CrsDefinitionFormat::Wkt:
      crs = QgsCoordinateReferenceSystem::fromWkt( mTextEditParameters->toPlainText() );
      break;

    case Qgis::CrsDefinitionFormat::Proj:
      crs = QgsCoordinateReferenceSystem::fromProj( mTextEditParameters->toPlainText() );
      break;
  }

  crs.setNativeFormat( format() );
  return crs;
}

void QgsCrsDefinitionWidget::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  setCrs( crs, crs.nativeFormat() );
}

void QgsCrsDefinitionWidget::setCrs( const QgsCoordinateReferenceSystem &crs, Qgis::CrsDefinitionFormat nativeFormat )
{
  switch ( nativeFormat )
  {
    case Qgis::CrsDefinitionFormat::Wkt:
      whileBlocking( mTextEditParameters )->setPlainText( crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false ) );
      break;
    case Qgis::CrsDefinitionFormat::Proj:
      whileBlocking( mTextEditParameters )->setPlainText( crs.toProj() );
      break;
  }

  whileBlocking( mFormatComboBox )->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( nativeFormat ) ) );
  emit crsChanged();
}

Qgis::CrsDefinitionFormat QgsCrsDefinitionWidget::format() const
{
  return static_cast< Qgis::CrsDefinitionFormat >( mFormatComboBox->currentData().toInt() );
}

void QgsCrsDefinitionWidget::setFormat( Qgis::CrsDefinitionFormat format )
{
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( format ) ) );
}

QString QgsCrsDefinitionWidget::definitionString() const
{
  return mTextEditParameters->toPlainText();
}

void QgsCrsDefinitionWidget::setDefinitionString( const QString &definition )
{
  mTextEditParameters->setPlainText( definition );
}

void QgsCrsDefinitionWidget::pbnCopyCRS_clicked()
{
  std::unique_ptr< QgsProjectionSelectionDialog > selector = std::make_unique< QgsProjectionSelectionDialog >( this );
  if ( selector->exec() )
  {
    const QgsCoordinateReferenceSystem srs = selector->crs();

    whileBlocking( mFormatComboBox )->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( Qgis::CrsDefinitionFormat::Wkt ) ) );
    mTextEditParameters->setPlainText( srs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, true ) );
  }
}

static void proj_collecting_logger( void *user_data, int /*level*/, const char *message )
{
  QStringList *dest = reinterpret_cast< QStringList * >( user_data );
  QString messageString( message );
  messageString.replace( QLatin1String( "internal_proj_create: " ), QString() );
  dest->append( messageString );
}

void QgsCrsDefinitionWidget::validateCurrent()
{
  const QString projDef = mTextEditParameters->toPlainText();

  PJ_CONTEXT *context = proj_context_create();

  QStringList projErrors;
  proj_log_func( context, &projErrors, proj_collecting_logger );
  QgsProjUtils::proj_pj_unique_ptr crs;

  switch ( static_cast< Qgis::CrsDefinitionFormat >( mFormatComboBox->currentData().toInt() ) )
  {
    case Qgis::CrsDefinitionFormat::Wkt:
    {
      PROJ_STRING_LIST warnings = nullptr;
      PROJ_STRING_LIST grammerErrors = nullptr;
      crs.reset( proj_create_from_wkt( context, projDef.toUtf8().constData(), nullptr, &warnings, &grammerErrors ) );
      QStringList warningStrings;
      QStringList grammerStrings;
      for ( auto iter = warnings; iter && *iter; ++iter )
        warningStrings << QString( *iter );
      for ( auto iter = grammerErrors; iter && *iter; ++iter )
        grammerStrings << QString( *iter );
      proj_string_list_destroy( warnings );
      proj_string_list_destroy( grammerErrors );

      if ( crs )
      {
        QMessageBox::information( this, tr( "Custom Coordinate Reference System" ),
                                  tr( "This WKT projection definition is valid." ) );
      }
      else
      {
        QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                              tr( "This WKT projection definition is not valid:" ) + QStringLiteral( "\n\n" ) + warningStrings.join( '\n' ) + grammerStrings.join( '\n' ) );
      }
      break;
    }

    case Qgis::CrsDefinitionFormat::Proj:
    {
      const QString projCrsString = projDef + ( projDef.contains( QStringLiteral( "+type=crs" ) ) ? QString() : QStringLiteral( " +type=crs" ) );
      crs.reset( proj_create( context, projCrsString.toUtf8().constData() ) );
      if ( crs )
      {
        QMessageBox::information( this, tr( "Custom Coordinate Reference System" ),
                                  tr( "This proj projection definition is valid." ) );
      }
      else
      {
        QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                              tr( "This proj projection definition is not valid:" ) + QStringLiteral( "\n\n" ) + projErrors.join( '\n' ) );
      }
      break;
    }
  }

  // reset logger to terminal output
  proj_log_func( context, nullptr, nullptr );
  proj_context_destroy( context );
  context = nullptr;
}

void QgsCrsDefinitionWidget::formatChanged()
{
  QgsCoordinateReferenceSystem crs;
  QString newFormatString;
  switch ( static_cast< Qgis::CrsDefinitionFormat >( mFormatComboBox->currentData().toInt() ) )
  {
    case Qgis::CrsDefinitionFormat::Proj:
    {
      crs.createFromWkt( multiLineWktToSingleLine( mTextEditParameters->toPlainText() ) );
      if ( crs.isValid() )
        newFormatString = crs.toProj();
      break;
    }

    case Qgis::CrsDefinitionFormat::Wkt:
    {
      PJ_CONTEXT *pjContext = QgsProjContext::get();
      QString proj = mTextEditParameters->toPlainText();
      proj.replace( QLatin1String( "+type=crs" ), QString() );
      proj += QLatin1String( " +type=crs" );
      QgsProjUtils::proj_pj_unique_ptr crs( proj_create( QgsProjContext::get(), proj.toUtf8().constData() ) );
      if ( crs )
      {
        const QByteArray multiLineOption = QStringLiteral( "MULTILINE=YES" ).toLocal8Bit();
        const char *const options[] = {multiLineOption.constData(), nullptr};
        newFormatString = QString( proj_as_wkt( pjContext, crs.get(), PJ_WKT2_2019, options ) );
      }
      break;
    }
  }
  if ( !newFormatString.isEmpty() )
    mTextEditParameters->setPlainText( newFormatString );
}

void QgsCrsDefinitionWidget::pbnCalculate_clicked()
{
  // We must check the prj def is valid!
  QString projDef = mTextEditParameters->toPlainText();

  // Get the WGS84 coordinates
  bool okN, okE;
  double latitude = mNorthWGS84Edit->text().toDouble( &okN );
  double longitude = mEastWGS84Edit->text().toDouble( &okE );

  if ( !okN || !okE )
  {
    QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                          tr( "Latitude and Longitude must be in decimal form." ) );
    mProjectedXLabel->clear();
    mProjectedYLabel->clear();
    return;
  }

  QgsCoordinateReferenceSystem target;
  if ( static_cast< Qgis::CrsDefinitionFormat >( mFormatComboBox->currentData().toInt() ) == Qgis::CrsDefinitionFormat::Proj )
  {
    projDef = projDef + ( projDef.contains( QStringLiteral( "+type=crs" ) ) ? QString() : QStringLiteral( " +type=crs" ) );
    target = QgsCoordinateReferenceSystem::fromProj( projDef );
  }
  else
  {
    target = QgsCoordinateReferenceSystem::fromWkt( projDef );
  }

  if ( !target.isValid() )
  {
    QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                          tr( "This CRS projection definition is not valid." ) );
    mProjectedXLabel->clear();
    mProjectedYLabel->clear();
    return;
  }

  const QgsCoordinateTransform transform( target.toGeographicCrs(), target, QgsCoordinateTransformContext() );
  try
  {
    const QgsPointXY res = transform.transform( QgsPointXY( longitude, latitude ) );
    const int precision = target.isGeographic() ? 7 : 4;
    mProjectedXLabel->setText( QLocale().toString( res.x(), 'f', precision ) );
    mProjectedYLabel->setText( QLocale().toString( res.y(), 'f', precision ) );
  }
  catch ( QgsCsException &e )
  {
    mProjectedXLabel->setText( tr( "Error" ) );
    mProjectedYLabel->setText( tr( "Error" ) );
    QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                          e.what() );
  }
}

QString QgsCrsDefinitionWidget::multiLineWktToSingleLine( const QString &wkt )
{
  QString res = wkt;
  QRegularExpression re( QStringLiteral( "\\s*\\n\\s*" ) );
  re.setPatternOptions( QRegularExpression::MultilineOption );
  res.replace( re, QString() );
  return res;
}
