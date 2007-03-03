/*
 * Copyright (c) 2006 Atheme Development Group
 * Rights to this code are as documented in doc/LICENSE.
 *
 * PLAIN mechanism provider
 *
 * $Id: plain.c 7771 2007-03-03 12:46:36Z pippijn $
 */

#include "atheme.h"

DECLARE_MODULE_V1
(
	"saslserv/plain", FALSE, _modinit, _moddeinit,
	"$Id: plain.c 7771 2007-03-03 12:46:36Z pippijn $",
	"Atheme Development Group <http://www.atheme.org>"
);

list_t *mechanisms;
node_t *mnode;
static int mech_start(sasl_session_t *p, char **out, int *out_len);
static int mech_step(sasl_session_t *p, char *message, int len, char **out, int *out_len);
static void mech_finish(sasl_session_t *p);
sasl_mechanism_t mech = {"PLAIN", &mech_start, &mech_step, &mech_finish};

void _modinit(module_t *m)
{
	MODULE_USE_SYMBOL(mechanisms, "saslserv/main", "sasl_mechanisms");
	mnode = node_create();
	node_add(&mech, mnode, mechanisms);
}

void _moddeinit()
{
	node_del(mnode, mechanisms);
}

static int mech_start(sasl_session_t *p, char **out, int *out_len)
{
	return ASASL_MORE;
}

static int mech_step(sasl_session_t *p, char *message, int len, char **out, int *out_len)
{
	char auth[256];
	char pass[256];
	myuser_t *mu;

	/* Skip the authzid entirely */
	len -= strlen(message) + 1;
	if(len <= 0)
		return ASASL_FAIL;
	message += strlen(message) + 1;

	/* Copy the authcid */
	if(strlen(message) > 255)
		return ASASL_FAIL;
	len -= strlen(message) + 1;
	if(len <= 0)
		return ASASL_FAIL;
	strcpy(auth, message);
	message += strlen(message) + 1;

	/* Copy the password */
	if(strlen(message) > 255)
		return ASASL_FAIL;
	strlcpy(pass, message, len + 1);

	/* Done dissecting, now check. */
	if(!(mu = myuser_find(auth)))
		return ASASL_FAIL;

	p->username = strdup(auth);
	return verify_password(mu, pass) ? ASASL_DONE : ASASL_FAIL;
}

static void mech_finish(sasl_session_t *p)
{
}

/* vim:cinoptions=>s,e0,n0,f0,{0,}0,^0,=s,ps,t0,c3,+s,(2s,us,)20,*30,gs,hs
 * vim:noexpandtab
 */
