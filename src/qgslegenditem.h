
/***************************************************************************
                          qgslegenditem.h  -  description
                             -------------------
    begin                : Sun Jul 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
        Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSLEGENDITEM_H
#define QGSLEGENDITEM_H

#include <qlistview.h>

class QgsMapLayer;
class QgsSymbol;


/**
 * \class QgsLegendItem
 * \brief An item in a QgsLegend

 *@author Gary E.Sherman
 */

class QgsLegendItem : public QCheckListItem
{
public:

    /*! Constructor
     * @param lyr Map layer this legend item represents
     * @param parent The parent listview
     */
    QgsLegendItem(QgsMapLayer * lyr = 0, QListView * parent = 0);

    //! Destructor
    virtual ~QgsLegendItem();

    /** Write property of QString layerName. */
    virtual void setLayerName(const QString & _newVal);

    /** Write property of QString displayName. */
    // DEPRECATED? virtual void setDisplayName(const QString & _newVal);

    /*! Responds to changes in the layer state (eg. visible vs non visible)
     *@param v True if layer is visible
     */
    void stateChange(bool v);

    /*! Gets the layer associated with this legend item
     * @return Pointer to the layer
     */
    virtual QgsMapLayer *layer();

    /** returns layer ID of associated map layer 
     */
    QString layerID() const;

    /** sets check box state and consequently the visibility of corresponding map layer */
    //void setOn( bool );

private:                       // Private attributes

    /**  */
    QgsMapLayer * m_layer;

    QgsSymbol *symbol;

public:                        // Public attributes

    /**  This is the name as rendered in the legend item pixmap */
    // DEPRECATED? QString displayName;

    /**  The layer name as stored originaly in the dataset */
    QString layerName;
};

#endif
