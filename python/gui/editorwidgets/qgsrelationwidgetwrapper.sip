/***************************************************************************
    qgsrelationwidgetwrapper.h
     --------------------------------------
    Date                 : 14.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


class QgsRelationWidgetWrapper : QgsWidgetWrapper
{
%TypeHeaderCode
#include <qgsrelationwidgetwrapper.h>
%End

  public:
    explicit QgsRelationWidgetWrapper( QgsVectorLayer* vl, const QgsRelation& relation, QWidget* editor = 0, QWidget* parent /TransferThis/ = 0 );

  protected:
    QWidget* createWidget( QWidget* parent );
    void initWidget( QWidget* editor );
    bool valid() const;

  public slots:
    void setFeature( const QgsFeature& feature );
};
