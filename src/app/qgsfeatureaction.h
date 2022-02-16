/***************************************************************************
                      qgsfeatureaction.h  -  description
                               ------------------
        begin                : 2010-09-20
        copyright            : (C) 2010 by Jürgen E. Fischer
        email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREACTION_H
#define QGSFEATUREACTION_H

#include "qgsfeature.h"

#include <QList>
#include <QPair>
#include <QAction>
#include <QUuid>
#include "qgis_app.h"

class QgsIdentifyResultsDialog;
class QgsVectorLayer;
class QgsHighlight;
class QgsAttributeDialog;
class QgsExpressionContextScope;

class APP_EXPORT QgsFeatureAction : public QAction
{
    Q_OBJECT

  public:
    QgsFeatureAction( const QString &name, QgsFeature &f, QgsVectorLayer *vl, QUuid actionId = QString(), int defaultAttr = -1, QObject *parent = nullptr );

  public slots:
    void execute();
    bool viewFeatureForm( QgsHighlight *h = nullptr );
    bool editFeature( bool showModal = true );

    /**
     * Add a new feature to the layer.
     * Will set the default values to recently used or provider defaults based on settings
     * and override with values in defaultAttributes if provided.
     *
     * \param defaultAttributes  Provide some default attributes here if desired.
     * \param scope              Scope of the expression
     * \param showModal          If the used dialog should be modal or not
     * \param hideParent         If the parent widget should be hidden, when the used dialog is opened
     *
     * \returns TRUE if feature was added if showModal is true. If showModal is FALSE, returns TRUE in every case
     */
    bool addFeature( const QgsAttributeMap &defaultAttributes = QgsAttributeMap(), bool showModal = true, QgsExpressionContextScope *scope = nullptr, bool hideParent = false );

    /**
     * Sets whether to force suppression of the attribute form popup after creating a new feature.
     * If \a force is TRUE, then regardless of any user settings, form settings, etc, the attribute
     * form will ALWAYS be suppressed.
     */
    void setForceSuppressFormPopup( bool force );

    /**
     * Returns the added feature or invalid feature in case addFeature() was not successful.
     */
    QgsFeature feature() const;

  signals:

    /**
     * This signal is emitted when the add feature process is finished.
     * Either during the call to addFeature() already or when the dialog is eventually
     * closed (accepted or canceled).
     *
     * \since QGIS 3.8
     */
    void addFeatureFinished();

  private slots:
    void onFeatureSaved( const QgsFeature &feature );
    void unhideParentWidget();
    void hideParentWidget();

  private:
    QgsAttributeDialog *newDialog( bool cloneFeature );

    QgsVectorLayer *mLayer = nullptr;
    QgsFeature *mFeature = nullptr;
    QUuid mActionId;
    int mIdx;

    bool mFeatureSaved;

    bool mForceSuppressFormPopup = false;

};

#endif
