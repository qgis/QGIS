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
#include "qgis_app.h"

class APP_EXPORT QgsLayoutScaleBarValidityCheck : public QgsAbstractValidityCheck
{
  public:

    QgsLayoutScaleBarValidityCheck *create() const override;
    QString id() const override;
    int checkType() const override;
    bool prepareCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;
    QList< QgsValidityCheckResult > runCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;

  private:
    QList<QgsValidityCheckResult> mResults;
};

class APP_EXPORT QgsLayoutOverviewValidityCheck : public QgsAbstractValidityCheck
{
  public:

    QgsLayoutOverviewValidityCheck *create() const override;
    QString id() const override;
    int checkType() const override;
    bool prepareCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;
    QList< QgsValidityCheckResult > runCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;

  private:
    QList<QgsValidityCheckResult> mResults;
};
