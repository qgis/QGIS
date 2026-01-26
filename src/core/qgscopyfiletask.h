/***************************************************************************
  qgscopyfiletask.h
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOPYFILETASK_H
#define QGSCOPYFILETASK_H

#include "qgstaskmanager.h"

/**
 * \ingroup core
 * \brief Task to copy a file on disk.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsCopyFileTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Creates a task that copy \a source file to \a destination
     */
    QgsCopyFileTask( const QString &source, const QString &destination );

    bool run() override;

    /**
     * Returns errorString if an error occurred, else returns an empty QString
     */
    const QString &errorString() const;

    /**
     * It could be different from the original one. If original destination was a directory
     * the returned destination is now the absolute file path of the copied file
     */
    const QString &destination() const;

  private:

    QString mSource;
    QString mDestination;
    QString mErrorString;
};

#endif // QGSSIMPLECOPYEXTERNALSTORAGE_H
