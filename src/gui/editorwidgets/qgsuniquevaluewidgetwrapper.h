/***************************************************************************
    qgsuniquevaluewidgetwrapper.h
     --------------------------------------
    Date                 : 5.1.2014
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

#ifndef QGSUNIQUEVALUEWIDGETWRAPPER_H
#define QGSUNIQUEVALUEWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QComboBox>
#include <QLineEdit>

/**
 * Wraps a unique value widget. Will offer any value previously used for this field.
 *
 * Options:
 * <ul>
 * <li><b>Editable</b> <i>If True the user can manually insert new (not yet used) values.</i></li>
 * </ul>
 *
 */

class GUI_EXPORT QgsUniqueValuesWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsUniqueValuesWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = 0, QWidget* parent = 0 );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() override;

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;
    bool valid() override;

  public slots:
    void setValue( const QVariant& value ) override;

  private:
    QComboBox* mComboBox;
    QLineEdit* mLineEdit;
};

#endif // QGSUNIQUEVALUEWIDGETWRAPPER_H
