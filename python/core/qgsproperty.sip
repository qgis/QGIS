class QgsPropertyDefinition
{
%TypeHeaderCode
#include <qgsproperty.h>
%End
  public:

    enum StandardPropertyTemplate
    {
      Boolean,
      Integer,
      IntegerPositive,
      IntegerPositiveGreaterZero,
      Double,
      DoublePositive,
      Double0To1,
      String,
      Transparency,
      RenderUnits,
      ColorWithAlpha,
      ColorNoAlpha,
      PenJoinStyle,
      BlendMode,
      Point,
      Size,
      Size2D,
      LineStyle,
      StrokeWidth,
      FillStyle,
      CapStyle,
      HorizontalAnchor,
      VerticalAnchor,
      SvgPath,
      Offset,
      Custom,
    };

    enum DataType
    {
      DataTypeString,
      DataTypeNumeric,
      DataTypeBoolean,
    };

    QgsPropertyDefinition();

    QgsPropertyDefinition( const QString& name, const QString& description, StandardPropertyTemplate type );

    QgsPropertyDefinition( const QString& name, DataType dataTypes, const QString& description, const QString& helpText );

    QString name() const;

    QString description() const;

    QString helpText() const;

    DataType dataType() const;

    StandardPropertyTemplate standardTemplate() const;

    bool supportsAssistant() const;
};

class QgsProperty
{
%TypeHeaderCode
#include <qgsproperty.h>
%End

  public:

    //! Property types
    enum Type
    {
      InvalidProperty, //! Invalid (not set) property
      StaticProperty, //!< Static property (QgsStaticProperty)
      FieldBasedProperty, //!< Field based property (QgsFieldBasedProperty)
      ExpressionBasedProperty, //!< Expression based property (QgsExpressionBasedProperty)
    };

    QgsProperty();
    virtual ~QgsProperty();

    static QgsProperty fromExpression( const QString& expression, bool isActive = true );
    static QgsProperty fromField( const QString& fieldName, bool isActive = true );
    static QgsProperty fromValue( const QVariant& value, bool isActive = true );

    QgsProperty( const QgsProperty& other );

    operator bool() const;

    Type propertyType() const;

    bool isActive() const;

    void setActive( bool active );

    void setStaticValue( const QVariant& value );

    QVariant staticValue() const;

    void setField( const QString& field );

    QString field() const;

    void setExpressionString( const QString& expression );

    QString expressionString() const;

    virtual QString asExpression() const;

    virtual bool prepare( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    virtual QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    QVariant value( const QgsExpressionContext& context, const QVariant& defaultValue = QVariant(), bool* ok /Out/ = nullptr  ) const;

    QString valueAsString( const QgsExpressionContext& context, const QString& defaultString = QString(), bool* ok /Out/ = nullptr ) const;

    QColor valueAsColor( const QgsExpressionContext& context, const QColor& defaultColor = QColor(), bool* ok /Out/ = nullptr  ) const;

    double valueAsDouble( const QgsExpressionContext& context, double defaultValue = 0.0, bool* ok /Out/ = nullptr  ) const;

    int valueAsInt( const QgsExpressionContext& context, int defaultValue = 0, bool* ok /Out/ = nullptr ) const;

    bool valueAsBool( const QgsExpressionContext& context, bool defaultValue = false, bool* ok /Out/ = nullptr ) const;

    virtual bool writeXml( QDomElement& propertyElem, QDomDocument& doc ) const;

    virtual bool readXml( const QDomElement& propertyElem, const QDomDocument& doc );

    void setTransformer( QgsPropertyTransformer* transformer /Transfer/ );

    const QgsPropertyTransformer* transformer() const;

    bool convertToTransformer();

};


