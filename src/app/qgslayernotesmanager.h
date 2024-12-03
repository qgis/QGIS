/***************************************************************************
  qgslayernotesmanager.h
  --------------------------------------
  Date                 : April 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERNOTESMANAGER_H
#define QGSLAYERNOTESMANAGER_H

#include <QString>
#include <QDialog>

class QgsMapLayer;
class QWidget;
class QgsRichTextEditor;

class QgsLayerNotesManager
{
  public:
    /**
     * Shows a dialog allowing users to edit the notes for the specified \a layer.
     */
    static void editLayerNotes( QgsMapLayer *layer, QWidget *parent );
};

class QgsLayerNotesDialog : public QDialog
{
    Q_OBJECT

  public:
    QgsLayerNotesDialog( QWidget *parent );

    void setNotes( const QString &notes );
    QString notes() const;

  private:
    QgsRichTextEditor *mEditor = nullptr;
};

#endif // QGSLAYERNOTESMANAGER_H
