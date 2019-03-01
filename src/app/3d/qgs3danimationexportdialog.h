/***************************************************************************
  qgs3danimationexportdialog.h
  ----------------------------
  Date                 : February 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DANIMATIONEXPORTDIALOG_H
#define QGS3DANIMATIONEXPORTDIALOG_H

#include <QWidget>
#include <memory>
#include <QSize>

#include "qgs3dmapsettings.h"
#include "qgs3danimationsettings.h"

#include "ui_animationexport3ddialog.h"

/**
 * Dialog for settings for 3D animation export
 */
class Qgs3DAnimationExportDialog : public QDialog, private Ui::AnimationExport3DDialog
{
    Q_OBJECT
  public:
    explicit Qgs3DAnimationExportDialog();
    ~Qgs3DAnimationExportDialog() override;

    //! Returns output directory for frames
    QString outputDirectory( ) const;

    //! Returns filename template for frames
    QString fileNameExpression( ) const;

    //! Returns frames per second
    int fps() const;

    //! Returns size of frame in pixels
    QSize frameSize() const;
};

#endif // QGS3DANIMATIONEXPORTDIALOG_H
