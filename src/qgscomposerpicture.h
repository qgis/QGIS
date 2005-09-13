/***************************************************************************
                         qgscomposerpicture.h
                             -------------------
    begin                : September 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERPICTURE_H
#define QGSCOMPOSERPICTURE_H

#include <qwidget.h>
#include <qcanvas.h>
#include <qobject.h>
#include <qpicture.h>
#include <qrect.h>
#include <qpointarray.h>

#include "qgsrect.h"
#include "qgscomposer.h"
#include "qgscomposition.h"
#include "qgscomposeritem.h"

#ifdef WIN32
#include "qgscomposerpicturebase.h"
#else
#include "qgscomposerpicturebase.uic.h"
#endif

class QCanvasItem;
class QCanvasRectangle;
class QPainter;
class QWidget;
class QDomNode;
class QDomDocument;
class QPixmap;
class QImage;
class QPen;
class QRect;

class QgsMapCanvas;
class QgsRect;
class QgsMapToPixel;
class QgsComposition;
class QgsComposerMap;
class QgsComposerItem;

/** \class QgsComposerPicture 
 *  \brief Object representing map window. 
 */
// NOTE: QgsComposerPictureBase must be first, otherwise does not compile
//                                public QCanvasRectangle, 
class QgsComposerPicture : public QgsComposerPictureBase, 
				public QCanvasPolygonalItem,
                                public QgsComposerItem
{
    Q_OBJECT

public:
    /** \brief Constructor  
     *  \param id object id
     */
    QgsComposerPicture( QgsComposition *composition, int id, QString file );

    /** \brief Constructor. Settings are read from project. 
     *  \param id object id
     */
    QgsComposerPicture( QgsComposition *composition, int id );
    ~QgsComposerPicture();

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


    // Reimplement QCanvasItem::boundingRect
    QRect boundingRect ( void ) const;

    QPointArray areaPoints() const;

    // Reimplemented
    void moveBy( double x, double y);
     
    /** \brief Reimplementation of QCanvasItem::draw - draw on canvas */
    void draw ( QPainter & painter );

    void drawShape( QPainter & painter );
    
    /** \brief Set values in GUI to current values */
    void setOptions ( void );

    /** \brief returns TRUE if picture is valid / loaded */
    bool pictureValid();

    // Set box, picture will be inside box, used when placed by mouse.
    // Coordinates do not need to be oriented
    void setBox ( int x1, int y1, int x2, int y2 );

public slots:
    // Called when picture file is changed in edit line
    void pictureChanged ( void );

    // Select new picture in dialog box
    void browsePicture ( void );

    // Picture dialog, returns file name or empty string
    static QString pictureDialog ( void );    

    // Frame settings changed
    void frameChanged ( void );

    // Angle changed
    void angleChanged ( void );

    // Width changed
    void widthChanged ();

private:
    // Pointer to composition
    QgsComposition *mComposition;

    // Picture name
    QString mPicturePath;

    // Picture
    QPicture mPicture;

    bool mPictureValid;

    // Coordinates of picture center
    int mCX, mCY;

    // Coordinates of upper left picture corner
    int mX, mY;

    // Picture width and height in paper
    int mWidth, mHeight;

    // Scale, number of canvas units / image unit

    // Rotation in degrees counter clockwise
    double mAngle;

    /** \brief Draw frame  */
    bool mFrame;

    // Frame pen
    QPen  mPen;

    // Load mPicturePath picture
    void loadPicture();
    
    /** \brief Calculate size according to current settings */
    void recalculate ( void );

    QPointArray mAreaPoints;

    // Current bounding box
    QRect mBoundingRect;

    // Adjust size so that picture fits to current box
    void adjustPictureSize();
};

#endif
