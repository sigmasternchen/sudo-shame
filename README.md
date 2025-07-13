# sudo-shame

**sudo-shame** is a fun [sudo](https://www.sudo.ws/) plugin that *publicly shames* users for failed sudo authentication attempts by posting about them to Mastodon, Misskey, or other services/webhooks.

When an authentication attempt fails (e.g. the user mistypes their password, or just isn't allowed to use `sudo`), `sudo-shame`'s audit plugin intercepts the event and runs an adapter script to notify the outside world. It’s a proof-of-concept, a joke, and an example of how to implement a custom plugin for sudo.

**⚠️ WARNING:** Don't use this on a production or important system. This is a toy for learning and demonstration only.

Check out the [companion blog post](https://blog.sigma-star.io/2025/07/on-writing-sudo-plugins/) for insights on writing `sudo` plugins and the story behind this project.

---

## Features

- Hooks into `sudo`'s audit interface to react to failed authentication
- Dispatches events to an external adapter (shell script) for full flexibility
- Out-of-the-box adapters for:
    - Mastodon
    - Misskey
    - Generic webhooks (Discord, Slack, etc.)
- All adapters are configurable via `/etc/sudo-shame.conf`

---

## Installation

#### Prerequisites

- Linux system with `sudo >= 1.8` supporting plugins
- `make`, `gcc`, etc. for build
- `jq`, `curl`, etc for the adapters

#### 1. Build and Install

Clone the repository and run:

```sh
cp sudo-shame.conf.example sudo-shame.conf
sudo make install
```

This will:
- Compile the plugin `shame.so`
- Install the shared object to `/usr/libexec/sudo/`
- Install all adapters to `/usr/local/lib/sudo-shame/`
- Create the adapter config in `/etc/sudo-shame.conf`
- Create a unix user/group `sudoshame` (used to drop privilege when running the adapters)

#### 2. Enable the Plugin

Edit `/etc/sudo.conf` and add one (or several) lines to activate the plugin. **Pick the line matching the service you want:**

```plaintext
# For Mastodon posting:
Module shame shame.so /usr/local/lib/sudo-shame/mastodon.sh 1002 1002

# For Misskey posting:
Module shame shame.so /usr/local/lib/sudo-shame/misskey.sh 1002 1002

# For a generic webhook:
Module shame shame.so /usr/local/lib/sudo-shame/webhook.sh 1002 1002
```

- `shame` is the symbol the plugin exports.
- `shame.so` is the file name of the shared object
- The following path points to the adapter script
- `1002 1002` should be changed to the UID and GID of the non-root user created above (the adapters drop privilege to this user). The Makefile will display the correct IDs in the `install` target.

#### 3. Configure the Adapters

In `/etc/sudo-shame.conf` you can change your adapter settings. This file is a shell-compatible script, and you can use predefined variables in your settings. For example: You could mention the user who failed authentication in the message.

The following predefined variables are available:

- `$username` contains the login name of the user
- `$user_command` is the command the user tried to execute
- `$hostname` is the hostname of the system

---

## How it Works

- The plugin is loaded by `sudo` at startup (due to `/etc/sudo.conf`)
- When a user fails sudo authentication, the plugin calls the specified adapter script, **as the safe non-root user**
- Context (username, hostname, command) is passed to the script
- The script reads `/etc/sudo-shame.conf` for service credentials and formatting
- It posts your ‘shame’ message to Mastodon, Misskey, or your webhook endpoint

---

## Adapter Scripts

All adapters live in `/usr/local/lib/sudo-shame/` after install. Available:

- **mastodon.sh** – posts a status to your Mastodon account
- **misskey.sh** – posts a note to Misskey
- **webhook.sh** – generic; can integrate with Discord, Slack, etc.


Feel free to extend or adapt them for other services!

---

## Security/Sanity

- Plugin drops root and runs adapters as a dedicated unprivileged user
- Output from adapters is relayed via `sudo`'s notice system
- **Never use this on real production systems—it’s for hacks and learning**

---

## See Also

- [On Writing Sudo Plugins — Original Blog Post](https://blog.sigma-star.io/2025/07/on-writing-sudo-plugins/)
- [sudo plugin docs](https://www.sudo.ws/man/1.8.27/sudo_plugin.man.html)

---

## License

[MIT](/LICENSE) — see LICENSE file for details.

---

*Happy shame-posting!*

---
