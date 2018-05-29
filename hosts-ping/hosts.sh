#!/bin/bash

LOCALIP1="127.0.0.1"
LOCALIP2="::1"
FILE="/etc/hosts"

while true
    do
    {
        LINE_NUMBER=0
        cat $FILE | while read LINE
        do
        {
             LINE_NUMBER=$(($LINE_NUMBER+1))
             if [ "$LINE" == "" ]; then
                continue
             fi
             IP=`echo $LINE|sed "s/^[ \t#]*//g"|awk '{print $1}'`

             if [ "$IP" == "$LOCALIP1" -o "$IP" == "$LOCALIP2" ]; then
               continue
             fi

             ping $IP -c 1  > /dev/null 2>&1
             if [ $?  -eq 0 ]; then
                    if [ "${LINE:0:1}" == "#" -o "${LINE:0:1}" == " " -o "${LINE:0:1}" == "\t" ]; then
                        echo line=$LINE_NUMBER,ping $IP=$? UP,remove#.
                        sed -i "${LINE_NUMBER}s/^[ \t#]*//g" $FILE
                    fi
             else
                    if [ "${LINE:0:1}" != "#" ]; then
                        echo line=$LINE_NUMBER,ping $IP=$? DOWN,add#.
                        sed -i "${LINE_NUMBER}s/^/#&/g" $FILE
                    fi
             fi
        }
        done
        sleep 3
    }
done
