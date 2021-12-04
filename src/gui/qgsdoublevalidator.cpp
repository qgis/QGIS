/***************************************************************************
                         qgsdoublevalidator.cpp  -  description
                             -------------------
    begin                : June 2020
    copyright            : (C) 2020 by Sebastien Peillet
    email                : sebastien.peillet@oslandia.com

  adapted version of Qgslonglongvalidator + QgsFieldValidator
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QLocale>
#include "qgis_gui.h"

#include "qgsdoublevalidator.h"

const QString PERMISSIVE_DOUBLE = R"(-?[\d]{0,1000}([\.%1][\d]{0,1000})?(e[+-]?[\d]{0,%2})?)";

QgsDoubleValidator::QgsDoubleValidator( QObject *parent )
  : QRegularExpressionValidator( parent )
  , mMinimum( std::numeric_limits<qreal>::lowest() )
  , mMaximum( std::numeric_limits<qreal>::max() )
{
  // The regular expression accept double with point as decimal point but also the locale decimal point
  const QRegularExpression reg( PERMISSIVE_DOUBLE.arg( locale().decimalPoint() ).arg( 1000 ) );
  setRegularExpression( reg );
}

QgsDoubleValidator::QgsDoubleValidator( const QRegularExpression &expression, double bottom, double top, QObject *parent )
  : QRegularExpressionValidator( parent )
  , mMinimum( bottom )
  , mMaximum( top )
{
  setRegularExpression( expression );
}

QgsDoubleValidator::QgsDoubleValidator( double bottom, double top, QObject *parent )
  : QRegularExpressionValidator( parent )
  , mMinimum( bottom )
  , mMaximum( top )
{
  // The regular expression accept double with point as decimal point but also the locale decimal point
  const QRegularExpression reg( PERMISSIVE_DOUBLE.arg( locale().decimalPoint() ).arg( 1000 ) );
  setRegularExpression( reg );
}

QgsDoubleValidator::QgsDoubleValidator( double bottom, double top, int decimal, QObject *parent )
  : QRegularExpressionValidator( parent )
  , mMinimum( bottom )
  , mMaximum( top )
{
  // The regular expression accept double with point as decimal point but also the locale decimal point
  const QRegularExpression reg( PERMISSIVE_DOUBLE.arg( locale().decimalPoint() ).arg( QString::number( decimal ) ) );
  setRegularExpression( reg );
}

QgsDoubleValidator::QgsDoubleValidator( int decimal, QObject *parent )
  : QRegularExpressionValidator( parent )
  , mMinimum( std::numeric_limits<qreal>::lowest() )
  , mMaximum( std::numeric_limits<qreal>::max() )
{
  // The regular expression accept double with point as decimal point but also the locale decimal point
  const QRegularExpression reg( PERMISSIVE_DOUBLE.arg( locale().decimalPoint() ).arg( QString::number( decimal ) ) );
  setRegularExpression( reg );
}

void QgsDoubleValidator::setMaxDecimals( int maxDecimals )
{
  const QRegularExpression reg( PERMISSIVE_DOUBLE.arg( locale().decimalPoint() ).arg( QString::number( maxDecimals ) ) );
  setRegularExpression( reg );
}

QValidator::State QgsDoubleValidator::validate( QString &input, int & ) const
{
  if ( input.isEmpty() )
    return Intermediate;


  bool ok = false;
  const double entered = QgsDoubleValidator::toDouble( input, &ok );
  if ( ! ok )
  {
    if ( regularExpression().match( input ).captured( 0 ) == input )
      return Intermediate;
    else
      return Invalid;
  }

  if ( entered >= mMinimum && entered <= mMaximum && regularExpression().match( input ).captured( 0 ) == input )
    return Acceptable;
  else
    return Intermediate;
}

QValidator::State QgsDoubleValidator::validate( QString &input ) const
{
  if ( input.isEmpty() )
    return Intermediate;


  bool ok = false;
  const double entered = QgsDoubleValidator::toDouble( input, &ok );
  if ( ! ok )
  {
    if ( regularExpression().match( input ).captured( 0 ) == input )
      return Intermediate;
    else
      return Invalid;
  }

  if ( entered >= mMinimum && entered <= mMaximum && regularExpression().match( input ).captured( 0 ) == input )
    return Acceptable;
  else
    return Intermediate;
}

double QgsDoubleValidator::toDouble( const QString &input )
{
  bool ok = false;
  return toDouble( input, &ok );
}

double QgsDoubleValidator::toDouble( const QString &input, bool *ok )
{
  double value = QLocale().toDouble( input, ok );

  if ( ! *ok )
  {
    value = QLocale( QLocale::C ).toDouble( input, ok );
  }
  // Still non ok? Try without locale's group separator
  if ( ! *ok && !( QLocale().numberOptions() & QLocale::NumberOption::OmitGroupSeparator ) )
  {
    value = QLocale( ).toDouble( QString( input ).replace( QLocale().groupSeparator(), QString() ), ok );
  }
  return value ;
}
