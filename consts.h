#ifndef CONSTS
#define CONSTS
#define UNICODE 1
#define _WIN32_IE 0x0400
#define _WIN32_WINNT 0x0501
#include "windows2.h"
#include<windows.h>

#define auto(n,e) typeof(e) n = e

#define toTString toWString
#define tstring wstring
#define tsnprintf wsnprintf
#define tsplit_iterator wsplit_iterator

#ifdef UNICODE
#define USE_UNICODE BASS_UNICODE
#else
#define USE_UNICODE 0
#endif

#define ID_TIMER 1000
#define IDL_INFO 1001
#define IDL_SONGS 1100
#define IDD_INPUTDLG "inputdlg"
#define IDD_INFODLG "infodlg"
#define IDD_CASTDLG "castdlg"
#define IDD_PLAYLIST "playlist"
#define IDD_MIDI "mididlg"
#define IDD_TEXTDLG "textdlg"
#define IDD_LEVELSDLG "levdlg"
#define IDD_RADIO "radios"
#define IDD_RADIOPROPDLG "radioprop"
#define IDD_PROGRESSDLG "progressdlg"
#define IDD_OPTIONS_DEVICES "optDevices"
#define IDD_OPTIONS_INTEGRATION "optIntegration"
#define IDD_OPTIONS_CASTING "optCasting"
#define IDD_OPTIONS_GENERAL "optGeneral"
#define IDD_OPTIONS_ADVANCED "optAdvanced"
#define IDD_ENCODESAVEDLG "encsavedlg"

#define IDC_STATIC 2500
#define IDC_INPUT 2501

#define IDM_OPEN 1002
#define IDM_OPENURL 1003
#define IDM_OPENAPPEND 1004
#define IDM_ABOUT 1005
#define IDM_QUIT 1006
#define IDM_OPENURLAPPEND 1007
#define IDM_OPENDIR 1008
#define IDM_OPENDIRAPPEND 1009
#define IDM_SHOWINFO 1010
#define IDM_OPTIONS 1011
#define IDM_OPTIONS_DEVICES 1012
#define IDM_VOLPLUS 1900
#define IDM_VOLMINUS 1901
#define IDM_SEEK_LEFT 1902
#define IDM_SEEK_RIGHT 1903
#define IDM_PAUSE 2001
#define IDM_NEXT 2002
#define IDM_PREV 2003
#define IDM_LOOP 2004
#define IDM_RANDOM 2005
#define IDM_SAVEPLAYLIST 2006
#define IDM_ENCODE 2007
#define IDM_RESET 2008
#define IDM_DELCURSONG 2009
#define IDM_GOTOTIME 2010
#define IDM_SHOWPLAYLIST 2011
#define IDM_INDEXPLAYLIST 2012
#define IDM_RECORD 2013
#define IDM_CROSSFADE 2014
#define IDM_EFF_MODE1 2015
#define IDM_EFF_MODE2 2016
#define IDM_EFF_MODE3 2017
#define IDM_CAST 2018
#define IDM_RADIO 2019
#define IDM_REVERSE 2020
#define IDM_INTRO_MODE 2021
#define IDM_SHOWMIDI 2022
#define IDM_SHOWTEXTPANEL 2023
#define IDM_SHOWLEVELS 2024
#define IDM_DEBUG 9999
#define IDM_EFFECT 9500
#define IDM_ENCODEALL 10000
#define IDM_CUSTOMCOMMAND 11000

#ifdef ENGLISH
#include "english.h"
#else
#include "french.h"
#endif

#endif
