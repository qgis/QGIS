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

/**
 * \ingroup gui
 * \class QgsSublayersDialog
 */
class GUI_EXPORT QgsSublayersDialog : public QDialog, private Ui::QgsSublayersDialogBase
{
    Q_OBJECT
  public:

    /**
     * Prompt behavior of the QgsSublayersDialog
     * \since QGIS 3.14
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
     * \since QGIS 2.16
     */
    struct LayerDefinition
    {
      int layerId = -1 ;        //!< Identifier of the layer (one unique layer id may have multiple types though)
      QString layerName;  //!< Name of the layer (not necessarily unique)
      int count = -1 ;          //!< Number of features (might be unused)
      QString type;       //!< Extra type depending on the use (e.g. geometry type for vector sublayers)
      QString description;  //!< Description. Added in QGIS 3.10
    };

    /**
     * List of layer definitions for the purpose of this dialog
     * \since QGIS 2.16
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
     */
    QgsSublayersDialog( ProviderType providerType,
                        const QString &name,
                        QWidget *parent SIP_TRANSFERTHIS = nullptr,
                        Qt::WindowFlags fl = Qt::WindowFlags(),
                        const QString &dataSourceUri = QString() );

    ~QgsSublayersDialog() override;

    /**
     * Populate the table with layers
     * \since QGIS 2.16
     */
    void populateLayerTable( const LayerDefinitionList &list );

    /**
     * Returns list of selected layers
     * \since QGIS 2.16
     */
    LayerDefinitionList selection();

    /**
     * Set if we should display the add to group checkbox
     * \since QGIS 3.0
     */
    void setShowAddToGroupCheckbox( bool showAddToGroupCheckbox ) { mShowAddToGroupCheckbox = showAddToGroupCheckbox; }

    /**
     * If we should display the add to group checkbox
     * \since QGIS 3.0
     */
    bool showAddToGroupCheckbox() const { return mShowAddToGroupCheckbox; }

    /**
     * If we should add layers in a group
     * \since QGIS 3.0
     */
    bool addToGroupCheckbox() const { return mCbxAddToGroup->isChecked(); }

    /**
     * Returns column with count or -1
     * \since QGIS 3.0
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
    bool mShowCount = false;  //!< Whether to show number of features in the table
    bool mShowType = false;   //!< Whether to show type in the table
    bool mShowDescription = false;   //!< Whether to show description in the table

  private:

    bool mShowAddToGroupCheckbox = false;   //!< Whether to show the add to group checkbox
};

#endif
