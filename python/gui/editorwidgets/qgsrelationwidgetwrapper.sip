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


class QgsRelationWidgetWrapper : QgsWidgetWrapper
{
%TypeHeaderCode
#include <qgsrelationwidgetwrapper.h>
%End

  public:
    explicit QgsRelationWidgetWrapper( QgsVectorLayer* vl, const QgsRelation& relation, QWidget* editor = 0, QWidget* parent /TransferThis/ = 0 );

    /**
     * Defines if a title lable should be shown for this widget.
     * Only has an effect after widget() has been called at least once.
     *
     * @note Added in QGIS 2.18
     */
    bool showLabel() const;

    /**
     * Defines if a title lable should be shown for this widget.
     * Only has an effect after widget() has been called at least once.
     *
     * @note Added in QGIS 2.18
     */
    void setShowLabel( bool showLabel );

    /**
     * Determines if the "link feature" button should be shown
     *
     * @note Added in QGIS 2.18
     */
    bool showLinkButton() const;
    /**
     * Determines if the "link feature" button should be shown
     *
     * @note Added in QGIS 2.18
     */
    void setShowLinkButton( bool showLinkButton );

    /**
     * Determines if the "unlink feature" button should be shown
     *
     * @note Added in QGIS 2.18
     */
    bool showUnlinkButton() const;
    /**
     * Determines if the "unlink feature" button should be shown
     *
     * @note Added in QGIS 2.18
     */
    void setShowUnlinkButton( bool showUnlinkButton );

  protected:
    QWidget* createWidget( QWidget* parent );
    void initWidget( QWidget* editor );
    bool valid() const;

  public slots:
    void setFeature( const QgsFeature& feature );

    /** Sets the visibility of the wrapper's widget.
     * @param visible set to true to show widget, false to hide widget
     * @note added in QGIS 2.16
     */
    void setVisible( bool visible );
};
