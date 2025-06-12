#!/bin/sh

. /etc/sudo-shame.conf

noteId="$(
  curl -s "https://$host/api/notes/create" \
    --request POST \
    --header "Authorization: Bearer $token" \
    --header 'Content-Type: application/json' \
    --data '{
      "visibility": "public",
      "cw": "'"$cw"'",
      "localOnly": false,
      "text": "'"$content"'",
      "poll": null
    }' |
  jq -r '.createdNote.id')"

printf "You've been publically shamed: https://%s/notes/%s\n" "$host" "$noteId"

