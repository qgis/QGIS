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

#include "qgslegendsettings.h"
#include "qgsogcutils.h"
#include "qgsprojectversion.h"
#include "qgsrectangle.h"
#include "qgsserverparameters.h"

#include <QColor>
#include <QMap>
#include <QMetaEnum>

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
      QStringList mSelection;                // list of string fid
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
      QString mHali;             //horizontal alignment
      QString mVali;             //vertical alignment
  };

  struct QgsWmsParametersComposerMap
  {
      int mId = 0;             // composer map id
      bool mHasExtent = false; // does the request contains extent for this composer map
      QgsRectangle mExtent;    // the request extent for this composer map
      float mScale = -1;
      float mRotation = 0;
      float mGridX = 0;
      float mGridY = 0;
      QList<QgsWmsParametersLayer> mLayers;                   // list of layers for this composer map
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
        SHOWRULEDETAILS,
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
        WITH_DISPLAY_NAME,
        WMTVER,
        ATLAS_PK,
        FORMAT_OPTIONS,
        SRCWIDTH,
        SRCHEIGHT,
        TILED,
        ADDLAYERGROUPS
      };
      Q_ENUM( Name )

      /**
       * Constructor for QgsWmsParameter.
       * \param name Name of the WMS parameter
       * \param type Type of the parameter
       * \param defaultValue Default value of the parameter
       */
      QgsWmsParameter( const QgsWmsParameter::Name name = QgsWmsParameter::UNKNOWN, const QMetaType::Type type = QMetaType::Type::QString, const QVariant defaultValue = QVariant( "" ) );

      /**
       * Default destructor for QgsWmsParameter.
       */
      ~QgsWmsParameter() override = default;

      /**
       * Returns TRUE if the parameter is valid, FALSE otherwise.
       */
      [[nodiscard]] bool isValid() const override;

      /**
       * Converts the parameter into a list of strings and keeps empty parts
       * Default style value is an empty string
       * \param delimiter The character used for delimiting
       * \param skipEmptyParts for splitting
       * \returns A list of strings
       * \since QGIS 3.8
       */
      [[nodiscard]] QStringList toStyleList( const char delimiter = ',', bool skipEmptyParts = false ) const;

      /**
       * Converts the parameter into a list of geometries.
       * \param delimiter The character delimiting string geometries
       * \param skipEmptyParts for splitting
       * \returns A list of geometries
       * \throws QgsBadRequestException Invalid parameter exception
       */
      [[nodiscard]] QList<QgsGeometry> toGeomList( const char delimiter = ',', bool skipEmptyParts = true ) const;

      /**
       * Converts the parameter into a list of integers.
       * \param delimiter The character delimiting string integers
       * \param skipEmptyParts for splitting
       * \returns A list of integers
       * \throws QgsBadRequestException Invalid parameter exception
       */
      [[nodiscard]] QList<int> toIntList( const char delimiter = ',', bool skipEmptyParts = true ) const;

      /**
       * Converts the parameter into a list of doubles.
       * \param delimiter The character delimiting string doubles
       * \param skipEmptyParts for splitting
       * \returns A list of doubles
       * \throws QgsBadRequestException Invalid parameter exception
       */
      [[nodiscard]] QList<double> toDoubleList( const char delimiter = ',', bool skipEmptyParts = true ) const;

      /**
       * Converts the parameter into a list of colors.
       * \param delimiter The character delimiting string colors
       * \param skipEmptyParts for splitting
       * \returns A list of colors
       * \throws QgsBadRequestException Invalid parameter exception
       */
      [[nodiscard]] QList<QColor> toColorList( const char delimiter = ',', bool skipEmptyParts = true ) const;

      /**
       * Converts the parameter into a rectangle.
       * \returns A rectangle
       * \throws QgsBadRequestException Invalid parameter exception
       */
      [[nodiscard]] QgsRectangle toRectangle() const;

      /**
       * Converts the parameter into an integer.
       * \returns An integer
       * \throws QgsBadRequestException Invalid parameter exception
       */
      [[nodiscard]] int toInt() const;

      /**
       * Converts the parameter into a double.
       * \returns A double
       * \throws QgsBadRequestException Invalid parameter exception
       */
      [[nodiscard]] double toDouble() const;

      /**
       * Converts the parameter into a color.
       * \returns A color
       * \throws QgsBadRequestException Invalid parameter exception
       */
      [[nodiscard]] QColor toColor() const;

      /**
       * Converts the parameter into an url.
       * \returns An url
       * \throws QgsBadRequestException Invalid parameter exception
       * \since QGIS 3.4
       */
      [[nodiscard]] QUrl toUrl() const;

      /**
       * Loads the data associated to the parameter converted into an url.
       * \returns The content loaded
       * \throws QgsBadRequestException Invalid parameter exception
       * \since QGIS 3.4
       */
      [[nodiscard]] QString loadUrl() const;

      /**
       * Raises an error in case of an invalid conversion.
       * \throws QgsBadRequestException Invalid parameter exception
       */
      void raiseError() const;

      /**
       * Returns the name of the parameter.
       * \since QGIS 3.8
       */
      [[nodiscard]] QString name() const;

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

      //! Map id for prefixed parameters (e.g. "0" for "map0:LAYERS" in GetPrint requests)
      int mMapId = -1;
  };

  /**
   * \ingroup server
   * \class QgsWms::QgsWmsParameters
   * \brief Provides an interface to retrieve and manipulate WMS parameters received from the client.
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
        FORCE_2D,
        EXPORT_LINES_WITH_ZERO_WIDTH
      };
      Q_ENUM( DxfFormatOption )

      enum PdfFormatOption
      {
        RASTERIZE_WHOLE_IMAGE,
        FORCE_VECTOR_OUTPUT,
        APPEND_GEOREFERENCE,
        EXPORT_METADATA,
        TEXT_RENDER_FORMAT,
        SIMPLIFY_GEOMETRY,
        WRITE_GEO_PDF,
        USE_ISO_32000_EXTENSION_FORMAT_GEOREFERENCING,
        USE_OGC_BEST_PRACTICE_FORMAT_GEOREFERENCING,
        INCLUDE_GEO_PDF_FEATURES,
        EXPORT_THEMES,
        PREDEFINED_MAP_SCALES,
        LOSSLESS_IMAGE_COMPRESSION,
        DISABLE_TILED_RASTER_RENDERING
      };
      Q_ENUM( PdfFormatOption )

      /**
       * Constructor for WMS parameters with specific values.
       * \param parameters Map of parameters where keys are parameters' names.
       */
      QgsWmsParameters( const QgsServerParameters &parameters );

      /**
       * Constructor for WMS parameters with default values only.
        */
      QgsWmsParameters();

      ~QgsWmsParameters() override = default;

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
      [[nodiscard]] QString crs() const;

      /**
       * Returns WIDTH parameter or an empty string if not defined.
       * \returns width parameter
       */
      [[nodiscard]] QString width() const;

      /**
       * Returns WIDTH parameter as an int or its default value if not
       * defined. An exception is raised if WIDTH is defined and cannot be
       * converted.
       * \returns width parameter
       * \throws QgsBadRequestException
       */
      [[nodiscard]] int widthAsInt() const;

      /**
       * Returns HEIGHT parameter or an empty string if not defined.
       * \returns height parameter
       */
      [[nodiscard]] QString height() const;

      /**
       * Returns HEIGHT parameter as an int or its default value if not
       * defined. An exception is raised if HEIGHT is defined and cannot be
       * converted.
       * \returns height parameter
       * \throws QgsBadRequestException
       */
      [[nodiscard]] int heightAsInt() const;

      /**
       * Returns SHOWRULEDETAILS as a bool. An exception is raised if an invalid
       * parameter is found.
       * \since QGIS 3.36
       */
      [[nodiscard]] bool showRuleDetailsAsBool() const;

      /**
       * Returns SRCWIDTH parameter or an empty string if not defined.
       * \returns srcWidth parameter
       * \since QGIS 3.8
       */
      [[nodiscard]] QString srcWidth() const;

      /**
       * Returns SRCWIDTH parameter as an int or its default value if not
       * defined. An exception is raised if SRCWIDTH is defined and cannot be
       * converted.
       * \returns srcWidth parameter
       * \throws QgsBadRequestException
       * \since QGIS 3.8
       */
      [[nodiscard]] int srcWidthAsInt() const;

      /**
       * Returns SRCHEIGHT parameter or an empty string if not defined.
       * \returns srcHeight parameter
       * \since QGIS 3.8
       */
      [[nodiscard]] QString srcHeight() const;

      /**
       * Returns SRCHEIGHT parameter as an int or its default value if not
       * defined. An exception is raised if SRCHEIGHT is defined and cannot be
       * converted.
       * \returns srcHeight parameter
       * \throws QgsBadRequestException
       * \since QGIS 3.8
       */
      [[nodiscard]] int srcHeightAsInt() const;

      /**
       * Returns VERSION parameter if defined or its default value.
       * \returns version
       */
      [[nodiscard]] QgsProjectVersion versionAsNumber() const;

      /**
       * Returns TRUE if \a version is valid, FALSE otherwise.
       * \since QGIS 3.4
       */
      [[nodiscard]] bool versionIsValid( const QString version ) const;

      /**
       * Returns BBOX if defined or an empty string.
       * \returns bbox parameter
       */
      [[nodiscard]] QString bbox() const;

      /**
       * Returns BBOX as a rectangle if defined and valid. An exception is
       * raised if the BBOX string cannot be converted into a rectangle.
       * \returns bbox as rectangle
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QgsRectangle bboxAsRectangle() const;

      /**
       * Returns SLD_body if defined or an empty string.
       * \returns sld body
       */
      [[nodiscard]] QString sldBody() const;

      /**
       * Returns the list of feature selection found in SELECTION parameter.
       * \returns the list of selection
       */
      [[nodiscard]] QStringList selections() const;

      /**
       * Returns the list of filters found in FILTER parameter.
       * \returns the list of filter
       */
      [[nodiscard]] QStringList filters() const;

      /**
       * Returns the filter geometry found in FILTER_GEOM parameter.
       * \returns the filter geometry as Well Known Text.
       */
      [[nodiscard]] QString filterGeom() const;

      /**
       * Returns the list of opacities found in OPACITIES parameter.
       * \returns the list of opacities in string
       */
      [[nodiscard]] QStringList opacities() const;

      /**
       * Returns the list of opacities found in OPACITIES parameter as
       * integers. If an opacity cannot be converted into an integer, then an
       * exception is raised
       * \returns a list of opacities as integers
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QList<int> opacitiesAsInt() const;

      /**
       * Returns nickname of layers found in LAYER and LAYERS parameters.
       * \returns nickname of layers
       */
      [[nodiscard]] QStringList allLayersNickname() const;

      /**
       * Returns nickname of layers found in QUERY_LAYERS parameter.
       * \returns nickname of layers
       */
      [[nodiscard]] QStringList queryLayersNickname() const;

      /**
       * Returns styles found in STYLE and STYLES parameters.
       * \returns name of styles
       */
      [[nodiscard]] QStringList allStyles() const;

      /**
       * Returns parameters for each layer found in LAYER/LAYERS.
       * \returns layer parameters
       */
      [[nodiscard]] QList<QgsWmsParametersLayer> layersParameters() const;

      /**
       * Returns FI_POLYGON_TOLERANCE parameter or an empty string if not
       * defined.
       * \since QGIS 3.4
       */
      [[nodiscard]] QString polygonTolerance() const;

      /**
       * Returns FI_LINE_TOLERANCE parameter or an empty string if not
       * defined.
       * \since QGIS 3.4
       */
      [[nodiscard]] QString lineTolerance() const;

      /**
       * Returns FI_POINT_TOLERANCE parameter or an empty string if not
       * defined.
       * \since QGIS 3.4
       */
      [[nodiscard]] QString pointTolerance() const;

      /**
       * Returns FI_POLYGON_TOLERANCE parameter as an integer.
       * \throws QgsBadRequestException
       * \since QGIS 3.4
       */
      [[nodiscard]] int polygonToleranceAsInt() const;

      /**
       * Returns FI_LINE_TOLERANCE parameter as an integer.
       * \throws QgsBadRequestException
       * \since QGIS 3.4
       */
      [[nodiscard]] int lineToleranceAsInt() const;

      /**
       * Returns FI_POINT_TOLERANCE parameter as an integer.
       * \throws QgsBadRequestException
       * \since QGIS 3.4
       */
      [[nodiscard]] int pointToleranceAsInt() const;

      /**
       * Returns FORMAT parameter as a string.
       * \returns FORMAT parameter as string
       */
      [[nodiscard]] QString formatAsString() const;

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
      [[nodiscard]] Format format() const;

      /**
       * Returns INFO_FORMAT parameter as a string.
       * \returns INFO_FORMAT parameter as string
       */
      [[nodiscard]] QString infoFormatAsString() const;

      /**
       * Checks if INFO_FORMAT parameter is one of the image formats (PNG, JPG).
       * \returns TRUE if the INFO_FORMAT is an image format
       */
      [[nodiscard]] bool infoFormatIsImage() const;

      /**
       * Returns IMAGE_QUALITY parameter or an empty string if not
       * defined.
       * \since QGIS 3.4
       */
      [[nodiscard]] QString imageQuality() const;

      /**
       * Returns IMAGE_QUALITY parameter as an integer.
       * \throws QgsBadRequestException
       * \since QGIS 3.4
       */
      [[nodiscard]] int imageQualityAsInt() const;

      /**
       * Returns TILED parameter or an empty string if not
       * defined.
       * \since QGIS 3.10
       */
      [[nodiscard]] QString tiled() const;

      /**
       * Returns TILED parameter as a boolean.
       * \throws QgsBadRequestException
       * \since QGIS 3.10
       */
      [[nodiscard]] bool tiledAsBool() const;

      /**
       * Returns true if layer groups shall be added to GetLegendGraphic results
       */
      [[nodiscard]] bool addLayerGroups() const;

      /**
       * Returns infoFormat. If the INFO_FORMAT parameter is not used, then the
       * default value is text/plain.
       * \returns infoFormat
       */
      [[nodiscard]] Format infoFormat() const;

      /**
       * Returns the infoFormat version for GML. If the INFO_FORMAT is not GML,
       * then the default value is -1.
       * \returns infoFormat version
       */
      [[nodiscard]] int infoFormatVersion() const;

      /**
       * Returns I parameter or an empty string if not defined.
       * \returns i parameter
       */
      [[nodiscard]] QString i() const;

      /**
       * Returns I parameter as an int or its default value if not
       * defined. An exception is raised if I is defined and cannot be
       * converted.
       * \returns i parameter
       * \throws QgsBadRequestException
       */
      [[nodiscard]] int iAsInt() const;

      /**
       * Returns J parameter or an empty string if not defined.
       * \returns j parameter
       */
      [[nodiscard]] QString j() const;

      /**
       * Returns J parameter as an int or its default value if not
       * defined. An exception is raised if J is defined and cannot be
       * converted.
       * \returns j parameter
       * \throws QgsBadRequestException
       */
      [[nodiscard]] int jAsInt() const;

      /**
       * Returns X parameter or an empty string if not defined.
       * \returns x parameter
       */
      [[nodiscard]] QString x() const;

      /**
       * Returns X parameter as an int or its default value if not
       * defined. An exception is raised if X is defined and cannot be
       * converted.
       * \returns x parameter
       * \throws QgsBadRequestException
       */
      [[nodiscard]] int xAsInt() const;

      /**
       * Returns Y parameter or an empty string if not defined.
       * \returns y parameter
       */
      [[nodiscard]] QString y() const;

      /**
       * Returns Y parameter as an int or its default value if not
       * defined. An exception is raised if Y is defined and cannot be
       * converted.
       * \returns j parameter
       * \throws QgsBadRequestException
       */
      [[nodiscard]] int yAsInt() const;

      /**
       * Returns RULE parameter or an empty string if none is defined
       * \returns RULE parameter or an empty string if none is defined
       */
      [[nodiscard]] QString rule() const;

      /**
       * Returns RULELABEL parameter or an empty string if none is defined
       * \returns RULELABEL parameter or an empty string if none is defined
       */
      [[nodiscard]] QString ruleLabel() const;

      /**
       * Returns RULELABEL as a bool. An exception is raised if an invalid
       * parameter is found.
       * \returns ruleLabel
       * \throws QgsBadRequestException
       */
      [[nodiscard]] bool ruleLabelAsBool() const;

      /**
       * Returns SHOWFEATURECOUNT parameter or an empty string if none is defined
       * \returns SHOWFEATURECOUNT parameter or an empty string if none is defined
       */
      [[nodiscard]] QString showFeatureCount() const;

      /**
       * Returns SHOWFEATURECOUNT as a bool. An exception is raised if an invalid
       * parameter is found.
       * \returns showFeatureCount
       * \throws QgsBadRequestException
       */
      [[nodiscard]] bool showFeatureCountAsBool() const;

      /**
       * Returns FEATURE_COUNT parameter or an empty string if none is defined
       * \returns FEATURE_COUNT parameter or an empty string if none is defined
       */
      [[nodiscard]] QString featureCount() const;

      /**
       * Returns FEATURE_COUNT as an integer. An exception is raised if an invalid
       * parameter is found.
       * \returns FeatureCount
       * \throws QgsBadRequestException
       */
      [[nodiscard]] int featureCountAsInt() const;

      /**
       * Returns SCALE parameter or an empty string if none is defined
       * \returns SCALE parameter or an empty string if none is defined
       */
      [[nodiscard]] QString scale() const;

      /**
       * Returns SCALE as a double. An exception is raised if an invalid
       * parameter is found.
       * \returns scale
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double scaleAsDouble() const;

      /**
       * Returns BOXSPACE parameter or an empty string if not defined.
       * \returns BOXSPACE parameter or an empty string if not defined.
       */
      [[nodiscard]] QString boxSpace() const;

      /**
       * Returns BOXSPACE as a double or its default value if not defined.
       * An exception is raised if an invalid parameter is found.
       * \returns boxSpace
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double boxSpaceAsDouble() const;

      /**
       * Returns LAYERSPACE parameter or an empty string if not defined.
       * \returns LAYERSPACE parameter or an empty string if not defined.
       */
      [[nodiscard]] QString layerSpace() const;

      /**
       * Returns LAYERSPACE as a double or its default value if not defined.
       * An exception is raised if an invalid parameter is found.
       * \returns layerSpace
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double layerSpaceAsDouble() const;

      /**
       * Returns LAYERTITLESPACE parameter or an empty string if not defined.
       * \returns LAYERTITLESPACE parameter or an empty string if not defined.
       */
      [[nodiscard]] QString layerTitleSpace() const;

      /**
       * Returns LAYERTITLESPACE as a double. An exception is raised if an invalid
       * parameter is found.
       * \returns layerTitleSpace
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double layerTitleSpaceAsDouble() const;

      /**
       * Returns SYMBOLSPACE parameter or an empty string if not defined.
       * \returns SYMBOLSPACE parameter or an empty string if not defined.
       */
      [[nodiscard]] QString symbolSpace() const;

      /**
       * Returns SYMBOLSPACE as a double or its default value if not defined.
       * An exception is raised if an invalid parameter is found.
       * \returns symbolSpace
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double symbolSpaceAsDouble() const;

      /**
       * Returns ICONLABELSPACE parameter or an empty string if not defined.
       * \returns ICONLABELSPACE parameter or an empty string if not defined.
       */
      [[nodiscard]] QString iconLabelSpace() const;

      /**
       * Returns ICONLABELSPACE as a double or its default value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns iconLabelSpace
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double iconLabelSpaceAsDouble() const;

      /**
       * Returns SYMBOLWIDTH parameter or an empty string if not defined.
       * \returns SYMBOLWIDTH parameter or an empty string if not defined.
       */
      [[nodiscard]] QString symbolWidth() const;

      /**
       * Returns SYMBOLWIDTH as a double or its default value if not defined.
       * An exception is raised if an invalid parameter is found.
       * \returns symbolWidth
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double symbolWidthAsDouble() const;

      /**
       * Returns SYMBOLHEIGHT parameter or an empty string if not defined.
       * \returns SYMBOLHEIGHT parameter or an empty string if not defined.
       */
      [[nodiscard]] QString symbolHeight() const;

      /**
       * Returns SYMBOLHEIGHT as a double or its default value if not defined.
       * An exception is raised if an invalid parameter is found.
       * \returns symbolHeight
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double symbolHeightAsDouble() const;

      /**
       * Returns the layer font (built thanks to the LAYERFONTFAMILY,
       * LAYERFONTSIZE, LAYERFONTBOLD, ... parameters).
       * \returns layerFont
       */
      [[nodiscard]] QFont layerFont() const;

      /**
       * Returns LAYERFONTFAMILY parameter or an empty string if not defined.
       * \returns LAYERFONTFAMILY parameter or an empty string if not defined.
       */
      [[nodiscard]] QString layerFontFamily() const;

      /**
       * Returns LAYERFONTBOLD parameter or an empty string if not defined.
       * \returns LAYERFONTBOLD parameter or an empty string if not defined.
       */
      [[nodiscard]] QString layerFontBold() const;

      /**
       * Returns LAYERFONTBOLD as a boolean or its default value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns layerFontBold
       * \throws QgsBadRequestException
       */
      [[nodiscard]] bool layerFontBoldAsBool() const;

      /**
       * Returns LAYERFONTITALIC parameter or an empty string if not defined.
       * \returns LAYERFONTITALIC parameter or an empty string if not defined.
       */
      [[nodiscard]] QString layerFontItalic() const;

      /**
       * Returns LAYERFONTITALIC as a boolean or its default value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns layerFontItalic
       * \throws QgsBadRequestException
       */
      [[nodiscard]] bool layerFontItalicAsBool() const;

      /**
       * Returns LAYERFONTSIZE parameter or an empty string if not defined.
       * \returns LAYERFONTSIZE parameter or an empty string if not defined.
       */
      [[nodiscard]] QString layerFontSize() const;

      /**
       * Returns LAYERFONTSIZE as a double. An exception is raised if an invalid
       * parameter is found.
       * \returns layerFontSize
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double layerFontSizeAsDouble() const;

      /**
       * Returns LAYERFONTCOLOR parameter or an empty string if not defined.
       * \returns LAYERFONTCOLOR parameter or an empty string if not defined.
       */
      [[nodiscard]] QString layerFontColor() const;

      /**
       * Returns LAYERFONTCOLOR as a color or its defined value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns layerFontColor
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QColor layerFontColorAsColor() const;

      /**
       * Returns the item font (built thanks to the ITEMFONTFAMILY,
       * ITEMFONTSIZE, ITEMFONTBOLD, ... parameters).
       * \returns itemFont
       */
      [[nodiscard]] QFont itemFont() const;

      /**
       * Returns ITEMFONTFAMILY parameter or an empty string if not defined.
       * \returns ITEMFONTFAMILY parameter or an empty string if not defined.
       */
      [[nodiscard]] QString itemFontFamily() const;

      /**
       * Returns ITEMFONTBOLD parameter or an empty string if not defined.
       * \returns ITEMFONTBOLD parameter or an empty string if not defined.
       */
      [[nodiscard]] QString itemFontBold() const;

      /**
       * Returns ITEMFONTBOLD as a boolean or its default value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns itemFontBold
       * \throws QgsBadRequestException
       */
      [[nodiscard]] bool itemFontBoldAsBool() const;

      /**
       * Returns ITEMFONTITALIC parameter or an empty string if not defined.
       * \returns ITEMFONTITALIC parameter or an empty string if not defined.
       */
      [[nodiscard]] QString itemFontItalic() const;

      /**
       * Returns ITEMFONTITALIC as a boolean or its default value if not
       * defined. An exception is raised if an invalid parameter is found.
       * \returns itemFontItalic
       * \throws QgsBadRequestException
       */
      [[nodiscard]] bool itemFontItalicAsBool() const;

      /**
       * Returns ITEMFONTSIZE parameter or an empty string if not defined.
       * \returns ITEMFONTSIZE parameter or an empty string if not defined.
       */
      [[nodiscard]] QString itemFontSize() const;

      /**
       * Returns ITEMFONTSIZE as a double. An exception is raised if an
       * invalid parameter is found.
       * \returns itemFontSize
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double itemFontSizeAsDouble() const;

      /**
       * Returns ITEMFONTCOLOR parameter or an empty string if not defined.
       * \returns ITEMFONTCOLOR parameter or an empty string if not defined.
       */
      [[nodiscard]] QString itemFontColor() const;

      /**
       * Returns ITEMFONTCOLOR as a color. An exception is raised if an
       * invalid parameter is found.
       * \returns itemFontColor
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QColor itemFontColorAsColor() const;

      /**
       * Returns LAYERTITLE parameter or an empty string if not defined.
       * \returns LAYERTITLE parameter or an empty string if not defined.
       */
      [[nodiscard]] QString layerTitle() const;

      /**
       * Returns LAYERTITLE as a bool or its default value if not defined. An
       * exception is raised if an invalid parameter is found.
       * \returns layerTitle
       * \throws QgsBadRequestException
       */
      [[nodiscard]] bool layerTitleAsBool() const;

      /**
       * Returns legend settings
       * \returns legend settings
       */
      [[nodiscard]] QgsLegendSettings legendSettings() const;

      /**
       * Returns parameters for each highlight layer.
       * \returns parameters for each highlight layer
       */
      [[nodiscard]] QList<QgsWmsParametersHighlightLayer> highlightLayersParameters() const;

      /**
       * Returns parameters for each external layer.
       * \since QGIS 3.8
       */
      [[nodiscard]] QList<QgsWmsParametersExternalLayer> externalLayersParameters() const;

      /**
       * Returns HIGHLIGHT_GEOM as a list of string in WKT.
       * \returns highlight geometries
       */
      [[nodiscard]] QStringList highlightGeom() const;

      /**
       * Returns HIGHLIGHT_GEOM as a list of geometries. An exception is
       * raised if an invalid geometry is found.
       * \returns highlight geometries
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QList<QgsGeometry> highlightGeomAsGeom() const;

      /**
       * Returns HIGHLIGHT_SYMBOL as a list of string.
       * \returns highlight sld symbols
       */
      [[nodiscard]] QStringList highlightSymbol() const;

      /**
       * Returns HIGHLIGHT_LABELSTRING as a list of string.
       * \returns highlight label string
       */
      [[nodiscard]] QStringList highlightLabelString() const;

      /**
       * Returns HIGHLIGHT_LABELCOLOR as a list of string.
       * \returns highlight label color
       */
      [[nodiscard]] QStringList highlightLabelColor() const;

      /**
       * Returns HIGHLIGHT_LABELCOLOR as a list of color. An exception is
       * raised if an invalid color is found.
       * \returns highlight label color
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QList<QColor> highlightLabelColorAsColor() const;

      /**
       * Returns HIGHLIGHT_LABELSIZE as a list of string.
       * \returns highlight label size
       */
      [[nodiscard]] QStringList highlightLabelSize() const;

      /**
       * Returns HIGHLIGHT_LABELSIZE as a list of int An exception is raised
       * if an invalid size is found.
       * \returns highlight label size
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QList<int> highlightLabelSizeAsInt() const;

      /**
       * Returns HIGHLIGHT_LABELWEIGHT as a list of string.
       * \returns highlight label weight
       */
      [[nodiscard]] QStringList highlightLabelWeight() const;

      /**
       * Returns HIGHLIGHT_LABELWEIGHT as a list of int. An exception is
       * raised if an invalid weight is found.
       * \returns highlight label weight
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QList<int> highlightLabelWeightAsInt() const;

      /**
       * Returns HIGHLIGHT_LABELFONT
       * \returns highlight label font
       */
      [[nodiscard]] QStringList highlightLabelFont() const;

      /**
       * Returns HIGHLIGHT_LABELBUFFERSIZE
       * \returns highlight label buffer size
       */
      [[nodiscard]] QStringList highlightLabelBufferSize() const;

      /**
       * Returns HIGHLIGHT_LABELBUFFERSIZE as a list of float. An exception is
       * raised if an invalid size is found.
       * \returns highlight label buffer size
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QList<double> highlightLabelBufferSizeAsFloat() const;

      /**
       * Returns HIGHLIGHT_LABELBUFFERCOLOR as a list of string.
       * \returns highlight label buffer color
       */
      [[nodiscard]] QStringList highlightLabelBufferColor() const;

      /**
       * Returns HIGHLIGHT_LABELBUFFERCOLOR as a list of colors. An axception
       * is raised if an invalid color is found.
       * \returns highlight label buffer color
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QList<QColor> highlightLabelBufferColorAsColor() const;

      /**
       * Returns HIGHLIGHT_LABEL_ROTATION as a list of double.
       * \returns highlight label rotation
       */
      [[nodiscard]] QList<double> highlightLabelRotation() const;

      /**
       * Returns HIGHLIGHT_LABEL_DISTANCE as a list of double.
       * \returns highlight label distance
       */
      [[nodiscard]] QList<double> highlightLabelDistance() const;

      /**
       * Returns HIGHLIGHT_LABEL_HORIZONTAL_ALIGNMENT as a list of string.
       * \returns highlight label horizontal alignment strings
       */
      [[nodiscard]] QStringList highlightLabelHorizontalAlignment() const;

      /**
       * Returns HIGHLIGHT_LABEL_VERTICAL_ALIGNMENT as a list of string.
       * \returns highlight label vertical alignment strings
       */
      [[nodiscard]] QStringList highlightLabelVerticalAlignment() const;

      /**
       * Returns WMS_PRECISION parameter or an empty string if not defined.
       * \returns wms precision parameter
       */
      [[nodiscard]] QString wmsPrecision() const;

      /**
       * Returns WMS_PRECISION parameter as an int or its default value if not
       * defined. An exception is raised if WMS_PRECISION is defined and cannot be
       * converted.
       * \returns wms precision parameter
       * \throws QgsBadRequestException
       */
      [[nodiscard]] int wmsPrecisionAsInt() const;

      /**
       * Returns TRANSPARENT parameter or an empty string if not defined.
       * \returns TRANSPARENT parameter
       */
      [[nodiscard]] QString transparent() const;

      /**
       * Returns TRANSPARENT parameter as a bool or its default value if not
       * defined. An exception is raised if TRANSPARENT is defined and cannot
       * be converted.
       * \returns transparent parameter
       * \throws QgsBadRequestException
       */
      [[nodiscard]] bool transparentAsBool() const;

      /**
       * Returns BGCOLOR parameter or an empty string if not defined.
       * \returns BGCOLOR parameter
       */
      [[nodiscard]] QString backgroundColor() const;

      /**
       * Returns BGCOLOR parameter as a QColor or its default value if not
       * defined. An exception is raised if BGCOLOR is defined and cannot
       * be converted.
       * \returns background color parameter
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QColor backgroundColorAsColor() const;

      /**
       * Returns DPI parameter or an empty string if not defined.
       * \returns DPI parameter
       */
      [[nodiscard]] QString dpi() const;

      /**
       * Returns DPI parameter as an int or its default value if not
       * defined. An exception is raised if DPI is defined and cannot
       * be converted.
       * \returns dpi parameter
       * \throws QgsBadRequestException
       */
      [[nodiscard]] double dpiAsDouble() const;

      /**
       * Returns TEMPLATE parameter or an empty string if not defined.
       * \returns TEMPLATE parameter
       */
      [[nodiscard]] QString composerTemplate() const;

      /**
       * Returns the requested parameters for a composer map parameter.
       * An exception is raised if parameters are defined and cannot be
       * converted like EXTENT, SCALE, ROTATION, GRID_INTERVAL_X and
       * GRID_INTERVAL_Y.
       * \param mapId the composer map id.
       * \returns parameters for the composer map.
       * \throws QgsBadRequestException
       */
      [[nodiscard]] QgsWmsParametersComposerMap composerMapParameters( int mapId ) const;

      /**
       * Returns the external WMS uri
       * \param id the id of the external wms
       * \return uri string or an empty string if the external wms id does not exist
       */
      [[nodiscard]] QString externalWMSUri( const QString &id ) const;

      /**
       * Returns if the client wants the feature info response with geometry information
       * \returns TRUE if geometry information is requested for feature info response
       */
      [[nodiscard]] bool withGeometry() const;

      /**
       * \brief withMapTipAsString
       * \returns WITH_MAPTIP parameter as string
       * \since QGIS 3.36
       */
      [[nodiscard]] QString withMapTipAsString() const;

      /**
       * \brief withMapTip
       * \returns TRUE if maptip information is requested for feature info response
       */
      [[nodiscard]] bool withMapTip() const;

      /**
       * Returns TRUE if only maptip information is requested for HTML
       * feature info response
       * \returns htmlInfoOnlyMapTip
       * \since QGIS 3.36
       */
      [[nodiscard]] bool htmlInfoOnlyMapTip() const;

      /**
       * \brief withDisplayName
       * \returns TRUE if the display name is requested for feature info response
       * \since QGIS 3.32
       */
      [[nodiscard]] bool withDisplayName() const;

      /**
       * Returns WMTVER parameter or an empty string if not defined.
       * \since QGIS 3.4
       */
      [[nodiscard]] QString wmtver() const;

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
      [[nodiscard]] QStringList atlasPk() const;

      /**
       * Returns the DXF LAYERATTRIBUTES parameter.
       * \since QGIS 3.8
       */
      [[nodiscard]] QStringList dxfLayerAttributes() const;

      /**
       * Returns the DXF USE_TITLE_AS_LAYERNAME parameter.
       * \since QGIS 3.8
       */
      [[nodiscard]] bool dxfUseLayerTitleAsName() const;

      /**
       * Returns the DXF SCALE parameter.
       * \since QGIS 3.8
       */
      [[nodiscard]] double dxfScale() const;

      /**
       * Returns the DXF MODE parameter.
       * \since QGIS 3.8
       */
      [[nodiscard]] Qgis::FeatureSymbologyExport dxfMode() const;

      /**
       * Returns the DXF CODEC parameter.
       * \since QGIS 3.8
       */
      [[nodiscard]] QString dxfCodec() const;

      /**
       * Returns the dimensions parameter.
       * \since QGIS 3.10
       */
      [[nodiscard]] QMap<QString, QString> dimensionValues() const;

      /**
       * \returns true if the FORCE_MTEXT parameter is set and the DXF should
       * be produced with MTEXT instead of TEXT.
       *
       * \since QGIS 3.12
       */
      [[nodiscard]] bool noMText() const;

      /**
       * \returns true if the FORCE_2D parameter is set and the DXF should
       * be produced in 2D.
       *
       * \since QGIS 3.12
       */
      [[nodiscard]] bool isForce2D() const;

      /**
       * \returns true if the lines are export to dxf with minimal (hairline) width
       *
       * \since QGIS 3.38
       */
      [[nodiscard]] bool exportLinesWithZeroWidth() const;

      /**
       * Returns if a geospatial PDF shall be exported
       * \since QGIS 3.32
       */
      [[nodiscard]] bool writeGeospatialPdf() const;

      /**
       * Returns if pdf should be exported as vector
       * \since QGIS 3.32
       */
      [[nodiscard]] bool pdfForceVectorOutput() const;

      /**
       * Returns true if georeference info shall be added to the pdf
       * \since QGIS 3.32
       */
      [[nodiscard]] bool pdfAppendGeoreference() const;

      /**
       * Returns if geometries shall to be simplified
       * \since QGIS 3.32
       */
      [[nodiscard]] bool pdfSimplifyGeometries() const;

      /**
       * Returns true if metadata shall be added to the pdf
       * \since QGIS 3.32
       */
      [[nodiscard]] bool pdfExportMetadata() const;

      /**
       * Returns text render format for pdf export
       * \since QGIS 3.32
       */
      [[nodiscard]] Qgis::TextRenderFormat pdfTextRenderFormat() const;

      /**
       * Returns true if images embedded in pdf must be compressed using a lossless algorithm
       * \since QGIS 3.32
       */
      [[nodiscard]] bool pdfLosslessImageCompression() const;

      /**
       * Returns true if rasters shall be untiled in the pdf
       * \since QGIS 3.32
       */
      [[nodiscard]] bool pdfDisableTiledRasterRendering() const;

      /**
       * Returns true, if Iso32000 georeferencing shall be used
       * \since QGIS 3.32
       */
      [[nodiscard]] bool pdfUseIso32000ExtensionFormatGeoreferencing() const;

      /**
       * Returns true if OGC best practice georeferencing shall be used
       * \since QGIS 3.32
       * \deprecated QGIS 3.42. Will always return false starting with 3.42. Only ISO 32000 georeferencing is handled.
       */
      Q_DECL_DEPRECATED bool pdfUseOgcBestPracticeFormatGeoreferencing() const;

      /**
       * Returns map themes for geospatial PDF export
       * \since QGIS 3.32
       */
      [[nodiscard]] QStringList pdfExportMapThemes() const;

      /**
       * Returns list of map scales
       * \since QGIS 3.32
       */
      [[nodiscard]] QVector<qreal> pdfPredefinedMapScales() const;

      [[nodiscard]] QString version() const override;

      [[nodiscard]] QString request() const override;

      /**
       * Returns the format options for an output format. Possible template types are QgsWmsParameters::PdfFormatOption or QgsWmsParameters::DxfFormatOption
       * \returns a key-value map
       * \since QGIS 3.32
       */
      template<typename T> [[nodiscard]] QMap<T, QString> formatOptions() const
      {
        QMap<T, QString> options;
        const QMetaEnum metaEnum( QMetaEnum::fromType<T>() );
        const QStringList opts = mWmsParameters.value( QgsWmsParameter::FORMAT_OPTIONS ).toStringList( ';' );

        for ( auto it = opts.constBegin(); it != opts.constEnd(); ++it )
        {
          const int equalIdx = it->indexOf( ':' );
          if ( equalIdx > 0 && equalIdx < ( it->length() - 1 ) )
          {
            const QString name = it->left( equalIdx ).toUpper();
            int metaEnumVal = metaEnum.keyToValue( name.toStdString().c_str() );
            if ( metaEnumVal < 0 )
            {
              continue; //option for a different format
            }
            const T option = ( T ) metaEnumVal;
            const QString value = it->right( it->length() - equalIdx - 1 );
            options.insert( option, value );
          }
        }
        return options;
      }

    private:
      static bool isExternalLayer( const QString &name );

      bool loadParameter( const QString &name, const QString &value ) override;

      void save( const QgsWmsParameter &parameter, bool multi = false );

      [[nodiscard]] QgsWmsParameter idParameter( QgsWmsParameter::Name name, int id ) const;

      void raiseError( const QString &msg ) const;
      void log( const QString &msg, const char *file = __builtin_FILE(), const char *function = __builtin_FUNCTION(), int line = __builtin_LINE() ) const;

      [[nodiscard]] QgsWmsParametersExternalLayer externalLayerParameter( const QString &name ) const;

      [[nodiscard]] QMultiMap<QString, QgsWmsParametersFilter> layerFilters( const QStringList &layers ) const;


      QMultiMap<QgsWmsParameter::Name, QgsWmsParameter> mWmsParameters;
      QMap<QString, QMap<QString, QString>> mExternalWMSParameters;
      QList<QgsProjectVersion> mVersions;
  };
} // namespace QgsWms

#endif
