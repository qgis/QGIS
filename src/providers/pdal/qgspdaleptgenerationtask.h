/***************************************************************************
  qgspdaleptgenerationtask.h
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

#ifndef QGSPDALEPTGENERATIONTASK_H
#define QGSPDALEPTGENERATIONTASK_H

#include <QObject>
#include "qgstaskmanager.h"

class QgsPdalEptGenerationTask: public QgsTask
{
    Q_OBJECT

  public:
    QgsPdalEptGenerationTask( const QString &file, const QString &outputDir, const QString &name = QString() );
    bool run() override;

    QString untwineExecutableBinary() const;
    void setUntwineExecutableBinary( const QString &untwineExecutableBinary );

    QString outputDir() const;
    const QString errorMessage() const { return mErrorMessage; };

  private:
    bool prepareOutputDir();
    bool runUntwine();
    void cleanTemp();

    QString guessUntwineExecutableBinary() const;
    QString mUntwineExecutableBinary;
    QString mOutputDir;
    QString mFile;
    QString mErrorMessage;
};

#endif // QGSPDALEPTGENERATIONTASK_H
