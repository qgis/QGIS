/***************************************************************************
 qgs3dmodelsourcelineedit.h
 ---------------------
 begin                : July 2020
 copyright            : (C) 2020 by Mathieu Pellerin
 email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMODELSOURCELINEEDIT_H
#define QGS3DMODELSOURCELINEEDIT_H

#include "qgsfilecontentsourcelineedit.h"

#include <QString>

/**
 * \class Qgs3DModelSourceLineEdit
 * A line edit widget with toolbutton for setting a 3D model source path.
 *
 * Designed for use with QgsSourceCache.
 *
 * \since QGIS 3.16
 */
class Qgs3DModelSourceLineEdit : public QgsAbstractFileContentSourceLineEdit
{
    Q_OBJECT
  public:
    /**
     * Constructor for Qgs3DModelSourceLineEdit, with the specified \a parent widget.
     */
    Qgs3DModelSourceLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr )
      : QgsAbstractFileContentSourceLineEdit( parent )
    {}

  private:
#ifndef SIP_RUN
    ///@cond PRIVATE
    QString fileFilter() const override;
    QString selectFileTitle() const override;
    QString fileFromUrlTitle() const override;
    QString fileFromUrlText() const override;
    QString embedFileTitle() const override;
    QString extractFileTitle() const override;
    QString defaultSettingsKey() const override;
///@endcond
#endif
};

#endif // QGS3DMODELSOURCELINEEDIT_H
