/***************************************************************************
    qgstexteditwrapper.h
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

#ifndef QGSTEXTEDITWRAPPER_H
#define QGSTEXTEDITWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>

/**
 * Wraps a text widget. Users will be able to modify text with this widget type.
 *
 * Options:
 * <ul>
 * <li><b>IsMultiline</b> <i>If set to True, a multiline widget will be used.</i></li>
 * <li><b>UseHtml</b> <i>Will represent the content as HTML. Only available for multiline widgets.</i></li>
 * </ul>
 *
 */

class GUI_EXPORT QgsTextEditWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsTextEditWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = 0, QWidget* parent = 0 );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value();

  protected:
    QWidget*createWidget( QWidget* parent );
    void initWidget( QWidget* editor );

  public slots:
    void setValue( const QVariant& value );
    void setEnabled( bool enabled );

  private:
    QTextEdit* mTextEdit;
    QPlainTextEdit* mPlainTextEdit;
    QLineEdit* mLineEdit;
    QPalette mReadOnlyPalette;
    QPalette mWritablePalette;
};

#endif // QGSTEXTEDITWRAPPER_H
