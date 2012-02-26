##layer=vector
##output=output table
##delimiter=selection (comma);(bar);(space)
##endline=selection CR-LF;LF
from sextante.mmqgis import mmqgis_library as mmqgis
from sextante.core.Sextante import Sextante
delimiter_strings = [",", "|", " "]
endline_strings=["\r\n", "\n"]
mmqgis.mmqgis_geometry_export_to_csv(Sextante.getInterface(), layer, output, output[:-3] + "_attributes.csv", delimiter_strings[delimiter], end_line_strings[end_line])

