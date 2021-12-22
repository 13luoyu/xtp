#/bin/bash

time_begin="090000"
time_end="153000"

while true
do
    time_now=$(date "+%H%M%S")
    echo $time_now
	xtp_pid=`ps -ef | grep XTP | grep -v grep | awk '{print $2}'`


	if [ -z "$xtp_pid" ]
	then
        if [ $time_now -ge $time_begin -a $time_now -lt $time_end ]
        then
            ./XTP &
			echo "start process"
            cat >> log.txt << EOF
$time_now: start process
EOF
        fi
    else
        if [ $time_now -ge $time_end ]
        then
            echo "end process"
            cat >> log.txt << EOF
$time_now: end process
EOF
        fi
	fi

	sleep 5
done
