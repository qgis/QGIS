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
class QgsDistanceArea;

class GUI_EXPORT QgsAttributeDialog : public QObject
{
    Q_OBJECT

  public:
    QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QgsDistanceArea myDa, QWidget* parent = 0, bool showDialogButtons = true );
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

    QgsFeature* feature() { return mFeature; }

  public slots:
    void accept();

    int exec();
    void show();

    void dialogDestroyed();

  protected:
    bool eventFilter( QObject *obj, QEvent *event );

  private:
    QDialog *mDialog;
    QString mSettingsPath;
    // Used to sync multiple widgets for the same field
    QMap<int, QWidget*> mProxyWidgets;
    QgsVectorLayer *mLayer;
    QgsFeature *mFeature;
    bool mFeatureOwner;
    QgsHighlight *mHighlight;
    int mFormNr;
    static int smFormCounter;
    bool mShowDialogButtons;
    QString mReturnvarname;
};

#endif
