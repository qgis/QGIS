/***************************************************************************
    qgsrelationwidgetwrapper.h
     --------------------------------------
    Date                 : 14.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONWIDGETWRAPPER_H
#define QGSRELATIONWIDGETWRAPPER_H

#include "qgswidgetwrapper.h"

class QgsRelationEditorWidget;

class GUI_EXPORT QgsRelationWidgetWrapper : public QgsWidgetWrapper
{
    Q_OBJECT

  public:
    explicit QgsRelationWidgetWrapper( QgsVectorLayer* vl, const QgsRelation& relation, QWidget* editor = 0, QWidget* parent = 0 );

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;

  public slots:
    void setFeature( const QgsFeature& feature ) override;

  private:
    QgsRelation mRelation;
    QgsRelationEditorWidget* mWidget;
};

#endif // QGSRELATIONWIDGETWRAPPER_H
