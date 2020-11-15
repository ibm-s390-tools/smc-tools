/*
 * SMC Tools - Shared Memory Communication Tools
 *
 * Copyright IBM Corp. 2020
 *
 * Author(s): Guvenc Gulce <guvenc@linux.ibm.com>
 *
 * Userspace program for SMC Information display
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "smctools_common.h"
#include "libnetlink.h"
#include "util.h"
#include "linkgroup.h"
#include "dev.h"

struct rtnl_handle rth = { .fd = -1 };
static int detail_level = 0;
#if defined(SMCD)
char *myname = "smcd";
#elif defined(SMCR)
char *myname = "smcr";
#else
char *myname = "smc";
#endif

static void version(void)
{
	fprintf(stderr,
		"%s utility, smc-tools-%s (%s)\n", myname, RELEASE_STRING, RELEASE_LEVEL);
	exit(-1);
}
static void usage(void)
{
	fprintf(stderr,
		"Usage: %s  [ OPTIONS ] OBJECT {COMMAND | help}\n"
		"where  OBJECT := {linkgroup | device}\n"
#if defined(SMCD)
		"       OPTIONS := {-V[ersion]}\n", myname);
#else
		"       OPTIONS := {-V[ersion] | -v[erbose] | -vv[erbose]}\n", myname);
#endif
	exit(-1);
}

static int invoke_help(int argc, char **argv, int k)
{
	usage();
	return 0;
}

static const struct cmd {
	const char *cmd;
	int (*func)(int argc, char **argv, int detail_level);
} cmds[] = {
	{ "device",	invoke_devs },
	{ "linkgroup",	invoke_lgs },
	{ "help",	invoke_help },
	{ 0 }
};

static int run_cmd(const char *argv0, int argc, char **argv)
{
	const struct cmd *c;

	for (c = cmds; c->cmd; ++c) {
		if (contains(argv0, c->cmd) == 0)
			return -(c->func(argc-1, argv+1, detail_level));
	}

	fprintf(stderr, "Object \"%s\" is unknown, try \"%s help\".\n", argv0, myname);
	return EXIT_FAILURE;
}

int main(int argc, char **argv)
{

	while (argc > 1) {
		char *opt = argv[1];

		if (strcmp(opt, "--") == 0) {
			argc--; argv++;
			break;
		}
		if (opt[0] != '-')
			break;
		if (opt[1] == '-')
			opt++;
		if (contains(opt, "-Version") == 0) {
			version();
		} else if (contains(opt, "-verbose") == 0) {
			detail_level = SMC_DETAIL_LEVEL_V;
		} else if (contains(opt, "-vverbose") == 0) {
			detail_level = SMC_DETAIL_LEVEL_VV;
		} else if (contains(opt, "-help") == 0) {
			usage();
		} else {
			fprintf(stderr,
				"Option \"%s\" is unknown, try \"%s help\".\n",
				opt, myname);
			exit(-1);
		}
		argc--;	argv++;
	}

	if (gen_nl_open(myname) < 0)
		exit(1);

	if (argc > 1)
		return run_cmd(argv[1], argc-1, argv+1);

	rtnl_close(&rth);
	usage();
}
