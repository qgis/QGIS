/***************************************************************************
  qgspdalindexingtask.h
  ------------------------
  Date                 : December 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPDALINDEXINGTASK_H
#define QGSPDALINDEXINGTASK_H

#include <QObject>
#include "qgstaskmanager.h"

class QgsPdalIndexingTask: public QgsTask
{
    Q_OBJECT

  public:
    enum class OutputFormat
    {
      Ept,
      Copc
    };

    QgsPdalIndexingTask( const QString &file, const QString &outputPath, OutputFormat outputFormat = OutputFormat::Ept, const QString &name = QString() );
    bool run() override;

    QString untwineExecutableBinary() const;
    void setUntwineExecutableBinary( const QString &untwineExecutableBinary );

    QString outputPath() const;
    const QString errorMessage() const { return mErrorMessage; };

  private:
    bool prepareOutputPath();
    bool runUntwine();
    void cleanTemp();

    QString guessUntwineExecutableBinary() const;
    QString mUntwineExecutableBinary;
    QString mOutputPath;
    QString mFile;
    OutputFormat mOutputFormat = OutputFormat::Copc;
    QString mErrorMessage;
};

#endif // QGSPDALINDEXINGTASK_H
