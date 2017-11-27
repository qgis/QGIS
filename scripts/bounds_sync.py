
import os
import sys
import sqlite3 as sqlite


def gen():
    db = sqlite.connect(":memory:")

    tablesfile = sys.argv[1]
    datafiles = sys.argv[2]

    with open(os.path.join("data", tablesfile)) as f:
        data = f.read()
        db.executescript(data)

    with open(os.path.join("data", datafiles)) as f:
        data = f.read()
        data = data.replace("CHR", "Char")
        db.executescript(data)

    query = """
    SELECT crs.coord_ref_sys_code,
    area_west_bound_lon,
    area_north_bound_lat,
    area_east_bound_lon,
    area_south_bound_lat
    FROM epsg_coordinatereferencesystem crs
        JOIN epsg_area area ON crs.area_of_use_code = area.area_code
    """

    rows = db.execute(query)
    srsdb = sqlite.connect("./resources/srs.db")
    srsdb.execute("DELETE FROM tbl_bounds")
    data = list(rows)
    srsdb.executemany("""insert into tbl_bounds(srid, west_bound_lon, 
    north_bound_lat, 
    east_bound_lon, 
    south_bound_lat) values (?,?,?,?,?)""", data)
    srsdb.commit()


def usage():
    print("""
    Generate the epsg_bounds.csv file to support viewing projection areas and bounds checks in QGIS.

    Usage:

    bounds_sync.py {table dump} {data dump}

    bounds_sync.py EPSG_v9_1.mdb_Tables_PostgreSQL.sql EPSG_v9_1.mdb_Data_PostgreSQL.sql
    """)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        usage()
    else:
        gen()
