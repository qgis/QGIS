/****************************************************************************
**
** Copyright (C) 2013 Jiří Procházka (Hobrasoft)
** Contact: http://www.hobrasoft.cz/
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file is under the terms of the GNU Lesser General Public License
** version 2.1 as published by the Free Software Foundation and appearing
** in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the
** GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QGSIMAGEDROPTEXTEDIT_H
#define QGSIMAGEDROPTEXTEDIT_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QTextEdit>
#include <memory>

#define SIP_NO_FILE

class QImage;
class QgsTemporaryCursorOverride;

/*
 * Originally ported from https://github.com/Anchakor/MRichTextEditor, courtesy of Hobrasoft.
 */

///@cond PRIVATE
class GUI_EXPORT QgsImageDropTextEdit : public QTextEdit
{
    Q_OBJECT

  public:
    QgsImageDropTextEdit( QWidget *parent = nullptr );
    ~QgsImageDropTextEdit() override;

    void dropImage( const QImage &image, const QString &format );
    void dropLink( const QUrl &url );

  protected:
    bool canInsertFromMimeData( const QMimeData *source ) const override;
    void insertFromMimeData( const QMimeData *source ) override;
    void mouseMoveEvent( QMouseEvent *e ) override;
    void mouseReleaseEvent( QMouseEvent *e ) override;

  private:
    QString mActiveAnchor;
    std::unique_ptr<QgsTemporaryCursorOverride> mCursorOverride;
};
///@endcond


#endif // QGSIMAGEDROPTEXTEDIT_H
