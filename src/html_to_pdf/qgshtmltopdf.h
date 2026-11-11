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
    // disable interactive javascript calls, these can't be used in the headless utility
    void javaScriptAlert( const QUrl &, const QString & ) override {}
    bool javaScriptConfirm( const QUrl &, const QString & ) override { return true; }
    bool javaScriptPrompt( const QUrl &, const QString &, const QString &, QString *result ) override
    {
      *result = QString();
      return true;
    }
};

class QgsHtmlToPdf : public QObject
{
    Q_OBJECT

  public:
    enum ReturnCode
    {
      Success = 0,
      LoadFailed = 1,
      LoadJsFailed = 2,
      PrintFailed = 3,
    };
    QgsHtmlToPdf();

    void run( const QString &url, const QString &pdfFileName, int width, const QString &userAgent );

  signals:
    void finished( int statusCode );

  private:
    // TO determine -- is this ALWAYS 96?
    static constexpr double RENDER_TO_PDF_DPI = 96;

    QString mPdfFileName;
    QgsLoggingWebEnginePage mPage;
    int mWidth = 0;
    QString mUserAgent;

    ReturnCode mReturnCode = ReturnCode::Success;

    void onLoadFinished( bool ok );
    void onLoadJsFinished( const QVariant &res );
    void onPdfFinished( const QString &filePath, bool ok );
};

#endif // QGSHTMLTOPDF_H
