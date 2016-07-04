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
#include <ui_qgssublayersdialogbase.h>
#include "qgscontexthelp.h"

/** \ingroup gui
 * \class QgsSublayersDialog
 */
class GUI_EXPORT QgsSublayersDialog : public QDialog, private Ui::QgsSublayersDialogBase
{
    Q_OBJECT
  public:

    enum ProviderType
    {
      Ogr,
      Gdal,
      Vsifile
    };

    //! A structure that defines layers for the purpose of this dialog
    //! @note added in 2.16
    typedef struct LayerDefinition
    {
      LayerDefinition() : layerId( -1 ), count( -1 ) {}

      int layerId;        //!< identifier of the layer (one unique layer id may have multiple types though)
      QString layerName;  //!< name of the layer (not necessarily unique)
      int count;          //!< number of features (might be unused)
      QString type;       //!< extra type depending on the use (e.g. geometry type for vector sublayers)
    } LayerDefinition;

    //! List of layer definitions for the purpose of this dialog
    //! @note added in 2.16
    typedef QList<LayerDefinition> LayerDefinitionList;

    QgsSublayersDialog( ProviderType providerType, const QString& name, QWidget* parent = nullptr, const Qt::WindowFlags& fl = nullptr );
    ~QgsSublayersDialog();

    //! Populate the table with layers
    //! @note added in 2.16
    void populateLayerTable( const LayerDefinitionList& list );

    //! Returns list of selected layers
    //! @note added in 2.16
    LayerDefinitionList selection();

    //! @deprecated since 2.16 - use other populateLayerTable() variant
    Q_DECL_DEPRECATED void populateLayerTable( const QStringList& theList, const QString& delim = ":" );
    //! @deprecated since 2.16 - use selection()
    Q_DECL_DEPRECATED QStringList selectionNames();
    //! @deprecated since 2.16 - use selection()
    Q_DECL_DEPRECATED QList<int> selectionIndexes();

  public slots:
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    int exec();

  protected:
    QString mName;
    QStringList mSelectedSubLayers;
    bool mShowCount;  //!< whether to show number of features in the table
    bool mShowType;   //!< whether to show type in the table
};

#endif
