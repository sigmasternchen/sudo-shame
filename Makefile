shame.so: shame.c
	gcc -fPIC -shared -o $@ $< -I/usr/include

dependencies:
	which gcc
	which jq
	which curl

install: dependencies shame.so
	cp shame.so /usr/libexec/sudo/shame.so
	mkdir -p /usr/local/lib/sudo-shame/
	cp adapters/* /usr/local/lib/sudo-shame/
	#cp --update=none sudo-shame.conf /etc/sudo-shame.conf
	cp sudo-shame.conf /etc/sudo-shame.conf
	@echo ""
	@echo "Enable sudo-shame by adding one of these lines to your /etc/sudo.conf:"
	@echo "Module shame shame.so /usr/local/lib/sudo-shame/misskey.sh"
	@echo "Module shame shame.so /usr/local/lib/sudo-shame/webhook.sh"
	@echo ""
	@echo "You can change the settings in /etc/sudo-shame.conf"
