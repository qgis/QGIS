/***************************************************************************
    qgscheckboxsearchwidgetwrapper.h
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

#ifndef QGSCHECKBOXSEARCHWIDGETWRAPPER_H
#define QGSCHECKBOXSEARCHWIDGETWRAPPER_H

#include "qgssearchwidgetwrapper.h"
#include "qgscheckboxwidgetwrapper.h"

#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>

class QgsCheckboxWidgetFactory;

/**
 * Wraps a checkbox search widget.
 */
class GUI_EXPORT QgsCheckboxSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT

  public:
    explicit QgsCheckboxSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent = nullptr );
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
    void stateChanged( int state );

  private:
    QCheckBox* mCheckBox;
    QgsVectorLayer* mLayer;

    friend class QgsCheckboxWidgetFactory;
};

#endif // QGSCHECKBOXSEARCHWIDGETWRAPPER_H
