/************************************************
** Copyright (C) 2006-2012, Cockos Incorporated
*/

#ifndef _REAPER_COLORTHEME_H_
#define _REAPER_COLORTHEME_H_

#define NUM_ENVELOPE_COLORS 6+10
typedef struct
{
  // NOTE: DO NOT REMOVE/INSERT IN THIS STRUCT. ONLY ADD.
  LOGFONT timeline_font,  mediaitem_font, label_font, label_font2;
  int timeline_fontcolor;
  int mediaitem_fontcolor;
  int itembgcolor;
  int DEPRRECATED_itembgselcolor;
  int timeline_bgcolor;
  int timeline_selbgcolor;
  int itemfadescolor;
  int itemfadedgecolor;
  int envelopecolors[NUM_ENVELOPE_COLORS];
  int trackbgs[2];
  int DEPRECATED_trackbgsel[2];
  int peaks[2]; // body of peaks when not selected, even/odd
  int DEPRECATED_peaksel[2];
  int offlinetext;
  int seltrack, seltrack2;
  int cursor;
  int grid_lines[2];
  int marker;
  int item_grouphl;
  int region;
  int itembgsel[2];
  int DEPRECATED_peaksedgecolor;

  int main_bg;
  int main_text;
  int main_3d[2];
  int main_editbk;
  // NOTE: DO NOT REMOVE/INSERT IN THIS STRUCT. ONLY ADD.

  int timesigmarker;

  int vu_dointerlace;
  int vu_bottom;
  int vu_mid;
  int vu_top;
  int vu_clip;
  int vu_intcol;

  int DEPRECATED_fader_tint[2];

  int button_bg;

  int fader_armed_bg[2];

  int grid_startofbeat;

  int fader_armed_butnotrec;

  int mixerbg_color, tracklistbg_color;

  int main_textshadow;

  int midi_rulerbg;
  int midi_rulerfg;
  int midi_grid[3];
  int midi_trackbg[2];
  int midi_endpt;
  int midi_notebg;
  int midi_notefg;
  int midi_itemctls;
  int midi_editcursor;
  int midi_pkey[3];
  int DEPRECATED_midi_pkeytext;

  int docker_shadow; // defaults to GetSysColor(COLOR_3DDKSHADOW)
  int docker_selface;  // GSC_mainwnd(COLOR_3DFACE)
  int docker_unselface; // GSC_mainwnd(COLOR_3DSHADOW)
  int docker_text; //GSC_mainwnd(COLOR_BTNTEXT)
  int docker_bg; // Getsyscolor(COLOR_3DDKSHADOW)

  int mcp_fx_normal;
  int mcp_fx_bypassed;
  int mcp_fx_offlined;

  int mcp_sends_normal;
  int mcp_sends_muted;
  int mcp_send_levels;

  int env_mute_track, env_mute_sends;

  int peakssel2[2]; // body of peaks when selected, even/odd
  int itembgcolor2;
  int DEPRECATED_itembgselcolor2;
  int DEPRECATED_peaksedgecolor2;

  int timeline_selbgcolor2;

  int trackpanel_text[2];

  int midi_offscreennote,midi_offscreennotesel;
  int env_item_vol,env_item_pan,env_item_mute;

  int group_colors[32];

  int marquee_fill,marquee_outline;

  int midi_notemute[2];

  int timesel_drawmode;
  int marquee_drawmode;
  int itembg_drawmode;

  int ruler_lane_bgcolor[3];  // region, marker, TS
  int ruler_lane_textcolor[3]; // region, marker, TS

  int mediaitem_fontcolor_sel;

  int mediaitem_fontcolor_floating[2]; 

  int peaksedge[2]; // edge of peaks when not selected, even/odd
  int peaksedge_sel[2]; // edge of peaks when selected, even/odd

  int midi_inline_trackbg[2];

  int vu_midi;

  int fadezone_drawmode;  // quiet zone
  int fadezone_color;     // quiet zone

  int cursor_altblink;

  int playcursor_drawmode;
  int playcursor_color;

  int itemfadedgecolor_drawmode;

  int guideline_color;
  int guideline_drawmode;

  int fadearea_drawmode;  // full area (not just quiet zone)
  int fadearea_color;     // full area (not just quiet zone)

  int midi_noteon_flash;

  int toolbar_button_text;

  int toolbararmed_color;
  int toolbararmed_drawmode;

  int track_divline[2];
  int envlane_divline[2];

  int mcp_sends_midihw;

  int tcp_locked_drawmode;
  int tcp_locked_color;

  int selitem_tag; // use only if 0x80000000
  int activetake_tag;  // use only if 0x80000000

  int midifont_col_dark;
  int midifont_col_light;

  int midioct;
  int midioct_inline;

  int midi_selbgcol;
  int midi_selbgmode;

  int arrange_bg;

  int mcp_fxparm_normal;
  int mcp_fxparm_bypassed;
  int mcp_fxparm_offlined;

  int env_item_pitch;

  int main_resize_color;

  LOGFONT transport_status_font;
  int transport_status_font_color;
  int transport_status_bg_color;

  // inherits from main_*
  int transport_editbk;
  int main_bg2; // actual main window / transport bg
  int main_text2;  // actual main window / transport text

  int toolbar_button_text_on;
  int toolbar_frame;

  int vu_ind_very_bot;
  int vu_ind_bot;
  int vu_ind_mid;
  int vu_ind_top;

  int io_text; // overrides for i/o window
  int io_3d[2];

  int marqueezoom_fill,marqueezoom_outline,marqueezoom_drawmode;

//JFB found by reverse eng. --->
  int unknown_1[7];
  int genlist_bg;
  int genlist_fg;
  int genlist_gridlines;
  int genlist_sel[2];
  int genlist_selinactive[2];
//JFB <---

  // NOTE: DO NOT REMOVE/INSERT IN THIS STRUCT. ONLY ADD.
} ColorTheme;

#endif
