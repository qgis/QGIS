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
     * \note
     * For use with Layer. Sets mMetadata if pointer is valid
     *  calls setMetadata, using mMetadata
     * \param layer to set the main QgsLayerMetadata with mLayer->metadata() when not nullptr
     * \see mMetadata
     * \see setMetadata
     * \since QGIS 3.0
     */
    QgsMetadataWidget( QWidget *parent, QgsMapLayer *layer = nullptr );

    /**
     * Set a QgsLayerMetadata object.
     * \note
     * Called from constructor and initializes child widged on first use
     * Can be called from outside to change the QgsLayerMetadata object.
     * \param layerMetadata to set the main  QgsLayerMetadata
     * \see mMetadata
     * \since QGIS 3.0
     */
    void setMetadata( const QgsLayerMetadata &layerMetadata );

    /**
     * Retrieved a QgsLayerMetadata object.
     * \note
     *  saveMetdata is called before returning QgsLayerMetadata
     * \see saveMetadata
     * \since QGIS 3.0
     */
    QgsLayerMetadata getMetadata();

    /**
     * Save all fields in a QgsLayerMetadata object.
     * \see getMetadata
     * \see acceptMetadata
     * \see checkMetadata
     * \since QGIS 3.0
     */
    bool saveMetadata( QgsLayerMetadata &layerMetadata ) const;

    /**
     * Check if values in the wizard are correct.
     * \see updatePanel
     * \see saveMetadata
     * \since QGIS 3.0
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

  signals:

    /**
     * Emitted when the layer's metadata is changed.
     * \see setMetadata()
     * \see getMetadata()
     * \since QGIS 3.0
     */
    void metadataChanged();

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
     * \brief LinkItemDelegate constructor
     * \param parent
     */
    explicit LinkItemDelegate( QObject *parent = nullptr );

    /**
     * Create a special editor with a QCombobox in the link view.
     */
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
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
     * \brief ConstraintItemDelegate constructor
     * \param parent
     */
    explicit ConstraintItemDelegate( QObject *parent = nullptr );

    /**
     * Create a special editor with a QCombobox in the constraint view.
     */
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

/**
 \\\@endcond
 */

#endif
#endif
