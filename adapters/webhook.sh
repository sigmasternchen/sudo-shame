#!/bin/sh
username="$1"
hostname="$2"
user_command="$3"

. /etc/sudo-shame.conf

curl -s "$webhook_url" \
    --request POST \
    --header "$webhook_custom_header" \
    --data "$webhook_body" > /dev/null

