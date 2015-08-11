/***************************************************************************
    qgsuuidwidgetwrapper.h
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

#ifndef QGSUUIDWIDGETWRAPPER_H
#define QGSUUIDWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QLineEdit>
#include <QLabel>


/**
 * Wraps a uuid widget. Will create a new UUID if empty or represent the current value if not empty.
 *
 */


class GUI_EXPORT QgsUuidWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsUuidWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = 0, QWidget* parent = 0 );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() override;

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;
    bool valid() override;

  public slots:
    void setValue( const QVariant& value ) override;
    void setEnabled( bool enabled ) override;

  private:
    QLabel* mLabel;
    QLineEdit* mLineEdit;
};

#endif // QGSUUIDWIDGETWRAPPER_H
