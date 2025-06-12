shame.so: shame.c
	gcc -fPIC -shared -o $@ $< -I/usr/include

make install: shame.so
	cp shame.so /usr/libexec/sudo/shame.so
	mkdir -p /usr/local/lib/sudo-shame/
	cp misskey.sh /usr/local/lib/sudo-shame/
	cp --update=none sudo-shame.conf /etc/sudo-shame.conf
	@echo ""
	@echo "Enable sudo-shame by adding this line to your /etc/sudo.conf:"
	@echo "Module shame shame.so /usr/local/lib/sudo-shame/misskey.sh"
	@echo ""
	@echo "You can change the settings in /etc/sudo-shame.conf"
