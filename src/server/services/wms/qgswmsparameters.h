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
#include <QMetaEnum>
#include <QColor>

#include "qgsrectangle.h"
#include "qgslegendsettings.h"
#include "qgsprojectversion.h"
#include "qgsogcutils.h"
#include "qgsserverparameters.h"
#include "qgsdxfexport.h"

namespace QgsWms
{
  struct QgsWmsParametersFilter
  {
    //! Filter type
    enum Type
    {
      UNKNOWN,
      SQL,
      OGC_FE
    };

    QString mFilter;
    QgsWmsParametersFilter::Type mType = QgsWmsParametersFilter::UNKNOWN;
    QgsOgcUtils::FilterVersion mVersion = QgsOgcUtils::FILTER_OGC_1_0; // only if FE
  };

  struct QgsWmsParametersLayer
  {
    QString mNickname; // name, id or short name
    int mOpacity = -1;
    QList<QgsWmsParametersFilter> mFilter; // list of filter
    QStringList mSelection; // list of string fid
    QString mStyle;
    QString mExternalUri;
  };

  struct QgsWmsParametersExternalLayer
  {
    QString mName;
    QString mUri;
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
    double mLabelRotation = 0;
    double mLabelDistance = 2; //label distance from feature in mm
    QString mHali; //horizontal alignment
    QString mVali; //vertical alignment
  };

  struct QgsWmsParametersComposerMap
  {
    int mId = 0; // composer map id
    bool mHasExtent = false; // does the request contains extent for this composer map
    QgsRectangle mExtent; // the request extent for this composer map
    float mScale = -1;
    float mRotation = 0;
    float mGridX = 0;
    float mGridY = 0;
    QList<QgsWmsParametersLayer> mLayers; // list of layers for this composer map
    QList<QgsWmsParametersHighlightLayer> mHighlightLayers; // list of highlight layers for this composer map
  };

  /**
   * \ingroup server
   * \class QgsWmsParameter
   * \brief WMS parameter received from the client.
   * \since QGIS 3.4
   */
  class QgsWmsParameter : public QgsServerParameterDefinition
  {
      Q_GADGET

    public:
      //! Available parameters for WMS requests
      enum Name
      {
        UNKNOWN,
        BOXSPACE,
        CRS,
        SRS,
        WIDTH,
        HEIGHT,
        BBOX,
        ICONLABELSPACE,
        IMAGE_QUALITY,
        ITEMFONTFAMILY,
        ITEMFONTBOLD,
        ITEMFONTITALIC,
        ITEMFONTSIZE,
        ITEMFONTCOLOR,
        LAYER,
        LAYERFONTFAMILY,
        LAYERFONTBOLD,
        LAYERFONTITALIC,
        LAYERFONTSIZE,
        LAYERFONTCOLOR,
        LAYERTITLE,
        LAYERS,
        LAYERSPACE,
        LAYERTITLESPACE,
        QUERY_LAYERS,
        FEATURE_COUNT,
        SHOWFEATURECOUNT,
        STYLE,
        STYLES,
        SYMBOLSPACE,
        SYMBOLHEIGHT,
        SYMBOLWIDTH,
        OPACITIES,
        SLD,
        SLD_BODY,
        FI_POLYGON_TOLERANCE,
        FI_LINE_TOLERANCE,
        FI_POINT_TOLERANCE,
        FILTER,
        FILTER_GEOM,
        FORMAT,
        INFO_FORMAT,
        I,
        J,
        X,
        Y,
        RULE,
        RULELABEL,
        SCALE,
        SELECTION,
        HIGHLIGHT_GEOM,
        HIGHLIGHT_SYMBOL,
        HIGHLIGHT_LABELSTRING,
        HIGHLIGHT_LABELFONT,
        HIGHLIGHT_LABELSIZE,
        HIGHLIGHT_LABELWEIGHT,
        HIGHLIGHT_LABELCOLOR,
        HIGHLIGHT_LABELBUFFERCOLOR,
        HIGHLIGHT_LABELBUFFERSIZE,
        HIGHLIGHT_LABEL_ROTATION,
        HIGHLIGHT_LABEL_DISTANCE,
        HIGHLIGHT_LABEL_HORIZONTAL_ALIGNMENT,
        HIGHLIGHT_LABEL_VERTICAL_ALIGNMENT,
        WMS_PRECISION,
        TRANSPARENT,
        BGCOLOR,
        DPI,
        TEMPLATE,
        EXTENT,
        ROTATION,
        GRID_INTERVAL_X,
        GRID_INTERVAL_Y,
        WITH_GEOMETRY,
        WITH_MAPTIP,
        WMTVER,
        ATLAS_PK,
        FORMAT_OPTIONS,
        SRCWIDTH,
        SRCHEIGHT,
        TILED
      };
      Q_ENUM( Name )

      /**
       * Constructor for QgsWmsParameter.
       * \param name Name of the WMS parameter
       * \param type Type of the parameter
       * \param defaultValue Default value of the parameter
       */
      QgsWmsParameter( const QgsWmsParameter::Name name = QgsWmsParameter::UNKNOWN,
                       const QVariant::Type type = QVariant::String,
                       const QVariant defaultValue = QVariant( "" ) );

      /**
       * Default destructor for QgsWmsParameter.
       */
      virtual ~QgsWmsParameter() override = default;

      /**
       * Returns TRUE if the parameter is valid, FALSE otherwise.
       */
      bool isValid() const override;

      /**
       * Converts the parameter into a list of strings and keeps empty parts
       * Default style value is an empty string
       * \param delimiter The character used for delimiting
       * \returns A list of strings
       * \since QGIS 3.8
       */
      QStringList toStyleList( const char delimiter = ',' ) const;

      /**
       * Converts the parameter into a list of geometries.
       * \param delimiter The character delimiting string geometries
       * \returns A list of geometries
       * \throws QgsBadRequestException Invalid parameter exception
       */
      QList<QgsGeometry> toGeomList( const char delimiter = ',' ) const;

      /**
       * Converts the parameter into a list of integers.
       * \param delimiter The character delimiting string integers
       * \returns A list of integers
       * \throws QgsBadRequestException Invalid parameter exception
       */
      QList<int> toIntList( const char delimiter = ',' ) const;

      /**
       * Converts the parameter into a list of doubles.
       * \param delimiter The character delimiting string doubles
       * \returns A list of doubles
       * \throws QgsBadRequestException Invalid parameter exception
       */
      QList<double> toDoubleList( const char delimiter = ',' ) const;

      /**
       * Converts the parameter into a list of colors.
       * \param delimiter The character delimiting string colors
       * \returns A list of colors
       * \throws QgsBadRequestException Invalid parameter exception
       */
      QList<QColor> toColorList( const char delimiter = ',' ) const;

      /**
       * Converts the parameter into a rectangle.
       * \returns A rectangle
       * \throws QgsBadRequestException Invalid parameter exception
       */
      QgsRectangle toRectangle() const;

      /**
       * Converts the parameter into an integer.
       * \returns An integer
       * \throws QgsBadRequestException Invalid parameter exception
       */
      int toInt() const;

      /**
       * Converts the parameter into a double.
       * \returns A double
       * \throws QgsBadRequestException Invalid parameter exception
       */
      double toDouble() const;

      /**
       * Converts the parameter into a color.
       * \returns A color
       * \throws QgsBadRequestException Invalid parameter exception
       */
      QColor toColor() const;

      /**
       * Converts the parameter into an url.
       * \returns An url
       * \throws QgsBadRequestException Invalid parameter exception
       * \since QGIS 3.4
       */
      QUrl toUrl() const;

      /**
       * Loads the data associated to the parameter converted into an url.
       * \returns The content loaded
       * \throws QgsBadRequestException Invalid parameter exception
       * \since QGIS 3.4
       */
      QString loadUrl() const;

      /**
       * Raises an error in case of an invalid conversion.
       * \throws QgsBadRequestException Invalid parameter exception
       */
      void raiseError() const;

      /**
       * Returns the name of the parameter.
       * \since QGIS 3.8
       */
      QString name() const;

      /**
       * Converts a parameter's name into its string representation.
       */
      static QString name( const QgsWmsParameter::Name );

      /**
       * Converts a string into a parameter's name (UNKNOWN in case of an
       * invalid string).
       */
      static QgsWmsParameter::Name name( const QString &name );

      QgsWmsParameter::Name mName;
      int mId = -1;
  };

  /**
   * \ingroup server
   * \class QgsWms::QgsWmsParameters
   * \brief Provides an interface to retrieve and manipulate WMS parameters received from the client.
   * \since QGIS 3.0
   */
  class QgsWmsParameters : public QgsServerParameters
  {
      Q_GADGET

    public:

      //! Output format for the response
      enum Format
      {
        NONE,
        JPG,
        PNG,
        SVG,
        PDF,
        TEXT,
        XML,
        HTML,
        GML,
        JSON
      };
      Q_ENUM( Format )

      //! Options for DXF format
      enum DxfFormatOption
      {
        SCALE,
        MODE,
        LAYERATTRIBUTES,
        USE_TITLE_AS_LAYERNAME,
        CODEC,
        NO_MTEXT,
        FORCE_2D
      };
      Q_ENUM( DxfFormatOption )

      /**
       * Constructor for WMS parameters with specific values.
       * \param parameters Map of parameters where keys are parameters' names.
       */
      QgsWmsParameters( const QgsServerParameters &parameters );

      /**
       * Constructor for WMS parameters with default values only.
        */
      QgsWmsParameters();

      virtual ~QgsWmsParameters() override = default;

      /**
       * Returns the parameter corresponding to \a name.
       * \since QGIS 3.8
       */
      QgsWmsParameter operator[]( QgsWmsParameter::Name name ) const;

      /**
       * Sets a parameter \a value thanks to its \a name.
       * \since QGIS 3.8
       */
      void set( QgsWmsParameter::Name name, const QVariant &value );

      /**
       * Dumps parameters.
       */
      void dump() const;

      /**
       * Returns CRS or an empty string if none is defined.
       * \returns crs parameter as string
       */
      QString crs() const;

      /**
       * Returns WIDTH parameter or an empty string if not defined.
       * \returns width parameter
       */
      QString width() const;

      /**
       * Returns WIDTH parameter as an int or its default value if not
       * defined. An exception is raised if WIDTH is defined and cannot be
       * converted.
       * \returns width parameter
       * \throws QgsBadRequestException
       */
      int widthAsInt() const;

      /**
       * Returns HEIGHT parameter or an empty string if not defined.
       * \returns height parameter
       */
      QString height() const;

      /**
       * Returns HEIGHT parameter as an int or its default value if not
       * defined. An exception is raised if HEIGHT is defined and cannot be
       * converted.
       * \returns height parameter
       * \throws QgsBadRequestException
       */
      int heightAsInt() const;

      /**
       * Returns SRCWIDTH parameter or an empty string if not defined.
       * \returns srcWidth parameter
       * \since QGIS 3.8
       */
      QString srcWidth() const;

      /**
       * Returns SRCWIDTH parameter as an int or its default value if not
       * defined. An exception is raised if SRCWIDTH is defined and cannot be
       * converted.
       * \returns srcWidth parameter
       * \throws QgsBadRequestException
       * \since QGIS 3.8
       */
      int srcWidthAsInt() const;

      /**
       * Returns SRCHEIGHT parameter or an empty string if not defined.
       * \returns srcHeight parameter
       * \since QGIS 3.8
       */
      QString srcHeight() const;

      /**
       * Returns SRCHEIGHT parameter as an int or its default value if not
       * defined. An exception is raised if SRCHEIGHT is defined and cannot be
       * converted.
       * \returns srcHeight parameter
       * \throws QgsBadRequestException
       * \since QGIS 3.8
       */
      int srcHeightAsInt() const;

      /**
       * Returns VERSION parameter if defined or its default value.
       * \returns version
       */
      QgsProjectVersion versionAsNumber() const;

      /**
       * Returns TRUE if \a version is valid, FALSE otherwise.
       * \since QGIS 3.4
       */
      bool versionIsValid( const QString version ) const;

      /**
       * Returns BBOX if defined or an empty string.
       * \returns bbox parameter
       */
      QString bbox() const;

      /**
       * Returns BBOX as a rectangle if defined and valid. An exception is
       * raised if the BBOX string cannot be converted into a rectangle.
       * \returns bbox as rectangle
       * \throws QgsBadRequestException
       */
      QgsRectangle bboxAsRectangle() const;

      /**
       * Returns SLD_body if defined or an empty string.
       * \returns sld body
       */
      QString sldBody() const;

      /**
       * Returns the list of feature selection found in SELECTION parameter.
       * \returns the list of selection
       */
      QStringList selections() const;

      /**
       * Returns the list of filters found in FILTER parameter.
       * \returns the list of filter
       */
      QStringList filters() const;

      /**
       * Returns the filter geometry found in FILTER_GEOM parameter.
       * \returns the filter geometry as Well Known Text.
       */
      QString filterGeom() const;

      /**
       * Returns the list of opacities found in OPACITIES parameter.
       * \returns the list of opacities in string
       */
      QStringList opacities() const;

      /**
       * Returns the list of opacities found in OPACITIES parameter as
       * integers. If an opacity cannot be converted into an integer, then an
       * exception is raised
       * \returns a list of opacities as integers
       * \throws QgsBadRequestException
       */
      QList<int> opacitiesAsInt() const;

      /**
       * Returns nickname of layers found in LAYER and LAYERS parameters.
       * \returns nickname of layers
       */
      QStringList allLayersNickname() const;

      /**
       * Returns nickname of layers found in QUERY_LAYERS parameter.
       * \returns nickname of layers
       */
      QStringList queryLayersNickname() const;

      /**
       * Returns styles found in STYLE and STYLES parameters.
       * \returns name of styles
       */
      QStringList allStyles() const;

      /**
       * Returns parameters for each layer found in LAYER/LAYERS.
       * \returns layer parameters
       */
      QList<QgsWmsParametersLayer> layersParameters() const;

      /**
       * Returns FI_POLYGON_TOLERANCE parameter or an empty string if not
       * defined.
       * \since QGIS 3.4
       */
      QString polygonTolerance() const;

      /**
       * Returns FI_LINE_TOLERANCE parameter or an empty string if not
       * defined.
       * \since QGIS 3.4
       */
      QString lineTolerance() const;

      /**
       * Returns FI_POINT_TOLERANCE parameter or an empty string if not
       * defined.
       * \since QGIS 3.4
       */
      QString pointTolerance() const;

      /**
       * Returns FI_POLYGON_TOLERANCE parameter as an integer.
       * \throws QgsBadRequestException
       * \since QGIS 3.4
       */
      int polygonToleranceAsInt() const;

      /**
       * Returns FI_LINE_TOLERANCE parameter as an integer.
       * \throws QgsBadRequestException
       * \since QGIS 3.4
       */
      int lineToleranceAsInt() const;

      /**
       * Returns FI_POINT_TOLERANCE parameter as an integer.
       * \throws QgsBadRequestException
       * \since QGIS 3.4
       */
      int pointToleranceAsInt() const;

      /**
       * Returns FORMAT parameter as a string.
       * \returns FORMAT parameter as string
       */
      QString formatAsString() const;

      /**
       * Returns format parameter as a string.
       * \since QGIS 3.8
       */
      static QString formatAsString( Format format );

      /**
       * Returns format. If the FORMAT parameter is not used, then the
       * default value is PNG.
       * \returns format
       */
      Format format() const;

      /**
       * Returns INFO_FORMAT parameter as a string.
       * \returns INFO_FORMAT parameter as string
       */
      QString infoFormatAsString() const;

      /**
       * Checks if INFO_FORMAT parameter is one of the image formats (PNG, JPG).
       * \returns TRUE if the INFO_FORMAT is an image format
       */
      bool infoFormatIsImage() const;

      /**
       * Returns IMAGE_QUALITY parameter or an empty string if not
       * defined.
       * \since QGIS 3.4
       */
      QString imageQuality() const;

      /**
       * Returns IMAGE_QUALITY parameter as an integer.
       * \throws QgsBadRequestException
       * \since QGIS 3.4
       */
      int imageQualityAsInt() const;

      /**
       * Returns TILED parameter or an empty string if not
       * defined.
       * \since QGIS 3.10
       */
      QString tiled() const;

      /**
       * Returns TILED parameter as a boolean.
       * \throws QgsBadRequestException
       * \since QGIS 3.10
       */
      bool tiledAsBool() const;

      /**
       * Returns infoFormat. If the INFO_FORMAT parameter is not used, then the
       * default value is text/plain.
       * \returns infoFormat
       */
      Format infoFormat() const;

      /**
       * Returns the infoFormat version for GML. If the INFO_FORMAT is not GML,
       * then the default value is -1.
       * \returns infoFormat version
       */
      int infoFormatVersion() const;

      /**
       * Returns I parameter or an empty string if not defined.
       * \returns i parameter
       */
      QString i() const;

      /**
       * Returns I parameter as an int or its default value if not
       * defined. An exception is raised if I is defined and cannot be
       * converted.
       * \returns i parameter
       * \throws QgsBadRequestException
       */
      int iAsInt() const;

      /**
       * Returns J parameter or an empty string if not defined.
       * \returns j parameter
       */
      QString j() const;

      /**
       * Returns J parameter as an int or its default value if not
       * defined. An exception is raised if J is defined and cannot be
       * converted.
       * \returns j parameter
       * \throws QgsBadRequestException
       */
      int jAsInt() const;

      /**
       * Returns X parameter or an empty string if not defined.
       * \returns x parameter
       */
      QString x() const;

      /**
       * Returns X parameter as an int or its default value if not
       * defined. An exception is raised if X is defined and cannot be
       * converted.
       * \returns x parameter
       * \throws QgsBadRequestException
       */
      int xAsInt() const;

      /**
       * Returns Y parameter or an empty string if not defined.
       * \returns y parameter
       */
      QString y() const;

      /**
       * Returns Y parameter as an int or its default value if not
       * defined. An exception is raised if Y is defined and cannot be
       * converted.
       * \returns j parameter
       * \throws QgsBadRequestException
       */
      int yAsInt() const;

      /**
       * Returns RULE parameter or an empty string if none is defined
       * \returns RULE parameter or an empty string if none is defined
       */
      QString rule() const;

      /**
       * Returns RULELABEL parameter or an empty string if none is defined
       * \returns RULELABEL parameter or an empty string if none is defined
       */
      QString ruleLabel() const;

      /**
       * Returns RULELABEL as a bool. An exception is raised if an invalid
       * parameter is found.
       * \returns ruleLabel
       * \throws QgsBadRequestException
       */
      bool ruleLabelAsBool() const;

      /**
       * Returns SHOWFEATURECOUNT parameter or an empty string if none is defined
       * \returns SHOWFEATURECOUNT parameter or an empty string if none is defined
       */
      QString showFeatureCount() const;

      /**
       * Returns SHOWFEATURECOUNT as a bool. An exception is raised if an invalid
       * parameter is found.
       * \returns showFeatureCount
       * \throws QgsBadRequestException
       */
      bool showFeatureCountAsBool() const;

      /**
       * Returns FEATURE_COUNT parameter or an empty string if none is defined
       * \returns FEATURE_COUNT parameter or an empty string if none is defined
       */
      QString featureCount() const;

      /**
       * Returns FEATURE_COUNT as an integer. An exception is raised if an invalid
       * parameter is found.
       * \returns FeatureCount
       * \throws QgsBadRequestException
       */
      int featureCountAsInt() const;

      /**
       * Returns SCALE parameter or an empty string if none is defined
       * \returns SCALE parameter or an empty string if none is defined
       */
      QString scale() const;

      /**
       * Returns SCALE as a double. An exception is raised if an invalid
       * parameter is found.
       * \returns scale
       * \throws QgsBadRequestException
       */
      double scaleAsDouble() const;

      /**
       * Returns BOXSPACE parameter or an empty string if not defined.
       * \returns BOXSPACE parameter or an empty string if not defined.
       */
      QString boxSpace() const;

      /**
       * Returns BOXSPACE as a double or its default value if not defined.
       * An exception is raised if an invalid parameter is found.
       * \returns boxSpace
       * \throws QgsBadRequestException
       */
      double boxSpaceAsDouble() const;

      /**
       * Returns LAYERSPACE parameter or an empty string if not defined.
       * \returns LAYERSPACE parameter or an empty string if not defined.
       */
      QString layerSpace() const;

      /**
       * Returns LAYERSPACE as a double or its default value if not defined.
       * An exception is raised if an invalid parameter is found.
       * \returns layerSpace
       * \throws QgsBadRequestException
       */
      double layerSpaceAsDouble() const;

      /**
       * Returns LAYERTITLESPACE parameter or an empty string if not defined.
       * \returns LAYERTITLESPACE parameter or an empty string if not defined.
       */
      QString layerTitleSpace() const;

      /**
       * Returns LAYERTITLESPACE as a double. An exception is raised if an invalid
       * parameter is found.
       * \returns layerTitleSpace
       * \throws QgsBadRequestException
       */
      double layerTitleSpaceAsDouble() const;

      /**
       * Returns SYMBOLSPACE parameter or an empty string if not defined.
       * \returns SYMBOLSPACE parameter or an empty string if not defined.
       */
      QString symbolSpace() const;

      /**
       * Returns SYMBOLSPACE as a double or its default value if not defined.
       * An exception is raised if an invalid parameter is found.
       * \returns symbolSpace
       * \throws QgsBadRequestException
       */
      double symbolSpaceAsDouble() const;

      /**
       * Returns ICONLABELSPACE parameter or an empty string if not defined.
       * \returns ICONLABELSPACE parameter or an empty string if not defined.
       */
      QString iconLabelSpace() const;

      /**
       * Returns ICONLABELSPACE as a double or its default value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns iconLabelSpace
       * \throws QgsBadRequestException
       */
      double iconLabelSpaceAsDouble() const;

      /**
       * Returns SYMBOLWIDTH parameter or an empty string if not defined.
       * \returns SYMBOLWIDTH parameter or an empty string if not defined.
       */
      QString symbolWidth() const;

      /**
       * Returns SYMBOLWIDTH as a double or its default value if not defined.
       * An exception is raised if an invalid parameter is found.
       * \returns symbolWidth
       * \throws QgsBadRequestException
       */
      double symbolWidthAsDouble() const;

      /**
       * Returns SYMBOLHEIGHT parameter or an empty string if not defined.
       * \returns SYMBOLHEIGHT parameter or an empty string if not defined.
       */
      QString symbolHeight() const;

      /**
       * Returns SYMBOLHEIGHT as a double or its default value if not defined.
       * An exception is raised if an invalid parameter is found.
       * \returns symbolHeight
       * \throws QgsBadRequestException
       */
      double symbolHeightAsDouble() const;

      /**
       * Returns the layer font (built thanks to the LAYERFONTFAMILY,
       * LAYERFONTSIZE, LAYERFONTBOLD, ... parameters).
       * \returns layerFont
       */
      QFont layerFont() const;

      /**
       * Returns LAYERFONTFAMILY parameter or an empty string if not defined.
       * \returns LAYERFONTFAMILY parameter or an empty string if not defined.
       */
      QString layerFontFamily() const;

      /**
       * Returns LAYERFONTBOLD parameter or an empty string if not defined.
       * \returns LAYERFONTBOLD parameter or an empty string if not defined.
       */
      QString layerFontBold() const;

      /**
       * Returns LAYERFONTBOLD as a boolean or its default value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns layerFontBold
       * \throws QgsBadRequestException
       */
      bool layerFontBoldAsBool() const;

      /**
       * Returns LAYERFONTITALIC parameter or an empty string if not defined.
       * \returns LAYERFONTITALIC parameter or an empty string if not defined.
       */
      QString layerFontItalic() const;

      /**
       * Returns LAYERFONTITALIC as a boolean or its default value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns layerFontItalic
       * \throws QgsBadRequestException
       */
      bool layerFontItalicAsBool() const;

      /**
       * Returns LAYERFONTSIZE parameter or an empty string if not defined.
       * \returns LAYERFONTSIZE parameter or an empty string if not defined.
       */
      QString layerFontSize() const;

      /**
       * Returns LAYERFONTSIZE as a double. An exception is raised if an invalid
       * parameter is found.
       * \returns layerFontSize
       * \throws QgsBadRequestException
       */
      double layerFontSizeAsDouble() const;

      /**
       * Returns LAYERFONTCOLOR parameter or an empty string if not defined.
       * \returns LAYERFONTCOLOR parameter or an empty string if not defined.
       */
      QString layerFontColor() const;

      /**
       * Returns LAYERFONTCOLOR as a color or its defined value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns layerFontColor
       * \throws QgsBadRequestException
       */
      QColor layerFontColorAsColor() const;

      /**
       * Returns the item font (built thanks to the ITEMFONTFAMILY,
       * ITEMFONTSIZE, ITEMFONTBOLD, ... parameters).
       * \returns itemFont
       */
      QFont itemFont() const;

      /**
       * Returns ITEMFONTFAMILY parameter or an empty string if not defined.
       * \returns ITEMFONTFAMILY parameter or an empty string if not defined.
       */
      QString itemFontFamily() const;

      /**
       * Returns ITEMFONTBOLD parameter or an empty string if not defined.
       * \returns ITEMFONTBOLD parameter or an empty string if not defined.
       */
      QString itemFontBold() const;

      /**
       * Returns ITEMFONTBOLD as a boolean or its default value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns itemFontBold
       * \throws QgsBadRequestException
       */
      bool itemFontBoldAsBool() const;

      /**
       * Returns ITEMFONTITALIC parameter or an empty string if not defined.
       * \returns ITEMFONTITALIC parameter or an empty string if not defined.
       */
      QString itemFontItalic() const;

      /**
       * Returns ITEMFONTITALIC as a boolean or its default value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns itemFontItalic
       * \throws QgsBadRequestException
       */
      bool itemFontItalicAsBool() const;

      /**
       * Returns ITEMFONTSIZE parameter or an empty string if not defined.
       * \returns ITEMFONTSIZE parameter or an empty string if not defined.
       */
      QString itemFontSize() const;

      /**
       * Returns ITEMFONTSIZE as a double. An exception is raised if an
       * invalid parameter is found.
       * \returns itemFontSize
       * \throws QgsBadRequestException
       */
      double itemFontSizeAsDouble() const;

      /**
       * Returns ITEMFONTCOLOR parameter or an empty string if not defined.
       * \returns ITEMFONTCOLOR parameter or an empty string if not defined.
       */
      QString itemFontColor() const;

      /**
       * Returns ITEMFONTCOLOR as a color. An exception is raised if an
       * invalid parameter is found.
       * \returns itemFontColor
       * \throws QgsBadRequestException
       */
      QColor itemFontColorAsColor() const;

      /**
       * Returns LAYERTITLE parameter or an empty string if not defined.
       * \returns LAYERTITLE parameter or an empty string if not defined.
       */
      QString layerTitle() const;

      /**
       * Returns LAYERTITLE as a bool or its default value if not defined. An
       * exception is raised if an invalid parameter is found.
       * \returns layerTitle
       * \throws QgsBadRequestException
       */
      bool layerTitleAsBool() const;

      /**
       * Returns legend settings
       * \returns legend settings
       */
      QgsLegendSettings legendSettings() const;

      /**
       * Returns parameters for each highlight layer.
       * \returns parameters for each highlight layer
       */
      QList<QgsWmsParametersHighlightLayer> highlightLayersParameters() const;

      /**
       * Returns parameters for each external layer.
       * \since QGIS 3.8
       */
      QList<QgsWmsParametersExternalLayer> externalLayersParameters() const;

      /**
       * Returns HIGHLIGHT_GEOM as a list of string in WKT.
       * \returns highlight geometries
       */
      QStringList highlightGeom() const;

      /**
       * Returns HIGHLIGHT_GEOM as a list of geometries. An exception is
       * raised if an invalid geometry is found.
       * \returns highlight geometries
       * \throws QgsBadRequestException
       */
      QList<QgsGeometry> highlightGeomAsGeom() const;

      /**
       * Returns HIGHLIGHT_SYMBOL as a list of string.
       * \returns highlight sld symbols
       */
      QStringList highlightSymbol() const;

      /**
       * Returns HIGHLIGHT_LABELSTRING as a list of string.
       * \returns highlight label string
       */
      QStringList highlightLabelString() const;

      /**
       * Returns HIGHLIGHT_LABELCOLOR as a list of string.
       * \returns highlight label color
       */
      QStringList highlightLabelColor() const;

      /**
       * Returns HIGHLIGHT_LABELCOLOR as a list of color. An exception is
       * raised if an invalid color is found.
       * \returns highlight label color
       * \throws QgsBadRequestException
       */
      QList<QColor> highlightLabelColorAsColor() const;

      /**
       * Returns HIGHLIGHT_LABELSIZE as a list of string.
       * \returns highlight label size
       */
      QStringList highlightLabelSize() const;

      /**
       * Returns HIGHLIGHT_LABELSIZE as a list of int An exception is raised
       * if an invalid size is found.
       * \returns highlight label size
       * \throws QgsBadRequestException
       */
      QList<int> highlightLabelSizeAsInt() const;

      /**
       * Returns HIGHLIGHT_LABELWEIGHT as a list of string.
       * \returns highlight label weight
       */
      QStringList highlightLabelWeight() const;

      /**
       * Returns HIGHLIGHT_LABELWEIGHT as a list of int. An exception is
       * raised if an invalid weight is found.
       * \returns highlight label weight
       * \throws QgsBadRequestException
       */
      QList<int> highlightLabelWeightAsInt() const;

      /**
       * Returns HIGHLIGHT_LABELFONT
       * \returns highlight label font
       */
      QStringList highlightLabelFont() const;

      /**
       * Returns HIGHLIGHT_LABELBUFFERSIZE
       * \returns highlight label buffer size
       */
      QStringList highlightLabelBufferSize() const;

      /**
       * Returns HIGHLIGHT_LABELBUFFERSIZE as a list of float. An exception is
       * raised if an invalid size is found.
       * \returns highlight label buffer size
       * \throws QgsBadRequestException
       */
      QList<double> highlightLabelBufferSizeAsFloat() const;

      /**
       * Returns HIGHLIGHT_LABELBUFFERCOLOR as a list of string.
       * \returns highlight label buffer color
       */
      QStringList highlightLabelBufferColor() const;

      /**
       * Returns HIGHLIGHT_LABELBUFFERCOLOR as a list of colors. An axception
       * is raised if an invalid color is found.
       * \returns highlight label buffer color
       * \throws QgsBadRequestException
       */
      QList<QColor> highlightLabelBufferColorAsColor() const;

      /**
       * Returns HIGHLIGHT_LABEL_ROTATION as a list of double.
       * \returns highlight label rotation
       */
      QList<double> highlightLabelRotation() const;

      /**
       * Returns HIGHLIGHT_LABEL_DISTANCE as a list of double.
       * \returns highlight label distance
       */
      QList<double> highlightLabelDistance() const;

      /**
       * Returns HIGHLIGHT_LABEL_HORIZONTAL_ALIGNMENT as a list of string.
       * \returns highlight label horizontal alignment strings
       */
      QStringList highlightLabelHorizontalAlignment() const;

      /**
       * Returns HIGHLIGHT_LABEL_VERTICAL_ALIGNMENT as a list of string.
       * \returns highlight label vertical alignment strings
       */
      QStringList highlightLabelVerticalAlignment() const;

      /**
       * Returns WMS_PRECISION parameter or an empty string if not defined.
       * \returns wms precision parameter
       */
      QString wmsPrecision() const;

      /**
       * Returns WMS_PRECISION parameter as an int or its default value if not
       * defined. An exception is raised if WMS_PRECISION is defined and cannot be
       * converted.
       * \returns wms precision parameter
       * \throws QgsBadRequestException
       */
      int wmsPrecisionAsInt() const;

      /**
       * Returns TRANSPARENT parameter or an empty string if not defined.
       * \returns TRANSPARENT parameter
       */
      QString transparent() const;

      /**
       * Returns TRANSPARENT parameter as a bool or its default value if not
       * defined. An exception is raised if TRANSPARENT is defined and cannot
       * be converted.
       * \returns transparent parameter
       * \throws QgsBadRequestException
       */
      bool transparentAsBool() const;

      /**
       * Returns BGCOLOR parameter or an empty string if not defined.
       * \returns BGCOLOR parameter
       */
      QString backgroundColor() const;

      /**
       * Returns BGCOLOR parameter as a QColor or its default value if not
       * defined. An exception is raised if BGCOLOR is defined and cannot
       * be converted.
       * \returns background color parameter
       * \throws QgsBadRequestException
       */
      QColor backgroundColorAsColor() const;

      /**
       * Returns DPI parameter or an empty string if not defined.
       * \returns DPI parameter
       */
      QString dpi() const;

      /**
       * Returns DPI parameter as an int or its default value if not
       * defined. An exception is raised if DPI is defined and cannot
       * be converted.
       * \returns dpi parameter
       * \throws QgsBadRequestException
       */
      double dpiAsDouble() const;

      /**
       * Returns TEMPLATE parameter or an empty string if not defined.
       * \returns TEMPLATE parameter
       */
      QString composerTemplate() const;

      /**
       * Returns the requested parameters for a composer map parameter.
       * An exception is raised if parameters are defined and cannot be
       * converted like EXTENT, SCALE, ROTATION, GRID_INTERVAL_X and
       * GRID_INTERVAL_Y.
       * \param mapId the composer map id.
       * \returns parameters for the composer map.
       * \throws QgsBadRequestException
       */
      QgsWmsParametersComposerMap composerMapParameters( int mapId ) const;

      /**
       * Returns the external WMS uri
       * \param id the id of the external wms
       * \return uri string or an empty string if the external wms id does not exist
       */
      QString externalWMSUri( const QString &id ) const;

      /**
       * Returns if the client wants the feature info response with geometry information
       * \returns TRUE if geometry information is requested for feature info response
       */
      bool withGeometry() const;

      /**
       * \brief withMapTip
       * \returns TRUE if maptip information is requested for feature info response
       */
      bool withMapTip() const;

      /**
       * Returns WMTVER parameter or an empty string if not defined.
       * \since QGIS 3.4
       */
      QString wmtver() const;

      /**
       * Returns a layout parameter thanks to its \a id.
       * \param id Parameter id
       * \param ok TRUE if the parameter is valid, FALSE otherwise
       * \returns The layout parameter
       * \since QGIS 3.4
       */
      QString layoutParameter( const QString &id, bool &ok ) const;

      /**
       * Returns the ATLAS_PK parameter
       * \returns The ATLAS_PK parameter
       * \since QGIS 3.6
      */
      QStringList atlasPk() const;

      /**
       * Returns a map of DXF options defined within FORMAT_OPTIONS parameter.
       * \since QGIS 3.8
       */
      QMap<DxfFormatOption, QString> dxfFormatOptions() const;

      /**
       * Returns the DXF LAYERATTRIBUTES parameter.
       * \since QGIS 3.8
       */
      QStringList dxfLayerAttributes() const;

      /**
       * Returns the DXF USE_TITLE_AS_LAYERNAME parameter.
       * \since QGIS 3.8
       */
      bool dxfUseLayerTitleAsName() const;

      /**
       * Returns the DXF SCALE parameter.
       * \since QGIS 3.8
       */
      double dxfScale() const;

      /**
       * Returns the DXF MODE parameter.
       * \since QGIS 3.8
       */
      QgsDxfExport::SymbologyExport dxfMode() const;

      /**
       * Returns the DXF CODEC parameter.
       * \since QGIS 3.8
       */
      QString dxfCodec() const;

      /**
       * Returns the dimensions parameter.
       * \since QGIS 3.10
       */
      QMap<QString, QString> dimensionValues() const;

      /**
       * \returns true if the FORCE_MTEXT parameter is set and the DXF should
       * be produced with MTEXT instead of TEXT.
       *
       * \since QGIS 3.12
       */
      bool noMText() const;

      /**
       * \returns true if the FORCE_2D parameter is set and the DXF should
       * be produced in 2D.
       *
       * \since QGIS 3.12
       */
      bool isForce2D() const;

      QString version() const override;

      QString request() const override;

    private:
      static bool isExternalLayer( const QString &name );

      bool loadParameter( const QString &name, const QString &value ) override;

      void save( const QgsWmsParameter &parameter, bool multi = false );

      QgsWmsParameter idParameter( QgsWmsParameter::Name name, int id ) const;

      void raiseError( const QString &msg ) const;
      void log( const QString &msg ) const;

      QgsWmsParametersExternalLayer externalLayerParameter( const QString &name ) const;

      QMultiMap<QString, QgsWmsParametersFilter> layerFilters( const QStringList &layers ) const;


      QMap<QgsWmsParameter::Name, QgsWmsParameter> mWmsParameters;
      QMap<QString, QMap<QString, QString> > mExternalWMSParameters;
      QList<QgsProjectVersion> mVersions;
  };
}

#endif
