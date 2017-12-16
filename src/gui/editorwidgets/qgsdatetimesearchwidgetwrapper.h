/***************************************************************************
    qgsdatetimesearchwidgetwrapper.h
     -------------------------------
    Date                 : 2016-05-23
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATETIMESEARCHWIDGETWRAPPER_H
#define QGSDATETIMESEARCHWIDGETWRAPPER_H

#include "qgssearchwidgetwrapper.h"
#include "qgis.h"

#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>
#include "qgis_gui.h"

class QgsDateTimeEditFactory;
class QgsDateTimeEdit;

/**
 * \ingroup gui
 * \class QgsDateTimeSearchWidgetWrapper
 * Wraps a date/time edit widget for searching.
 * \since QGIS 2.16
 */

class GUI_EXPORT QgsDateTimeSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsDateTimeSearchWidgetWrapper.
     * \param vl associated vector layer
     * \param fieldIdx index of associated field
     * \param parent parent widget
     */
    explicit QgsDateTimeSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a variant representing the current state of the widget, respecting
     * the editor widget's configured field format for date/time values.
     */
    QVariant value() const;

    bool applyDirectly() override;
    QString expression() const override;
    bool valid() const override;
    QgsSearchWidgetWrapper::FilterFlags supportedFlags() const override;
    QgsSearchWidgetWrapper::FilterFlags defaultFlags() const override;
    QString createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const override;

  public slots:

    void clearWidget() override;
    void setEnabled( bool enabled ) override;

  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;

  protected slots:
    void setExpression( const QString &exp ) override;

  private slots:
    void dateTimeChanged( const QDateTime &date );

  private:
    QgsDateTimeEdit *mDateTimeEdit = nullptr;
    QgsVectorLayer *mLayer = nullptr;

    friend class QgsDateTimeEditFactory;
};

#endif // QGSDATETIMESEARCHWIDGETWRAPPER_H
