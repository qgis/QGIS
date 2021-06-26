/***************************************************************************
  qgslabelthinningsettings.cpp
  ----------------------------
  Date                 : December 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelthinningsettings.h"
#include "qgspropertycollection.h"
#include "qgsexpressioncontext.h"
#include "qgspallabeling.h"


void QgsLabelThinningSettings::updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context )
{
  Q_UNUSED( properties )
  Q_UNUSED( context )

  // temporarily avoid warnings
  int unused = 1;
  ( void )unused;
}
