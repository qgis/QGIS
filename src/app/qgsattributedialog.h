/***************************************************************************
                         qgsattributedialog.h  -  description
                             -------------------
    begin                : October 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEDIALOG_H
#define QGSATTRIBUTEDIALOG_H

#include "qgsfeature.h"

class QDialog;
class QgsFeature;
class QLayout;
class QgsField;
class QgsVectorLayer;
class QgsHighlight;

class QgsAttributeDialog : public QObject
{
    Q_OBJECT

  public:
    QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner );
    ~QgsAttributeDialog();

    /** Saves the size and position for the next time
     *  this dialog box was used.
     */

    void saveGeometry();

    /** Restores the size and position from the last time
     *  this dialog box was used.
     */
    void restoreGeometry();

    void setHighlight( QgsHighlight *h );

    QDialog *dialog() { return mDialog; }

  public slots:
    void accept();

    int exec();
    void show();

    void dialogDestroyed();

  private:
    bool eventFilter( QObject *obj, QEvent *event );

    QDialog *mDialog;
    QString mSettingsPath;
    QList<QWidget *> mpWidgets;
    QList<int> mpIndizes;
    QgsVectorLayer *mLayer;
    QgsFeature *mFeature;
    bool mFeatureOwner;
    QgsHighlight *mHighlight;
    int mFormNr;
    static int smFormCounter;
};

#endif
