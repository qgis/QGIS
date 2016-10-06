/***************************************************************************
    qgstexteditwrapper.h
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

#ifndef QGSTEXTEDITWRAPPER_H
#define QGSTEXTEDITWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>

/** \ingroup gui
 * Wraps a text widget. Users will be able to modify text with this widget type.
 *
 * Options:
 * <ul>
 * <li><b>IsMultiline</b> <i>If set to True, a multiline widget will be used.</i></li>
 * <li><b>UseHtml</b> <i>Will represent the content as HTML. Only available for multiline widgets.</i></li>
 * </ul>
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsTextEditWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsTextEditWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = nullptr, QWidget* parent = nullptr );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() const override;
    void showIndeterminateState() override;

  protected:
    QWidget*createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;
    bool valid() const override;

  public slots:
    void setValue( const QVariant& value ) override;
    void setEnabled( bool enabled ) override;

  private slots:
    void textChanged( const QString& text );

  private:
    QTextEdit* mTextEdit;
    QPlainTextEdit* mPlainTextEdit;
    QLineEdit* mLineEdit;
    QPalette mReadOnlyPalette;
    QPalette mWritablePalette;
    QString mPlaceholderText;

    void setWidgetValue( const QVariant& value );
};

#endif // QGSTEXTEDITWRAPPER_H
