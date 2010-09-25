/***************************************************************************
                     qgsfeatureaction.cpp  -  description
                              -------------------
      begin                : 2010-09-20
      copyright            : (C) 2010 by Jürgen E. Fischer
      email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsfeatureaction.h"
#include "qgsvectorlayer.h"
#include "qgsidentifyresults.h"

QgsFeatureAction::QgsFeatureAction( const QString &name, QgsIdentifyResults *results, QgsVectorLayer *vl, int action, QTreeWidgetItem *featItem )
    : QAction( name, results )
    , mLayer( vl )
    , mAction( action )
{
  results->retrieveAttributes( featItem, mAttributes, mIdx );
}

QgsFeatureAction::QgsFeatureAction( const QString &name, QgsFeature &f, QgsVectorLayer *layer, int action, QObject *parent )
    : QAction( name, parent )
    , mLayer( layer )
    , mAction( action )
{
  mAttributes = f.attributeMap();
}

void QgsFeatureAction::execute()
{
  mLayer->actions()->doAction( mAction, mAttributes, mIdx );
}
