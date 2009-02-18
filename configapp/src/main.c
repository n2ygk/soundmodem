/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "configapp.h"

#include "getopt.h"

#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"

#ifdef WIN32

#include <windows.h>

#else

/* libxml includes */
#include <libxml/tree.h>
#include <libxml/parser.h>

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "simd.h"

/* ---------------------------------------------------------------------- */
  
GtkWidget *mainwindow, *specwindow, *scopewindow, *receivewindow, *p3dwindow;
GtkTreeModel *configmodel;

/* ---------------------------------------------------------------------- */

extern struct modulator afskmodulator;
extern struct demodulator afskdemodulator;

extern struct modulator fskmodulator;
extern struct demodulator fskdemodulator;
extern struct demodulator fskpspdemodulator;
extern struct modulator fskeqmodulator;
extern struct demodulator fskeqdemodulator;

extern struct modulator pammodulator;
extern struct demodulator pamdemodulator;

extern struct modulator pskmodulator;
extern struct demodulator pskdemodulator;

extern struct modulator newqpskmodulator;
extern struct demodulator newqpskdemodulator;

/* ---------------------------------------------------------------------- */

struct modulator modchain_x = {
	&afskmodulator,
	"Off",
	NULL,
	NULL,
	NULL,
	NULL
};

struct demodulator demodchain_x = {
	&afskdemodulator,
	"Off",
	NULL,
	NULL,
	NULL
};

#ifdef HAVE_ALSA
#define ALSA_STR , "alsa"
#else /* HAVE_ALSA */
#define ALSA_STR 
#endif /* HAVE_ALSA */

struct modemparams ioparam_type[] = {
	{ "type", "Audio IO Mode", "Audio IO Mode", "soundcard", MODEMPAR_COMBO, 
	  { c: { { "soundcard", "file", "simulation" ALSA_STR } } } },
	{ NULL }
};

#undef ALSA_STR

#ifdef WIN32

struct modemparams chaccparams_x[] = {
	{ "txdelay", "TxDelay", "Transmitter Keyup delay in ms", "150", MODEMPAR_NUMERIC, { n: { 0, 2550, 10, 50 } } },
	{ "fulldup", "Full Duplex", "Full Duplex", "0", MODEMPAR_CHECKBUTTON },
	{ NULL }
};

#else /* WIN32 */

struct modemparams pktkissparams_x[] = {
	{ "file", "File", "File (symlink) to send the KISS stream to", "/dev/soundmodem0",
	  MODEMPAR_COMBO, { c: { { "/dev/soundmodem0", "/dev/soundmodem1", "/dev/soundmodem2", "/dev/soundmodem3" } } } },
	{ "unlink", "Unlink File", "Unlink File (above) on setup", "1", MODEMPAR_CHECKBUTTON },
	{ NULL }
};

struct modemparams pktmkissparams_x[] = {
	{ "ifname", "Interface Name", "Name of the Kernel KISS Interface", "sm0", MODEMPAR_COMBO,
	  { c: { { "sm0", "sm1", "sm2", "ax0" } } } },
	{ "hwaddr", "Callsign", "Callsign (Hardware Address)", "", MODEMPAR_STRING },
	{ "ip", "IP Address", "IP Address (mandatory)", "10.0.0.1", MODEMPAR_STRING },
	{ "netmask", "Network Mask", "Network Mask", "255.255.255.0", MODEMPAR_STRING },
	{ "broadcast", "Broadcast Address", "Broadcast Address", "10.0.0.255", MODEMPAR_STRING },
	{ NULL }
};

struct modemparams chaccparams_x[] = {
	{ "txdelay", "TxDelay", "Transmitter Keyup delay in ms", "150", MODEMPAR_NUMERIC, { n: { 0, 2550, 10, 50 } } },
	{ "slottime", "Slot Time", "Slot Time in ms (normally 100ms)", "100", MODEMPAR_NUMERIC, { n: { 0, 2550, 10, 50 } } },
	{ "ppersist", "P-Persistence", "P-Persistence", "40", MODEMPAR_NUMERIC, { n: { 0, 255, 1, 10 } } },
	{ "fulldup", "Full Duplex", "Full Duplex", "0", MODEMPAR_CHECKBUTTON },
        { "txtail", "TxTail", "Transmitter Tail delay in ms", "10", MODEMPAR_NUMERIC, { n: { 0, 2550, 10, 50 } } },
	{ NULL }
};

#endif /* WIN32 */

/* ---------------------------------------------------------------------- */

#ifdef WIN32

#define REGISTRYPATH "SOFTWARE\\FlexNet\\SoundModem"
#define REGISTRYKEY  HKEY_LOCAL_MACHINE

#define MAXCHAN 16

static struct {
        unsigned int nrchan;
        struct onechannel {
                char name[16];
        } chan[MAXCHAN];
} channels;

static int chancompare(struct onechannel *i, struct onechannel *j)
{
        return strcmp(i->name, j->name);
}

static int enumchannels(const char *cfgname)
{
        char name[256];
        HKEY regkey;
        LONG err;
        DWORD len;
        DWORD index = 0;
        
        channels.nrchan = 0;
        snprintf(name, sizeof(name), "%s\\%s", REGISTRYPATH, cfgname);
        if ((err = RegOpenKeyEx(REGISTRYKEY, name, 0, KEY_READ, &regkey)) != ERROR_SUCCESS) {
                g_printerr("RegOpenKeyEx(%s) returned 0x%lx\n", name, err);
                return -1;
        }
        while (channels.nrchan < MAXCHAN) {
                len = sizeof(channels.chan[channels.nrchan].name);
                if ((RegEnumKeyEx(regkey, index, channels.chan[channels.nrchan].name, &len,
                                  NULL, NULL, NULL, NULL)) != ERROR_SUCCESS)
                        break;
                index++;
                if (isdigit(channels.chan[channels.nrchan].name[0]))
                        channels.nrchan++;
        }
        RegCloseKey(regkey);
        if (!channels.nrchan)
                return 0;
        qsort(&channels.chan[0], channels.nrchan, sizeof(channels.chan[0]), chancompare);
        return 0;
}

int xml_newconfig(const char *newname)
{
        HKEY regkey;
        LONG err;
        DWORD dispo;
        char name[256];

        if (strchr(newname, '\\'))
                return -1;
        snprintf(name, sizeof(name), "%s\\%s", REGISTRYPATH, newname);
        if ((err = RegCreateKeyEx(REGISTRYKEY, name, 0, "", REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, NULL, &regkey, &dispo)) != ERROR_SUCCESS) {
                g_printerr("RegCreateKeyEx(%s) returned 0x%lx\n", name, err);
                return -1;
        }
        RegCloseKey(regkey);
        return (dispo == REG_CREATED_NEW_KEY) ? 0 : -1;
}

const char *xml_newchannel(const char *cfgname)
{
        if (enumchannels(cfgname))
                return NULL;
        if (channels.nrchan >= MAXCHAN)
                return NULL;
        if (channels.nrchan == 0)
                strncpy(channels.chan[channels.nrchan].name, "0",
                        sizeof(channels.chan[channels.nrchan].name));
        else     
                snprintf(channels.chan[channels.nrchan].name,
                         sizeof(channels.chan[channels.nrchan].name),
                         "%ld", strtoul(channels.chan[channels.nrchan-1].name, NULL, 0)+1);
        channels.nrchan++;
        return channels.chan[channels.nrchan-1].name;
}

static int deletekeyx(HKEY key)
{
        char name[128];
        LONG err;
        DWORD len;
        HKEY key2;
        int ret = 0;
        
        for (;;) {
                len = sizeof(name);
                if ((RegEnumKeyEx(key, 0, name, &len, NULL, NULL, NULL, NULL)) != ERROR_SUCCESS)
                        return ret;
                if ((err = RegOpenKeyEx(key, name, 0, KEY_ALL_ACCESS, &key2)) != ERROR_SUCCESS) {
                        g_printerr("RegOpenKeyEx(%s) returned 0x%lx\n", name, err);
                        return -1;
                }
                ret |= deletekeyx(key2);
                RegCloseKey(key2);
                RegDeleteKey(key, name);
        }
        return ret;
}

static int deletekey(const char *name)
{
        HKEY key;
        int ret;
        DWORD err;
        
        if ((err = RegOpenKeyEx(REGISTRYKEY, name, 0, KEY_ALL_ACCESS, &key)) != ERROR_SUCCESS) {
                g_printerr("RegOpenKeyEx(%s) returned 0x%lx\n", name, err);
                return -1;
        }
        ret = deletekeyx(key);
        RegCloseKey(key);
        RegDeleteKey(REGISTRYKEY, name);
        return ret;
}

int xml_deleteconfig(const char *newname)
{
        char name[256];

        snprintf(name, sizeof(name), "%s\\%s", REGISTRYPATH, newname);
        return deletekey(name);
}

int xml_deletechannel(const char *cfgname, const char *chname)
{
        char name[256];

        snprintf(name, sizeof(name), "%s\\%s\\%s", REGISTRYPATH, cfgname, chname);
        return deletekey(name);
}

int xml_setprop(const char *cfgname, const char *chname, const char *typname, const char *propname, const char *data)
{
        char name[256];
        HKEY key;
        DWORD err;
        
        if (chname)
                snprintf(name, sizeof(name), "%s\\%s\\%s\\%s", REGISTRYPATH, cfgname, chname, typname);
        else
                snprintf(name, sizeof(name), "%s\\%s\\%s", REGISTRYPATH, cfgname, typname);
        if ((err = RegCreateKeyEx(REGISTRYKEY, name, 0, "", REG_OPTION_NON_VOLATILE,
                                        KEY_WRITE, NULL, &key, NULL)) != ERROR_SUCCESS) {
                g_printerr("RegCreateKeyEx(%s) returned 0x%lx\n", name, err);
                return -1;
        }
        err = RegSetValueEx(key, propname, 0, REG_SZ, data, strlen(data)+1);
        RegCloseKey(key);
        if (err != ERROR_SUCCESS) {
                g_printerr("RegSetValueEx(%s,%s) returned 0x%lx\n", propname, data, err);
                return -1;
        }
        return 0;
}

int xml_getprop(const char *cfgname, const char *chname, const char *typname, const char *propname, char *buf, unsigned int bufsz)
{
        HKEY key;
        DWORD err, vtype, len;
        char name[256];
        
        buf[0] = 0;
        if (chname)
                snprintf(name, sizeof(name), "%s\\%s\\%s\\%s", REGISTRYPATH, cfgname, chname, typname);
        else
                snprintf(name, sizeof(name), "%s\\%s\\%s", REGISTRYPATH, cfgname, typname);
        if ((err = RegCreateKeyEx(REGISTRYKEY, name, 0, "", REG_OPTION_NON_VOLATILE,
                                  KEY_READ, NULL, &key, NULL)) != ERROR_SUCCESS) {
                g_printerr("RegCreateKeyEx(%s) returned 0x%lx\n", name, err);
                return -1;
        }
        len = bufsz;
        err = RegQueryValueEx(key, propname, NULL, &vtype, buf, &len);
        RegCloseKey(key);
        if (len >= bufsz)
                len = bufsz-1;
        buf[len] = 0;
        if (err != ERROR_SUCCESS) {
                g_printerr("RegQueryValueEx(%s) returned 0x%lx\n", propname, err);
                return -1;
        }
        if (vtype != REG_SZ)
                return -1;
       return len;
}

static void buildtree(void)
{
        DWORD err;
        HKEY key;
        DWORD index = 0;
        DWORD len;
        char name[64];
        unsigned int i;
        
        if ((err = RegOpenKeyEx(REGISTRYKEY, REGISTRYPATH, 0, KEY_READ, &key)) != ERROR_SUCCESS) {
                g_printerr("RegOpenKeyEx(" REGISTRYPATH ") returned 0x%lx\n", err);
                return;
        }
        for (;;) {
                len = sizeof(name);
                if ((RegEnumKeyEx(key, index, name, &len, NULL, NULL, NULL, NULL)) != ERROR_SUCCESS)
                        break;
                g_print("Configuration: %s\n", name);
                index++;
		new_configuration(name);
                if (enumchannels(name))
                        continue;
                for (i = 0; i < channels.nrchan; i++)
                        new_channel(name, channels.chan[i].name);
        }
        RegCloseKey(key);
}

#else /* WIN32 */

static xmlDocPtr doc = NULL;
static const char *cfgfile = "/etc/ax25/soundmodem.conf";

static xmlNodePtr findconfig(const char *newname)
{
	xmlNodePtr node;
	const char *name;

	for (node = doc->children->children; node; node = node->next) {
                if (!node->name || strcmp(node->name, "configuration"))
                        continue;
                name = xmlGetProp(node, "name");
                if (!name)
			continue;
		if (!strcmp(name, newname))
			return node;
	}
	return NULL;
}

static xmlNodePtr findchannel(xmlNodePtr cfg, const char *newname)
{
	xmlNodePtr node;
	const char *name;

	if (!cfg)
		return NULL;
	for (node = cfg->children; node; node = node->next) {
                if (!node->name || strcmp(node->name, "channel"))
                        continue;
                name = xmlGetProp(node, "name");
                if (!name)
			continue;
		if (!strcmp(name, newname))
			return node;
	}
	return NULL;
}

static void namechannels(xmlNodePtr cfg)
{
	xmlNodePtr node, node2;
	unsigned int ch = 0;
	char buf[64];
	const char *name;
	
	for (node = cfg->children; node; node = node->next) {
                if (!node->name || strcmp(node->name, "channel"))
                        continue;
                name = xmlGetProp(node, "name");
                if (name) {
			node2 = findchannel(cfg, name);
			if (node2 == node)
				continue;
		}
		for (;;) {
			sprintf(buf, "Channel %u", ch++);
			xmlSetProp(node, "name", buf);
			node2 = findchannel(cfg, buf);
			if (node2 == node)
				break;
		}
	}
}

int xml_newconfig(const char *newname)
{
	xmlNodePtr node = findconfig(newname);

	if (node)
		return -1;
	node = xmlNewChild(doc->children, NULL, "configuration", NULL);
	xmlSetProp(node, "name", newname);
	return 0;
}

const char *xml_newchannel(const char *cfgname)
{
	xmlNodePtr node, node2;

	node = findconfig(cfgname);
	if (!node)
		return NULL;
	node2 = xmlNewChild(node, NULL, "channel", NULL);
	namechannels(node);
	return xmlGetProp(node2, "name");
}

int xml_deleteconfig(const char *newname)
{
	xmlNodePtr node = findconfig(newname);
	if (!node)
		return -1;
	xmlUnlinkNode(node);
	xmlFreeNode(node);
	return 0;
}

int xml_deletechannel(const char *cfgname, const char *chname)
{
	xmlNodePtr node = findconfig(cfgname);
	node = findchannel(node, chname);
	if (!node)
		return -1;
	xmlUnlinkNode(node);
	xmlFreeNode(node);
	return 0;
}

static xmlNodePtr propnode(const char *cfgname, const char *chname, const char *typname, int create)
{
	xmlNodePtr node2, node = findconfig(cfgname);
	if (chname)
		node = findchannel(node, chname);
	if (!node)
		return NULL;
	for (node2 = node->children; node2; node2 = node2->next) {
		if (!node2->name || strcmp(node2->name, typname))
			continue;
		return node2;
	}
	if (!create)
		return NULL;
	return xmlNewChild(node, NULL, typname, NULL);
}

int xml_setprop(const char *cfgname, const char *chname, const char *typname, const char *propname, const char *data)
{
	xmlNodePtr node = propnode(cfgname, chname, typname, 1);
	if (!node)
		return -1;
	xmlSetProp(node, propname, data);
	return 0;
}

int xml_getprop(const char *cfgname, const char *chname, const char *typname, const char *propname, char *buf, unsigned int bufsz)
{
	xmlNodePtr node = propnode(cfgname, chname, typname, 0);
        const char *cp;
        
        buf[0] = 0;
	if (!node)
		return -1;
        if (!(cp = xmlGetProp(node, propname)))
                return -1;
        strncpy(buf, cp, bufsz);
        buf[bufsz-1] = 0;
	return strlen(buf);
}

static void buildtree(xmlNodePtr xnode)
{
	xmlNodePtr node;
	const char *name;

	for (; xnode; xnode = xnode->next) {
                if (!xnode->name || strcmp(xnode->name, "configuration"))
                        continue;
                name = xmlGetProp(xnode, "name");
                if (!name)
			continue;
		namechannels(xnode);
		new_configuration(name);
		/* now add channels */
		for (node = xnode->children; node; node = node->next) {
			if (!node->name || strcmp(node->name, "channel"))
				continue;
			new_channel(name, xmlGetProp(node, "name"));
		}
	}
}

#endif /* WIN32 */

/* ---------------------------------------------------------------------- */

int main (int argc, char *argv[])
{
        static const struct option long_options[] = {
		{ "expert", no_argument, 0, 'x' },
		{ 0, 0, 0, 0 }
        };
        int c, err = 0;
        unsigned int verblevel = 10, tosyslog = 0, simd = 1, expert = 0;

        afskmodulator.next = &fskmodulator;
        afskdemodulator.next = &fskdemodulator;
        fskmodulator.next = &pammodulator;
        fskdemodulator.next = &fskpspdemodulator;
        fskpspdemodulator.next = &pamdemodulator;
        fskeqdemodulator.next = &pamdemodulator;
        pammodulator.next = &pskmodulator;
        pamdemodulator.next = &pskdemodulator;
	pskmodulator.next = &newqpskmodulator;
	pskdemodulator.next = &newqpskdemodulator;
	newqpskdemodulator.next = &p3ddemodulator;

#ifdef ENABLE_NLS
	bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain(PACKAGE);
#endif

	gtk_set_locale();
	gtk_init(&argc, &argv);

	add_pixmap_directory(PACKAGE_DATA_DIR "/pixmaps");
	add_pixmap_directory(PACKAGE_SOURCE_DIR "/pixmaps");

	mainwindow = create_mainwindow();
	gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_object_get_data(GTK_OBJECT(mainwindow), "confignotebook")), 0);
	gtk_notebook_remove_page(GTK_NOTEBOOK(gtk_object_get_data(GTK_OBJECT(mainwindow), "confignotebook")), 0);
	gtk_widget_hide(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "newchannel")));
	gtk_widget_hide(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "deleteconfiguration")));
	gtk_widget_hide(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "deletechannel")));
	gtk_widget_hide(GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(mainwindow), "diagnostics")));
	specwindow = create_specwindow();
	scopewindow = create_scopewindow();
	receivewindow = create_receivewindow();
	p3dwindow = create_p3dwindow();
	configmodel = create_configmodel();

        {
                GtkWidget *w;
                GtkStyle *st;
		PangoFontDescription *font_desc = pango_font_description_from_string ("monospace 10");
                if (!font_desc) {
                        g_printerr("Cannot load monospace\n");
                } else {
                        w = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(p3dwindow), "packetraw"));
                        st = gtk_style_copy(w->style);
                        gtk_style_unref(w->style);
                        st->font_desc = font_desc;
                        gtk_style_ref(st);
                        w->style = st;
                        w = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(p3dwindow), "packetcooked"));
                        gtk_style_unref(w->style);
                        gtk_style_ref(st);
                        w->style = st;
                }
	}

        while ((c = getopt_long(argc, argv, "v:sS", long_options, NULL)) != EOF) {
                switch (c) {
                case 'v':
                        verblevel = strtoul(optarg, NULL, 0);
                        break;

                case 's':
                        tosyslog = 1;
                        break;

                case 'S':
                        simd = 0;
                        break;

                case 'x':
                        expert = 1;
                        break;

                default:
                        err++;
                        break;
                }
        }
        if (err) {
                fprintf(stderr, "usage: [<configfile>]\n");
                exit(1);
        }
	if (expert) {
		fskpspdemodulator.next = &fskeqdemodulator;
	}
        loginit(verblevel, tosyslog);
        initsimd(simd);
        ioinit_sim();
        ioinit_filein();
        ioinit_soundcard();
#ifdef WIN32
        buildtree();
#else /* WIN32 */
        if (optind < argc)
		cfgfile = argv[optind];
        doc = xmlParseFile(cfgfile);
	if (doc && (!doc->children || !doc->children->name || strcmp(doc->children->name, "modem"))) {
		g_printerr("SoundModem Config: Invalid configuration file %s\n", cfgfile);
		exit(1);
	}
	if (!doc && (doc = xmlNewDoc("1.0")))
		doc->children = xmlNewDocNode(doc, NULL, "modem", NULL);
	if (!doc || !doc->children) {
		g_printerr("SoundModem Config: out of memory\n");
		exit(1);
	}
	buildtree(doc->children->children);
#endif /* WIN32 */
	renumber_channels();
	gtk_tree_view_expand_all(GTK_TREE_VIEW(g_object_get_data(G_OBJECT(mainwindow), "configtree")));
	gtk_widget_show(mainwindow);
	gtk_main();
#ifdef WIN32
#else /* WIN32 */
	if (!xmlSaveFile(cfgfile, doc)) 
	    g_printerr("SoundModem Config: error saving configuration file %s\n", cfgfile);
        xmlFreeDoc(doc);
#endif /* WIN32 */
	return 0;
}

