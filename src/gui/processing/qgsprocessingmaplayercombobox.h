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
#include "qgsprocessingtoolboxmodel.h"
#include "qgsfeatureid.h"
#include "qgsmimedatautils.h"
#include "qgsprocessingcontext.h"


class QgsMapLayerComboBox;
class QToolButton;
class QCheckBox;
class QgsProcessingParameterDefinition;

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
    QgsProcessingMapLayerComboBox( const QgsProcessingParameterDefinition *parameter, QWidget *parent = nullptr );

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

  signals:

    /**
     * Emitted whenever the value is changed in the widget.
     */
    void valueChanged();

    /**
     * Emitted when the widget has triggered a file selection operation (to be
     * handled in Python for now).
     */
    void triggerFileSelection();

  protected:

    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragLeaveEvent( QDragLeaveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private slots:

    void onLayerChanged( QgsMapLayer *layer );
    void selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect );

  private:
    std::unique_ptr< QgsProcessingParameterDefinition > mParameter;
    QgsMapLayerComboBox *mCombo = nullptr;
    QToolButton *mSelectButton = nullptr;
    QCheckBox *mUseSelectionCheckBox = nullptr;
    bool mDragActive = false;
    QPointer< QgsMapLayer> mPrevLayer;
    int mBlockChangedSignal = 0;
    QgsMapLayer *compatibleMapLayerFromMimeData( const QMimeData *data, bool &incompatibleLayerSelected ) const;
    QString compatibleUriFromMimeData( const QMimeData *data ) const;
};

///@endcond
#endif // QGSPROCESSINGMAPLAYERCOMBOBOX_H
