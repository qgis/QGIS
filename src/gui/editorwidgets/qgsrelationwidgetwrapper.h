/***************************************************************************
    qgsrelationwidgetwrapper.h
     --------------------------------------
    Date                 : 14.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONWIDGETWRAPPER_H
#define QGSRELATIONWIDGETWRAPPER_H

#include "qgswidgetwrapper.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsRelationEditorWidget;

/**
 * \ingroup gui
 * \class QgsRelationWidgetWrapper
 */

class GUI_EXPORT QgsRelationWidgetWrapper : public QgsWidgetWrapper
{
    Q_OBJECT

  public:

    //! Constructor for QgsRelationWidgetWrapper
    explicit QgsRelationWidgetWrapper( QgsVectorLayer *vl, const QgsRelation &relation, QWidget *editor = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Defines if a title label should be shown for this widget.
     * Only has an effect after widget() has been called at least once.
     *
     * \since QGIS 2.18
     */
    bool showLabel() const;

    /**
     * Defines if a title label should be shown for this widget.
     * Only has an effect after widget() has been called at least once.
     *
     * \since QGIS 2.18
     */
    void setShowLabel( bool showLabel );

    /**
     * Determines if the "link feature" button should be shown
     *
     * \since QGIS 2.18
     */
    bool showLinkButton() const;

    /**
     * Determines if the "link feature" button should be shown
     *
     * \since QGIS 2.18
     */
    void setShowLinkButton( bool showLinkButton );

    /**
     * Determines if the "save child layer edits" button should be shown
     *
     * \since QGIS 3.14
     */
    bool showSaveChildEditsButton() const;

    /**
     * Determines if the "save child layer edits" button should be shown
     *
     * \since QGIS 3.14
     */
    void setShowSaveChildEditsButton( bool showSaveChildEditsButton );

    /**
     * Determines if the "unlink feature" button should be shown
     *
     * \since QGIS 2.18
     */
    bool showUnlinkButton() const;

    /**
     * Determines if the "unlink feature" button should be shown
     *
     * \since QGIS 2.18
     */
    void setShowUnlinkButton( bool showUnlinkButton );

    /**
     * The relation for which this wrapper is created.
     *
     * \since QGIS 3.0
     */
    QgsRelation relation() const;

    /**
     * Will be called when a value in the current edited form or table row
     * changes
     *
     * Forward the signal to the embedded form
     *
     * \param attribute The name of the attribute that changed.
     * \param newValue     The new value of the attribute.
     * \param attributeChanged If TRUE, it corresponds to an actual change of the feature attribute
     * \since QGIS 3.14
     */
    void widgetValueChanged( const QString &attribute, const QVariant &newValue, bool attributeChanged );


  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    bool valid() const override;

  public slots:
    void setFeature( const QgsFeature &feature ) override;

    /**
     * Sets the visibility of the wrapper's widget.
     * \param visible set to TRUE to show widget, FALSE to hide widget
     * \since QGIS 2.16
     */
    void setVisible( bool visible );

  private:
    void aboutToSave() override;
    QgsRelation mRelation;
    QgsRelation mNmRelation;
    QgsRelationEditorWidget *mWidget = nullptr;
};

#endif // QGSRELATIONWIDGETWRAPPER_H
