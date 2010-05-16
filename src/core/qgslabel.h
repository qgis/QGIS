/***************************************************************************
                         qgslabel.h - render vector labels
                             -------------------
    begin                : August 2004
    copyright            : (C) 2004 by Radim Blazek
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
/* $Id$ */
#ifndef QGSLABEL_H
#define QGSLABEL_H

#include <vector>

#include <QColor>
#include <QList>
#include <QMap>

#include "qgspoint.h"

class QDomNode;
class QDomDocument;
class QDomElement;
class QString;
class QPainter;
class QPaintDevice;

class QgsFeature;
class QgsField;
class QgsLabelAttributes;

#include "qgsfield.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"

typedef QList<int> QgsAttributeList;

typedef QMap<int, QgsField> QgsFieldMap;

/** \ingroup core
  * A class to render labels.
  * Label rendering properties can be either specified directly or
  * in most cases determined dynamically based on the value of an attribute.
  **/
class CORE_EXPORT QgsLabel
{
  public:
    QgsLabel( const QgsFieldMap & fields );

    ~QgsLabel();

    /* Fields */
    enum LabelField
    {
      Text = 0,
      Family,
      Size,
      SizeType,
      Bold,
      Italic,
      Underline,
      Color,
      XCoordinate,
      YCoordinate,
      XOffset,
      YOffset,
      Angle,
      Alignment,
      BufferEnabled,
      BufferSize,
      BufferColor,
      BufferBrush,
      BorderWidth,
      BorderColor,
      BorderStyle,
      MultilineEnabled,
      StrikeOut,     // added in 1.5
      LabelFieldCount
    };

    struct labelpoint
    {
      QgsPoint p;
      double angle;
    };

    /** \brief render label
     *  \param sizeScale global scale factor for size in pixels, labels in map units are not scaled
     *  \note deprecated
     */
    void renderLabel( QPainter* painter, const QgsRectangle& viewExtent,
                      QgsCoordinateTransform* coordinateTransform,
                      const QgsMapToPixel *transform,
                      QgsFeature &feature, bool selected, QgsLabelAttributes *classAttributes = 0, double sizeScale = 1., double rasterScaleFactor = 1.0 )
    {
      QgsRenderContext r;
      r.setExtent( viewExtent );
      r.setCoordinateTransform( new QgsCoordinateTransform( coordinateTransform->sourceCrs(), coordinateTransform->destCRS() ) );
      r.setMapToPixel( *transform );
      r.setPainter( painter );
      r.setScaleFactor( sizeScale );
      r.setRasterScaleFactor( rasterScaleFactor );
      renderLabel( r, feature, selected, classAttributes );
    }

    /** \brief render label
     *  \param sizeScale global scale factor for size in pixels, labels in map units are not scaled
     *  \note added in 1.2
     */
    void renderLabel( QgsRenderContext &renderContext, QgsFeature &feature, bool selected, QgsLabelAttributes *classAttributes = 0 );

    /** Reads the renderer configuration from an XML file
     @param rnode the Dom node to read
    */
    void readXML( const QDomNode& node );

    /** Writes the contents of the renderer to a configuration file */
    void writeXML( QDomNode & label_node, QDomDocument & document ) const;

    //! add vector of required fields to existing list of fields
    void addRequiredFields( QgsAttributeList& fields ) const;

    //! Set available fields
    void setFields( const QgsFieldMap & fields );

    //! Available vector fields
    QgsFieldMap & fields( void );

    /** Pointer to default attributes.
     * @note this will be deprecated in version 2 as it is
     * badly named. Rather use attributes.
     * @see labelAttributes method rather */
    QgsLabelAttributes *layerAttributes( void );

    /** Pointer to default attributes.
     * @note this replaces the to-be-deprecated layerAttributes method.
     * @note introduced in QGIS 1.4
     */
    QgsLabelAttributes *labelAttributes( void );

    //! Set label field
    void setLabelField( int attr, int fieldIndex );

    //! Set label field by name
    bool setLabelFieldName( int attr, QString name );

    //! label field
    QString labelField( int attr ) const;

    /** Get field value if : 1) field name is not empty
     *                       2) field exists
     *                       3) value is defined
     *  otherwise returns empty string
    */
    QString fieldValue( int attr, QgsFeature& feature );

    /** Accessor and mutator for the minimum scale member */
    void setMinScale( float theMinScale );
    float minScale() const;

    /** Accessor and mutator for the maximum scale member */
    void setMaxScale( float theMaxScale );
    float maxScale() const;

    /** Accessor and mutator for the scale based visilibility flag */
    void setScaleBasedVisibility( bool theVisibilityFlag );
    bool scaleBasedVisibility() const;

  private:
    /** Does the actual rendering of a label at the given point
     *
     */
    void renderLabel( QgsRenderContext &renderContext, QgsPoint point,
                      QString text, QFont font, QPen pen,
                      int dx, int dy,
                      double xoffset, double yoffset,
                      double ang,
                      int width, int height, int alignment );

    bool readLabelField( QDomElement &el, int attr, QString prefix );

    /** Get label point for simple feature in map units */
    void labelPoint( std::vector<labelpoint>&, QgsFeature &feature );

    /** Get label point for the given feature in wkb format. */
    unsigned char* labelPoint( labelpoint& point, unsigned char* wkb, size_t wkblen );

    /** Color to draw selected features */
    QColor mSelectionColor;

    //! Default layer attributes
    QgsLabelAttributes *mLabelAttributes;

    //! Available layer fields
    QgsFieldMap mField;

    //! Label fields
    std::vector<QString> mLabelField;

    //! Label field indexes
    std::vector<int> mLabelFieldIdx;

    /** Minimum scale at which this label should be displayed */
    float mMinScale;
    /** Maximum scale at which this label should be displayed */
    float mMaxScale;
    /** A flag that tells us whether to use the above vars to restrict the label's visibility */
    bool mScaleBasedVisibility;
};

#endif
