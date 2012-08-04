##operation=selection opening after closing;closing after opening;opening after closing after opening;closing after opening after closing
##iterations=number 1
from sextante.pymorph.mmorph import asf
operations=['OC', 'CO', 'OCO', 'COC']
output_array=asf(input_array,operations[operation], n = iterations)