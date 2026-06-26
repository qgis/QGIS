# freespa
## About freespa
This is a free implementation in c of the Solar Position Algorithm (SPA) [1-2]. Another open and free c implementation is provided by NREL [here](https://midcdmz.nrel.gov/spa/). NREL's SPA is free as in beer! However, it's like beer which you are not allowed to share with your friends (the License does not allow redistribution). The freespa code, on the other hand, you may share under the conditions of the [GPL v3](https://www.gnu.org/licenses/gpl-3.0.en.html). 

## Installation
Package is not designed to be installed but rather to be inserted in a project. You can add freespa as a submodule to your code.

## Usage
See the documentation in DOC.md.

## Δt
For accurate timing the SPA algorithm needs Δt values. Both historic and predicted future values may be obtained here:
[https://maia.usno.navy.mil/ser7/](https://maia.usno.navy.mil/ser7/)

Another source of Δt values is

[https://hpiers.obspm.fr/eop-pc/index.php?index=analysis&lang=en](https://hpiers.obspm.fr/eop-pc/index.php?index=analysis&lang=en)

This database provides Δt values over various time ranges in various resolutions.

Using these datasources we compiled a table of Δt values in freespa_dt_table.h. Beyond the values in the table we use long term predictions of Δt according to the empirical model from Morrison and  Stephenson [3]. It is possible to update the tables in freespa_dt_table.h. To this end the "GetDeltaT.sh" script is provided, which generates a new header "freespa_dt_table_new.h", which can be inspected and included if correct. The script downloads new  Δt values and mid-term predictions from [https://maia.usno.navy.mil/ser7/](https://maia.usno.navy.mil/ser7/).

## Testing
To test the correct functioning of freespa you may use 

`make check`

which will run various checks and report back.

A simple commandline interfece to freespa (called "spa") is compiled per default, or explicitely using 

`make spa`

In case you download [NREL's SPA](https://midcdmz.nrel.gov/spa/), this commandline spa interface may also use NREL's SPA as a backend, allowing a direct comparison of the two SPA implementations.

Finally, with

`make compare`

you can compile a program to compare NREL's SPA with freespa (provided you have a copy of NREL spa).

## References
[1] I.  Reda and A. Andreas, "Solar position algorithm for solar radiation applications." Solar Energy 76.5 (2004): 577-589

[2] I. Reda and A. Andreas, "Corrigendum to Solar position algorithm for solar radiation applications." Solar Energy 81.6 (2007): 838
 
[3] L. V. Morrison and  F. R. Stephenson "Historical Values of the Earth’s Clock Error ΔT and the Calculation of Eclipses." Journal for the History of Astronomy, 35(3) (2004): 327–336. 
