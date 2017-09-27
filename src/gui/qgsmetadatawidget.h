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

    /**
     * Save all fields in a QgsLayerMetadata object.
     */
    void saveMetadata( QgsLayerMetadata &layerMetadata ) const;

    /**
     * Check if values in the wizard are correct.
     */
    bool checkMetadata() const;

    /**
     * Saves the metadata to the layer.
     */
    void acceptMetadata();

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

  private:
    void updatePanel() const;
    void fillSourceFromLayer() const;
    void fillCrsFromLayer() const;
    void addDefaultCategory() const;
    void addNewCategory();
    void removeSelectedCategory() const;
    void addVocabulary() const;
    void removeSelectedVocabulary() const;
    void addLicence();
    void removeSelectedLicence() const;
    void addRight();
    void removeSelectedRight() const;
    void addConstraint() const;
    void removeSelectedConstraint() const;
    void addContact() const;
    void removeSelectedContact() const;
    void addLink() const;
    void removeSelectedLink() const;
    void addHistory();
    void removeSelectedHistory() const;
    void updateContactDetails() const;
    void fillComboBox() const;
    void setPropertiesFromLayer() const;
    void syncFromCategoriesTabToKeywordsTab() const;
    QStringList mDefaultCategories;
    QgsMapLayer *mLayer = nullptr;
    QgsLayerMetadata mMetadata;
    QStandardItemModel *mConstraintsModel = nullptr;
    QStandardItemModel *mLinksModel = nullptr;
    QStringListModel *mCategoriesModel = nullptr;
    QStringListModel *mDefaultCategoriesModel = nullptr;
    QStringListModel *mRightsModel = nullptr;
    QStringListModel *mHistoryModel = nullptr;
};

#ifndef SIP_RUN


/**
 \\\@cond PRIVATE
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

/**
 \\\@endcond
 */

#endif
#endif
