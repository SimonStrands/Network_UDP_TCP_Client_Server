#!/bin/bash

##Variables update as to fit your scenario
portFORK=8285
portTHREAD=8282

head -c 10000 < /dev/urandom > big
head -c 1000 < /dev/urandom > small 

## Clean up statistics files
rm -rf statistics_fork.log
rm -rf statistics_thread.log

echo "Create a RANDOM text file."
randomString=$(echo $RANDOM | md5sum | head -c 20)
echo -e "$randomString"  > randomFile

echo "Calling curl"
##echo "Testing that the forked server can retrevie the newly created randomFile"
fString=$(curl -s http://127.0.0.1:$portFORK/randomFile)
echo "Done calling curl"

if [[ "$randomString" == "$fString" ]]; then
    echo "SUMMARY: The thread server, correctly delivered the file (randomFile)."
else
    echo "ERRROR: The thread server, _did not_ work as expectec (randomFile)."
    echo -e "RandomSRC: $randomString"
    echo $randomString | cat -v
    echo -e "Recived Server: $fString"
    echo $fString | cat -v
    exit 1
fi