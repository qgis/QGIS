/***************************************************************************
                              qgslayoutvaliditychecks.h
                              ---------------------------
    begin                : November 2018
    copyright            : (C) 2018 Nyall Dawson
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

#include "qgsabstractvaliditycheck.h"

class QgsLayoutMapCrsValidityCheck : public QgsAbstractValidityCheck
{
    Q_OBJECT

  public:

    QString id() const override { return "map_crs_check"; }
    int checkType() const override { return QgsAbstractValidityCheck::TypeLayoutCheck; }
    QString name() const override { return "Map CRS Check"; }
    QList< QgsValidityCheckResult > runCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) const override;




};
