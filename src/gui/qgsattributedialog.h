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
#include "qgsattributeeditorcontext.h"

class QDialog;
class QLayout;

class QgsDistanceArea;
class QgsFeature;
class QgsField;
class QgsHighlight;
class QgsVectorLayer;
class QgsVectorLayerTools;

class GUI_EXPORT QgsAttributeDialog : public QObject
{
    Q_OBJECT

  public:
    /**
     * Create an attribute dialog for a given layer and feature
     *
     * @param vl                The layer for which the dialog will be generated
     * @param thepFeature       A feature for which the dialog will be generated
     * @param featureOwner      Set to true, if the dialog should take ownership of the feature
     * @param myDa              A QgsDistanceArea which will be used for expressions
     * @param parent            A parent widget for the dialog
     * @param showDialogButtons True: Show the dialog buttons accept/cancel
     *
     * @deprecated
     */
    QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QgsDistanceArea myDa, QWidget* parent = 0, bool showDialogButtons = true );

    /**
     * Create an attribute dialog for a given layer and feature
     *
     * @param vl                The layer for which the dialog will be generated
     * @param thepFeature       A feature for which the dialog will be generated
     * @param featureOwner      Set to true, if the dialog should take ownership of the feature
     * @param parent            A parent widget for the dialog
     * @param showDialogButtons True: Show the dialog buttons accept/cancel
     * @param context           The context in which this dialog is created
     *
     */
    QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QWidget* parent = 0, bool showDialogButtons = true, QgsAttributeEditorContext context = QgsAttributeEditorContext() );

    ~QgsAttributeDialog();

    /** Saves the size and position for the next time
     *  this dialog box will be used.
     */
    void saveGeometry();

    /** Restores the size and position from the last time
     *  this dialog box was used.
     */
    void restoreGeometry();

    void setHighlight( QgsHighlight *h );

    QDialog *dialog() { return mDialog; }

    QgsFeature* feature() { return mFeature; }

    /**
     * Is this dialog editable?
     *
     * @return returns true, if this dialog was created in an editable manner.
     */
    bool editable() { return mEditable; }

  public slots:
    void accept();

    int exec();
    void show();

    void dialogDestroyed();

  protected:
    bool eventFilter( QObject *obj, QEvent *event );

  private:
    void init();

    QDialog *mDialog;
    QString mSettingsPath;
    // Used to sync multiple widgets for the same field
    QgsAttributeEditorContext mContext;
    QgsVectorLayer *mLayer;
    QgsFeature* mFeature;
    bool mFeatureOwner;
    QgsHighlight *mHighlight;
    int mFormNr;
    bool mShowDialogButtons;
    QString mReturnvarname;

    // true if this dialog is editable
    bool mEditable;

    static int sFormCounter;
    static QString sSettingsPath;
};

#endif
