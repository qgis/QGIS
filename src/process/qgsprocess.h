/***************************************************************************
                          qgsprocess.h
                          -------------------
    begin                : February 2019
    copyright            : (C) 2019 Nyall Dawson
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
#ifndef QGSPROCESS_H
#define QGSPROCESS_H

#include "qgsprocessingfeedback.h"
#include "qgspythonrunner.h"
#include "qgspythonutils.h"
#include <QElapsedTimer>

class QgsApplication;

class ConsoleFeedback : public QgsProcessingFeedback
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingAlgorithmDialogFeedback.
     */
    ConsoleFeedback();

  public slots:

    void setProgressText( const QString &text ) override;
    void reportError( const QString &error, bool fatalError ) override;
    void pushInfo( const QString &info ) override;
    void pushCommandInfo( const QString &info ) override;
    void pushDebugInfo( const QString &info ) override;
    void pushConsoleInfo( const QString &info ) override;

  private slots:
    void showTerminalProgress( double progress );

  private:
    QElapsedTimer mTimer;
    int mLastTick = -1;
};


class QgsProcessingExec
{

  public:

    QgsProcessingExec();
    int run( const QStringList &args );

  private:

    void showUsage( const QString &appName );
    void loadPlugins();
    void listAlgorithms();
    void listPlugins();
    int showAlgorithmHelp( const QString &id );
    int execute( const QString &algId, const QVariantMap &parameters );

    std::unique_ptr< QgsPythonUtils > mPythonUtils;
    std::unique_ptr<QgsPythonUtils> loadPythonSupport();
};

#endif // QGSPROCESS_H

