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

/** \ingroup gui
 * \class QgsCheckboxSearchWidgetWrapper
 * Wraps a checkbox edit widget for searching.
 * \note Added in version 2.16
 */

class GUI_EXPORT QgsCheckboxSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT

  public:

    /** Constructor for QgsCheckboxSearchWidgetWrapper.
     * @param vl associated vector layer
     * @param fieldIdx index of associated field
     * @param parent parent widget
     */
    explicit QgsCheckboxSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* parent = nullptr );

    /** Returns a variant representing the current state of the widget.
     * @note this will not be a boolean true or false value, it will instead
     * be the values configured to represent checked and unchecked states in
     * the editor widget configuration.
     */
    QVariant value() const;

    bool applyDirectly() override;
    QString expression() override;
    bool valid() const override;
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
