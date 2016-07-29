/***************************************************************************
    qgscheckboxwidgetwrapper.h
     --------------------------------------
    Date                 : 5.1.2014
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

#ifndef QGSCHECKBOXWIDGETWRAPPER_H
#define QGSCHECKBOXWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QCheckBox>
#include <QGroupBox>

/** \ingroup gui
 * Wraps a checkbox widget. This will offer a checkbox to represent boolean values.
 *
 * Options:
 * <ul>
 * <li><b>CheckedState</b> <i>The value used to represent "True" in the data.</i></li>
 * <li><b>UncheckedState</b> <i>The value used to represent "False" in the data.</i></li>
 * </ul>
 *
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsCheckboxWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsCheckboxWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = nullptr, QWidget* parent = nullptr );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() const override;

    void showIndeterminateState() override;

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;
    bool valid() const override;

  public slots:
    void setValue( const QVariant& value ) override;

  private:
    QCheckBox* mCheckBox;
    QGroupBox* mGroupBox;
};

#endif // QGSCHECKBOXWIDGETWRAPPER_H
