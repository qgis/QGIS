DO $$
DECLARE
  layerid INTEGER;

BEGIN
  IF EXISTS ( SELECT * FROM pg_catalog.pg_available_extensions
              WHERE name = 'postgis_topology' )
  THEN
    RAISE NOTICE 'Loading postgis_topology';
    CREATE EXTENSION IF NOT EXISTS postgis_topology;

  -- Topology: qgis_test_Topo1

  IF EXISTS ( SELECT * FROM pg_catalog.pg_namespace WHERE
    nspname = 'qgis_test_Topo1' )
  THEN
    PERFORM topology.DropTopology('qgis_test_Topo1');
  END IF;

  PERFORM topology.CreateTopology('qgis_test_Topo1');

  -- TopoLayer: qgis_test.TopoLayer1

  DROP TABLE IF EXISTS qgis_test."TopoLayer1";
  CREATE TABLE qgis_test."TopoLayer1" (id serial primary key);
  layerid := topology.AddTopoGeometryColumn('qgis_test_Topo1',
                                        'qgis_test',
                                        'TopoLayer1',
                                        'topogeom',
                                        'POLYGON');
  INSERT INTO qgis_test."TopoLayer1" (topogeom) SELECT
    topology.toTopoGeom('POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))',
        'qgis_test_Topo1', layerid);

  END IF;
END;
$$;

