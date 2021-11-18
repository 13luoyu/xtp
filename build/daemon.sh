#/bin/bash

time_begin="090000"
time_end="160000"
start_pid=0

while true
do
    time_now=$(date "+%H%M%S")
    echo $time_now
	xtpapidemo_pid=`ps -ef | grep XTPApiDemo | grep -v grep | awk '{print $2}'`


	if [ -z "$xtpapidemo_pid" ]
	then
        if [ $time_now -ge $time_begin -a $time_now -lt $time_end ]
        then
            if [ $start_pid -ne 0 ]
            then
                kill -9 $start_pid
            fi
            echo "restart XTPApiDemo"
            source start.sh &
            start_pid=$!
        fi
    else
        if [ $time_now -ge $time_end ]
        then
            echo "stop XTPApiDemo"
            kill -9 $start_pid
            start_pid=0
        fi
	fi

	sleep 5
done
