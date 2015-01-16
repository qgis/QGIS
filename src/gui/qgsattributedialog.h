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
#include "qgsattributeform.h"

#include <QDialog>
#include <QMenuBar>
#include <QGridLayout>

class QgsDistanceArea;
class QgsFeature;
class QgsField;
class QgsHighlight;
class QgsVectorLayer;
class QgsVectorLayerTools;

class GUI_EXPORT QgsAttributeDialog : public QDialog
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

    /**
     * @brief setHighlight
     * @param h The highlight. Ownership is taken.
     */
    void setHighlight( QgsHighlight *h );

    /**
     * @brief Returns reference to self. Only here for legacy compliance
     *
     * @return this
     *
     * @deprecated Do not use. Just use this object itself. Or QgsAttributeForm if you want to embed.
     */
    Q_DECL_DEPRECATED QDialog *dialog() { return this; }

    QgsAttributeForm* attributeForm() { return mAttributeForm; }

    const QgsFeature* feature() { return &mAttributeForm->feature(); }

    /**
     * Is this dialog editable?
     *
     * @return returns true, if this dialog was created in an editable manner.
     */
    bool editable() { return mAttributeForm->editable(); }

    /**
     * Toggles the form mode between edit feature and add feature.
     * If set to true, the dialog will be editable even with an invalid feature.
     * If set to true, the dialog will add a new feature when the form is accepted.
     *
     * @param isAddDialog If set to true, turn this dialog into an add feature dialog.
     */
    void setIsAddDialog( bool isAddDialog ) { mAttributeForm->setIsAddDialog( isAddDialog ); }

    /**
     * Sets the edit command message (Undo) that will be used when the dialog is accepted
     *
     * @param message The message
     */
    void setEditCommandMessage( const QString& message ) { mAttributeForm->setEditCommandMessage( message ); }

  public slots:
    void accept() override;

    //! Show the dialog non-blocking. Reparents this dialog to be a child of the dialog form and is deleted when
    //! closed.
    void show( bool autoDelete = true );

  private:
    void init( QgsVectorLayer* layer, QgsFeature* feature, QgsAttributeEditorContext& context, QWidget* parent );

    QString mSettingsPath;
    // Used to sync multiple widgets for the same field
    QgsHighlight *mHighlight;
    int mFormNr;
    bool mShowDialogButtons;
    QString mReturnvarname;
    QgsAttributeForm* mAttributeForm;
    QMenuBar* mMenuBar;
    QgsFeature *mOwnedFeature;

    // true if this dialog is editable
    bool mEditable;

    static int sFormCounter;
    static QString sSettingsPath;
};

#endif
