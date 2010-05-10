

def suitableTagKeys(featType):
    """Function is used to find out typical tag keys to given feature type.
    With help of this function plugin gives advice to user on feature tags editing.
    Information on typical/recommended tag keys was taken from wiki.openstreetmap.org.

    @param featType name of feature type; one of 'Point','Line','Polygon'
    @return list of typical keys to given feature type
    """

    if featType=='Point':

      return ["highway","traffic_calming","barrier","waterway","lock","railway","aeroway","aerialway","power","man_made","leisure"
             ,"amenity","shop","tourism","historic","landuse","military","natural","route","sport","internet_access","motorroad","bridge","crossing"
             ,"mountain_pass","ele","incline","operator","opening_hours","disused","wheelchair","TMC:LocationCode","wood","traffic_sign","disused"
             ,"name","alt_name"
             ,"alt_name","int_name","nat_name","reg_name","loc_name","old_name","name:lg","ref","int_ref","nat_ref","reg_ref","loc_ref","old_ref"
             ,"source_ref","icao","iata","place","place_numbers","postal_code","is_in","population","addr:housenumber","addr:housename","addr:street"
             ,"addr:postcode","addr:city","addr:country","note","description","image","source","source_ref","source_name","source:ref"
             ,"attribution","url","website","wikipedia","created_by","history"]

    elif featType=='Line':

      return ["highway","construction","junction","traffic_calming","service","smoothness","passing_places","barrier","cycleway"
             ,"tracktype","waterway","lock","mooring","railway","usage","electrified","frequency","voltage","bridge","tunnel","service"
             ,"aeroway","aerialway","power","cables","wires","voltage","man_made","leisure","amenity","natural","route","abutters","fenced"
             ,"lit","motorroad","bridge","tunnel","cutting","embankment","lanes","layer","surface","width","est_width","depth","est_depth"
             ,"incline","start_date","end_date","operator","opening_hours","disused","wheelchair","narrow","sac_scale","trail_visibility"
             ,"mtb:scale","mtb:scale:uphill","mtb:scale:imba","mtb:description","TMC:LocationCode","access","vehicle","bicycle","foot","goods"
             ,"hgv","hazmat","agricultural","horse","motorcycle","motorcar","motor_vehicle","psv","motorboat","boat","oneway","noexit"
             ,"date_on","date_off","hour_on","hour_off","maxweight","maxheight","maxwidth","maxlength","maxspeed","minspeed","maxstay"
             ,"disused","toll","charge","name"
             ,"alt_name","int_name","nat_name","reg_name","loc_name","old_name","name:lg","ref","int_ref","nat_ref","reg_ref","loc_ref","old_ref"
             ,"ncn_ref","rcn_ref","lcn_ref"
             ,"source_ref","icao","iata","place_numbers","postal_code","is_in","addr:interpolation","note","description","image","source"
             ,"source_ref","source_name","source:ref","attribution","url","website","wikipedia","created_by","history"]


    elif featType=='Polygon':

      return ["highway","junction","barrier","waterway","railway","landuse","aeroway","aerialway","power","man_made","building","leisure"
             ,"amenity","shop","tourism","historic","landuse","military","natural","route","boundary","sport","area","ele","depth","est_depth"
             ,"operator","opening_hours","disused","wheelchair","wood","admin_level","disused","name"
             ,"alt_name","int_name","nat_name","reg_name","loc_name","old_name","name:lg","ref","int_ref","nat_ref","reg_ref","loc_ref","old_ref"
             ,"source_ref","icao","iata","place","place_name","place_numbers","postal_code","is_in","population"
             ,"addr:housenumber","addr:housename","addr:street","addr:postcode","addr:city","addr:country"
             ,"note","description","image","source","source_ref","source_name","source:ref","attribution","url","website","wikipedia"
             ,"created_by","history"]

    return []

def suitableTagValues(featType,tagKey):
    """Function is used to find out typical tag values to given feature type and key.
    With help of this function plugin gives advice to user on feature tags editing.
    Information on typical/recommended tag values was taken from wiki.openstreetmap.org.

    @param featType name of feature type; one of 'Point','Line','Polygon'
    @param tagKey key of tag
    @return list of typical values to given feature type and key
    """

    vals=[]
    # POINT TAGS
    if featType=='Point':
      pointTagValues = {
        "highway":  ["services","mini_roundabout","stop","traffic_signals","crossing","incline","incline_steep","ford",
                     "bus_stop","turning_circle", "emergency_access_point","speed_camera","motorway_junction","passing_place"],
        "traffic_calming": ["yes","bump","chicane","cushion","hump","rumble_strip","table","choker"],
        "barrier":  ["bollard","cycle_barrier","cattle_grid","toll_booth","entrance","gate","stile","sally_port"],
        "waterway": ["dock","lock_gate","turning_point","boatyard","weir"],
        "lock":     ["yes"],
        "railway":  ["station","halt","tram_stop","crossing","level_crossing","subway_entrance","turntable","buffer_stop"],
        "aeroway":  ["aerodrome","terminal","helipad","gate","windsock"],
        "aerialway":["station"],
        "power":    ["tower","station","sub_station","generator"],
        "man_made": ["beacon","crane","gasometer","lighthouse","reservoir_covered","surveillance","survey_point","tower",
                     "wastewater_plant","watermill","water_tower","water_works","windmill","works"],
        "leisure":  ["sports_centre ","sports_centre ","stadium","track","pitch","water_park","marina","slipway","fishing",
                     "nature_reserve","park","playground","garden","common","ice_rink","miniature_golf"],
        "amenity":  ["restaurant","pub","food_court","fast_food","drinking_water","bbq","biergarten","cafe","kindergarten","school","college",
                     "library","university","ferry_terminal","bicycle_parking","bicycle_rental","bus_station","car_rental","car_sharing","fuel",
                     "grit_bin","parking","signpost","taxi","atm","bank","bureau_de_change","pharmacy","hospital","baby_hatch","dentist","doctors","veterinary",
                     "arts_centre","cinema","fountain","nightclub","studio","theatre","bench","brothel","courthouse","crematorium","embassy","emergency_phone",
                     "fire_station","grave_yard","hunting_stand","place_of_worship","police","post_box","post_office","prison","public_building","recycling",
                     "shelter","telephone","toilets","townhall","vending_machine","waste_basket","waste_disposal"],
        "shop":     ["alcohol","bakery","beverages","bicycle","books","butcher","car","car_repair","chemist","clothes","computer","confectionery","convenience",
                     "department_store","dry_cleaning","doityourself","electronics","florist","furniture","garden_centre","greengrocer","hairdresser",
                     "hardware","hifi","kiosk","laundry","mall","motorcycle","newsagent","optician","organic","outdoor","sports","stationery","supermarket",
                     "shoes","toys","travel_agency","video"],
        "tourism":  ["alpine_hut","attraction","artwork","camp_site","caravan_site","chalet","guest_house","hostel","hotel","information","motel","museum",
                     "picnic_site","theme_park","viewpoint","zoo","yes"],
        "historic": ["castle","monument","memorial","archaeological_site","ruins","battlefield","wreck","yes"],
        "landuse":  ["quarry","landfill","basin","reservoir","forest","allotments","vineyard","residential","retail","commercial","industrial","brownfield",
                     "greenfield","construction","military","meadow","village_green","wood","recreation_ground"],
        "military": ["airfield","bunker","barracks","danger_area","range","naval_base"],
        "natural":  ["bay","beach","cave_entrance","cliff","coastline","fell","glacier","heath","land","marsh","mud","peak","scree","scrub","spring","tree",
                     "volcano","water","wetland","wood"],
        "sport":    ["9pin","10pin","archery","athletics","australian_football","baseball","basketball","beachvolleyball","boules","bowls","canoe","chess",
                     "climbing","cricket","cricket_nets","croquet","cycling","diving","dog_racing","equestrian","football","golf","gymnastics","hockey",
                     "horse_racing","korfball","motor","multi","orienteering","paddle_tennis","pelota","racquet","rowing","rugby","shooting","skating",
                     "skateboard","skiing","soccer","swimming","table_tennis","team_handball","tennis","volleyball"],
        "internet_access": ["public","service","terminal","wired","wlan"],
        "motorroad": ["yes","no"],
        "bridge":   ["yes","aqueduct","viaduct","swing"],
        "crossing": ["no","traffic_signals","uncontrolled"],
        "mountain_pass": ["yes"],
        "disused":  ["yes"],
        "wheelchair": ["yes","no","limited"],
        "wood":     ["coniferous","deciduous","mixed"],
        "place":    ["continent","country","state","region","country","city","town","village","hamlet","suburb","locality","island"],
        "source":   ["extrapolation","knowledge","historical","image","survey","voice"]
      }
      return pointTagValues.get(tagKey, [])

    # LINE TAGS
    elif featType=='Line':
      lineTagValues = {
        "highway": ["motorway","motorway_link","trunk","trunk_link","primary","primary_link","secondary","secondary_link","tertiary","unclassified",
                    "road","residential","living_street","service","track","pedestrian","bus_guideway","path","cycleway","footway","bridleway",
                    "byway","steps","ford","construction"],
        "traffic_calming": ["yes","bump","chicane","cushion","hump","rumble_strip","table","choker"],
        "service":   ["parking_aisle","driveway","alley","yard","siding","spur"],
        "smoothness":["excellent","good","intermediate","bad","very_bad","horrible","very_horrible","impassable"],
        "passing_places": ["yes"],
        "barrier":   ["hedge","fence","wall","ditch","retaining_wall","city_wall","bollard"],
        "cycleway":  ["lane","track","opposite_lane","opposite_track","opposite"],
        "tracktype": ["grade1","grade2","grade3","grade4","grade5"],
        "waterway":  ["stream","river","canal","drain","weir","dam"],
        "lock":      ["yes"],
        "mooring":   ["yes","private","no"],
        "railway":   ["rail","tram","light_rail","abandoned","disused","subway","preserved","narrow_gauge","construction","monorail","funicular","platform"],
        "usage":     ["main","branch","industrial","military","tourism"],
        "electrified": ["contact_line","rail","yes","no"],
        "bridge":    ["yes"],
        "tunnel":    ["yes","no"],
        "aeroway":   ["runway","taxiway"],
        "aerialway": ["cable_car","gondola","chair_lift","drag_lift"],
        "power":     ["line"],
        "cables":    ["3","4","6","8","9","12","15","18"],
        "wires":     ["single","double","triple","quad"],
        "voltage":   ["110000","220000","380000","400000"],
        "man_made":  ["pier","pipeline"],
        "leisure":   ["track"],
        "amenity":   ["marketplace"],
        "tourism":   ["artwork"],
        "natural":   ["cliff","coastline"],
        "route":     ["bus","detour","ferry","flight","subsea","hiking","bicycle","mtb","road","ski","tour","tram","pub_crawl"],
        "abutters":  ["residential","retail","commercial","industrial","mixed"],
        "fenced":    ["yes","no"],
        "lit":       ["yes","no"],
        "motorroad": ["yes","no"],
        "bridge":    ["yes","aqueduct","viaduct","swing"],
        "tunnel":    ["yes"],
        "cutting":   ["yes"],
        "embankment":["yes"],
        "layer":     ["-5","-4","-3","-2","-1","0","1","2","3","4","5"],
        "surface":   ["paved","unpaved","asphalt","concrete","paving_stones","cobblestone","metal","wood","grass_paver","gravel","pebblestone",
                      "grass","ground","earth","dirt","mud","sand","ice_road"],
        "disused":   ["yes"],
        "wheelchair":["yes","no","limited"],
        "narrow":    ["yes"],
        "sac_scale": ["hiking","mountain_hiking","demanding_mountain_hiking","alpine_hiking","demanding_alpine_hiking","difficult_alpine_hiking"],
        "trail_visibility": ["excellent","good","intermediate","bad","horrible","no"],
        "mtb:scale": ["0","1","2","3","4","5"],
        "mtb:scale:uphill":  ["0","1","2","3","4","5"],
        "mtb:scale:imba":    ["0","1","2","3","4"],
        "access":    ["yes","designated","official","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "vehicle":   ["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "bicycle":   ["yes","designated","official","private","permissive","dismount","destination","delivery","agricultural","forestry","unknown","no"],
        "foot":      ["yes","designated","official","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "goods":     ["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "hgv":       ["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "hazmat":    ["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "agricultural":["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "horse":     ["yes","designated","official","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "motorcycle":["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "motorcar":  ["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "motor_vehicle":["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "psv":       ["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "motorboat": ["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "boat":      ["yes","designated","private","permissive","destination","delivery","agricultural","forestry","unknown","no"],
        "oneway":    ["yes","no","-1"],
        "noexit":    ["yes"],
        "toll":      ["yes"],
        "addr:interpolation": ["all","even","odd","alphabetic"],
        "source":    ["extrapolation","knowledge","historical","image","survey","voice"]
      }
      return lineTagValues.get(tagKey, [])


    # POLYGON TAGS
    elif featType=='Polygon':
      polygonTagValues = {
        "highway":   ["pedestrian","services"],
        "junction":  ["roundabout"],
        "barrier":   ["hedge","fence","wall","ditch","retaining_wall","city_wall"],
        "waterway":  ["riverbank","dock","dam"],
        "railway":   ["station","turntable","platform"],
        "aeroway":   ["aerodrome","terminal","helipad","apron"],
        "aerialway": ["station"],
        "power":     ["station","sub_station","generator"],
        "man_made":  ["crane","gasometer","pier","reservoir_covered","surveillance","wastewater_plant","watermill","water_tower","water_works","windmill","works"],
        "building":  ["yes"],
        "leisure":   ["sports_centre ","sports_centre ","stadium","track","pitch","water_park","marina","fishing","nature_reserve",
                      "park","playground","garden","common","ice_rink","miniature_golf"],
        "amenity":   ["restaurant","pub","food_court","fast_food","biergarten","cafe","kindergarten","school","college",
                      "library","university","ferry_terminal","bicycle_parking","bicycle_rental","bus_station","car_rental","car_sharing","fuel",
                      "parking","taxi","bank","pharmacy","hospital","baby_hatch","dentist","doctors","veterinary",
                      "arts_centre","cinema","fountain","nightclub","studio","theatre","brothel","courthouse","crematorium","embassy",
                      "fire_station","grave_yard","hunting_stand","marketplace","place_of_worship","police","post_office","prison","public_building","recycling",
                      "shelter","townhall"],
        "shop":      ["alcohol","bakery","beverages","bicycle","books","butcher","car","car_repair","chemist","clothes","computer","confectionery","convenience",
                      "department_store","dry_cleaning","doityourself","electronics","florist","furniture","garden_centre","greengrocer","hairdresser",
                      "hardware","hifi","kiosk","laundry","mall","motorcycle","newsagent","optician","organic","outdoor","sports","stationery","supermarket",
                      "shoes","toys","travel_agency","video"],
        "tourism":   ["alpine_hut","attraction","artwork","camp_site","caravan_site","chalet","museum","picnic_site","theme_park","zoo","yes"],
        "historic":  ["castle","monument","memorial","archaeological_site","ruins","battlefield","wreck","yes"],
        "landuse":   ["farm","farmyard","quarry","landfill","basin","reservoir","forest","allotments","vineyard","residential","retail","commercial","industrial",
                      "brownfield","greenfield","construction","railway","military","cemetery","meadow","village_green","wood","recreation_ground","salt_pond"],
        "military":  ["airfield","bunker","barracks","danger_area","range","naval_base"],
        "natural":   ["bay","beach","cave_entrance","cliff","coastline","fell","glacier","heath","land","marsh","mud","scree","scrub",
                      "water","wetland","wood"],
        "boundary":  ["administrative","civil","political","national_park"],
        "sport":     ["9pin","10pin","archery","athletics","australian_football","baseball","basketball","beachvolleyball","boules","bowls","canoe","chess",
                      "climbing","cricket","cricket_nets","croquet","cycling","diving","dog_racing","equestrian","football","golf","gymnastics","hockey",
                      "horse_racing","korfball","motor","multi","paddle_tennis","pelota","racquet","rowing","rugby","shooting","skating",
                      "skateboard","skiing","soccer","swimming","table_tennis","team_handball","tennis","volleyball"],
        "area":      ["yes"],
        "disused":   ["yes"],
        "wheelchair":["yes","no","limited"],
        "wood":      ["coniferous","deciduous","mixed"],
        "place":     ["continent","state","region","country","city","town","village","hamlet","suburb","locality","island"],
        "source":    ["extrapolation","knowledge","historical","image","survey","voice"]
      }
      return polygonTagValues.get(tagKey, [])

    return []
