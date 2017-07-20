/***************************************************************************
                          qgsmetadatawizard.h  -  description
                             -------------------
    begin                : 17/05/2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne at kartoza.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMETADATAWIZARD_H
#define QGSMETADATAWIZARD_H

#include "QStandardItemModel"
#include "QStyledItemDelegate"

#include "qgis_app.h"
#include "qgsmaplayer.h"
#include "qgslayermetadata.h"
#include "ui_qgsmetadatawizard.h"

/**
 * \ingroup app
 * \class QgsMetadataWizard
 * \brief A wizard to edit metadata on a map layer.
 *
 * \since QGIS 3.0
 */

class APP_EXPORT QgsMetadataWizard : public QDialog, private Ui::QgsMetadataWizard
{
    Q_OBJECT

  public:

    /**
     * Constructor for the wizard.
     */
    QgsMetadataWizard( QWidget *parent, QgsMapLayer *layer = nullptr );
    ~QgsMetadataWizard();

    /**
     * Set the source field from the layer.
     */
    void setAutoSource();

    /**
     * Add a new vocabulary.
     */
    void addVocabulary();

    /**
     * Remove a selected vocabulary.
     */
    void removeVocabulary();

    /**
     * Add a new licence.
     */
    void addLicence();

    /**
     * Remove a selected licence.
     */
    void removeLicence();

    /**
     * Set the CRS field from the layer.
     */
    void setAutoCrs();

    /**
     * Add a new contact.
     */
    void addContact();

    /**
     * Remove a selected contact.
     */
    void removeContact();

    /**
     * Update the contact details according to the selection in the contact list.
     */
    void updateContactDetails();

    /**
     * Add a new link.
     */
    void addLink();

    /**
     * Remove a selected link.
     */
    void removeLink();

    /**
     * Function to fill combobox like language, type.
     */
    void fillComboBox();

    /**
     * Fill the wizard from values in the layer metadata object.
     */
    void setPropertiesFromLayer();

    /**
     * Save all fields in a QgsLayerMetadata object.
     */
    void saveMetadata( QgsLayerMetadata &layerMetadata );

    /**
     * Check if values in the wizard are correct.
     */
    bool checkMetadata();

    /**
     * Returns a list of languages by default available in the wizard.
     */
    static QStringList parseLanguages();

    /**
     * Returns a list of licences by default available in the wizard.
     */
    static QStringList parseLicenses();

    /**
     * Returns a list of link types by default available in the wizard.
     * \see https://github.com/OSGeo/Cat-Interop/blob/master/LinkPropertyLookupTable.csv
     */
    static QStringList parseLinkTypes();

  private:
    void cancelClicked();
    void backClicked();
    void nextClicked();
    void finishedClicked();
    void updatePanel();
    QgsMapLayer *mLayer = nullptr;
    QgsLayerMetadata mMetadata;
    QStandardItemModel *mLinksModel = nullptr;
};

class LinkItemDelegate : public QStyledItemDelegate
{

    Q_OBJECT

  public:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

#endif
