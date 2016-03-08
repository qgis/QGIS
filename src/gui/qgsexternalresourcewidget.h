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
class QgsPixmapLabel;

#include <QWidget>
#include <QVariant>

#include "qgsfilewidget.h"




/** \ingroup gui
 * Widget to display file path with a push button for an "open file" dialog
 * It can also be used to display a picture or a web page.
 **/
class GUI_EXPORT QgsExternalResourceWidget : public QWidget
{

    Q_OBJECT
    Q_PROPERTY( bool fileWidgetVisible READ fileWidgetVisible WRITE setFileWidgetVisible )
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

    /**
     * @brief QgsExternalResourceWidget creates a widget with a file widget and a document viewer
     * Both part of the widget are optional.
     * @see QgsFileWidget
     */
    explicit QgsExternalResourceWidget( QWidget *parent = 0 );

    /**
     * @brief documentPath returns the path of the current document in the widget
     * @param type determines the type of the returned null variant if the document is not defined yet
     */
    QVariant documentPath( QVariant::Type type = QVariant::String ) const;
    void setDocumentPath( const QVariant& documentPath );

    //! access the file widget to allow its configuration
    QgsFileWidget* fileWidget();

    //! returns if the file widget is visible in the widget
    bool fileWidgetVisible() const;
    //! set the visiblity of the file widget in the layout
    void setFileWidgetVisible( bool visible );

    //! returns the type of content used in the document viewer
    QgsExternalResourceWidget::DocumentViewerContent documentViewerContent() const;
    //! setDocumentViewerContent defines the type of content to be shown. Widget will be adapated accordingly
    void setDocumentViewerContent( QgsExternalResourceWidget::DocumentViewerContent content );

    //! returns the height of the document viewer
    int documentViewerHeight() const;
    /**
     * @brief setDocumentViewerWidth set the height of the document viewer.
     * @param height the height. Use 0 for automatic best display.
     */
    void setDocumentViewerHeight( int height );
    //! returns the width of the document viewer
    int documentViewerWidth() const ;
    /**
     * @brief setDocumentViewerWidth set the width of the document viewer.
     * @param width the width. Use 0 for automatic best display.
     */
    void setDocumentViewerWidth( int width );

    //! defines if the widget is readonly
    void setReadOnly( bool readOnly );

  signals:
    //! emitteed as soon as the current document changes
    void valueChanged( const QString& );

  private slots:
    void loadDocument( const QString& path );

  private:
    void updateDocumentViewer();

    //! properties
    bool mFileWidgetVisible;
    DocumentViewerContent mDocumentViewerContent;
    int mDocumentViewerHeight;
    int mDocumentViewerWidth;

    //! UI objects
    QgsFileWidget* mFileWidget;
    QgsPixmapLabel* mPixmapLabel;
#ifdef WITH_QTWEBKIT
    //! This webview is used as a container to display the picture
    QWebView* mWebView;
#endif

};

#endif // QGSEXTERNALRESOURCEWIDGET_H
