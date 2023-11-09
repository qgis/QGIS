# README World Map GeoPackage
The `world_map.gpkg` contains the following layers:

- countries
- states_provinces
- disputed_borders

These layers were processed from source data. The purpose of this readme is to explain briefly the data sources and the processing steps.

### Data source
Natural Earth 1:10m detail level Cultural vectors: https://www.naturalearthdata.com/downloads/10m-cultural-vectors/

The specific layers used for input are:

[0]: Admin 1 – States, Provinces v4.1.0

[1]: Admin 0 – Countries v4.1.0

[2]: Admin 0 – Breakaway, Disputed Areas v4.1.0 (boundary lines)

### Processing
The layer-specific processing steps are described below. 

##### _countries_ and _states_provinces_ 
These two layers are derived from data source [0].
In order to keep the size small, the geometry was simplified with GRASS `v.generalize`, then dissolved 
on state/province level and on country level respectively. These above steps were done with a QGIS model `world_map_generalize.model3` that is included in this folder.
For the **countries** layer, the attribute table of data source [1] was joined in manually and the country attribute fields were copied.

*Side notes to the QGIS model:*
 
 - *Input data are in WGS84 (EPSG:4326); re-projection to a meter-based CRS was necessary 
in order to set meaningful thresholds in `v.generalize`. 
I chose Robinson (EPSG:54030) for that purpose. I think it is an OK
compromise for a global projected CRS in this case.*

 - *The model also creates a safe extent polygon for WGS84 coordinate space
and clips the simplified layers to that extent before re-projecting them back to WGS84. This was necessary in order to avoid polygons
crossing the 180˚ meridian. Those would be rendered as large invalid polygons crossing the entire world.*

##### _disputed_borders_ 
This layer was created by importing data source [2] as-is. 

##### Manual clean-up of attribute tables (all layers)
In the GeoPackage the less meaningful fields in the attribute tables were dropped.

And finally Crimea and Sevastopol were restored to belong to the Ukraine using

```sql
update states_provinces set iso_a2='UA',sov_a3='UA',adm0_a3='UA',admin='Ukraine',gu_a3='UKR' where name IN ('Crimea','Sevastopol');
update countries set geom=st_union(geom, (select st_union(geom) from states_provinces where name IN ('Crimea','Sevastopol'))) where name='Ukraine';
update countries set geom=st_difference(geom, (select st_union(geom) from states_provinces where name IN ('Crimea','Sevastopol'))) where name='Russia';
```
