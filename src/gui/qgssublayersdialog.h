/***************************************************************************
    qgssublayersdialog.h  - dialog for selecting sublayers
    ---------------------
    begin                : January 2009
    copyright            : (C) 2009 by Florian El Ahdab
    email                : felahdab at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSUBLAYERSDIALOG_H
#define QGSSUBLAYERSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include "ui_qgssublayersdialogbase.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgis.h"

/**
 * \ingroup gui
 * \class QgsSublayersDialog
 * \deprecated QGIS 3.40. Will be removed in QGIS 4.0.
 */
class GUI_EXPORT QgsSublayersDialog : public QDialog, private Ui::QgsSublayersDialogBase
{
    Q_OBJECT
  public:
    /**
     * Prompt behavior of the QgsSublayersDialog
     * \deprecated QGIS 3.40. Use Qgis::SublayerPromptMode instead.
     */
    enum PromptMode
    {

      /**
       * always ask if there are existing sublayers
       */
      PromptAlways,

      /**
       * always ask if there are existing sublayers, but skip if there are bands for rasters
       */
      PromptIfNeeded,

      /**
       * never prompt, will not load anything
       */
      PromptNever,

      /**
       * never prompt, but load all sublayers
       */
      PromptLoadAll
    };
    Q_ENUM( PromptMode )

    enum ProviderType
    {
      Ogr,
      Gdal,
      Vsifile,
      Mdal //!< \since QGIS 3.14
    };

    /**
     * A structure that defines layers for the purpose of this dialog
     */
    struct LayerDefinition
    {
        //! Identifier of the layer (one unique layer id may have multiple types though)
        int layerId = -1;

        //! Name of the layer (not necessarily unique)
        QString layerName;

        //! Number of features (might be unused)
        int count = -1;

        //! Extra type depending on the use (e.g. geometry type for vector sublayers)
        QString type;

        /**
       * Description.
       *
       * \since QGIS 3.10
       */
        QString description;
    };

    /**
     * List of layer definitions for the purpose of this dialog
     */
    typedef QList<QgsSublayersDialog::LayerDefinition> LayerDefinitionList;

    //! Constructor for QgsSublayersDialog

    /**
     * Construct a new QgsSublayersDialog object - a dialog to select which sub layers to be imported from a data source (e.g. from geopackage or zipfile)
     *
     * \param providerType provider type
     * \param name provider type name
     * \param parent parent widget of the dialog
     * \param fl window flags
     * \param dataSourceUri data source URI
     *
     * \deprecated QGIS 3.40. Will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED QgsSublayersDialog( ProviderType providerType, const QString &name, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = Qt::WindowFlags(), const QString &dataSourceUri = QString() ) SIP_DEPRECATED;

    ~QgsSublayersDialog() override;

    /**
     * Populate the table with layers
     */
    void populateLayerTable( const LayerDefinitionList &list );

    /**
     * Returns list of selected layers
     */
    LayerDefinitionList selection();

    /**
     * Set if we should display the add to group checkbox
     */
    void setShowAddToGroupCheckbox( bool showAddToGroupCheckbox ) { mShowAddToGroupCheckbox = showAddToGroupCheckbox; }

    /**
     * If we should display the add to group checkbox
     */
    bool showAddToGroupCheckbox() const { return mShowAddToGroupCheckbox; }

    /**
     * If we should add layers in a group
     */
    bool addToGroupCheckbox() const { return mCbxAddToGroup->isChecked(); }

    /**
     * Returns column with count or -1
     */
    int countColumn() const { return mShowCount ? 2 : -1; }

  public slots:
    int exec() override;

  private slots:
    void layersTable_selectionChanged( const QItemSelection &, const QItemSelection & );
    void mBtnDeselectAll_pressed();

  protected:
    /**
     * Provider type name
     */
    QString mName;
    QStringList mSelectedSubLayers;

    //! Whether to show number of features in the table
    bool mShowCount = false;
    //! Whether to show type in the table
    bool mShowType = false;
    //! Whether to show description in the table
    bool mShowDescription = false;

  private:
    //! Whether to show the add to group checkbox
    bool mShowAddToGroupCheckbox = false;
};

#endif
