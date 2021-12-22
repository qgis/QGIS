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

#ifdef WITH_BINDINGS
#include "qgspythonrunner.h"
#include "qgspythonutils.h"
#endif

#include "qgsprocessingfeedback.h"
#include "qgsunittypes.h"
#include "qgsprocessingcontext.h"
#include <QElapsedTimer>

class QgsApplication;

class QgsProcessingAlgorithm;

class ConsoleFeedback : public QgsProcessingFeedback
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingAlgorithmDialogFeedback.
     */
    ConsoleFeedback( bool useJson );

  public slots:

    void setProgressText( const QString &text ) override;
    void reportError( const QString &error, bool fatalError ) override;
    void pushWarning( const QString &warning ) override;
    void pushInfo( const QString &info ) override;
    void pushCommandInfo( const QString &info ) override;
    void pushDebugInfo( const QString &info ) override;
    void pushConsoleInfo( const QString &info ) override;
    QVariantMap jsonLog() const;

  private slots:
    void showTerminalProgress( double progress );

  private:
    QElapsedTimer mTimer;
    int mLastTick = -1;
    bool mUseJson = false;
    QVariantMap mJsonLog;
};


class QgsProcessingExec
{

  public:

    QgsProcessingExec();
    int run( const QStringList &args );

  private:

    void showUsage( const QString &appName );
    void loadPlugins();
    void listAlgorithms( bool useJson );
    void listPlugins( bool useJson, bool showLoaded );
    int enablePlugin( const QString &name, bool enabled );
    int showAlgorithmHelp( const QString &id, bool useJson );
    int execute( const QString &algId,
                 const QVariantMap &parameters,
                 const QString &ellipsoid,
                 QgsUnitTypes::DistanceUnit distanceUnit,
                 QgsUnitTypes::AreaUnit areaUnit,
                 QgsProcessingContext::LogLevel logLevel,
                 bool useJson,
                 const QString &projectPath = QString() );

    void addVersionInformation( QVariantMap &json );
    void addAlgorithmInformation( QVariantMap &json, const QgsProcessingAlgorithm *algorithm );
    void addProviderInformation( QVariantMap &json, QgsProcessingProvider *provider );


    bool mSkipPython = false;
#ifdef WITH_BINDINGS
    std::unique_ptr< QgsPythonUtils > mPythonUtils;
    std::unique_ptr<QgsPythonUtils> loadPythonSupport();
#endif
};

#endif // QGSPROCESS_H

