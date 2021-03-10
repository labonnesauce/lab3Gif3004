#!/bin/bash
set -e

# Sync executable
bn=$(basename $1)
rpiaddr="192.168.2.55"

if [ "$2" == "all" ]; then
    rsync -az $1/build/compositeur $1/build/decodeur $1/build/convertisseur $1/build/filtreur $1/build/redimensionneur "pi@$rpiaddr:/home/pi/projects/laboratoire3/"
else
    rsync -az $1/build/$2 "pi@$rpiaddr:/home/pi/projects/laboratoire3/"
    # Execute GDB
    ssh "pi@$rpiaddr" "nohup gdbserver :$3 /home/pi/projects/laboratoire3/$2 --debug > /dev/null 2> /dev/null < /dev/null &"
    sleep 1
fi
