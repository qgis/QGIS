/***************************************************************************
    qgsdatetimeeditwrapper.h
     --------------------------------------
    Date                 : 03.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATETIMEEDITWRAPPER_H
#define QGSDATETIMEEDITWRAPPER_H

#include <QDateTimeEdit>

#include "qgseditorwidgetwrapper.h"
#include "qgsdatetimeedit.h"
#include "qgsdatetimeeditfactory.h"


class GUI_EXPORT QgsDateTimeEditWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsDateTimeEditWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent = 0 );

  private slots:
    void dateTimeChanged( const QDateTime &dateTime );

  private:
    QDateTimeEdit* mQDateTimeEdit;
    QgsDateTimeEdit* mQgsDateTimeEdit;


    // QgsEditorWidgetWrapper interface
  public:
    QVariant value();
    QWidget *createWidget( QWidget *parent );
    void initWidget( QWidget *editor );

  public slots:
    void setValue( const QVariant &value );
    void setEnabled( bool enabled );
};

#endif // QGSDATETIMEEDITWRAPPER_H
