/***************************************************************************
  qgspointlocatorinittask.h
  --------------------------------------
  Date                 : September 2019
  Copyright            : (C) 2019 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTLOCATORINITTASK_H
#define QGSPOINTLOCATORINITTASK_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE

#include "qgstaskmanager.h"

class QgsPointLocator;

class QgsPointLocatorInitTask : public QgsTask
{
    Q_OBJECT

  public:

    QgsPointLocatorInitTask( QgsPointLocator *loc );

    /**
     * Returns TRUE when the task has finished and the index build was ok
     */
    bool isBuildOK() const;

    bool run() override;

  private:

    QgsPointLocator *mLoc = nullptr;
    bool mBuildOK = false;
};

/// @endcond

#endif // QGSPOINTLOCATORINITTASK_H
