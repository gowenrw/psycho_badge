#!/bin/bash

########################################
## VARIABLE DEFAULTS
########################################
BoardSerial="not-set"
CodeSketch="psycho_badge_02"

########################################
## COLOR TEXT SETUP
########################################
green='\e[32m'
blue='\e[34m'
yellow='\e[33m'
red='\e[31m'
ERED='\e[91m'
clear='\e[0m'
CTxtGreen(){
	echo -ne $green$1$clear
}
CTxtBlue(){
	echo -ne $blue$1$clear
}
CTxtYellow(){
	echo -ne $yellow$1$clear
}
CTxtRed(){
	echo -ne $red$1$clear
}
CTxtERROR(){
	echo -ne $ERED$1$clear
}
# Test Color Text
#echo -ne $(CTxtGreen 'Test Green ')
#echo -ne $(CTxtBlue 'Test Blue ')
#echo $(CTxtYellow 'Test Yellow ')

########################################
## MENU FUNCTIONS
########################################
GetBoardSerial(){
  # BoardSerial=$(arduino-cli --config-file arduino-cli.yml board list | grep serial | head -n 1 | awk '{print $1}')
  # if [ ! $BoardSerial ]
  # then
  #   BoardSerial="not-set"
  # fi

  local serial_ports
  local port_count
  local selected_index

  mapfile -t serial_ports < <(arduino-cli --config-file arduino-cli.yml board list | grep serial | awk '{print $1}')
  port_count=${#serial_ports[@]}

  if [ "$port_count" -eq 0 ]
  then
    BoardSerial="not-set"
  elif [ "$port_count" -eq 1 ]
  then
    BoardSerial="${serial_ports[0]}"
  else
    echo "$(CTxtYellow 'MULTIPLE SERIAL PORTS DETECTED')"
    for i in "${!serial_ports[@]}"
    do
      echo "$(CTxtGreen "$((i + 1)).") $(CTxtBlue "${serial_ports[$i]}")"
    done

    while true
    do
      read -p "$(CTxtGreen '####') $(CTxtBlue 'CHOOSE A SERIAL PORT >>') " selected_index
      if [[ "$selected_index" =~ ^[0-9]+$ ]] && [ "$selected_index" -ge 1 ] && [ "$selected_index" -le "$port_count" ]
      then
        BoardSerial="${serial_ports[$((selected_index - 1))]}"
        break
      fi
      echo "$(CTxtERROR 'INVALID SERIAL PORT SELECTION')"
    done
  fi
}
GetCodeSketch(){
  local sketch_dirs
  local sketch_count
  local selected_index

  mapfile -t sketch_dirs < <(find . -maxdepth 1 -mindepth 1 -type d -name 'psycho_badge_*' -printf '%f\n' | sort)
  sketch_count=${#sketch_dirs[@]}

  if [ "$sketch_count" -eq 0 ]
  then
    echo "$(CTxtERROR 'NO CODE SKETCH DIRECTORIES FOUND')"
    return
  elif [ "$sketch_count" -eq 1 ]
  then
    CodeSketch="${sketch_dirs[0]}"
    return
  fi

  echo "$(CTxtYellow 'AVAILABLE CODE SKETCHES')"
  for i in "${!sketch_dirs[@]}"
  do
    echo "$(CTxtGreen "$((i + 1)).") $(CTxtBlue "${sketch_dirs[$i]}")"
  done

  while true
  do
    read -p "$(CTxtGreen '####') $(CTxtBlue 'CHOOSE A CODE SKETCH >>') " selected_index
    if [[ "$selected_index" =~ ^[0-9]+$ ]] && [ "$selected_index" -ge 1 ] && [ "$selected_index" -le "$sketch_count" ]
    then
      CodeSketch="${sketch_dirs[$((selected_index - 1))]}"
      break
    fi
    echo "$(CTxtERROR 'INVALID CODE SKETCH SELECTION')"
  done
}
MinicomSerialMonitor(){
  minicom -D $BoardSerial -b 115200
}
ArduinoSerialMonitor(){
  arduino-cli monitor --config baudrate=115200 --port $BoardSerial
}
CompileCode(){
  if ! [ -d "$CodeSketch" ]
  then
    echo "$(CTxtERROR 'CODE SKETCH') $(CTxtYellow $CodeSketch) $(CTxtERROR 'NOT FOUND')"
    return;
  elif [ -f "$CodeSketch/build/esp32.esp32.esp32/$CodeSketch.ino.bin" ]
  then
    echo "$(CTxtERROR 'BINARY FOUND FOR CODE SKETCH') $(CTxtYellow $CodeSketch)"
    read -p "$(CTxtERROR 'THIS WILL OVERWRITE EXISTING BINARY, TYPE Y TO PROCEED >>') " CompileOverwrite
    if [ $CompileOverwrite == "Y" ] || [ $CompileOverwrite == "y" ]
    then
      echo "$(CTxtGreen 'PROCEEDING...')"
    else
      return;
    fi
  fi
  echo "$(CTxtYellow 'arduino-cli --config-file arduino-cli.yml compile --fqbn esp32:esp32:esp32 --build-path _build --export-binaries') $(CTxtRed $CodeSketch) $(CTxtYellow '--build-property build.partitions=min_spiffs --build-property upload.maximum_size=1966080') "
  arduino-cli --config-file arduino-cli.yml compile --fqbn esp32:esp32:esp32 --build-path _build --export-binaries $CodeSketch --build-property build.partitions=min_spiffs --build-property upload.maximum_size=1966080
  read -s -p 'Press any key to continue...'
  return;
}
UploadCode(){
  if [ $BoardSerial == "not-set" ]
  then
    echo "$(CTxtERROR 'BADGE SERIAL NOT SET')"
    return;
  elif ! [ -f "$CodeSketch/build/esp32.esp32.esp32/$CodeSketch.ino.bin" ]
  then
    echo "$(CTxtERROR 'BINARY NOT FOUND FOR CODE SKETCH') $(CTxtYellow $CodeSketch)"
    return;
  fi
  echo "$(CTxtYellow 'arduino-cli --config-file arduino-cli.yml upload -p') $(CTxtRed $BoardSerial) $(CTxtYellow '--fqbn esp32:esp32:esp32 --input-file') $(CTxtRed $CodeSketch)$(CTxtYellow '/build/esp32.esp32.esp32/')$(CTxtRed $CodeSketch)$(CTxtYellow '.ino.bin') "
  arduino-cli --config-file arduino-cli.yml upload -p $BoardSerial --fqbn esp32:esp32:esp32 --input-file $CodeSketch/build/esp32.esp32.esp32/$CodeSketch.ino.bin
  read -s -p 'Press any key to continue...'
  return;
}

########################################
## MENU SETUP
########################################
menu(){
if [ -f "$CodeSketch/build/esp32.esp32.esp32/$CodeSketch.ino.bin" ]
then
  CodeBinFlag="$(CTxtYellow 'YES')"
else
  CodeBinFlag="$(CTxtRed 'NO')"
fi
if [ $BoardSerial == "not-set" ]
then
  BoardSerialTxt="$(CTxtRed 'not-set')"
else
  BoardSerialTxt="$(CTxtYellow $BoardSerial)"
fi
echo -ne "
$(CTxtGreen '######################################################################')
$(CTxtGreen '####. . . . . . . . ARDUINO CLI BADGE MENU SCRIPT  . . . . . . . .####')
$(CTxtGreen '######################################################################')
$(CTxtGreen '#### BADGE SERIAL:') $BoardSerialTxt $(CTxtGreen 'CODE:') $(CTxtYellow $CodeSketch) $(CTxtGreen 'BIN:') $CodeBinFlag
$(CTxtGreen '######################################################################')
$(CTxtGreen '####') $(CTxtBlue '1 - UPDATE BADGE SERIAL . .') $(CTxtGreen '####') $(CTxtBlue '2 - CHANGE CODE SKETCH. . .') $(CTxtGreen '####')
$(CTxtGreen '####') $(CTxtBlue '3 - COMPILE CODE SKETCH . .') $(CTxtGreen '####') $(CTxtBlue '4 - UPLOAD CODE TO BADGE. .') $(CTxtGreen '####')
$(CTxtGreen '####') $(CTxtBlue '5 - MINICOM SERIAL MONITOR.') $(CTxtGreen '####') $(CTxtBlue '6 - ARDUINO SERIAL MONITOR ') $(CTxtGreen '####')
$(CTxtGreen '####') $(CTxtBlue '0 - EXIT MENU . . . . . . .') $(CTxtGreen '####') $(CTxtBlue '. . . . . . . . . . . . . .') $(CTxtGreen '####')
$(CTxtGreen '######################################################################')
"
        read -p "$(CTxtGreen '####') $(CTxtBlue 'CHOOSE AN OPTION >>') " a
        case $a in
          1) GetBoardSerial ; menu ;;
          2) GetCodeSketch ; menu ;;
          3) CompileCode ; menu ;;
          4) UploadCode ; menu ;;
          5) MinicomSerialMonitor ; menu ;;
          6) ArduinoSerialMonitor ; menu ;;
          0) exit 0 ;;
          *) echo "$(CTxtERROR 'WRONG OPTION')"; menu;;
        esac
}

########################################
## MENU LAUNCH
########################################
GetBoardSerial
menu
