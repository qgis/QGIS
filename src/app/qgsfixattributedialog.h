/***************************************************************************
    qgsfixattributedialog.h
    ---------------------
    begin                : January 2020
    copyright            : (C) 2020 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIXATTRIBUTEDIALOG_H
#define QGSFIXATTRIBUTEDIALOG_H

#include "qgsattributeeditorcontext.h"
#include "qgis_sip.h"
#include "qgsattributeform.h"
#include "qgstrackedvectorlayertools.h"

#include <QDialog>
#include <QGridLayout>
#include <QProgressBar>
#include "qgis_app.h"

/**
 * \ingroup app
 * \class QgsFixAttributeDialog
 * \brief Dialog to fix a list of invalid feature regarding constraints
 * \since QGIS 3.12
 */

class APP_EXPORT QgsFixAttributeDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Feedback code on closing the dialog
     */
    enum Feedback
    {
      DiscardAll, //!< Feedback to discard all features (even valid ones)
      PasteValid, //!< Feedback to paste the valid features and vanishe the invalid ones
      PasteAll    //!< Feedback to paste all features, no matter if valid or invalid
    };

    /**
     * Constructor for QgsFixAttributeDialog
     */
    QgsFixAttributeDialog( QgsVectorLayer *vl, QgsFeatureList &features, QWidget *parent SIP_TRANSFERTHIS = nullptr, const QgsAttributeEditorContext &context = QgsAttributeEditorContext() );

    /**
     * Returns fixed features
     */
    QgsFeatureList fixedFeatures() { return mFixedFeatures; }

    /**
     * Returns unfixed features (canceled or not handled)
     */
    QgsFeatureList unfixedFeatures() { return mUnfixedFeatures; }

  public slots:
    void accept() override;
    void reject() override;

  private:
    void init( QgsVectorLayer *layer, const QgsAttributeEditorContext &context );
    QString descriptionText();

    QgsFeatureList mFeatures;
    QgsFeatureList::iterator mCurrentFeature;

    QgsFeatureList mFixedFeatures;
    QgsFeatureList mUnfixedFeatures;

    QgsAttributeForm *mAttributeForm = nullptr;
    QProgressBar *mProgressBar = nullptr;
    QLabel *mDescription = nullptr;
};

#endif // QGSFIXATTRIBUTEDIALOG_H
