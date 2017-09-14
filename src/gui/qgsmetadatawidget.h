/***************************************************************************
                          qgsmetadatawidget.h  -  description
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
#ifndef QGSMETADATAWIDGET_H
#define QGSMETADATAWIDGET_H

#include "QStandardItemModel"
#include "QStyledItemDelegate"

#include "qgis_gui.h"
#include "qgsmaplayer.h"
#include "qgslayermetadata.h"
#include "ui_qgsmetadatawidget.h"

/**
 * \ingroup gui
 * \class QgsMetadataWidget
 * \brief A wizard to edit metadata on a map layer.
 *
 * \since QGIS 3.0
 */

class GUI_EXPORT QgsMetadataWidget : public QWidget, private Ui::QgsMetadataWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for the wizard.
     */
    QgsMetadataWidget( QWidget *parent, QgsMapLayer *layer = nullptr );
    ~QgsMetadataWidget();

    /**
     * Set the source field from the layer.
     */
    void setAutoSource() const;

    /**
     * Add a new vocabulary.
     */
    void addVocabulary() const;

    /**
     * Remove a selected vocabulary.
     */
    void removeVocabulary() const;

    /**
     * Add a new licence.
     */
    void addLicence();

    /**
     * Remove a selected licence.
     */
    void removeLicence() const;

    /**
     * Add a new right.
     */
    void addRight();

    /**
     * Remove a selected right.
     */
    void removeRight() const;

    /**
     * Add a new constraint.
     */
    void addConstraint() const;

    /**
     * Remove a constraint.
     */
    void removeConstraint() const;

    /**
     * Set the CRS field from the layer.
     */
    void setAutoCrs() const;

    /**
     * Add a new contact.
     */
    void addContact() const;

    /**
     * Remove a selected contact.
     */
    void removeContact() const;

    /**
     * Update the contact details according to the selection in the contact list.
     */
    void updateContactDetails() const;

    /**
     * Add a new link.
     */
    void addLink() const;

    /**
     * Remove a selected link.
     */
    void removeLink() const;

    /**
     * Function to fill combobox like language, type.
     */
    void fillComboBox() const;

    /**
     * Fill the wizard from values in the layer metadata object.
     */
    void setPropertiesFromLayer() const;

    /**
     * Save all fields in a QgsLayerMetadata object.
     */
    void saveMetadata( QgsLayerMetadata &layerMetadata ) const;

    /**
     * Check if values in the wizard are correct.
     */
    bool checkMetadata() const;

    /**
     * Returns a list of languages available by default in the wizard.
     */
    static QMap<QString, QString> parseLanguages();

    /**
     * Returns a list of licences available by default in the wizard.
     */
    static QStringList parseLicenses();

    /**
     * Returns a list of link types available by default in the wizard.
     * \see https://github.com/OSGeo/Cat-Interop/blob/master/LinkPropertyLookupTable.csv
     */
    static QStringList parseLinkTypes();

    /**
     * Returns a list of MIME types available by default in the wizard.
     * \see https://fr.wikipedia.org/wiki/Type_MIME
     */
    static QStringList parseMimeTypes();

    /**
     * Returns a list of types available by default in the wizard.
     */
    static QMap<QString, QString> parseTypes();

    /**
     * Saves the metadata to the layer.
     */
    void acceptMetadata();

  private:
    void updatePanel() const;
    void addDefaultCategory() const;
    void addNewCategory();
    void removeCategory() const;
    QStringList mDefaultCategories;
    QgsMapLayer *mLayer = nullptr;
    QgsLayerMetadata mMetadata;
    QStandardItemModel *mConstraintsModel = nullptr;
    QStandardItemModel *mLinksModel = nullptr;
    QStringListModel *mCategoriesModel = nullptr;
    QStringListModel *mDefaultCategoriesModel = nullptr;
    QStringListModel *mRightsModel = nullptr;
    void syncFromCategoriesTabToKeywordsTab() const;
};

#ifndef SIP_RUN

/**
 * \ingroup gui
 * \class LinkItemDelegate
 * \brief Special delegate for the link view in the metadata wizard.
 *
 * \since QGIS 3.0
 */
class LinkItemDelegate : public QStyledItemDelegate
{

    Q_OBJECT

  public:

    /**
     * Create a special editor with a QCombobox in the link view.
     */
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

/**
 * \ingroup gui
 * \class ConstraintItemDelegate
 * \brief Special delegate for the constraint view in the metadata wizard.
 *
 * \since QGIS 3.0
 */
class ConstraintItemDelegate : public QStyledItemDelegate
{

    Q_OBJECT

  public:

    /**
     * Create a special editor with a QCombobox in the constraint view.
     */
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

#endif
#endif
