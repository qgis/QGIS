/***************************************************************************
                      qgsfeatureaction.h  -  description
                               ------------------
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
#ifndef QGSFEATUREACTION_H
#define QGSFEATUREACTION_H

#include "qgsfeature.h"

#include <QList>
#include <QPair>
#include <QAction>

class QgsIdentifyResults;
class QgsVectorLayer;
class QTreeWidgetItem;

class QgsFeatureAction : public QAction
{
    Q_OBJECT

  public:
    QgsFeatureAction( const QString &name, QgsFeature &f, QgsVectorLayer *vl, int action, QObject *parent );
    QgsFeatureAction( const QString &name, QgsIdentifyResults *results, QgsVectorLayer *vl, int action, QTreeWidgetItem *featItem );

  public slots:
    void execute();

  private:
    QgsVectorLayer *mLayer;
    int mAction;
    int mIdx;
    QgsAttributeMap mAttributes;
};

#endif
