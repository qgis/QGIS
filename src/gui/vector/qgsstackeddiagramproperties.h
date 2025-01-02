/***************************************************************************
  qgsstackeddiagramproperties.h
  Properties for stacked diagram layers
  -------------------
         begin                : August 2024
         copyright            : (C) Germán Carrillo
         email                : german at opengis dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACKEDDIAGRAMPROPERTIES_H
#define QGSSTACKEDDIAGRAMPROPERTIES_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "qgsdiagramrenderer.h"
#include "ui_qgsstackeddiagrampropertiesbase.h"

#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>

class QgsVectorLayer;
class QgsMapCanvas;
class QgsDiagramProperties;
class QgsDiagramRenderer;


/**
 * \ingroup gui
 * \brief Model for sub diagrams in a stacked diagram view.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsStackedDiagramPropertiesModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    //! constructor
    QgsStackedDiagramPropertiesModel( QObject *parent = nullptr );

    ~QgsStackedDiagramPropertiesModel() override;

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex & = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;

    // editing support
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    // new methods

    //! Returns the diagram renderer at the specified index. Does not transfer ownership.
    QgsDiagramRenderer *subDiagramForIndex( const QModelIndex &index ) const;

    //! Inserts a new diagram at the specified position. Takes ownership.
    void insertSubDiagram( const int index, QgsDiagramRenderer *newSubDiagram );
    //! Replaces the diagram located at \a index by \a dr. Takes ownership.
    void updateSubDiagram( const QModelIndex &index, QgsDiagramRenderer *dr );

    //! Returns the list of diagram renderers from the model. Does not transfer ownership.
    QList<QgsDiagramRenderer *> subRenderers() const;

    //! Returns the diagram layer settings from the model
    QgsDiagramLayerSettings diagramLayerSettings() const;

    /**
     * Sets the diagram layer settings for the model.
     */
    void updateDiagramLayerSettings( QgsDiagramLayerSettings dls );

  protected:
    QList<QgsDiagramRenderer *> mRenderers;
    QgsDiagramLayerSettings mDiagramLayerSettings;
};


/**
 * \ingroup gui
 * \class QgsStackedDiagramProperties
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsStackedDiagramProperties : public QgsPanelWidget, private Ui::QgsStackedDiagramPropertiesBase
{
    Q_OBJECT

  public:
    explicit QgsStackedDiagramProperties( QgsVectorLayer *layer, QWidget *parent, QgsMapCanvas *canvas );

    /**
     * Updates the widget to reflect the layer's current diagram settings.
     */
    void syncToLayer();

  signals:
    void auxiliaryFieldCreated();

  public slots:
    void apply();

  private slots:

    /**
     * Adds a sub diagram renderer to the current QgsStackedDiagramProperties.
     */
    void addSubDiagramRenderer();

    /**
     * Appends a sub diagram renderer to the current QgsStackedDiagramProperties.
     * Takes ownership.
     */
    void appendSubDiagramRenderer( QgsDiagramRenderer *dr );

    /**
     * Edits the properties of the current diagram renderer.
     */
    void editSubDiagramRenderer();

    /**
     * Edits the properties of a diagram renderer located at a given \a index.
     */
    void editSubDiagramRenderer( const QModelIndex &index );

    /**
     * Removes a diagram from the current QgsStackedDiagramProperties.
     */
    void removeSubDiagramRenderer();

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;

    QgsStackedDiagramPropertiesModel *mModel = nullptr;

    /**
     * Determines whether the subdiagram in the given \a index may be
     * the first sub diagram in the stacked diagram. This includes the
     * first enabled sub diagram, as well as disabled sub diagrams that,
     * after being edited, can become the first enabled one.
     */
    bool couldBeFirstSubDiagram( const QModelIndex &index ) const;

  private slots:
    void subDiagramWidgetPanelAccepted( QgsPanelWidget *panel );
    void liveUpdateSubDiagramFromPanel();
};

/**
 * \ingroup gui
 * \class QgsStackedDiagramPropertiesDialog
 * \brief Dialog for editing sub diagrams
 *
 * \note This class is not a part of public API
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsStackedDiagramPropertiesDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsStackedDiagramPropertiesDialog
     * \param layer source vector layer
     * \param parent parent widget
     * \param mapCanvas map canvas
     */
    QgsStackedDiagramPropertiesDialog( QgsVectorLayer *layer, QWidget *parent = nullptr, QgsMapCanvas *mapCanvas = nullptr );

    /**
     * Delegates to the diagram properties widget to sync with the given renderer.
     */
    void syncToRenderer( const QgsDiagramRenderer *dr ) const;

    /**
     * Delegates to the diagram properties widget to sync with the given diagram layer settings.
     */
    void syncToSettings( const QgsDiagramLayerSettings *dls ) const;

    /**
     * Gets a renderer object built from the diagram properties widget.
     * Transfers ownership.
     */
    QgsDiagramRenderer *renderer();

    /**
     * Gets diagram layer settings built from the diagram properties widget.
     */
    QgsDiagramLayerSettings diagramLayerSettings() const;

    /**
     * Delegates to the main widget to set whether the widget should show
     * diagram layer settings to be edited.
     *
     * \param allowed Whether the main widget should be allowed to edit diagram layer settings.
    */
    void setAllowedToEditDiagramLayerSettings( bool allowed ) const;

    /**
     * Returns whether the main widget is allowed to edit diagram layer settings.
     */
    bool isAllowedToEditDiagramLayerSettings() const;

  public slots:

    /**
     * Applies changes from the widget to the internal renderer and diagram layer settings.
     */
    void accept() override;

  private slots:
    void showHelp();

  private:
    QgsDiagramProperties *mPropsWidget = nullptr;
    std::unique_ptr<QgsDiagramRenderer> mRenderer;
    QgsDiagramLayerSettings mDiagramLayerSettings;
    QDialogButtonBox *buttonBox = nullptr;
};

#endif // QGSSTACKEDDIAGRAMPROPERTIES_H
