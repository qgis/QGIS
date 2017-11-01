/***************************************************************************
    qgsaggregatetoolbutton.h
     --------------------------------------
    Date                 : Nov 2017
    Copyright            : (C) 2017 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAGGREGATETOOLBUTTON_H
#define QGSAGGREGATETOOLBUTTON_H

#include <QToolButton>
#include <QVariant>

#include "qgsaggregatecalculator.h"
#include "qgis_gui.h"

class GUI_EXPORT QgsAggregateToolButton : public QToolButton
{
    Q_OBJECT

  public:
    QgsAggregateToolButton();

    void setType( QVariant::Type type );

    QVariant::Type type() const;

    void setActive( bool active );
    bool active() const;

    QString aggregate() const;
    void setAggregate( const QString &aggregate );

  signals:
    void aggregateChanged();
    void activeChanged();

  private slots:
    void aboutToShowMenu();

  private:
    void updateAvailableAggregates();
    QMenu *mMenu = nullptr;
    QVariant::Type mType;
    bool mActive = false;
    QString mAggregate;
    QList<QgsAggregateCalculator::AggregateInfo> mAvailableAggregates;
};

#endif // QGSAGGREGATETOOLBUTTON_H
