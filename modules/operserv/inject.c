/*
 * Copyright (c) 2003-2004 E. Will et al.
 * Copyright (c) 2005-2006 Atheme Development Group
 * Rights to this code are documented in doc/LICENSE.
 *
 * This file contains functionality which implements the OService INJECT command.
 *
 * $Id: inject.c 7771 2007-03-03 12:46:36Z pippijn $
 */

#include "atheme.h"
#include "uplink.h"

DECLARE_MODULE_V1
(
	"operserv/inject", FALSE, _modinit, _moddeinit,
	"$Id: inject.c 7771 2007-03-03 12:46:36Z pippijn $",
	"Atheme Development Group <http://www.atheme.org>"
);

static void os_cmd_inject(sourceinfo_t *si, int parc, char *parv[]);

command_t os_inject = { "INJECT", "Fakes data from the uplink (debugging tool).", PRIV_ADMIN, 1, os_cmd_inject };

list_t *os_cmdtree;
list_t *os_helptree;

void _modinit(module_t *m)
{
	MODULE_USE_SYMBOL(os_cmdtree, "operserv/main", "os_cmdtree");
	MODULE_USE_SYMBOL(os_helptree, "operserv/main", "os_helptree");

        command_add(&os_inject, os_cmdtree);
	help_addentry(os_helptree, "INJECT", "help/oservice/inject", NULL);
}

void _moddeinit()
{
	command_delete(&os_inject, os_cmdtree);
	help_delentry(os_helptree, "INJECT");
}

static void os_cmd_inject(sourceinfo_t *si, int parc, char *parv[])
{
	char *inject;
	static boolean_t injecting = FALSE;
	inject = parv[0];

	if (!config_options.raw)
		return;

	if (!inject)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "INJECT");
		command_fail(si, fault_needmoreparams, "Syntax: INJECT <parameters>");
		return;
	}

	logcommand(si, CMDLOG_ADMIN, "INJECT %s", inject);

	/* looks like someone INJECT'd an INJECT command.
	 * this is probably a bad thing.
	 */
	if (injecting == TRUE)
	{
		command_fail(si, fault_noprivs, "You cannot inject an INJECT command.");
		return;
	}

	injecting = TRUE;
	irc_parse(inject);
	injecting = FALSE;
}

/* vim:cinoptions=>s,e0,n0,f0,{0,}0,^0,=s,ps,t0,c3,+s,(2s,us,)20,*30,gs,hs
 * vim:noexpandtab
 */
