#!/bin/sh
username="$1"
hostname="$2"
user_command="$3"

. /etc/sudo-shame.conf

note_id="$(
  curl -s "https://$misskey_host/api/notes/create" \
    --request POST \
    --header "Authorization: Bearer $misskey_token" \
    --header 'Content-Type: application/json' \
    --data '{
      "visibility": "public",
      "cw": "'"$misskey_cw"'",
      "localOnly": false,
      "text": "'"$misskey_content"'",
      "poll": null
    }' |
  jq -r '.createdNote.id')"

printf "You've been publically shamed: https://%s/notes/%s\n" "$misskey_host" "$note_id"

