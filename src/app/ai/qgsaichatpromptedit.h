/***************************************************************************
    qgsaichatpromptedit.h
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAICHATPROMPTEDIT_H
#define QGSAICHATPROMPTEDIT_H

#include "qgis_app.h"

#include <QTextEdit>

class APP_EXPORT QgsAiChatPromptEdit : public QTextEdit
{
    Q_OBJECT

  public:
    explicit QgsAiChatPromptEdit( QWidget *parent = nullptr );

  signals:
    void filesDropped( const QStringList &paths );

  protected:
    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;
    bool canInsertFromMimeData( const QMimeData *source ) const override;
    void insertFromMimeData( const QMimeData *source ) override;

  private:
    QStringList extractLocalFilePaths( const QMimeData *source ) const;
    void handleFilePaths( const QStringList &paths );

    friend class TestQgsAiChatDockWidget;
};

#endif // QGSAICHATPROMPTEDIT_H
