/***************************************************************************
    qgswebviewwidgetwrapper.h
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

#ifndef QGSWEBVIEWWIDGETWRAPPER_H
#define QGSWEBVIEWWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QWebView>
#include <QLineEdit>
#include <QPushButton>

/** \ingroup gui
 * Wraps a web view widget. Will show the content available at the URL of the value in a web browser.
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsWebViewWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsWebViewWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = nullptr, QWidget* parent = nullptr );


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
    void setEnabled( bool enabled ) override;

  private slots:
    void loadUrl( const QString &url );
    void selectFileName();

  private:
    void updateConstraintWidgetStatus( bool constraintValid ) override;

    //! This label is used as a container to display the picture
    QWebView* mWebView;
    //! The line edit containing the path to the picture
    QLineEdit* mLineEdit;
    //! The button to open the file chooser dialog
    QPushButton* mButton;
};

#endif // QGSWEBVIEWWIDGETWRAPPER_H
