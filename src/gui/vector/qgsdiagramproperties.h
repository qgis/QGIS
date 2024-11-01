/***************************************************************************
  qgsdiagramproperties.h
  Properties for diagram layers
  -------------------
         begin                : August 2012
         copyright            : (C) Matthias Kuhn
         email                : matthias at opengis dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDIAGRAMPROPERTIES_H
#define QGSDIAGRAMPROPERTIES_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsdiagrampropertiesbase.h"

#include "qgis_gui.h"
#include "qgsdiagramrenderer.h"
#include "qgscolorschemelist.h"

#include <QDialog>
#include <QStyledItemDelegate>

class QgsVectorLayer;
class QgsMapCanvas;

/**
 * \ingroup gui
 * \class QgsDiagramProperties
 *
 * \note This class is not a part of public API
 */
class GUI_EXPORT QgsDiagramProperties : public QgsPanelWidget, private Ui::QgsDiagramPropertiesBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsDiagramProperties( QgsVectorLayer *layer, QWidget *parent, QgsMapCanvas *canvas );
    ~QgsDiagramProperties() override;

    /**
     * Updates the widget to reflect the layer's current diagram settings.
     *
     * \since QGIS 3.16
     */
    void syncToLayer();

    /**
     * Updates the widget to reflect the diagram renderer. Does not take ownership.
     * \param dr Diagram renderer where settings are taken from.
     *
     * \since QGIS 3.40
     */
    void syncToRenderer( const QgsDiagramRenderer *dr );

    /**
     * Updates the widget to reflect the diagram layer settings. Does not take ownership.
     * \param dls Diagram Layer Settings to update the widget.
     *
     * \since QGIS 3.40
     */
    void syncToSettings( const QgsDiagramLayerSettings *dls );

    //! Adds an attribute from the list of available attributes to the assigned attributes with a random color.
    void addAttribute( QTreeWidgetItem *item );

    /**
     * Sets the widget in dock mode.
     * \param dockMode TRUE for dock mode.
     */
    void setDockMode( bool dockMode ) override;

    /**
     * Defines the widget's diagram type and lets it know it should hide the type comboBox.
     *
     * \param diagramType Type of diagram to be set
     */
    void setDiagramType( const QString diagramType );

    /**
     * Sets whether the widget should show diagram layer settings.
     *
     * Used by stacked diagrams, which disable editing of DLS for sub diagrams
     * other than the first one.
     *
     * \param allowed Whether this widget should be allowed to edit diagram layer settings.
    */
    void setAllowedToEditDiagramLayerSettings( bool allowed );

    /**
     * Returns whether this widget is allowed to edit diagram layer settings.
     */
    bool isAllowedToEditDiagramLayerSettings() const;

  signals:

    void auxiliaryFieldCreated();

  public slots:
    void apply();
    void mDiagramTypeComboBox_currentIndexChanged( int index );
    void mAddCategoryPushButton_clicked();
    void mAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void mFindMaximumValueButton_clicked();
    void mRemoveCategoryPushButton_clicked();
    void mDiagramAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void showAddAttributeExpressionDialog();
    void mDiagramStackedWidget_currentChanged( int index );
    void updatePlacementWidgets();
    void scalingTypeChanged();
    void showSizeLegendDialog();

  private slots:

    void updateProperty();
    void showHelp();

    void createAuxiliaryField();

  private:
    QgsVectorLayer *mLayer = nullptr;
    //! Point placement button group
    QButtonGroup *mPlacePointBtnGrp = nullptr;
    //! Line placement button group
    QButtonGroup *mPlaceLineBtnGrp = nullptr;
    //! Polygon placement button group
    QButtonGroup *mPlacePolygonBtnGrp = nullptr;

    std::unique_ptr<QgsPaintEffect> mPaintEffect;

    enum Columns
    {
      ColumnAttributeExpression = 0,
      ColumnColor,
      ColumnLegendText,
    };

    enum Roles
    {
      RoleAttributeExpression = Qt::UserRole,
    };

    QString showExpressionBuilder( const QString &initialExpression );

    QgsPropertyCollection mDataDefinedProperties;

    // Keeps track of the diagram type to properly save / restore settings when the diagram type combo box is set to no diagram.
    QString mDiagramType;
    std::unique_ptr<QgsDataDefinedSizeLegend> mSizeLegend;

    QString guessLegendText( const QString &expression );
    QgsMapCanvas *mMapCanvas = nullptr;

    QgsExpressionContext createExpressionContext() const override;

    bool mAllowedToEditDls = true;

    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsDiagramLayerSettings::Property key );

    /**
     * Convenience function to chain widgets' change value signal to another signal.
     *
     * \param widgets List of widgets.
     */
    void connectValueChanged( const QList<QWidget *> &widgets );

    /**
     * Creates a QgsDiagram object from the GUI settings.
     *
     * \since QGIS 3.40
     */
    std::unique_ptr<QgsDiagram> createDiagramObject();

    /**
     * Creates a QgsDiagramSettings object from the GUI settings.
     *
     * \since QGIS 3.40
     */
    std::unique_ptr<QgsDiagramSettings> createDiagramSettings();

    /**
     * Creates a QgsDiagramRenderer object from the GUI settings.
     *
     * \since QGIS 3.40
     */
    std::unique_ptr<QgsDiagramRenderer> createRenderer();

    /**
     * Creates a QgsDiagramLayerSettings object from the GUI settings.
     *
     * \since QGIS 3.40
     */
    QgsDiagramLayerSettings createDiagramLayerSettings();

    /**
     * Insert reasonable defaults to have an initial diagram widget status.
     */
    void insertDefaults();

    /**
     * Sets widgets to reflect the \a enabled status of the diagram.
     * \param enabled Whether the diagram is enabled or not.
     *
     * \see isDiagramEnabled()
     *
     * \since QGIS 3.40
     */
    void setDiagramEnabled( const bool enabled );

    /**
     * Returns whether the current diagram should be enabled or not,
     * according to changes in the corresponding widgets.
     *
     * \see setDiagramEnabled()
     *
     * \since QGIS 3.40
     */
    bool isDiagramEnabled() const;

    friend class QgsStackedDiagramProperties;
    friend class QgsStackedDiagramPropertiesDialog;
};


/**
 * \ingroup gui
 * \class EditBlockerDelegate
 */
class EditBlockerDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    EditBlockerDelegate( QObject *parent = nullptr )
      : QStyledItemDelegate( parent )
    {}

    QWidget *createEditor( QWidget *, const QStyleOptionViewItem &, const QModelIndex & ) const override
    {
      return nullptr;
    }
};


#endif // QGSDIAGRAMPROPERTIES_H
