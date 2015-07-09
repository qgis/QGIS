/***************************************************************************
                         qgscomposerlabel.h
                             -------------------
    begin                : January 2005
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
#ifndef QGSCOMPOSERLABEL_H
#define QGSCOMPOSERLABEL_H

#include "qgscomposeritem.h"
#include <QFont>

class QgsVectorLayer;
class QgsFeature;
class QgsDistanceArea;

/** \ingroup MapComposer
 * A label that can be placed onto a map composition.
 */
class CORE_EXPORT QgsComposerLabel: public QgsComposerItem
{
    Q_OBJECT
  public:
    QgsComposerLabel( QgsComposition *composition );
    ~QgsComposerLabel();

    /** return correct graphics item type. */
    virtual int type() const override { return ComposerLabel; }

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget ) override;

    /**resizes the widget such that the text fits to the item. Keeps top left point*/
    void adjustSizeToText();

    QString text() { return mText; }
    void setText( const QString& text );

    int htmlState() { return mHtmlState; }
    void setHtmlState( int state );

    /**Returns the text as it appears on screen (with replaced data field) */
    QString displayText() const;

    /** Sets the current feature, the current layer and a list of local variable substitutions for evaluating expressions */
    void setExpressionContext( QgsFeature* feature, QgsVectorLayer* layer, QMap<QString, QVariant> substitutions = ( QMap<QString, QVariant>() ) );

    QFont font() const;
    void setFont( const QFont& f );
    /** Accessor for the vertical alignment of the label
     * @returns Qt::AlignmentFlag
     */
    Qt::AlignmentFlag vAlign() const { return mVAlignment; }
    /** Accessor for the horizontal alignment of the label
     * @returns Qt::AlignmentFlag
     */
    Qt::AlignmentFlag hAlign() const { return mHAlignment; }
    /** Mutator for the horizontal alignment of the label
     * @param a alignment
     * @returns void
     */
    void setHAlign( Qt::AlignmentFlag a ) {mHAlignment = a;}
    /** Mutator for the vertical alignment of the label
     * @param a alignment
     * @returns void
     */
    void setVAlign( Qt::AlignmentFlag a ) { mVAlignment = a; }

    /**Returns the margin between the edge of the frame and the label contents
     * @returns margin in mm
     * @deprecated use marginX and marginY instead
    */
    Q_DECL_DEPRECATED double margin() { return mMarginX; }

    /**Returns the horizontal margin between the edge of the frame and the label
     * contents.
     * @returns horizontal margin in mm
     * @note added in QGIS 2.7
    */
    double marginX() const { return mMarginX; }

    /**Returns the vertical margin between the edge of the frame and the label
     * contents.
     * @returns vertical margin in mm
     * @note added in QGIS 2.7
    */
    double marginY() const { return mMarginY; }

    /**Sets the margin between the edge of the frame and the label contents.
     * This method sets both the horizontal and vertical margins to the same
     * value. The margins can be individually controlled using the setMarginX
     * and setMarginY methods.
     * @param m margin in mm
     * @see setMarginX
     * @see setMarginY
    */
    void setMargin( const double m );

    /**Sets the horizontal margin between the edge of the frame and the label
     * contents.
     * @param margin horizontal margin in mm
     * @see setMargin
     * @see setMarginY
     * @note added in QGIS 2.7
    */
    void setMarginX( const double margin );

    /**Sets the vertical margin between the edge of the frame and the label
     * contents.
     * @param margin vertical margin in mm
     * @see setMargin
     * @see setMarginX
     * @note added in QGIS 2.7
    */
    void setMarginY( const double margin );

    /**Sets text color */
    void setFontColor( const QColor& c ) { mFontColor = c; }
    /**Get font color */
    QColor fontColor() const { return mFontColor; }

    /** stores state in Dom element
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const override;

    /** sets state from Dom document
       * @param itemElem is Dom element corresponding to 'ComposerLabel' tag
       * @param doc document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) override;

    //Overriden to contain part of label's text
    virtual QString displayName() const override;

    /**In case of negative margins, the bounding rect may be larger than the
     * label's frame
    */
    QRectF boundingRect() const override;

    /**Reimplemented to call prepareGeometryChange after toggling frame
    */
    virtual void setFrameEnabled( const bool drawFrame ) override;

    /**Reimplemented to call prepareGeometryChange after changing outline width
    */
    virtual void setFrameOutlineWidth( const double outlineWidth ) override;

  public slots:
    void refreshExpressionContext();


  private slots:
    void loadingHtmlFinished( bool );

  private:
    // Text
    QString mText;

    // Html state
    int mHtmlState;
    double mHtmlUnitsToMM;
    double htmlUnitsToMM(); //calculate scale factor
    bool mHtmlLoaded;

    /**Helper function to calculate x/y shift for adjustSizeToText() depending on rotation, current size and alignment*/
    void itemShiftAdjustSize( double newWidth, double newHeight, double& xShift, double& yShift ) const;

    // Font
    QFont mFont;

    /**Horizontal margin between contents and frame (in mm)*/
    double mMarginX;
    /**Vertical margin between contents and frame (in mm)*/
    double mMarginY;

    // Font color
    QColor mFontColor;

    // Horizontal Alignment
    Qt::AlignmentFlag mHAlignment;

    // Vertical Alignment
    Qt::AlignmentFlag mVAlignment;

    /**Replaces replace '$CURRENT_DATE<(FORMAT)>' with the current date (e.g. $CURRENT_DATE(d 'June' yyyy)*/
    void replaceDateText( QString& text ) const;

    QgsFeature* mExpressionFeature;
    QgsVectorLayer* mExpressionLayer;
    QMap<QString, QVariant> mSubstitutions;
    QgsDistanceArea* mDistanceArea;
};

#endif


