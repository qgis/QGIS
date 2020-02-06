DO $$
DECLARE
  layerid INTEGER;

BEGIN
  IF EXISTS ( SELECT *
              FROM pg_catalog.pg_available_extensions e1,
                   pg_catalog.pg_available_extensions e2
              WHERE e1.name = 'pointcloud'
                AND e2.name = 'pointcloud_postgis' )
  THEN
    RAISE NOTICE 'Loading pointcloud';
    CREATE EXTENSION IF NOT EXISTS pointcloud;
    CREATE EXTENSION IF NOT EXISTS pointcloud_postgis;

    TRUNCATE pointcloud_formats;
    INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (1, 4326, $S$
      <?xml version="1.0" encoding="UTF-8"?>
      <pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1"
      xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
        <pc:dimension>
          <pc:position>1</pc:position>
          <pc:size>4</pc:size>
          <pc:name>X</pc:name>
          <pc:interpretation>int32_t</pc:interpretation>
          <pc:scale>0.01</pc:scale>
        </pc:dimension>
        <pc:dimension>
          <pc:position>2</pc:position>
          <pc:size>4</pc:size>
          <pc:name>Y</pc:name>
          <pc:interpretation>int32_t</pc:interpretation>
          <pc:scale>0.01</pc:scale>
        </pc:dimension>
        <pc:dimension>
          <pc:position>3</pc:position>
          <pc:size>4</pc:size>
          <pc:name>Z</pc:name>
          <pc:interpretation>int32_t</pc:interpretation>
          <pc:scale>0.01</pc:scale>
        </pc:dimension>
        <pc:metadata>
          <Metadata name="compression">dimensional</Metadata>
          <Metadata name="spatialreference" type="id">4326</Metadata>
        </pc:metadata>
      </pc:PointCloudSchema>
    $S$);

    -- Pointcloud layer: qgis_test.PointCloudPointLayer
    CREATE TABLE IF NOT EXISTS qgis_test."PointCloudPointLayer" (
      id serial primary key,
      pt PCPOINT(1)
    );
    INSERT INTO qgis_test."PointCloudPointLayer" (pt)
      SELECT PC_MakePoint(1, ARRAY[-127, 45, 124.0]);
    INSERT INTO qgis_test."PointCloudPointLayer" (pt)
      SELECT PC_MakePoint(1, ARRAY[127, -45, 224.0]);

    -- Pointcloud layer: qgis_test.PointCloudPatchLayer
    CREATE TABLE IF NOT EXISTS qgis_test."PointCloudPatchLayer" (
      id serial primary key,
      pc PCPATCH(1)
    );
    INSERT INTO qgis_test."PointCloudPatchLayer" (pc)
      SELECT PC_Patch(pt) FROM qgis_test."PointCloudPointLayer";

  END IF;
END;
$$;

