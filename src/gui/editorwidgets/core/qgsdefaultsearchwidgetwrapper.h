/***************************************************************************
    qgsdefaultsearchwidgetwrapper.h
     --------------------------------------
    Date                 : 21.5.2015
    Copyright            : (C) 2015 Karolina Alexiou
    Email                : carolinegr at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDEFAULTSEARCHWIDGETWRAPPER_H
#define QGSDEFAULTSEARCHWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"
#include <qgsfilterlineedit.h>


/**
 * Wraps a search widget. Default form is just a QgsLineFilterEdit
 *
 */

class GUI_EXPORT QgsDefaultSearchWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsDefaultSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = 0, QWidget* parent = 0 );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() override;

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;

  public slots:
    void setValue( const QVariant& value ) override;
    void setEnabled( bool enabled ) override;

  private:
    QgsFilterLineEdit* mLineEdit;
};

#endif // QGSDEFAULTSEARCHWIDGETWRAPPER_H
