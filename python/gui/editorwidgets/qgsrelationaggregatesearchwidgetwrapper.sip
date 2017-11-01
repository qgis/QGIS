/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsrelationaggregatesearchwidgetwrapper.h      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRelationAggregateSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%Docstring
*************************************************************************
qgsrelationaggregatesearchwidget.h
-----------------------------
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
#include "qgsrelationaggregatesearchwidgetwrapper.h"
%End
  public:
    explicit QgsRelationAggregateSearchWidgetWrapper( QgsVectorLayer *vl, QgsRelationWidgetWrapper *wrapper, QWidget *parent /TransferThis/ = 0 );

    virtual QString expression() const;

    virtual bool valid() const;
    virtual QWidget *createWidget( QWidget *parent );
    virtual bool applyDirectly();
    virtual void setExpression( const QString &value );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsrelationaggregatesearchwidgetwrapper.h      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
