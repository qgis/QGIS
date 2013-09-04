INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckVariableExists)
INCLUDE(CheckTypeSize)
INCLUDE(CMakeDetermineCCompiler)

IF(CMAKE_COMPILER_IS_GNUCC)
	OPTION(BUILD_EXTRA_WARNINGS "extra warnings")

	IF(BUILD_EXTRA_WARNINGS)
		SET(WARN_FLAGS "-g -O2 -g2 -Wall -Wmissing-declarations -Wdeclaration-after-statement -Wcast-align -Winline -Wsign-compare -Wnested-externs -Wpointer-arith -Wl,--as-needed -Wformat-security")
		# -Wno-pointer-sign -Werror -Wbad-function-cast -Wmissing-prototypes
	ELSE(BUILD_EXTRA_WARNINGS)
		SET(WARN_FLAGS "")
	ENDIF(BUILD_EXTRA_WARNINGS)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# awk '/#include <(.*)>/ { h = substr($2,2,length($2)-2); h2=toupper(h); gsub("\\.|\\/", "_", h2); printf "%s%s%s%s%s", "CHECK_INCLUDE_FILES(", h, " HAVE_", h2, ")\n" }' spawn-fcgi.c | sort
CHECK_INCLUDE_FILES(arpa/inet.h HAVE_ARPA_INET_H)
CHECK_INCLUDE_FILES(errno.h HAVE_ERRNO_H)
CHECK_INCLUDE_FILES(fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILES(getopt.h HAVE_GETOPT_H)
CHECK_INCLUDE_FILES(grp.h HAVE_GRP_H)
CHECK_INCLUDE_FILES(netdb.h HAVE_NETDB_H)
CHECK_INCLUDE_FILES(netinet/in.h HAVE_NETINET_IN_H)
CHECK_INCLUDE_FILES(netinet/tcp.h HAVE_NETINET_TCP_H)
CHECK_INCLUDE_FILES(pwd.h HAVE_PWD_H)
CHECK_INCLUDE_FILES(stdio.h HAVE_STDIO_H)
CHECK_INCLUDE_FILES(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILES(string.h HAVE_STRING_H)
CHECK_INCLUDE_FILES(sys/ioctl.h HAVE_SYS_IOCTL_H)
CHECK_INCLUDE_FILES(sys/socket.h HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILES(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES(sys/time.h HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES(sys/un.h HAVE_SYS_UN_H)
CHECK_INCLUDE_FILES(sys/wait.h HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES(winsock2.h HAVE_WINSOCK2_H)

CHECK_FUNCTION_EXISTS(issetugid HAVE_ISSETUGID)
CHECK_FUNCTION_EXISTS(inet_pton HAVE_INET_PTON)

CHECK_C_SOURCE_COMPILES("
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	
	int main() {
		struct sockaddr_in6 s; struct in6_addr t=in6addr_any; int i=AF_INET6; s; t.s6_addr[0] = 0;
		return 0;
	}" HAVE_IPV6)

SET(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
CHECK_TYPE_SIZE(socklen_t HAVE_SOCKLEN_T)
SET(CMAKE_EXTRA_INCLUDE_FILES)

## Write out config.h
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

ADD_DEFINITIONS(-DHAVE_CONFIG_H)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(spawn-fcgi spawn-fcgi.c)

IF(CMAKE_COMPILER_IS_GNUCC)
	SET_TARGET_PROPERTIES(spawn-fcgi PROPERTIES COMPILE_FLAGS "-std=gnu99 -Wall -g -Wshadow -W -pedantic -fPIC -D_GNU_SOURCE ${WARN_FLAGS}")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

INSTALL(TARGETS spawn-fcgi DESTINATION bin)
