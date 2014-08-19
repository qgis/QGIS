/***************************************************************************
    qgsrelationreferencewidget.h
     --------------------------------------
    Date                 : 20.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONREFERENCEWIDGETWRAPPER_H
#define QGSRELATIONREFERENCEWIDGETWRAPPER_H

#include "qgsrelationreferencewidget.h"
#include "qgseditorwidgetwrapper.h"

class GUI_EXPORT QgsRelationReferenceWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsRelationReferenceWidgetWrapper( QgsVectorLayer* vl,
        int fieldIdx,
        QWidget* editor,
        QgsAttributeEditorContext context,
        QgsMapCanvas* canvas,
        QgsMessageBar* messageBar,
        QWidget* parent = 0 );

    virtual QWidget* createWidget( QWidget* parent );
    virtual void initWidget( QWidget* editor );
    virtual QVariant value();

  public slots:
    virtual void setValue( const QVariant& value );
    virtual void setEnabled( bool enabled );

  private:
    QgsRelationReferenceWidget* mWidget;
    QgsAttributeEditorContext mEditorContext;
    QgsMapCanvas* mCanvas;
    QgsMessageBar* mMessageBar;
};

#endif // QGSRELATIONREFERENCEWIDGETWRAPPER_H
