# Electronic Design Automation

This is where all the Electronic Design Automation files for the PCB(s) will be stored.
The EDA used is KiCad 9.x unless otherwise specified.

The KiCad projects have been configured to use local libraries for long term sustainability per the best practices listed [HERE](https://hackaday.com/2017/05/18/kicad-best-practises-library-management/) and elsewhere.

## File Structure

This is the file structure of this repository

* [/](/README.md) - Home
* [/eda](/eda/) - Electronic Design Automation files - YOU ARE HERE
  * [/eda/psycho_badge/](/eda/psycho_badge/) - KiCad 9.x project folder for the psycho_badge PCB
    * ./3d_models/ - This directory contains footprint 3d model files
    * ./gerber/ - This directory contains gerber formatted files for manufacturing
    * ./lib_fp/ - This directory contains footprint module directories
    * ./lib_sh/ - This directory contains schematic library files
    * ./snapshots/ - This directory contains prints of schematic / cad files and images of preview / final product
  * [/eda/psycho_sao/](/eda/psycho_sao/) - KiCad 9.x project folder for the psycho_badge SAO PCBs
    * ./3d_models/ - This directory contains footprint 3d model files
    * ./gerber/ - This directory contains gerber formatted files for manufacturing
    * ./lib_fp/ - This directory contains footprint module directories
    * ./lib_sh/ - This directory contains schematic library files
    * ./snapshots/ - This directory contains prints of schematic / cad files and images of preview / final product

