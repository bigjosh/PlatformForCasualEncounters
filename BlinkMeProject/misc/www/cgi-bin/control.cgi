#!/bin/sh
echo $QUERY_STRING >/dev/ttyATH0
echo Cache-Control: no-cache
echo Content-type: text/plain
echo 
echo Command sent
