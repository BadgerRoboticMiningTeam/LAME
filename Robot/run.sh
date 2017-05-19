#!/bin/bash

done=0
while [ $done -eq 0 ]
do
    ping -w 1 -c 1 192.168.1.1
    ret=$?
    if [ $ret -eq 0 ]; then
        done=1
    fi
done


#nohup /home/pi/LAME/Robot/LAME -s /dev/ttyACM0 &
nohup /home/pi/LAME/Robot/LAME -s /dev/ttyUSB0 &

