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

/**
 * \ingroup gui
 *
 * Search widget for the children of a relation.
 * For each attribute of the child, an additional QgsAggregateToolButton will be shown
 * to determine how the values should be aggregated for searching.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsRelationAggregateSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT

  public:

    /**
     * Constructor
     */
    explicit QgsRelationAggregateSearchWidgetWrapper( QgsVectorLayer *layer, QgsRelationWidgetWrapper *wrapper, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QString expression() const override;

    bool valid() const override;
    QWidget *createWidget( QWidget *parent ) override;
    bool applyDirectly() override;
    void setExpression( const QString &value ) override;
    bool eventFilter( QObject *watched, QEvent *event ) override;

  private:
    QgsRelationWidgetWrapper *mWrapper = nullptr;
    QgsAttributeForm *mAttributeForm = nullptr;
    QWidget *mContainerWidget = nullptr;
};

#endif // QGSRELATIONAGGREGATESEARCHWIDGETWRAPPER_H
