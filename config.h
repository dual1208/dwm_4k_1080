#include <X11/XF86keysym.h>
#include <X11/Xutil.h>
/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "NotoSansCJK-Regular:pixelsize=15:antialias=true:autohint=true" };
static const char dmenufont[]       = "LXGWWenKaiMono:size=20";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
    	{ "Surf",     "surf",       NULL,       0,            1,           -1 },
	{ "firefox",  NULL,       NULL,       1 << 6,       0,           -1 },
	{ "Chromium",  NULL,       NULL,       1 << 6,       0,           2 },
	{ "Cursor",  NULL,       NULL,       1 << 8,       0,           -1 },
	{ "Code",  NULL,       NULL,       1 << 7,       0,           1 },
	{ "mpv",  NULL,       NULL,       1 << 1,       0,           1 },
};



/* layout(s) */
static const float mfact     = 0.83; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "=[]",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "/bin/sh", "-c", "[ \"$GTK_THEME\" = \"Adwaita:dark\" ] && dt || st", NULL };
static const char *roficmd[]  = { "/usr/bin/rofi", "-show", "drun", "-show-icons", NULL };
static const char *oald[]  = { "/home/new/.local/bin/oald", NULL };
static const char *dict[]  = { "/home/new/.local/bin/dict", NULL };
static const char *tts[]  = { "/home/new/.local/bin/tts", NULL };
static const char *anki[]  = { "/usr/bin/anki", NULL };
static const char *xmind[]  = { "/usr/bin/xmind", NULL };
static const char *upvol[]   = { "/bin/sh", "-c", "/usr/bin/amixer -q sset Master 10%+; touch /tmp/dwm-status-change", NULL };
static const char *downvol[] = { "/bin/sh", "-c", "/usr/bin/amixer -q sset Master 10%-; touch /tmp/dwm-status-change", NULL };
static const char *mutevol[] = { "/bin/sh", "-c", "/usr/bin/amixer -q sset Master toggle; touch /tmp/dwm-status-change", NULL };
static const char *lightup[] = { "/bin/sh", "-c", "/usr/bin/ddcutil -d 2 setvcp 10 + 10; /usr/bin/ddcutil -d 1 setvcp 10 + 10; touch /tmp/dwm-status-change", NULL };
static const char *lightdown[] = { "/bin/sh", "-c", "/usr/bin/ddcutil -d 2 setvcp 10 - 10; /usr/bin/ddcutil -d 1 setvcp 10 - 10; touch /tmp/dwm-status-change", NULL };
static const char *fd[]  = { "/usr/bin/firefox-developer-edition", "--kiosk", NULL };
static const char *ch[]  = { "/usr/bin/google-chrome-stable", "--kiosk", NULL };
static const char *pp[]  = { "/usr/bin/polypane", NULL };
static const char *bss[]  = { "/usr/bin/bootstrap-studio", NULL };
static const char *mpv[]  = { "/usr/bin/mpv", NULL };
static const char *ff[]  = { "/usr/bin/firefox",  NULL };
static const char *mgdbc[]  = { "/usr/bin/mongodb-compass", "--password-store='gnome-libsecret'", "--ignore-additional-command-line-flags",  NULL };
static const char *editor[] = { "/usr/bin/code", NULL };

static const As aslist[] = {
	/*{.cmd = termcmd, .tags = 511, .monnum = 2 },
	{.cmd = ff, .tags = 1 << 6, .monnum = 2 },
	{.cmd = editor, .tags = 256, .monnum = 2 },
	{.cmd = fd, .tags = 1, .monnum = 0 }, */
	/*
	{.cmd = termcmd, .tags = 511, .monnum = 1 },
	{.cmd = fd, .tags = 1, .monnum = 0 },
	{.cmd = ff, .tags = 1, .monnum = 1 },
	{.cmd = xmind, .tags = 1, .monnum = 2 },
	{.cmd = editor, .tags = 256, .monnum = 1 },
	*/
	/*{.cmd = fd, .tags = 1, .monnum = 0 },
	{.cmd = editor, .tags = 129, .monnum = 1 },
	{.cmd = fd, .tags = 1, .monnum = 0 },
	{.cmd = pp, .tags = 1, .monnum = 0 },
	{.cmd = mgdbc, .tags = 1, .monnum = 2 },
	{.cmd = bss, .tags = 1, .monnum = 1 },*/
	{.cmd = fd, .tags = 1, .monnum = 0 },
	{.cmd = termcmd, .tags = 2, .monnum = 1 },
	{.cmd = termcmd, .tags = 1, .monnum = 1 },
	{.cmd = ff, .tags = 1, .monnum = 1 },
	{.cmd = NULL, .tags = 0, .monnum = -1 },
};

static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ 0, XF86XK_AudioMute,        spawn, {.v = mutevol } },
	{ 0, XF86XK_AudioLowerVolume, spawn, {.v = downvol } },
	{ 0, XF86XK_AudioRaiseVolume, spawn, {.v = upvol   } },
	{ 0, XF86XK_MonBrightnessUp,  spawn, {.v = lightup } },
	{ 0, XF86XK_MonBrightnessDown,spawn, {.v = lightdown } },
	{ Mod1Mask,                     XK_space,  spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_d,      spawn,          {.v = roficmd } },
	{ Mod1Mask,                     XK_d,      spawn,          {.v = dict } },
	{ Mod1Mask,                     XK_t,      spawn,          {.v = tts } },
	{ MODKEY,                       XK_s,      spawn,          {.v = oald } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
//	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
//	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } }, 
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.01} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.01} },
	{ MODKEY|ShiftMask,             XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_e,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

