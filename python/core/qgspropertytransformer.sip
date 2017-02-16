class QgsCurveTransform
{
%TypeHeaderCode
#include <qgspropertytransformer.h>
%End

  public:

    QgsCurveTransform();
    QgsCurveTransform( const QList< QgsPoint >& controlPoints );
    ~QgsCurveTransform();
    QgsCurveTransform( const QgsCurveTransform& other );

    //QgsCurveTransform& operator=( const QgsCurveTransform& other );

    QList< QgsPoint > controlPoints() const;

    void setControlPoints( const QList< QgsPoint >& points );

    void addControlPoint( double x, double y );

    void removeControlPoint( double x, double y );

    double y( double x ) const;

    QVector< double > y( const QVector< double >& x ) const;

    bool readXml( const QDomElement& elem, const QDomDocument& doc );

    bool writeXml( QDomElement& transformElem, QDomDocument& doc ) const;

};
class QgsPropertyTransformer
{
%TypeHeaderCode
#include <qgspropertytransformer.h>
%End

%ConvertToSubClassCode
  if (sipCpp->transformerType() == QgsPropertyTransformer::GenericNumericTransformer)
    sipType = sipType_QgsGenericNumericTransformer;
  else if (sipCpp->transformerType() == QgsPropertyTransformer::SizeScaleTransformer)
    sipType = sipType_QgsSizeScaleTransformer;
  else if (sipCpp->transformerType() == QgsPropertyTransformer::ColorRampTransformer)
    sipType = sipType_QgsColorRampTransformer;
  else
    sipType = sipType_QgsPropertyTransformer;
%End

  public:

    enum Type
    {
      GenericNumericTransformer,
      SizeScaleTransformer,
      ColorRampTransformer,
    };

    static QgsPropertyTransformer* create( Type type ) /Factory/;

    QgsPropertyTransformer( double minValue = 0.0, double maxValue = 1.0 );

    virtual ~QgsPropertyTransformer();

    virtual Type transformerType() const = 0;

    virtual QgsPropertyTransformer* clone() = 0 /Factory/;

    virtual bool readXml( const QDomElement& transformerElem, const QDomDocument& doc );

    virtual bool writeXml( QDomElement& transformerElem, QDomDocument& doc ) const;

    double minValue() const;

    void setMinValue( double min );

    double maxValue() const;

    void setMaxValue( double max );

    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const = 0;
    virtual QString toExpression( const QString& baseExpression ) const = 0;

    static QgsPropertyTransformer* fromExpression( const QString& expression, QString& baseExpression /Out/, QString& fieldName /Out/ ) /Factory/;

};
class QgsGenericNumericTransformer : QgsPropertyTransformer
{
%TypeHeaderCode
#include <qgspropertytransformer.h>
%End

  public:
    QgsGenericNumericTransformer( double minValue = 0.0,
                                  double maxValue = 1.0,
                                  double minOutput = 0.0,
                                  double maxOutput = 1.0,
                                  double nullOutput = 0.0,
                                  double exponent = 1.0 );

    virtual Type transformerType() const;
    virtual QgsGenericNumericTransformer* clone() /Factory/;
    virtual bool writeXml( QDomElement& transformerElem, QDomDocument& doc ) const;
    virtual bool readXml( const QDomElement& transformerElem, const QDomDocument& doc );
    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const;
    virtual QString toExpression( const QString& baseExpression ) const;
    static QgsGenericNumericTransformer* fromExpression( const QString& expression, QString& baseExpression, QString& fieldName ) /Factory/;
    double value( double input ) const;
    double minOutputValue() const;
    void setMinOutputValue( double size );
    double maxOutputValue() const;
    void setMaxOutputValue( double size );
    double nullOutputValue() const;
    void setNullOutputValue( double size );
    double exponent() const;
    void setExponent( double exponent );

};
class QgsSizeScaleTransformer : QgsPropertyTransformer
{
%TypeHeaderCode
#include <qgspropertytransformer.h>
%End
  public:

    //! Size scaling methods
    enum ScaleType
    {
      Linear, /*!< Linear scaling */
      Area, /*!< Area based scaling */
      Flannery, /*!< Flannery scaling method */
      Exponential, /*!< Scale using set exponent */
    };

    QgsSizeScaleTransformer( ScaleType type = Linear,
                             double minValue = 0.0,
                             double maxValue = 1.0,
                             double minSize = 0.0,
                             double maxSize = 1.0,
                             double nullSize = 0.0,
                             double exponent = 1.0 );

    virtual Type transformerType() const;
    virtual QgsSizeScaleTransformer* clone() /Factory/;
    virtual bool writeXml( QDomElement& transformerElem, QDomDocument& doc ) const;
    virtual bool readXml( const QDomElement& transformerElem, const QDomDocument& doc );
    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const;
    virtual QString toExpression( const QString& baseExpression ) const;

    static QgsSizeScaleTransformer* fromExpression( const QString& expression, QString& baseExpression /Out/, QString& fieldName /Out/ ) /Factory/;

    double size( double value ) const;

    double minSize() const;

    void setMinSize( double size );

    double maxSize() const;

    void setMaxSize( double size );

    double nullSize() const;

    void setNullSize( double size );

    double exponent() const;

    void setExponent( double exponent );

    ScaleType type() const;

    void setType( ScaleType type );

};

class QgsColorRampTransformer : QgsPropertyTransformer
{
%TypeHeaderCode
#include <qgspropertytransformer.h>
%End
  public:

    QgsColorRampTransformer( double minValue = 0.0,
                             double maxValue = 1.0,
                             QgsColorRamp* ramp /Transfer/ = 0,
                             const QColor& nullColor = QColor( 0, 0, 0, 0 ) );

    QgsColorRampTransformer( const QgsColorRampTransformer& other );

    //QgsColorRampTransformer& operator=( const QgsColorRampTransformer& other );

    ~QgsColorRampTransformer();

    virtual Type transformerType() const;
    virtual QgsColorRampTransformer* clone() /Factory/;
    virtual bool writeXml( QDomElement& transformerElem, QDomDocument& doc ) const;
    virtual bool readXml( const QDomElement& transformerElem, const QDomDocument& doc );
    virtual QVariant transform( const QgsExpressionContext& context, const QVariant& value ) const;
    virtual QString toExpression( const QString& baseExpression ) const;

    QColor color( double value ) const;

    QgsColorRamp* colorRamp() const;

    void setColorRamp( QgsColorRamp* ramp /Transfer/ );

    QColor nullColor() const;

    void setNullColor( const QColor& color );

    QString rampName() const;

    void setRampName( const QString& name );

};

