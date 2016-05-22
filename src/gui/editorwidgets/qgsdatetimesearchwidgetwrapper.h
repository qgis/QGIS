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
#include "qgsdatetimeeditwrapper.h"

#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>

class QgsDateTimeEditFactory;

/**
 * Wraps a date time search widget.
 */
class GUI_EXPORT QgsDateTimeSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT

  public:
    explicit QgsDateTimeSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent = nullptr );
    bool applyDirectly() override;
    QString expression() override;
    bool valid() const override;
    QVariant value() const;
    FilterFlags supportedFlags() const override;
    FilterFlags defaultFlags() const override;
    virtual QString createExpression( FilterFlags flags ) const override;

  public slots:

    virtual void clearWidget() override;
    virtual void setEnabled( bool enabled ) override;

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;

  protected slots:
    void setExpression( QString exp ) override;

  private slots:
    void dateTimeChanged( const QDateTime &date );

  private:
    QgsDateTimeEdit* mDateTimeEdit;
    QgsVectorLayer* mLayer;

    friend class QgsDateTimeEditFactory;
};

#endif // QGSDATETIMESEARCHWIDGETWRAPPER_H
