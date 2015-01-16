/***************************************************************************
    qgsphotowidgetwrapper.h
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

#ifndef QGSPHOTOWIDGETWRAPPER_H
#define QGSPHOTOWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QWebView>


/**
 * Wraps a photo widget. Will show a picture and a file chooser to change the picture.
 *
 * Options:
 *
 * <ul>
 * <li><b>Width</b> <i>The width of the picture widget. If 0 and "Height" &gt; 0 will be determined automatically.</i></li>
 * <li><b>Height</b> <i>The height of the picture widget. If 0 and "Width" &gt; 0 will be determined automatically.</i></li>
 * </ul>
 *
 */

class GUI_EXPORT QgsPhotoWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsPhotoWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = 0, QWidget* parent = 0 );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() override;

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;

  public slots:
    void setValue( const QVariant& value ) override;
    void setEnabled( bool enabled ) override;

  private slots:
    void selectFileName();
    void loadPixmap( const QString& fileName );

  private:
    //! This label is used as a container to display the picture
    QLabel* mPhotoLabel;
    //! This webview is used as a container to display the picture
    QWebView* mWebView;
    //! The line edit containing the path to the picture
    QLineEdit* mLineEdit;
    //! The button to open the file chooser dialog
    QPushButton* mButton;
};

#endif // QGSPHOTOWIDGETWRAPPER_H
