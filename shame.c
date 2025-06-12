#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <alloca.h>
#include <unistd.h>
#include <sys/wait.h>

#include <sudo_plugin.h>

int fallback_printf(int _, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int n = fprintf(stderr, "fallback printf: ");
	vfprintf(stderr, fmt, args);
	va_end(args);
	return n;
}

static sudo_printf_t plugin_printf = fallback_printf;

static char* command = NULL;
static char* username = NULL;

static int plugin_open(
	unsigned int version,
	sudo_conv_t conversation,
	sudo_printf_t sudo_plugin_plugin_printf,
	char * const settings[],
	char * const user_info[],
	int submit_optind,
	char * const submit_argv[],
	char * const submit_envp[],
	char * const plugin_options[],
	const char **errstr
) {
	plugin_printf = sudo_plugin_plugin_printf;
	//plugin_printf(SUDO_CONV_INFO_MSG, "open\n");

	const char* user_key = "user=";
	size_t user_key_length = strlen(user_key);

	for(int i = 0; user_info[i] != NULL; i++) {
		if (strncmp(user_info[i], user_key, user_key_length) == 0) {
			username = strdup(user_info[i] + user_key_length);
		}
	}
	if (!username) {
		username = "";
	}

	for(int i = 0; plugin_options != NULL && plugin_options[i] != NULL; i++) {
		//plugin_printf(SUDO_CONV_INFO_MSG, "opt: %s\n", plugin_options[i]);
		if (!command) {
			command = strdup(plugin_options[i]);
		}
	}

	return 1;
}

static void plugin_close(int status_type, int status) {
	//plugin_printf(SUDO_CONV_INFO_MSG, "close\n");
}

static int plugin_accept(
	const char *plugin_name,
	unsigned int plugin_type,
	char * const command_info[],
	char * const run_argv[],
	char * const run_envp[],
	const char **errstr
) {
	//plugin_printf(SUDO_CONV_INFO_MSG, "accept\n");
	return 1;
}

static void send_message() {
	if (!command) {
		//plugin_printf(SUDO_CONV_INFO_MSG, "Command missing in plugin options.\n");
		return;
	}

	char* args[] = {
		command,
		username,
		NULL
	};
	char* env[] = {
		NULL
	};

	int pipefd[2];
	pipe(pipefd);

	pid_t child = fork();
	if (child == 0) {
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		execve(command, args, env);
		exit(1);	
	} else {
		close(pipefd[1]);
		char buffer[1024];
		ssize_t bytesRead;

		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
			buffer[bytesRead] = '\0';
			plugin_printf(SUDO_CONV_INFO_MSG, "%s", buffer);
		}

		close(pipefd[0]);
		waitpid(child, NULL, 1);
	}
}

static int plugin_reject(
	const char *plugin_name,
	unsigned int plugin_type,
	const char *audit_msg,
	char * const command_info[],
	const char **errstr
) {
	//plugin_printf(SUDO_CONV_INFO_MSG, "reject\n");
	send_message();
	return 1;
}

static int plugin_error(
	const char *plugin_name,
	unsigned int plugin_type,
	const char *audit_msg,
	char * const command_info[],
	const char **errstr
) {
	//plugin_printf(SUDO_CONV_INFO_MSG, "error\n");
	return 1;
}

static int show_version(int verbose) {
	//plugin_printf(SUDO_CONV_INFO_MSG, "show_version\n");
	return 1;
}

static void register_hooks(
	int version,
	int (*register_hook)(struct sudo_hook *hook)
) {
	//plugin_printf(SUDO_CONV_INFO_MSG, "register_hooks\n");
}

static void deregister_hooks(
	int version,
	int (*deregister_hook)(struct sudo_hook *hook)
) {
	//plugin_printf(SUDO_CONV_INFO_MSG, "deregister_hooks\n");
}

struct audit_plugin shame = {
	.type = SUDO_AUDIT_PLUGIN,
	.version = SUDO_API_VERSION,
	.open = plugin_open,
	.close = plugin_close,
	.accept = plugin_accept,
	.reject = plugin_reject,
	.error = plugin_error,
	.show_version = show_version,
	.register_hooks = register_hooks,
	.deregister_hooks = deregister_hooks,
	.event_alloc = NULL,
};
