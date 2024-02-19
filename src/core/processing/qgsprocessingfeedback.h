/***************************************************************************
                         qgsprocessingfeedback.h
                         -----------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#ifndef QGSPROCESSINGFEEDBACK_H
#define QGSPROCESSINGFEEDBACK_H

#include "qgis_core.h"
#include "qgsfeedback.h"

class QgsProcessingProvider;
class QgsProcessingAlgorithm;
class QgsProcessingContext;

/**
 * \class QgsProcessingFeedback
 * \ingroup core
 * \brief Base class for providing feedback from a processing algorithm.
 *
 * This base class implementation silently ignores all feedback reported by algorithms.
 * Subclasses of QgsProcessingFeedback can be used to log this feedback or report
 * it to users via the GUI.
 */
class CORE_EXPORT QgsProcessingFeedback : public QgsFeedback
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingFeedback.
     *
     * If \a logFeedback is TRUE, then all feedback received will be directed
     * to QgsMessageLog.
     */
    QgsProcessingFeedback( bool logFeedback = true );

    /**
     * Sets a progress report text string. This can be used in conjunction with
     * setProgress() to provide detailed progress reports, such as "Transformed
     * 4 of 5 layers".
     * \see setProgress()
     */
    virtual void setProgressText( const QString &text );

    /**
     * Reports that the algorithm encountered an \a error while executing.
     *
     * If \a fatalError is TRUE then the error prevented the algorithm from executing.
     */
    virtual void reportError( const QString &error, bool fatalError = false );

    /**
     * Pushes a warning informational message from the algorithm. This
     * should only be used sparsely as to maintain the importance of visual
     * queues associated to this type of message.
     * \see pushInfo()
     * \see pushCommandInfo()
     * \see pushDebugInfo()
     * \see pushConsoleInfo()
     * \since QGIS 3.16.2
     */
    virtual void pushWarning( const QString &warning );

    /**
     * Pushes a general informational message from the algorithm. This can
     * be used to report feedback which is neither a status report or an
     * error, such as "Found 47 matching features".
     *
     * \see pushFormattedMessage()
     * \see pushWarning()
     * \see pushCommandInfo()
     * \see pushDebugInfo()
     * \see pushConsoleInfo()
     */
    virtual void pushInfo( const QString &info );

    /**
     * Pushes a pre-formatted message from the algorithm.
     *
     * This can be used to push formatted HTML messages to the feedback object.
     * A plain \a text version of the message must also be specified.
     *
     * \see pushInfo()
     * \see pushWarning()
     * \see pushCommandInfo()
     * \see pushDebugInfo()
     * \see pushConsoleInfo()
     *
     * \since QGIS 3.36
     */
    virtual void pushFormattedMessage( const QString &html, const QString &text );

    /**
     * Pushes an informational message containing a command from the algorithm.
     * This is usually used to report commands which are executed in an external
     * application or as subprocesses.
     * \see pushWarning()
     * \see pushInfo()
     * \see pushDebugInfo()
     * \see pushConsoleInfo()
     */
    virtual void pushCommandInfo( const QString &info );

    /**
     * Pushes an informational message containing debugging helpers from
     * the algorithm.
     * \see pushWarning()
     * \see pushInfo()
     * \see pushCommandInfo()
     * \see pushConsoleInfo()
     */
    virtual void pushDebugInfo( const QString &info );

    /**
     * Pushes a console feedback message from the algorithm. This is used to
     * report the output from executing an external command or subprocess.
     * \see pushWarning()
     * \see pushInfo()
     * \see pushDebugInfo()
     * \see pushCommandInfo()
     */
    virtual void pushConsoleInfo( const QString &info );

    /**
     * Pushes a summary of the QGIS (and underlying library) version information to the log.
     * \since QGIS 3.4.7
     */
    void pushVersionInfo( const QgsProcessingProvider *provider = nullptr );

    /**
     * Pushes a summary of the execution \a results to the log
     *
     * \since QGIS 3.36
     */
    void pushFormattedResults( const QgsProcessingAlgorithm *algorithm, QgsProcessingContext &context, const QVariantMap &results );

    /**
     * Returns the HTML formatted contents of the log, which contains all messages pushed to the feedback object.
     *
     * \see textLog()
     * \since QGIS 3.14
     */
    virtual QString htmlLog() const;

    /**
     * Returns the plain text contents of the log, which contains all messages pushed to the feedback object.
     *
     * \see htmlLog()
     * \since QGIS 3.14
     */
    virtual QString textLog() const;

  private:

    void log( const QString &htmlMessage, const QString &textMessage );

    bool mLogFeedback = true;
    QString mHtmlLog;
    QString mTextLog;
    int mMessageLoggedCount = 0;

};


/**
 * \class QgsProcessingMultiStepFeedback
 * \ingroup core
 *
 * \brief Processing feedback object for multi-step operations.
 *
 * A processing feedback object which proxies its calls to an underlying
 * feedback object, but scales overall progress reports to account
 * for a number of child steps which each report their own feedback.
 *
 */
class CORE_EXPORT QgsProcessingMultiStepFeedback : public QgsProcessingFeedback
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingMultiStepFeedback, for a process with the specified
     * number of \a steps. This feedback object will proxy calls
     * to the specified \a feedback object.
     */
    QgsProcessingMultiStepFeedback( int steps, QgsProcessingFeedback *feedback );

    /**
     * Sets the \a step which is being executed. This is used
     * to scale the current progress to account for progress through the overall process.
     */
    void setCurrentStep( int step );

    void setProgressText( const QString &text ) override;
    void reportError( const QString &error, bool fatalError = false ) override;
    void pushWarning( const QString &warning ) override;
    void pushInfo( const QString &info ) override;
    void pushCommandInfo( const QString &info ) override;
    void pushDebugInfo( const QString &info ) override;
    void pushConsoleInfo( const QString &info ) override;
    void pushFormattedMessage( const QString &html, const QString &text ) override;

    QString htmlLog() const override;
    QString textLog() const override;
  private slots:

    void updateOverallProgress( double progress );

  private:

    int mChildSteps = 0;
    int mCurrentStep = 0;
    QgsProcessingFeedback *mFeedback = nullptr;

};

#endif // QGSPROCESSINGFEEDBACK_H


