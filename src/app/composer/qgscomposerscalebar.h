/***************************************************************************
                            qgscomposerscalebar.h
                             -------------------
    begin                : March 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERSCALEBAR_H
#define QGSCOMPOSERSCALEBAR_H

#include "ui_qgscomposerscalebarbase.h"
#include "qgscomposeritem.h"
#include <QAbstractGraphicsShapeItem>
#include <Q3PointArray>
#include <QRect>
#include <QPen>

class QgsMapCanvas;
class QgsComposition;
class QBrush;
class QDomNode;
class QDomDocument;
class QFont;
class QPainter;
class QPen;

/** \class QgsComposerScalebar
 *  \brief Object representing map window. 
 */
// NOTE: QgsComposerScalebarBase must be first, otherwise does not compile
class QgsComposerScalebar : public QWidget, private Ui::QgsComposerScalebarBase, public QAbstractGraphicsShapeItem, public QgsComposerItem
{
    Q_OBJECT

public:
    /** \brief Constructor  
     *  \param id object id
     *  \param fontSize font size in typographic points!
     */
    QgsComposerScalebar( QgsComposition *composition, int id, int x, int y );

    /** \brief Constructor. Settings are read from project. 
     *  \param id object id
     */
    QgsComposerScalebar( QgsComposition *composition, int id );
    ~QgsComposerScalebar();

    /** \brief Initialise GUI etc., shared by constructors. */
    void init(void);

    // Reimplement QgsComposerItem:
    void setSelected( bool s );
    bool selected( void );
    QWidget *options ( void );
    bool writeSettings ( void );
    bool readSettings ( void );
    bool removeSettings ( void );
    bool writeXML( QDomNode & node, QDomDocument & document, bool temp = false );
    bool readXML( QDomNode & node );

    QRectF boundingRect ( void ) const;
     
    /** \brief Draw to paint device, internal use 
     *  \param painter painter or 0
     *  \return bounding box 
     */
    QRectF render (QPainter *painter);

    /** \brief Reimplementation of QGraphicsItem::paint - draw on canvas */
    void paint ( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget);

    //void drawShape(QPainter&);
    QPolygonF areaPoints() const;
    
    /** \brief Calculate size according to current settings */
    void recalculate ( void );
    
    /** \brief Set values in GUI to current values */
    void setOptions ( void );

    // Move to position
//    void moveBy ( double x, double y );

public slots:
    // Open font dialog
    void on_mFontButton_clicked ( void );

    // Title changed
    void on_mUnitLabelLineEdit_editingFinished ( void );

    // Size changed
    void on_mLineWidthSpinBox_valueChanged ( void );
    void on_mMapUnitsPerUnitLineEdit_editingFinished ( void );
    void on_mNumSegmentsLineEdit_editingFinished ( void );
    void on_mSegmentLengthLineEdit_editingFinished ( void );
    
    // Called by GUI when map selection changed
    void on_mMapComboBox_activated ( int i );

    // Called when map was changed
    void mapChanged ( int id );

private:
    // Pointer to composition
    QgsComposition *mComposition;
    
    // Pointer to map canvas
    QgsMapCanvas *mMapCanvas;
    
    // Composer map id or 0
    int mMap;

    // Vector of map id for maps in combobox
    std::vector<int> mMaps;

    // Current bounding box
    QRectF mBoundingRect;

    // Number of map units in scalebar unit
    double mMapUnitsPerUnit;

    // Unit label
    QString mUnitLabel;

    // Font. Font size in typographic points!
    QFont mFont;

    // Pen
    QPen mPen;

    // Brush
    QBrush mBrush;

    // Number of parts 
    int mNumSegments;

    // Segment size in map units
    double mSegmentLength;

    // Height of scalebar box in canvas units (box style only)
    double mHeight;

    // Margin
    int mMargin;

    // Size changed
    void sizeChanged ( void );
};

#endif
