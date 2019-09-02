/***************************************************************************
    qgsclassificationstandarddeviation.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QObject>

#include "qgsclassificationstandarddeviation.h"
#include "qgsgraduatedsymbolrenderer.h"

const QString QgsClassificationStandardDeviation::METHOD_ID = QStringLiteral( "StdDev" );


QgsClassificationStandardDeviation::QgsClassificationStandardDeviation()
  : QgsClassificationMethod( SymmetricModeAvailable )
{

}

QString QgsClassificationStandardDeviation::name() const
{
  return QObject::tr( "Standard Deviation" );
}

QString QgsClassificationStandardDeviation::id() const
{
  return METHOD_ID;
}

QgsClassificationMethod *QgsClassificationStandardDeviation::clone() const
{
  QgsClassificationStandardDeviation *c = new QgsClassificationStandardDeviation();
  copyBase( c );
  c->mStdDev = mStdDev;
  return c;
}


QList<double> QgsClassificationStandardDeviation::calculateBreaks( double minimum, double maximum,
    const QList<double> &values, int nclasses )
{
  // C++ implementation of the standard deviation class interval algorithm
  // as implemented in the 'classInt' package available for the R statistical
  // prgramming language.

  // Returns breaks based on 'prettyBreaks' of the centred and scaled
  // values of 'values', and may have a number of classes different from 'classes'.

  // If there are no values to process: bail out
  if ( values.isEmpty() )
    return QList<double>();

  double mean = 0.0;
  mStdDev = 0.0;
  int n = values.count();

  for ( int i = 0; i < n; i++ )
  {
    mean += values[i];
  }
  mean = mean / static_cast< double >( n );

  double sd = 0.0;
  for ( int i = 0; i < n; i++ )
  {
    sd = values[i] - mean;
    mStdDev += sd * sd;
  }
  mStdDev = std::sqrt( mStdDev / n );

  // if not symmetric, the symmetry point is the mean
  mEffectiveSymmetryPoint = symmetricModeEnabled() ? symmetryPoint() : mean;

  QList<double> breaks = QgsSymbolLayerUtils::prettyBreaks( ( minimum - mEffectiveSymmetryPoint ) / mStdDev, ( maximum - mEffectiveSymmetryPoint ) / mStdDev, nclasses );
  makeBreaksSymmetric( breaks, 0.0, symmetryAstride() ); //0.0 because breaks where computed on a centered distribution

  for ( int i = 0; i < breaks.count(); i++ )
    breaks[i] = ( breaks[i] * mStdDev ) + mEffectiveSymmetryPoint;

  return breaks;
}

QString QgsClassificationStandardDeviation::labelForRange( const double lowerValue, const double upperValue, QgsClassificationMethod::ClassPosition position ) const
{
  const QString lowerLabel = valueToLabel( lowerValue );
  const QString upperLabel = valueToLabel( upperValue );

  switch ( position )
  {
    case LowerBound:
      return "< " + upperLabel + " Std Dev";
    case Inner:
    {
      QString label( labelFormat() );
      label.replace( QLatin1String( "%1" ), lowerLabel ).replace( QLatin1String( "%2" ), upperLabel );
      return label;
    }
    case UpperBound:
      return ">= " + lowerLabel + " Std Dev";
  }
}


QString QgsClassificationStandardDeviation::valueToLabel( const double value ) const
{
  double normalized = ( value - mEffectiveSymmetryPoint ) / mStdDev;
  return QString::number( normalized, 'f', 2 ) + " Std Dev";
}


void QgsClassificationStandardDeviation::writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  element.setAttribute( QLatin1Literal( "std_dev" ), QString::number( mStdDev, 'f', 16 ) );
  element.setAttribute( QLatin1Literal( "effective_symmetry_point" ), QString::number( mEffectiveSymmetryPoint, 'f', 16 ) );
}

void QgsClassificationStandardDeviation::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mStdDev = element.attribute( "std_dev", QLatin1Literal( "1.0" ) ).toDouble();
  mEffectiveSymmetryPoint = element.attribute( "effective_symmetry_point", QLatin1Literal( "0.0" ) ).toDouble();
}
