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

/**
 * Wraps a date time widget. Users will be able to choose date and time from an appropriate dialog.
 *
 * Options:
 * <ul>
 * <li><b>display_format</b> <i>The format used to represent the date/time to the user.</i></li>
 * <li><b>calendar_popup</b> <i>If True, will offer a calendar popup.</i></li>
 * <li><b>allow_null</b> <i>If True, will allow NULL values. Requires QgsDateTimeEdit as widget (Be aware if you work with .ui files).</i></li>
 * <li><b>field_format</b> <i>The format used to save the date/time.</i></li>
 * </ul>
 *
 */

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
    QVariant value() override;
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    bool valid() override;

  public slots:
    void setValue( const QVariant &value ) override;
    void setEnabled( bool enabled ) override;
};

#endif // QGSDATETIMEEDITWRAPPER_H
