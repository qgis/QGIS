#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_PWD_H
# include <grp.h>
# include <pwd.h>
#endif

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#define FCGI_LISTENSOCK_FILENO 0

/* "sys-socket.h" */
#ifdef __WIN32

# include <winsock2.h>

# define ECONNRESET WSAECONNRESET
# define EINPROGRESS WSAEINPROGRESS
# define EALREADY WSAEALREADY
# define ECONNABORTED WSAECONNABORTED
# define ioctl ioctlsocket
# define hstrerror(x) ""

#else /* _WIN32 */

# include <sys/socket.h>
# include <sys/ioctl.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/un.h>
# include <arpa/inet.h>

# include <netdb.h>

#endif /* _WIN32 */
/* end "sys-socket.h" */

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

/* for solaris 2.5 and netbsd 1.3.x */
#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif

#ifndef HAVE_ISSETUGID
static int issetugid() {
	return (geteuid() != getuid() || getegid() != getgid());
}
#endif

#if defined(HAVE_IPV6) && defined(HAVE_INET_PTON)
# define USE_IPV6
#endif

#ifdef USE_IPV6
#define PACKAGE_FEATURES " (ipv6)"
#else
#define PACKAGE_FEATURES ""
#endif

#define PACKAGE_DESC "spawn-fcgi v" PACKAGE_VERSION PACKAGE_FEATURES " - spawns FastCGI processes\n"

#define CONST_STR_LEN(s) s, sizeof(s) - 1

static int bind_socket(const char *addr, unsigned short port, const char *unixsocket, uid_t uid, gid_t gid, int mode, int backlog) {
	int fcgi_fd, socket_type, val;

	struct sockaddr_un fcgi_addr_un;
	struct sockaddr_in fcgi_addr_in;
#ifdef USE_IPV6
	struct sockaddr_in6 fcgi_addr_in6;
#endif
	struct sockaddr *fcgi_addr;

	socklen_t servlen;

	if (unixsocket) {
		memset(&fcgi_addr_un, 0, sizeof(fcgi_addr_un));

		fcgi_addr_un.sun_family = AF_UNIX;
		strcpy(fcgi_addr_un.sun_path, unixsocket);

#ifdef SUN_LEN
		servlen = SUN_LEN(&fcgi_addr_un);
#else
		/* stevens says: */
		servlen = strlen(fcgi_addr_un.sun_path) + sizeof(fcgi_addr_un.sun_family);
#endif
		socket_type = AF_UNIX;
		fcgi_addr = (struct sockaddr *) &fcgi_addr_un;

		/* check if some backend is listening on the socket
		 * as if we delete the socket-file and rebind there will be no "socket already in use" error
		 */
		if (-1 == (fcgi_fd = socket(socket_type, SOCK_STREAM, 0))) {
			fprintf(stderr, "spawn-fcgi: couldn't create socket: %s\n", strerror(errno));
			return -1;
		}

		if (0 == connect(fcgi_fd, fcgi_addr, servlen)) {
			fprintf(stderr, "spawn-fcgi: socket is already in use, can't spawn\n");
			close(fcgi_fd);
			return -1;
		}

		/* cleanup previous socket if it exists */
		if (-1 == unlink(unixsocket)) {
			switch (errno) {
			case ENOENT:
				break;
			default:
				fprintf(stderr, "spawn-fcgi: removing old socket failed: %s\n", strerror(errno));
				return -1;
			}
		}

		close(fcgi_fd);
	} else {
		memset(&fcgi_addr_in, 0, sizeof(fcgi_addr_in));
		fcgi_addr_in.sin_family = AF_INET;
		fcgi_addr_in.sin_port = htons(port);

		servlen = sizeof(fcgi_addr_in);
		socket_type = AF_INET;
		fcgi_addr = (struct sockaddr *) &fcgi_addr_in;

#ifdef USE_IPV6
		memset(&fcgi_addr_in6, 0, sizeof(fcgi_addr_in6));
		fcgi_addr_in6.sin6_family = AF_INET6;
		fcgi_addr_in6.sin6_port = fcgi_addr_in.sin_port;
#endif

		if (addr == NULL) {
			fcgi_addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef HAVE_INET_PTON
		} else if (1 == inet_pton(AF_INET, addr, &fcgi_addr_in.sin_addr)) {
			/* nothing to do */
#ifdef HAVE_IPV6
		} else if (1 == inet_pton(AF_INET6, addr, &fcgi_addr_in6.sin6_addr)) {
			servlen = sizeof(fcgi_addr_in6);
			socket_type = AF_INET6;
			fcgi_addr = (struct sockaddr *) &fcgi_addr_in6;
#endif
		} else {
			fprintf(stderr, "spawn-fcgi: '%s' is not a valid IP address\n", addr);
			return -1;
#else
		} else {
			if ((in_addr_t)(-1) == (fcgi_addr_in.sin_addr.s_addr = inet_addr(addr))) {
				fprintf(stderr, "spawn-fcgi: '%s' is not a valid IPv4 address\n", addr);
				return -1;
			}
#endif
		}
	}


	if (-1 == (fcgi_fd = socket(socket_type, SOCK_STREAM, 0))) {
		fprintf(stderr, "spawn-fcgi: couldn't create socket: %s\n", strerror(errno));
		return -1;
	}

	val = 1;
	if (setsockopt(fcgi_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		fprintf(stderr, "spawn-fcgi: couldn't set SO_REUSEADDR: %s\n", strerror(errno));
		return -1;
	}

	if (-1 == bind(fcgi_fd, fcgi_addr, servlen)) {
		fprintf(stderr, "spawn-fcgi: bind failed: %s\n", strerror(errno));
		return -1;
	}

	if (unixsocket) {
		if (0 != uid || 0 != gid) {
			if (0 == uid) uid = -1;
			if (0 == gid) gid = -1;
			if (-1 == chown(unixsocket, uid, gid)) {
				fprintf(stderr, "spawn-fcgi: couldn't chown socket: %s\n", strerror(errno));
				close(fcgi_fd);
				unlink(unixsocket);
				return -1;
			}
		}

		if (-1 != mode && -1 == chmod(unixsocket, mode)) {
			fprintf(stderr, "spawn-fcgi: couldn't chmod socket: %s\n", strerror(errno));
			close(fcgi_fd);
			unlink(unixsocket);
			return -1;
		}
	}

	if (-1 == listen(fcgi_fd, backlog)) {
		fprintf(stderr, "spawn-fcgi: listen failed: %s\n", strerror(errno));
		return -1;
	}

	return fcgi_fd;
}

static int fcgi_spawn_connection(char *appPath, char **appArgv, int fcgi_fd, int fork_count, int child_count, int pid_fd, int nofork) {
	int status, rc = 0;
	struct timeval tv = { 0, 100 * 1000 };

	pid_t child;

	while (fork_count-- > 0) {

		if (!nofork) {
			child = fork();
		} else {
			child = 0;
		}

		switch (child) {
		case 0: {
			char cgi_childs[64];
			int max_fd = 0;

			int i = 0;

			if (child_count >= 0) {
				snprintf(cgi_childs, sizeof(cgi_childs), "PHP_FCGI_CHILDREN=%d", child_count);
				putenv(cgi_childs);
			}

			if(fcgi_fd != FCGI_LISTENSOCK_FILENO) {
				close(FCGI_LISTENSOCK_FILENO);
				dup2(fcgi_fd, FCGI_LISTENSOCK_FILENO);
				close(fcgi_fd);
			}

			/* loose control terminal */
			if (!nofork) {
				setsid();

				max_fd = open("/dev/null", O_RDWR);
				if (-1 != max_fd) {
					if (max_fd != STDOUT_FILENO) dup2(max_fd, STDOUT_FILENO);
					if (max_fd != STDERR_FILENO) dup2(max_fd, STDERR_FILENO);
					if (max_fd != STDOUT_FILENO && max_fd != STDERR_FILENO) close(max_fd);
				} else {
					fprintf(stderr, "spawn-fcgi: couldn't open and redirect stdout/stderr to '/dev/null': %s\n", strerror(errno));
				}
			}

			/* we don't need the client socket */
			for (i = 3; i < max_fd; i++) {
				if (i != FCGI_LISTENSOCK_FILENO) close(i);
			}

			/* fork and replace shell */
			if (appArgv) {
				execv(appArgv[0], appArgv);

			} else {
				char *b = malloc((sizeof("exec ") - 1) + strlen(appPath) + 1);
				strcpy(b, "exec ");
				strcat(b, appPath);

				/* exec the cgi */
				execl("/bin/sh", "sh", "-c", b, (char *)NULL);
			}

			/* in nofork mode stderr is still open */
			fprintf(stderr, "spawn-fcgi: exec failed: %s\n", strerror(errno));
			exit(errno);

			break;
		}
		case -1:
			/* error */
			fprintf(stderr, "spawn-fcgi: fork failed: %s\n", strerror(errno));
			break;
		default:
			/* father */

			/* wait */
			select(0, NULL, NULL, NULL, &tv);

			switch (waitpid(child, &status, WNOHANG)) {
			case 0:
				fprintf(stdout, "spawn-fcgi: child spawned successfully: PID: %d\n", child);

				/* write pid file */
				if (pid_fd != -1) {
					/* assume a 32bit pid_t */
					char pidbuf[12];

					snprintf(pidbuf, sizeof(pidbuf) - 1, "%d", child);

					write(pid_fd, pidbuf, strlen(pidbuf));
					/* avoid eol for the last one */
					if (fork_count != 0) {
						write(pid_fd, "\n", 1);
					}
				}

				break;
			case -1:
				break;
			default:
				if (WIFEXITED(status)) {
					fprintf(stderr, "spawn-fcgi: child exited with: %d\n",
						WEXITSTATUS(status));
					rc = WEXITSTATUS(status);
				} else if (WIFSIGNALED(status)) {
					fprintf(stderr, "spawn-fcgi: child signaled: %d\n",
						WTERMSIG(status));
					rc = 1;
				} else {
					fprintf(stderr, "spawn-fcgi: child died somehow: exit status = %d\n",
						status);
					rc = status;
				}
			}

			break;
		}
	}
	close(pid_fd);

	close(fcgi_fd);

	return rc;
}

static int find_user_group(const char *user, const char *group, uid_t *uid, gid_t *gid, const char **username) {
	uid_t my_uid = 0;
	gid_t my_gid = 0;
	struct passwd *my_pwd = NULL;
	struct group *my_grp = NULL;
	char *endptr = NULL;
	*uid = 0; *gid = 0;
	if (username) *username = NULL;

	if (user) {
		my_uid = strtol(user, &endptr, 10);

		if (my_uid <= 0 || *endptr) {
			if (NULL == (my_pwd = getpwnam(user))) {
				fprintf(stderr, "spawn-fcgi: can't find user name %s\n", user);
				return -1;
			}
			my_uid = my_pwd->pw_uid;

			if (my_uid == 0) {
				fprintf(stderr, "spawn-fcgi: I will not set uid to 0\n");
				return -1;
			}

			if (username) *username = user;
		} else {
			my_pwd = getpwuid(my_uid);
			if (username && my_pwd) *username = my_pwd->pw_name;
		}
	}

	if (group) {
		my_gid = strtol(group, &endptr, 10);

		if (my_gid <= 0 || *endptr) {
			if (NULL == (my_grp = getgrnam(group))) {
				fprintf(stderr, "spawn-fcgi: can't find group name %s\n", group);
				return -1;
			}
			my_gid = my_grp->gr_gid;

			if (my_gid == 0) {
				fprintf(stderr, "spawn-fcgi: I will not set gid to 0\n");
				return -1;
			}
		}
	} else if (my_pwd) {
		my_gid = my_pwd->pw_gid;

		if (my_gid == 0) {
			fprintf(stderr, "spawn-fcgi: I will not set gid to 0\n");
			return -1;
		}
	}

	*uid = my_uid;
	*gid = my_gid;
	return 0;
}

static void show_version () {
	write(1, CONST_STR_LEN(
		PACKAGE_DESC \
		"Build-Date: " __DATE__ " " __TIME__ "\n"
	));
}

static void show_help () {
	write(1, CONST_STR_LEN(
		"Usage: spawn-fcgi [options] [-- <fcgiapp> [fcgi app arguments]]\n" \
		"\n" \
		PACKAGE_DESC \
		"\n" \
		"Options:\n" \
		" -f <path>      filename of the fcgi-application (deprecated; ignored if\n" \
		"                <fcgiapp> is given; needs /bin/sh)\n" \
		" -d <directory> chdir to directory before spawning\n" \
		" -a <address>   bind to IPv4/IPv6 address (defaults to 0.0.0.0)\n" \
		" -p <port>      bind to TCP-port\n" \
		" -s <path>      bind to Unix domain socket\n" \
		" -M <mode>      change Unix domain socket mode\n" \
		" -C <children>  (PHP only) numbers of childs to spawn (default: not setting\n" \
		"                the PHP_FCGI_CHILDREN environment variable - PHP defaults to 0)\n" \
		" -F <children>  number of children to fork (default 1)\n" \
		" -b <backlog>   backlog to allow on the socket (default 1024)\n" \
		" -P <path>      name of PID-file for spawned process (ignored in no-fork mode)\n" \
		" -n             no fork (for daemontools)\n" \
		" -v             show version\n" \
		" -?, -h         show this help\n" \
		"(root only)\n" \
		" -c <directory> chroot to directory\n" \
		" -S             create socket before chroot() (default is to create the socket\n" \
		"                in the chroot)\n" \
		" -u <user>      change to user-id\n" \
		" -g <group>     change to group-id (default: primary group of user if -u\n" \
		"                is given)\n" \
		" -U <user>      change Unix domain socket owner to user-id\n" \
		" -G <group>     change Unix domain socket group to group-id\n" \
	));
}


int main(int argc, char **argv) {
	char *fcgi_app = NULL, *changeroot = NULL, *username = NULL,
	     *groupname = NULL, *unixsocket = NULL, *pid_file = NULL,
	     *sockusername = NULL, *sockgroupname = NULL, *fcgi_dir = NULL,
	     *addr = NULL;
	char **fcgi_app_argv = { NULL };
	char *endptr = NULL;
	unsigned short port = 0;
	int sockmode = -1;
	int child_count = -1;
	int fork_count = 1;
	int backlog = 1024;
	int i_am_root, o;
	int pid_fd = -1;
	int nofork = 0;
	int sockbeforechroot = 0;
	struct sockaddr_un un;
	int fcgi_fd = -1;

	if (argc < 2) { /* no arguments given */
		show_help();
		return -1;
	}

	i_am_root = (getuid() == 0);

	while (-1 != (o = getopt(argc, argv, "c:d:f:g:?hna:p:b:u:vC:F:s:P:U:G:M:S"))) {
		switch(o) {
		case 'f': fcgi_app = optarg; break;
		case 'd': fcgi_dir = optarg; break;
		case 'a': addr = optarg;/* ip addr */ break;
		case 'p': port = strtol(optarg, &endptr, 10);/* port */
			if (*endptr) {
				fprintf(stderr, "spawn-fcgi: invalid port: %u\n", (unsigned int) port);
				return -1;
			}
			break;
		case 'C': child_count = strtol(optarg, NULL, 10);/*  */ break;
		case 'F': fork_count = strtol(optarg, NULL, 10);/*  */ break;
		case 'b': backlog = strtol(optarg, NULL, 10);/*  */ break;
		case 's': unixsocket = optarg; /* unix-domain socket */ break;
		case 'c': if (i_am_root) { changeroot = optarg; }/* chroot() */ break;
		case 'u': if (i_am_root) { username = optarg; } /* set user */ break;
		case 'g': if (i_am_root) { groupname = optarg; } /* set group */ break;
		case 'U': if (i_am_root) { sockusername = optarg; } /* set socket user */ break;
		case 'G': if (i_am_root) { sockgroupname = optarg; } /* set socket group */ break;
		case 'S': if (i_am_root) { sockbeforechroot = 1; } /* open socket before chroot() */ break;
		case 'M': sockmode = strtol(optarg, NULL, 8); /* set socket mode */ break;
		case 'n': nofork = 1; break;
		case 'P': pid_file = optarg; /* PID file */ break;
		case 'v': show_version(); return 0;
		case '?':
		case 'h': show_help(); return 0;
		default:
			show_help();
			return -1;
		}
	}

	if (optind < argc) {
		fcgi_app_argv = &argv[optind];
	}

	if (NULL == fcgi_app && NULL == fcgi_app_argv) {
		fprintf(stderr, "spawn-fcgi: no FastCGI application given\n");
		return -1;
	}

	if (0 == port && NULL == unixsocket) {
		fprintf(stderr, "spawn-fcgi: no socket given (use either -p or -s)\n");
		return -1;
	} else if (0 != port && NULL != unixsocket) {
		fprintf(stderr, "spawn-fcgi: either a Unix domain socket or a TCP-port, but not both\n");
		return -1;
	}

	if (unixsocket && strlen(unixsocket) > sizeof(un.sun_path) - 1) {
		fprintf(stderr, "spawn-fcgi: path of the Unix domain socket is too long\n");
		return -1;
	}

	/* SUID handling */
	if (!i_am_root && issetugid()) {
		fprintf(stderr, "spawn-fcgi: Are you nuts? Don't apply a SUID bit to this binary\n");
		return -1;
	}

	if (nofork) pid_file = NULL; /* ignore pid file in no-fork mode */

	if (pid_file &&
	    (-1 == (pid_fd = open(pid_file, O_WRONLY | O_CREAT | O_EXCL | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))) {
		struct stat st;
		if (errno != EEXIST) {
			fprintf(stderr, "spawn-fcgi: opening PID-file '%s' failed: %s\n",
				pid_file, strerror(errno));
			return -1;
		}

		/* ok, file exists */

		if (0 != stat(pid_file, &st)) {
			fprintf(stderr, "spawn-fcgi: stating PID-file '%s' failed: %s\n",
				pid_file, strerror(errno));
			return -1;
		}

		/* is it a regular file ? */

		if (!S_ISREG(st.st_mode)) {
			fprintf(stderr, "spawn-fcgi: PID-file exists and isn't regular file: '%s'\n",
				pid_file);
			return -1;
		}

		if (-1 == (pid_fd = open(pid_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
			fprintf(stderr, "spawn-fcgi: opening PID-file '%s' failed: %s\n",
				pid_file, strerror(errno));
			return -1;
		}
	}

	if (i_am_root) {
		uid_t uid, sockuid;
		gid_t gid, sockgid;
		const char* real_username;

		if (-1 == find_user_group(username, groupname, &uid, &gid, &real_username))
			return -1;

		if (-1 == find_user_group(sockusername, sockgroupname, &sockuid, &sockgid, NULL))
			return -1;

		if (uid != 0 && gid == 0) {
			fprintf(stderr, "spawn-fcgi: WARNING: couldn't find the user for uid %i and no group was specified, so only the user privileges will be dropped\n", (int) uid);
		}

		if (0 == sockuid) sockuid = uid;
		if (0 == sockgid) sockgid = gid;

		if (sockbeforechroot && -1 == (fcgi_fd = bind_socket(addr, port, unixsocket, sockuid, sockgid, sockmode, backlog)))
			return -1;

		/* Change group before chroot, when we have access
		 * to /etc/group
		 */
		if (gid != 0) {
			setgid(gid);
			setgroups(0, NULL);
			if (real_username) {
				initgroups(real_username, gid);
			}
		}

		if (changeroot) {
			if (-1 == chroot(changeroot)) {
				fprintf(stderr, "spawn-fcgi: chroot('%s') failed: %s\n", changeroot, strerror(errno));
				return -1;
			}
			if (-1 == chdir("/")) {
				fprintf(stderr, "spawn-fcgi: chdir('/') failed: %s\n", strerror(errno));
				return -1;
			}
		}

		if (!sockbeforechroot && -1 == (fcgi_fd = bind_socket(addr, port, unixsocket, sockuid, sockgid, sockmode, backlog)))
			return -1;

		/* drop root privs */
		if (uid != 0) {
			setuid(uid);
		}
	} else {
		if (-1 == (fcgi_fd = bind_socket(addr, port, unixsocket, 0, 0, sockmode, backlog)))
			return -1;
	}

	if (fcgi_dir && -1 == chdir(fcgi_dir)) {
		fprintf(stderr, "spawn-fcgi: chdir('%s') failed: %s\n", fcgi_dir, strerror(errno));
		return -1;
	}

	return fcgi_spawn_connection(fcgi_app, fcgi_app_argv, fcgi_fd, fork_count, child_count, pid_fd, nofork);
}
