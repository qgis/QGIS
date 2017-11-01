/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsaggregatetoolbutton.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAggregateToolButton : QToolButton
{
%Docstring
*************************************************************************
qgsaggregatetoolbutton.h
--------------------------------------
Date                 : Nov 2017
Copyright            : (C) 2017 Matthias Kuhn
Email                : matthias@opengis.ch
**************************************************************************
                                                                         *
   This program is free software; you can redistribute it and/or modify  *
   it under the terms of the GNU General Public License as published by  *
   the Free Software Foundation; either version 2 of the License, or     *
   (at your option) any later version.                                   *
                                                                         *
**************************************************************************
%End

%TypeHeaderCode
#include "qgsaggregatetoolbutton.h"
%End
  public:
    QgsAggregateToolButton();

    void setType( QVariant::Type type );

    QVariant::Type type() const;
%Docstring
 :rtype: QVariant.Type
%End

    void setActive( bool active );
    bool active() const;
%Docstring
 :rtype: bool
%End

    QString aggregate() const;
%Docstring
 :rtype: str
%End
    void setAggregate( const QString &aggregate );

  signals:
    void aggregateChanged();
    void activeChanged();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsaggregatetoolbutton.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
