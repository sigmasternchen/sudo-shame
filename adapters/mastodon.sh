#!/bin/bash
username="$1"
hostname="$2"
user_command="$3"

. /etc/sudo-shame.conf

post_id="$(
  curl -s "https://$mastodon_host/api/v1/statuses" \
    --request POST \
    --header "Authorization: Bearer $mastodon_token" \
    --header 'Content-Type: application/json' \
    --data '{
      "visibility": "public",
      "spoiler_text": "'"$mastodon_cw"'",
      "status": "'"$mastodon_content"'"
    }' |
  jq -r '.id')"

username="$(
curl -s "https://$mastodon_host/api/v1/statuses/$post_id" \
    --header "Authorization: Bearer $mastodon_token" |
  jq -r '.account.username')"

printf "You've been publically shamed: $mastodon_url_format\n" "$mastodon_host" "$username" "$post_id"

