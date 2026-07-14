/***************************************************************************
                          qgshtmltopdf.h
                          -------------------
    begin                : July 2026
    copyright            : (C) 2026 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSHTMLTOPDF_H
#define QGSHTMLTOPDF_H

#include <QObject>
#include <QString>
#include <QWebEnginePage>

class QgsLoggingWebEnginePage : public QWebEnginePage
{
  protected:
    void javaScriptConsoleMessage( JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID ) override;
};

class QgsHtmlToPdf : public QObject
{
    Q_OBJECT

  public:
    QgsHtmlToPdf();

    void run( const QString &url, const QString &pdfFileName );

  signals:
    void finished( int statusCode );

  private:
    // TO determine -- is this ALWAYS 96?
    static constexpr double RENDER_TO_PDF_DPI = 96;

    QString mPdfFileName;
    QgsLoggingWebEnginePage mPage;

    void onLoadFinished( bool ok );
    void onJsFinished( const QVariant &res );
    void onPdfFinished( const QString &filePath, bool ok );
};

#endif // QGSHTMLTOPDF_H
