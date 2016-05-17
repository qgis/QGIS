/***************************************************************************
 *  qgsgeometrycheckfactory.h                                              *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_CHECK_FACTORY_H
#define QGS_GEOMETRY_CHECK_FACTORY_H

#include "qgis.h"
#include "ui_qgsgeometrycheckersetuptab.h"

class QgsFeaturePool;
class QgsGeometryCheck;
class QgsMapSettings;

class QgsGeometryCheckFactory
{
  public:
    virtual ~QgsGeometryCheckFactory() {}
    virtual void restorePrevious( Ui::QgsGeometryCheckerSetupTab& /*ui*/ ) const = 0;
    virtual bool checkApplicability( Ui::QgsGeometryCheckerSetupTab& /*ui*/, QGis::GeometryType /*geomType*/ ) const = 0;
    virtual QgsGeometryCheck* createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double mapToLayerUnits ) const = 0;

  protected:
    static QString sSettingsGroup;
};

template<class T>
class QgsGeometryCheckFactoryT : public QgsGeometryCheckFactory
{
    void restorePrevious( Ui::QgsGeometryCheckerSetupTab& /*ui*/ ) const override;
    bool checkApplicability( Ui::QgsGeometryCheckerSetupTab& ui, QGis::GeometryType geomType ) const override;
    QgsGeometryCheck* createInstance( QgsFeaturePool* featurePool, const Ui::QgsGeometryCheckerSetupTab& ui, double mapToLayerUnits ) const override;
};

class QgsGeometryCheckFactoryRegistry
{
  public:
    static bool registerCheckFactory( const QgsGeometryCheckFactory* factory )
    {
      instance()->mFactories.append( factory );
      return true;
    }
    static const QList<const QgsGeometryCheckFactory*>& getCheckFactories()
    {
      return instance()->mFactories;
    }
  private:
    QList<const QgsGeometryCheckFactory*> mFactories;
    QgsGeometryCheckFactoryRegistry() {}
    ~QgsGeometryCheckFactoryRegistry() { qDeleteAll( mFactories ); }

    static QgsGeometryCheckFactoryRegistry* instance()
    {
      static QgsGeometryCheckFactoryRegistry reg;
      return &reg;
    }
};

#define QGSGEOMETRYCHECKFACTORY_CONCAT(X, Y) X##Y
#define QGSGEOMETRYCHECKFACTORY_UNIQUEVAR_(X, Y) QGSGEOMETRYCHECKFACTORY_CONCAT(X, Y)
#if (__GNUC__ > 2) || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
# define QGSGEOMETRYCHECKFACTORY_UNUSED __attribute__((__unused__))
#else
# define QGSGEOMETRYCHECKFACTORY_UNUSED
#endif
#define REGISTER_QGS_GEOMETRY_CHECK_FACTORY(CheckFactory) namespace { static QGSGEOMETRYCHECKFACTORY_UNUSED bool QGSGEOMETRYCHECKFACTORY_UNIQUEVAR_(b, __LINE__) = QgsGeometryCheckFactoryRegistry::registerCheckFactory(new CheckFactory()); }

#endif // QGS_GEOMETRY_CHECK_FACTORY_H
