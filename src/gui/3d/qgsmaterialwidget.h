/***************************************************************************
  qgsmaterialwidget.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMATERIALWIDGET_H
#define QGSMATERIALWIDGET_H

#include "ui_materialwidget.h"

#include <memory>

#include "qgis.h"
#include "qgis_gui.h"

#include <QDialog>
#include <QPointer>
#include <QWidget>

#define SIP_NO_FILE

class QgsAbstractMaterialSettings;
class QgsVectorLayer;
class QDialogButtonBox;

/**
 * \ingroup gui
 * \class QgsMaterialWidget
 *
 * \brief A widget allowing users to customize a 3d material.
 *
 * \note Not available in Python bindings
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsMaterialWidget : public QgsPanelWidget, private Ui::MaterialWidgetBase
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsMaterialWidget.
     */
    explicit QgsMaterialWidget( QWidget *parent = nullptr );
    ~QgsMaterialWidget() override;

    /**
     * Sets the required rendering \a technique which the material must support.
     *
     * This is used to filter the available material choices in the widget.
     *
     * \note This setting is only respected when filterByTechnique() is TRUE.
     *
     * \see technique()
     * \see filterByTechnique()
     */
    void setTechnique( Qgis::MaterialRenderingTechnique technique );

    /**
     * Returns the required rendering technique which the material must support.
     *
     * This is used to filter the available material choices in the widget.
     *
     * \note This setting is only respected when filterByTechnique() is TRUE.
     *
     * \see setTechnique()
     * \see filterByTechnique()
     */
    Qgis::MaterialRenderingTechnique technique() const { return mTechnique; }

    /**
     * Sets whether available materials should be filtered by technique.
     *
     * \see filterByTechnique()
     * \see setTechnique()
     */
    void setFilterByTechnique( bool enabled );

    /**
     * Returns whether available materials are filtered by technique.
     *
     * \see setFilterByTechnique()
     * \see setTechnique()
     */
    bool filterByTechnique() const { return mFilterByTechnique; }

    /**
     * Sets the widget state to match material \a settings.
     */
    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer );

    /**
     * Returns the current settings defined by the widget.
     */
    std::unique_ptr< QgsAbstractMaterialSettings > settings();

    /**
     * Sets the current material \a type.
     */
    void setType( const QString &type );

  public slots:

    /**
     * Sets whether the material preview widget should be visible.
     */
    void setPreviewVisible( bool visible );

  signals:

    /**
     * Emitted when the material defined by the widget is changed.
     */
    void changed();

  private slots:
    void materialTypeChanged();
    void materialWidgetChanged();

  private:
    void updateMaterialWidget();
    void rebuildAvailableTypes();
    bool mPreviewVisible = false;
    QPointer< QgsVectorLayer > mLayer;

    std::unique_ptr<QgsAbstractMaterialSettings> mCurrentSettings;

    bool mFilterByTechnique = false;
    Qgis::MaterialRenderingTechnique mTechnique = Qgis::MaterialRenderingTechnique::Triangles;
};


/**
 * \ingroup gui
 * \brief A dialog for configuring a 3D material.
 *
 * \note Not available in Python bindings
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsMaterialWidgetDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
   * Constructor for QgsMaterialWidgetDialog, initially showing the specified material \a settings.
   */
    QgsMaterialWidgetDialog( const QgsAbstractMaterialSettings *settings, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
    * Returns the current settings defined by the dialog.
    */
    std::unique_ptr< QgsAbstractMaterialSettings > settings();

    /**
     * Returns the dialog's button box.
     */
    QDialogButtonBox *buttonBox();

  private:
    QgsMaterialWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};


#endif // QGSMATERIALWIDGET_H
