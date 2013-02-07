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
#ifndef QGSFEATUREACTION_H
#define QGSFEATUREACTION_H

#include "qgsfeature.h"

#include <QList>
#include <QPair>
#include <QAction>

class QgsIdentifyResultsDialog;
class QgsVectorLayer;
class QgsHighlight;
class QgsAttributeDialog;

class QgsFeatureAction : public QAction
{
    Q_OBJECT

  public:
    QgsFeatureAction( const QString &name, QgsFeature &f, QgsVectorLayer *vl, int action, int defaultAttr, QObject *parent );

  public slots:
    void execute();
    bool viewFeatureForm( QgsHighlight *h = 0 );
    bool editFeature();
    bool addFeature();

  private:
    QgsAttributeDialog *newDialog( bool cloneFeature );

    QgsVectorLayer *mLayer;
    QgsFeature &mFeature;
    int mAction;
    int mIdx;

    static QMap<QgsVectorLayer *, QgsAttributeMap> mLastUsedValues;
};

#endif
