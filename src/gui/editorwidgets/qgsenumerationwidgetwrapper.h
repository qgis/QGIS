/***************************************************************************
    qgsenumerationwidgetwrapper.h
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

#ifndef QGSENUMERATIONWIDGETWRAPPER_H
#define QGSENUMERATIONWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QComboBox>

/** \ingroup gui
 * \class QgsEnumerationWidgetWrapper
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsEnumerationWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsEnumerationWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = nullptr, QWidget* parent = nullptr );

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
    QComboBox* mComboBox;
};

#endif // QGSENUMERATIONWIDGETWRAPPER_H
