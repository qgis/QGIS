/***************************************************************************
  qgsexternalresourcewidget.h

 ---------------------
 begin                : 16.12.2015
 copyright            : (C) 2015 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTERNALRESOURCEWIDGET_H
#define QGSEXTERNALRESOURCEWIDGET_H

class QWebView;

class QVariant;
class QgsPixmapLabel;

#include <QWidget>

#include "qgsfilepickerwidget.h"




/** \ingroup gui
 * Widget to display file path with a push button for an "open file" dialog
 * It can also be used to display a picture.
 **/
class GUI_EXPORT QgsExternalResourceWidget : public QWidget
{

    Q_OBJECT
    Q_PROPERTY( bool filePickerVisible READ filePickerVisible WRITE setFilePickerVisible )
    Q_PROPERTY( DocumentViewerContent documentViewerContent READ documentViewerContent WRITE setDocumentViewerContent )
    Q_PROPERTY( int documentViewerHeight READ documentViewerHeight WRITE setDocumentViewerHeight )
    Q_PROPERTY( int documentViewerWidth READ documentViewerWidth WRITE setDocumentViewerWidth )

  public:
    enum DocumentViewerContent
    {
      NoContent,
      Image,
      Web
    };

    explicit QgsExternalResourceWidget( QWidget *parent = 0 );

    //! returns the value of the widget
    QVariant documentPath();
    void setDocumentPath( QVariant documentPath );

    //! access the file picker widget to allow its configuration
    QgsFilePickerWidget* filePickerwidget();

    //! returns if the file picker is visible in the widget
    bool filePickerVisible() const;
    //! set the visiblity of the file picker in the widget
    void setFilePickerVisible( bool visible );

    //! returns the type of content used in the document viewer
    QgsExternalResourceWidget::DocumentViewerContent documentViewerContent() const;
    //! setDocumentViewerContent defines the type of content to be shown. Widget will be adapated accordingly
    void setDocumentViewerContent( QgsExternalResourceWidget::DocumentViewerContent content );

    //! set the configuration of the document viewer
    int documentViewerHeight() const;
    void setDocumentViewerHeight( int height );
    int documentViewerWidth() const ;
    void setDocumentViewerWidth( int width );

    //! defines if the widget is readonly
    void setReadOnly( bool readOnly );

  signals:
    void valueChanged( QString );

  private slots:
    void loadDocument( QString path );

  private:
    void updateDocumentViewer();

    //! properties
    bool mFilePickerVisible;
    DocumentViewerContent mDocumentViewerContent;
    int mDocumentViewerHeight;
    int mDocumentViewerWidth;

    //! UI objects
    QgsFilePickerWidget* mFilePicker;
    QgsPixmapLabel* mPixmapLabel;
#ifdef WITH_QTWEBKIT
    //! This webview is used as a container to display the picture
    QWebView* mWebView;
#endif

};

#endif // QGSEXTERNALRESOURCEWIDGET_H
