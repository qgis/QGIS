#ifndef QGSPOINTCLOUD3DSYMBOL_H
#define QGSPOINTCLOUD3DSYMBOL_H

#include "qgis_3d.h"

#include "qgsabstract3dsymbol.h"

#define SIP_NO_FILE

class QgsPointCloud3DSymbol : public QgsAbstract3DSymbol
{
  public:
    //! Constructor for QgsPointCloud3DSymbol
    QgsPointCloud3DSymbol() : QgsAbstract3DSymbol() {  }
    ~QgsPointCloud3DSymbol() override = default;

    QString type() const override { return "pointcloud"; }
    QgsAbstract3DSymbol *clone() const override SIP_FACTORY
    {
      // TODO: use unique_ptr
      QgsPointCloud3DSymbol *result = new QgsPointCloud3DSymbol;
      result->mEnabled = mEnabled;
      result->mPointSize = mPointSize;
      copyBaseSettings( result );
      return result;
    }


    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override
    {
      Q_UNUSED( context )

      elem.setAttribute( QStringLiteral( "enabled" ), mEnabled );
      elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );
    }

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override
    {
      Q_UNUSED( context )

      mEnabled = elem.attribute( "enabled", QStringLiteral( "0" ) ).toInt();
      mPointSize = elem.attribute( "point-size", QStringLiteral( "5.0" ) ).toFloat();
    }

    bool isEnabled() const { return mEnabled; }
    void setIsEnabled( bool enabled ) { mEnabled = enabled; }

    //! Returns the point size
    float pointSize() const { return mPointSize; }
    //! Sets the point size
    void setPointSize( float size ) { mPointSize = size; }

  private:
    bool mEnabled;
    float mPointSize = 10.0f;
};

#endif // QGSPOINTCLOUD3DSYMBOL_H
