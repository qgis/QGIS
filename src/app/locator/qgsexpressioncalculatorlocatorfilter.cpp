/***************************************************************************
                        qgsexpressionlocatorfilters.cpp
                        ----------------------------
   begin                : May 2017
   copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsexpressioncalculatorlocatorfilter.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsproject.h"
#include "qgisapp.h"

#include <QClipboard>



QgsExpressionCalculatorLocatorFilter::QgsExpressionCalculatorLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{
  setUseWithoutPrefix( false );
}

QgsExpressionCalculatorLocatorFilter *QgsExpressionCalculatorLocatorFilter::clone() const
{
  return new QgsExpressionCalculatorLocatorFilter();
}

void QgsExpressionCalculatorLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback * )
{
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
          << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
          << QgsExpressionContextUtils::layerScope( QgisApp::instance()->activeLayer() );

  QString error;
  if ( QgsExpression::checkExpression( string, &context, error ) )
  {
    QgsExpression exp( string );
    const QString resultString = exp.evaluate( &context ).toString();
    if ( !resultString.isEmpty() )
    {
      QgsLocatorResult result;
      result.filter = this;
      result.displayString = tr( "Copy “%1” to clipboard" ).arg( resultString );
      result.setUserData( resultString );
      result.score = 1;
      emit resultFetched( result );
    }
  }
}

void QgsExpressionCalculatorLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QApplication::clipboard()->setText( result.userData().toString() );
}
