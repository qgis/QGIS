/***************************************************************************
    qgsprocessingmaplayercombobox.h
    -----------------------------
    begin                : June 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGMAPLAYERCOMBOBOX_H
#define QGSPROCESSINGMAPLAYERCOMBOBOX_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QTreeView>
#include "qgsfeatureid.h"
#include "qgsmimedatautils.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessinggui.h"

class QgsMapLayerComboBox;
class QToolButton;
class QCheckBox;
class QgsProcessingParameterDefinition;
class QgsBrowserGuiModel;
class QgsProcessingParameterWidgetContext;

///@cond PRIVATE

/**
 * Processing map layer combo box.
 * \ingroup gui
 * \warning Not part of stable API and may change in future QGIS releases.
 * \since QGIS 3.8
 */
class GUI_EXPORT QgsProcessingMapLayerComboBox : public QWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingMapLayerComboBox, with the specified \a parameter definition.
     */
    QgsProcessingMapLayerComboBox( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    ~QgsProcessingMapLayerComboBox() override;

    /**
     * Sets the combo box to the specified \a layer, if \a layer is compatible with the
     * widget's parameter definition.
     */
    void setLayer( QgsMapLayer *layer );

    /**
     * Returns the current layer selected in the combobox, or NULLPTR if the selection cannot
     * be represented as a map layer.
     *
     * \warning Prefer calling value() instead, as it correctly encapsulates all valid
     * values which can be represented by the widget.
     *
     * \see currentText()
     */
    QgsMapLayer *currentLayer();

    /**
     * Returns the current text of the selected item in the combobox.
     *
     * \warning Prefer calling value() instead, as it correctly encapsulates all valid
     * values which can be represented by the widget.
     *
     * \see currentLayer()
     */
    QString currentText();

    /**
     * Sets the \a value shown in the widget.
     *
     * \see value()
     */
    void setValue( const QVariant &value, QgsProcessingContext &context );

    /**
     * Returns the current value of the widget.
     *
     * \see setValue()
     */
    QVariant value() const;

    /**
     * Sets the \a context in which the widget is shown.
     * \since QGIS 3.14
     */
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context );

    /**
     * Sets whether the combo box value can be freely edited.
     *
     * \see isEditable()
     * \since QGIS 3.14
     */
    void setEditable( bool editable );

    /**
     * Returns whether the combo box value can be freely edited.
     *
     * \see setEditable()
     * \since QGIS 3.14
     */
    bool isEditable() const;

  signals:

    /**
     * Emitted whenever the value is changed in the widget.
     */
    void valueChanged();

  protected:
    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragLeaveEvent( QDragLeaveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private slots:

    void onLayerChanged( QgsMapLayer *layer );
    void selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect );
    void showSourceOptions();
    void selectFromFile();
    void browseForLayer();

  private:
    std::unique_ptr<QgsProcessingParameterDefinition> mParameter;
    QgsMapLayerComboBox *mCombo = nullptr;
    QToolButton *mSelectButton = nullptr;
    QToolButton *mIterateButton = nullptr;
    QToolButton *mSettingsButton = nullptr;
    QCheckBox *mUseSelectionCheckBox = nullptr;
    bool mDragActive = false;
    long long mFeatureLimit = -1;
    QString mFilterExpression;
    bool mIsOverridingDefaultGeometryCheck = false;
    Qgis::InvalidGeometryCheck mGeometryCheck = Qgis::InvalidGeometryCheck::AbortOnInvalid;
    QPointer<QgsMapLayer> mPrevLayer;
    int mBlockChangedSignal = 0;

    QgsBrowserGuiModel *mBrowserModel = nullptr;

    QMenu *mFeatureSourceMenu = nullptr;
    QgsMapLayer *compatibleMapLayerFromMimeData( const QMimeData *data, bool &incompatibleLayerSelected ) const;
    QString compatibleUriFromMimeData( const QMimeData *data ) const;
};

///@endcond
#endif // QGSPROCESSINGMAPLAYERCOMBOBOX_H
