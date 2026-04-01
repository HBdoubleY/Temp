#!/bin/sh

###########################
EXE_PATH=/opt/work/app/carplay/carplaylib
###########################

$EXE_PATH/mdnsd &
while true;do
    $EXE_PATH/z-link 
    sleep 1 
done
