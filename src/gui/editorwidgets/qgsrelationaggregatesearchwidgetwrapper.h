/***************************************************************************
     qgsrelationaggregatesearchwidget.h
     -----------------------------
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


#ifndef QGSRELATIONAGGREGATESEARCHWIDGETWRAPPER_H
#define QGSRELATIONAGGREGATESEARCHWIDGETWRAPPER_H

#include "qgis_gui.h"
#include "qgssearchwidgetwrapper.h"
#include "qgsattributeform.h"

class QgsRelationWidgetWrapper;

class GUI_EXPORT QgsRelationAggregateSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT

  public:
    explicit QgsRelationAggregateSearchWidgetWrapper( QgsVectorLayer *vl, QgsRelationWidgetWrapper *wrapper, QWidget *parent SIP_TRANSFERTHIS = 0 );

    virtual QString expression() const override;

    virtual bool valid() const override;
    virtual QWidget *createWidget( QWidget *parent ) override;
    virtual bool applyDirectly() override;
    virtual void setExpression( const QString &value ) override;

  private:
    QgsRelationWidgetWrapper *mWrapper;
    QgsAttributeForm *mAttributeForm;


};

#endif // QGSRELATIONAGGREGATESEARCHWIDGETWRAPPER_H
