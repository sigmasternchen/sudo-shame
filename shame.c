#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
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

// Probably undefined behavior in POSIX, but I'll mainly target Linux for now.
// In glibc seteuid defaults to no-ops on -1 according to its man page, so I'm just going 
// to assume that uid_t is a signed type.
static uid_t uid = -1;
static gid_t gid = -1;
static char* script = NULL;
static char* username = "";
static char* hostname = "";
static char* command = "";

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

	const char* user_key = "user=";
	size_t user_key_length = strlen(user_key);

	const char* host_key = "host=";
	size_t host_key_length = strlen(host_key);

	for(int i = 0; user_info[i] != NULL; i++) {
		if (strncmp(user_info[i], user_key, user_key_length) == 0) {
			username = strdup(user_info[i] + user_key_length);
		}
		if (strncmp(user_info[i], host_key, host_key_length) == 0) {
			hostname = strdup(user_info[i] + host_key_length);
		}
	}


	size_t number_of_options = 0;
	for(int i = 0; plugin_options != NULL && plugin_options[i] != NULL; i++) {
		number_of_options++;
	}

	if (number_of_options >= 1) {
		script = strdup(plugin_options[0]);
	}
	if (number_of_options >= 2) {
		const char* uid_option = plugin_options[1];
		char* endptr;
		uid = strtol(uid_option, &endptr, 10);
		if (*endptr != '\0') {
			plugin_printf(SUDO_CONV_INFO_MSG, "invalid uid for sudo-shame: %s\n", uid_option);
			uid = -1;
		}
	}
	if (number_of_options >= 3) {
		const char* gid_option = plugin_options[2];
		char* endptr;
		gid = strtol(gid_option, &endptr, 10);
		if (*endptr != '\0') {
			plugin_printf(SUDO_CONV_INFO_MSG, "invalid gid for sudo-shame: %s\n", gid_option);
			gid = -1;
		}
	}

	return 1;
}

static void plugin_close(int status_type, int status) {
}

static int plugin_accept(
	const char *plugin_name,
	unsigned int plugin_type,
	char * const command_info[],
	char * const run_argv[],
	char * const run_envp[],
	const char **errstr
) {
	return 1;
}

static void invoke_script() {
	if (!script) {
		plugin_printf(SUDO_CONV_INFO_MSG, "Script missing in plugin options.\n");
		return;
	}

	char* args[] = {
		script,
		username,
		hostname,
		command,
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

		if (setuid(uid) < 0) {
			printf("seteuid: %s\n", strerror(errno));
		}
		if (setgid(gid) < 0) {
			printf("setegid: %s\n", strerror(errno));
		}

		execve(script, args, env);
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
	const char* command_key = "command=";
	size_t command_key_length = strlen(command_key);

	for(int i = 0; command_info[i] != NULL; i++) {
		if (strncmp(command_info[i], command_key, command_key_length) == 0) {
			command = strdup(command_info[i] + command_key_length);
		}
	}

	invoke_script();
	return 1;
}

static int plugin_error(
	const char *plugin_name,
	unsigned int plugin_type,
	const char *audit_msg,
	char * const command_info[],
	const char **errstr
) {
	return 1;
}

static int show_version(int verbose) {
	return 1;
}

static void register_hooks(
	int version,
	int (*register_hook)(struct sudo_hook *hook)
) {
}

static void deregister_hooks(
	int version,
	int (*deregister_hook)(struct sudo_hook *hook)
) {
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
