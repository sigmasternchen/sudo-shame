shame.so: shame.c
	gcc -fPIC -shared -Wall -Wpedantic -o $@ $< -I/usr/include

dependencies:
	which gcc
	which jq
	which curl

install: dependencies shame.so
	cp shame.so /usr/libexec/sudo/shame.so
	mkdir -p /usr/local/lib/sudo-shame/
	cp adapters/* /usr/local/lib/sudo-shame/
	cp --update=none sudo-shame.conf /etc/sudo-shame.conf
	useradd sudoshame || true
	chown sudoshame /etc/sudo-shame.conf
	chmod 544 /etc/sudo-shame.conf
	@echo ""
	@echo "Enable sudo-shame by adding one of these lines to your /etc/sudo.conf:"
	@echo "Module shame shame.so /usr/local/lib/sudo-shame/mastodon.sh $$(id -u sudoshame) $$(id -g sudoshame)"
	@echo "Module shame shame.so /usr/local/lib/sudo-shame/misskey.sh $$(id -u sudoshame) $$(id -g sudoshame)"
	@echo "Module shame shame.so /usr/local/lib/sudo-shame/webhook.sh $$(id -u sudoshame) $$(id -g sudoshame)"
	@echo ""
	@echo "You can change the settings in /etc/sudo-shame.conf"
