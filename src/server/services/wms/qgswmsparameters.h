/***************************************************************************
                              qgswmsparameters.h
                              ------------------
  begin                : March 17, 2017
  copyright            : (C) 2017 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMSPARAMETERS_H
#define QGSWMSPARAMETERS_H

#include <QMap>
#include <QObject>
#include <QMetaEnum>
#include <QColor>
#include "qgsrectangle.h"
#include "qgswmsserviceexception.h"
#include "qgsserverrequest.h"
#include "qgsgeometry.h"

/** QgsWmsParameters provides an interface to retrieve and manipulate WMS
 *  parameters received from the client.
 * \since QGIS 3.0
 */
namespace QgsWms
{
  struct QgsWmsParametersLayer
  {
    QString mNickname; // name, id or short name
    int mOpacity = -1;
    QStringList mFilter; // list of filter
    QStringList mSelection; // list of string fid
    QString mStyle;
  };

  struct QgsWmsParametersHighlightLayer
  {
    QString mName;
    QgsGeometry mGeom;
    QString mSld;
    QString mLabel;
    QColor mColor;
    int mSize = 0;
    int mWeight = 0;
    QString mFont;
    float mBufferSize = 0;
    QColor mBufferColor;
  };

  class QgsWmsParameters
  {
      Q_GADGET

    public:
      enum ParameterName
      {
        CRS, // instead of SRS for WMS 1.3.0
        // SRS, // for WMS 1.1.1
        WIDTH,
        HEIGHT,
        BBOX,
        LAYER,
        LAYERS,
        STYLE,
        STYLES,
        OPACITIES,
        SLD,
        FILTER,
        SELECTION,
        HIGHLIGHT_GEOM,
        HIGHLIGHT_SYMBOL,
        HIGHLIGHT_LABELSTRING,
        HIGHLIGHT_LABELFONT,
        HIGHLIGHT_LABELSIZE,
        HIGHLIGHT_LABELWEIGHT,
        HIGHLIGHT_LABELCOLOR,
        HIGHLIGHT_LABELBUFFERCOLOR,
        HIGHLIGHT_LABELBUFFERSIZE
      };
      Q_ENUM( ParameterName )

      struct Parameter
      {
        ParameterName mName;
        QVariant::Type mType;
        QVariant mDefaultValue;
        QVariant mValue;
      };

      /** Constructor.
       * \param map of parameters where keys are parameters' names.
       */
      QgsWmsParameters( const QgsServerRequest::Parameters &parameters );

      /** Constructor.
        */
      QgsWmsParameters();

      /** Loads new parameters.
       * \param map of parameters
       */
      void load( const QgsServerRequest::Parameters &parameters );

      /** Dumps parameters.
       */
      void dump() const;

      /** Returns CRS or an empty string if none is defined.
       * \returns crs parameter as string
       */
      QString crs() const;

      /** Returns WIDTH parameter as an int (0 if not defined). An exception is
       *  raised if it cannot be converted.
       * \returns width parameter
       * \throws QgsBadRequestException
       */
      int width() const;

      /** Returns HEIGHT parameter as an int (0 if not defined). An exception
       *  is raised if it cannot be converted.
       * \returns height parameter
       * \throws QgsBadRequestException
       */
      int height() const;

      /** Returns BBOX if defined or an empty string.
       * \returns bbox parameter
       */
      QString bbox() const;

      /** Returns BBOX as a rectangle if defined and valid. An exception is
       *  raised if the BBOX string cannot be converted into a rectangle.
       * \returns bbox as rectangle
       * \throws QgsBadRequestException
       */
      QgsRectangle bboxAsRectangle() const;

      /** Returns SLD if defined or an empty string.
       * \returns sld
       */
      QString sld() const;

      /** Returns the list of feature selection found in SELECTION parameter.
       * \returns the list of selection
       */
      QStringList selections() const;

      /** Returns the list of filters found in FILTER parameter.
       * \returns the list of filter
       */
      QStringList filters() const;

      /** Returns the list of opacities found in OPACITIES parameter.
       * \returns the list of opacities in string
       */
      QStringList opacities() const;

      /** Returns the list of opacities found in OPACITIES parameter as
       * integers. If an opacity cannot be converted into an integer, then an
       * exception is raised
       * \returns a list of opacities as integers
       * \throws QgsBadRequestException
       */
      QList<int> opacitiesAsInt() const;

      /** Returns nickname of layers found in LAYER and LAYERS parameters.
       * \returns nickname of layers
       */
      QStringList allLayersNickname() const;

      /** Returns styles found in STYLE and STYLES parameters.
       * \returns name of styles
       */
      QStringList allStyles() const;

      /** Returns parameters for each layer found in LAYER/LAYERS.
       * \returns layer parameters
       */
      QList<QgsWmsParametersLayer> layersParameters() const;

      /** Returns parameters for each highlight layer.
       * \returns parameters for each highlight layer
       */
      QList<QgsWmsParametersHighlightLayer> highlightLayersParameters() const;

      /** Returns HIGHLIGHT_GEOM as a list of string in WKT.
       * \returns highlight geometries
       */
      QStringList highlightGeom() const;

      /** Returns HIGHLIGHT_GEOM as a list of geometries. An exception is
       *  raised if an invalid geometry is found.
       * \returns highlight geometries
       * \throws QgsBadRequestException
       */
      QList<QgsGeometry> highlightGeomAsGeom() const;

      /** Returns HIGHLIGHT_SYMBOL as a list of string.
       * \returns highlight sld symbols
       */
      QStringList highlightSymbol() const;

      /** Returns HIGHLIGHT_LABELSTRING as a list of string.
       * \returns highlight label string
       */
      QStringList highlightLabelString() const;

      /** Returns HIGHLIGHT_LABELCOLOR as a list of string.
       * \returns highlight label color
       */
      QStringList highlightLabelColor() const;

      /** Returns HIGHLIGHT_LABELCOLOR as a list of color. An exception is
       *  raised if an invalid color is found.
       * \returns highlight label color
       * \throws QgsBadRequestException
       */
      QList<QColor> highlightLabelColorAsColor() const;

      /** Returns HIGHLIGHT_LABELSIZE as a list of string.
       * \returns highlight label size
       */
      QStringList highlightLabelSize() const;

      /** Returns HIGHLIGHT_LABELSIZE as a list of int An exception is raised
       *  if an invalid size is found.
       * \returns highlight label size
       * \throws QgsBadRequestException
       */
      QList<int> highlightLabelSizeAsInt() const;

      /** Returns HIGHLIGHT_LABELWEIGHT as a list of string.
       * \returns highlight label weight
       */
      QStringList highlightLabelWeight() const;

      /** Returns HIGHLIGHT_LABELWEIGHT as a list of int. An exception is
       *  raised if an invalid weight is found.
       * \returns highlight label weight
       * \throws QgsBadRequestException
       */
      QList<int> highlightLabelWeightAsInt() const;

      /** Returns HIGHLIGHT_LABELFONT
       * \returns highlight label font
       */
      QStringList highlightLabelFont() const;

      /** Returns HIGHLIGHT_LABELBUFFERSIZE
       * \returns highlight label buffer size
       */
      QStringList highlightLabelBufferSize() const;

      /** Returns HIGHLIGHT_LABELBUFFERSIZE as a list of float. An exception is
       *  raised if an invalid size is found.
       * \returns highlight label buffer size
       * \throws QgsBadRequestException
       */
      QList<float> highlightLabelBufferSizeAsFloat() const;

      /** Returns HIGHLIGHT_LABELBUFFERCOLOR as a list of string.
       * \returns highlight label buffer color
       */
      QStringList highlightLabelBufferColor() const;

      /** Returns HIGHLIGHT_LABELBUFFERCOLOR as a list of colors. An axception
       *  is raised if an invalid color is found.
       * \returns highlight label buffer color
       * \throws QgsBadRequestException
       */
      QList<QColor> highlightLabelBufferColorAsColor() const;

    private:
      QString name( ParameterName name ) const;
      void raiseError( ParameterName name ) const;
      void raiseError( const QString &msg ) const;
      void initParameters();
      QVariant value( ParameterName name ) const;
      void log( const QString &msg ) const;
      void save( const Parameter &parameter );
      QStringList toStringList( ParameterName name, char delimiter = ',' ) const;
      QList<int> toIntList( QStringList l, ParameterName name ) const;
      QList<float> toFloatList( QStringList l, ParameterName name ) const;
      QList<QColor> toColorList( QStringList l, ParameterName name ) const;

      QgsServerRequest::Parameters mRequestParameters;
      QMap<ParameterName, Parameter> mParameters;
  };
}

#endif
