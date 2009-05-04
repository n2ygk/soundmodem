/*****************************************************************************/

/*
 *      ptt.c  --  PTT signalling.
 *
 *      Copyright (C) 1999-2000, 2002
 *        Thomas Sailer (t.sailer@alumni.ethz.ch)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*****************************************************************************/

#define _GNU_SOURCE
#define _REENTRANT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "modem.h"
#include "pttio.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef HAVE_SYS_IOCCOM_H
#include <sys/ioccom.h>
#endif

#ifdef HAVE_LINUX_PPDEV_H
#include <linux/ppdev.h>
#else
#include "ppdev.h"
#endif

#if defined HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

/* ---------------------------------------------------------------------- */

struct modemparams pttparams[] = {
	{ "file", "PTT Driver", "Path name of the serial or parallel port driver for outputting PTT", "none", MODEMPAR_COMBO, 
	  { c: { { "none", "/dev/ttyS0", "/dev/ttyS1", "/dev/parport0", "/dev/parport1" } } } },
#ifdef HAVE_LIBHAMLIB
	{ "hamlib_model", "Hamlib model", "Model number", "", MODEMPAR_STRING }, 
	{ "hamlib_params", "Rig configuration params", "Rig configuration params", "", MODEMPAR_STRING },
#endif
	{ NULL }
};

/* ---------------------------------------------------------------------- */

int pttinit(struct pttio *state, const char *params[])
{
	const char *path = params[0];
	int fd;
	unsigned char x;
	unsigned int y = 0;

	state->mode = noport;
	if (!path || !path[0] || !strcasecmp(path, "none"))
		return 0;
#ifdef HAVE_LIBHAMLIB
	const char *hamlib_model = params[1];
	if (hamlib_model && hamlib_model[0]) {
		int my_rig_error = ~RIG_OK;
		char *hamlib_params = params[2] ? strdup(params[2]) : NULL;
		int rig_modl ;
		char *ptr_key = hamlib_params;

		logprintf(MLOG_INFO, "Hamlib: pttinit: path=%s model=%s params=%s\n",
			  path ? path : "NULL",
			  hamlib_model ? hamlib_model : "NULL",
			  hamlib_params ? hamlib_params : "NULL");
		if (1 != sscanf(hamlib_model, "%d", &rig_modl)) {
			logprintf(MLOG_ERROR, "Hamlib: Invalid model:\"%s\"\n", hamlib_model);
			goto the_end;
		}
		state->mode = hamlibport;
		state->u.rig_ptr = rig_init(rig_modl);
		if(state->u.rig_ptr == NULL) {
			logprintf(MLOG_ERROR, "Hamlib: rig_init model=%dn", rig_modl);
			goto the_end;
		}
		strncpy(state->u.rig_ptr->state.rigport.pathname, path, FILPATHLEN);

		logprintf(MLOG_INFO, "Hamlib: pttinit parsing %s\n", ptr_key ? ptr_key : "NULL" );

		while (ptr_key && *ptr_key != '\0') {
			char * ptr_val = strchr(ptr_key, '=');
			if (ptr_val) {
				*ptr_val++ = '\0';
			}

			char * ptr_key_next = ptr_val ? strchr(ptr_val, ',') : NULL ;
			if (ptr_key_next) {
				*ptr_key_next++ = '\0';
			}
			my_rig_error = rig_set_conf(state->u.rig_ptr,
						    rig_token_lookup(state->u.rig_ptr, ptr_key), ptr_val);
			if (my_rig_error != RIG_OK) {
				logprintf(MLOG_ERROR,
					  "Hamlib: rig_set_conf: %s=%s : %s\n",
					  ptr_key ? ptr_key : NULL,
					  ptr_val ? ptr_val : NULL,
					  rigerror(my_rig_error));
				goto the_end;
			}
			ptr_key = ptr_key_next;
		}
		my_rig_error = rig_open(state->u.rig_ptr);
		if (RIG_OK != my_rig_error) {
			logprintf(MLOG_ERROR, "Hamlib: rig_open: %s\n", rigerror(my_rig_error));
			goto the_end;
		}
		pttsetptt(state, 0);
		pttsetdcd(state, 0);
		my_rig_error = RIG_OK;
	the_end:
		free(hamlib_params);
        	return my_rig_error == RIG_OK ? 0 : -1 ;
	}
#endif

	if ((fd = open(path, O_RDWR, 0)) < 0) {
		logprintf(MLOG_ERROR, "Cannot open PTT device \"%s\"\n", path);
		return -1;
	}
	if (!ioctl(fd, TIOCMBIC, &y)) {
		state->u.fd = fd;
		state->mode = serport;
	} else if (!ioctl(fd, PPCLAIM, 0) && !ioctl(fd, PPRDATA, &x)) {
		state->u.fd = fd;
		state->mode = parport;
	} else {
		logprintf(MLOG_ERROR, "Device \"%s\" neither parport nor serport\n", path);
		close(fd);
		return -1;
	}
	pttsetptt(state, 0);
	pttsetdcd(state, 0);
        return 0;
}

void pttsetptt(struct pttio *state, int pttx)
{
	unsigned char reg;

	if (!state)
		return;
	state->ptt = !!pttx;
	switch (state->mode) {
#ifdef HAVE_LIBHAMLIB
	case hamlibport:
	{
		if (!state->u.rig_ptr)
			return;
		logprintf(MLOG_INFO, "Hamlib: pttsetptt state=%d\n", state->ptt);
		ptt_t my_ptt ;
		int my_rig_error;
		if (state->ptt)
		    my_ptt = RIG_PTT_ON;
		else
		    my_ptt = RIG_PTT_OFF;
		my_rig_error = rig_set_ptt(state->u.rig_ptr, RIG_VFO_CURR, my_ptt);
		if(RIG_OK != my_rig_error) {
			logprintf(MLOG_ERROR, "Hamlib: rig_set_ptt %s\n", rigerror(my_rig_error) );
		}
		return;
	}	
#endif

	case serport:
	{
		if (state->u.fd == -1)
			return;
#if 0
		unsigned int y = TIOCM_RTS;
		ioctl(state->u.fd, state->ptt ? TIOCMBIS : TIOCMBIC, &y);
#else
		unsigned int y;
		ioctl(state->u.fd, TIOCMGET, &y);
		if (state->ptt)
		    y |= TIOCM_RTS;
		else
		    y &= ~TIOCM_RTS;
		ioctl(state->u.fd, TIOCMSET, &y);
#endif
		return;
	}

	case parport:
	{
		if (state->u.fd == -1)
			return;
		reg = state->ptt | (state->dcd << 1);
		ioctl(state->u.fd, PPWDATA, &reg);
		return;
	}

	default:
		return;
	}
}

void pttsetdcd(struct pttio *state, int dcd)
{
	unsigned char reg;

	if (!state)
		return;
	state->dcd = !!dcd;
	switch (state->mode) {
#ifdef HAVE_LIBHAMLIB
	/* For Hamlib, it does not make sense to set DCD. */
	case hamlibport:
		return;
#endif

	case serport:
	{
		if (state->u.fd == -1)
			return;
#if 0
		unsigned int y = TIOCM_DTR;
		ioctl(state->u.fd, state->dcd ? TIOCMBIS : TIOCMBIC, &y);
#else
		unsigned int y;
		ioctl(state->u.fd, TIOCMGET, &y);
		if (state->dcd)
		    y |= TIOCM_DTR;
		else
		    y &= ~TIOCM_DTR;
		ioctl(state->u.fd, TIOCMSET, &y);
#endif
		return;
	}

	case parport:
	{
		if (state->u.fd == -1)
			return;
		reg = state->ptt | (state->dcd << 1);
		ioctl(state->u.fd, PPWDATA, &reg);
		return;
	}

	default:
		return;
	}
}

void pttrelease(struct pttio *state)
{
	if (!state)
		return;
	pttsetptt(state, 0);
	pttsetdcd(state, 0);
	switch (state->mode) {
#ifdef HAVE_LIBHAMLIB
	case hamlibport:
	{
		logprintf(MLOG_INFO, "Hamlib: pttrelease\n");
		int my_rig_error;
		my_rig_error = rig_close(state->u.rig_ptr);
		if(RIG_OK != my_rig_error) {
			logprintf(MLOG_ERROR, "Hamlib: rig_close: %s\n", rigerror(my_rig_error) );
		}
		my_rig_error = rig_cleanup(state->u.rig_ptr);
		if(RIG_OK != my_rig_error) {
			logprintf(MLOG_ERROR, "Hamlib: rig_cleanup: %s\n", rigerror(my_rig_error) );
		}
		break;
	}
#endif

	case serport:
	case parport:
		close(state->u.fd);
		break;

	default:
		break;
	}
	state->mode = noport;
}
