
/*
	List of all included tracks and their actual filenames.
	As we're dealing with FAT 8+3 I had to rename most modules.
	
	Amiga playlist at the bottom!
*/

#pragma once

const unsigned kNumDiskTracks = 18; // 21;
const unsigned kInternalTracks = 2;
const unsigned kNumTracks = kNumDiskTracks+kInternalTracks;

// 3 strings per track: filename, name & artist(s) (latter two in capitals for the font renderer).
static const char *s_tracks[kNumTracks*3] =
{
	// Disk modules.
	"FUNNYFRT.MOD", "FUNNY OLD FARTS", "DASCON & TRIACE"
,	"RELOAD.MOD", "NEVER MIND", "TRIACE"
// ,	"SMOOTH.AHX", "SMOOTH TALKER", "JERRY"
,	"RELEG.MOD", "RELEGATION", "DASCON"
,	"SSPIRAL.MOD", "SIDEWAY SPIRAL", "TECON"
,	"ELVIS.MOD", "MY MUM IS ELVIS", "OPTIC"
,	"PANDA.MOD", "PANDA BLUES", "JERRY"
,	"LEGENDS.MOD", "LEGENDS NEVER SLEEP", "WOTW"
,	"SCREWER.MOD", "SCREWER", "DEETROY"
//,	"DOODLE.AHX", "2K BLUES DOODLE", "TECON"
,	"RANDOM.MOD", "RANDOM PARTS", "ALPHA C"
,	"RUINE.MOD", "RUINE", "XYCE"
,	"TRAVEL.MOD", "TRAVELLING MINDS", "VEDDER & SLASH"
,	"ELIMTRIB.MOD", "ELIMINATOR TRIBUTE", "MOBY"
//,	"GATES.AHX", "GATES", "VIRGILL"
,	"DCHIPPER.MOD", "DEAD CHIPPER", "BEX"
,	"DUST.MOD", "DUST IN THE WIND", "NET"
,	"PHOTO.MOD", "PHOTOGRAPHIC CHIP", "OLLE"
,	"BIZARDRY.MOD", "BIZARDRY", "CURT COOL"
,	"CHEAVEN.MOD", "CHIP HEAVEN", "DASCON"
,	"CCHOP.MOD", "LETS CHIP CHOP", "MYGG"

	// Internal modules.
,	"GPRIX.MOD",   "", "" // Triace's Grand Prix Circuit cover.
,	"THEDUEL.MOD", "", "" // Triace's Test Drive 2: The Duel cover.
};

/*
		section data,data
		
		xdef	PlayListEntries
		
TUNE		MACRO
		dc.l	(\1)*3000+(\2)*50,\3,\4,\5
		ENDM
		
		
PlayListEntries:
		TUNE 2,12,Entry1,ScrollText1,SelectText1
		TUNE 5,44,Entry2,ScrollText2,SelectText2
		TUNE 3,36,Entry3,ScrollText3,SelectText3
		TUNE 1,20,Entry4,ScrollText4,SelectText4
		TUNE 1,07,Entry5,ScrollText5,SelectText5
		TUNE 1,10,Entry6,ScrollText6,SelectText6
		TUNE 3,18,Entry7,ScrollText7,SelectText7
		TUNE 1,18,Entry8,ScrollText8,SelectText8
		TUNE 0,51,Entry9,ScrollText9,SelectText9
		TUNE 0,58,Entry10,ScrollText10,SelectText10
		
		TUNE 0,57,Entry11,ScrollText11,SelectText11
		TUNE 2,18,Entry12,ScrollText12,SelectText12
		TUNE 2,10,Entry13,ScrollText13,SelectText13
		TUNE 2,18,Entry14,ScrollText14,SelectText14
		TUNE 1,33,Entry15,ScrollText15,SelectText15
		TUNE 1,10,Entry16,ScrollText16,SelectText16
		TUNE 2,54,Entry17,ScrollText17,SelectText17
		TUNE 2,41,Entry18,ScrollText18,SelectText18
		TUNE 2,02,Entry19,ScrollText19,SelectText19
		TUNE 2,04,Entry20,ScrollText20,SelectText20
		
		;--- TUNE 3,40,Entry21,ScrollText21,SelectText21
		TUNE 1,54,Entry22,ScrollText22,SelectText22
		
		
Entry1:		dc.b	"1FUNNYFARTS",0,0
Entry2:		dc.b	"2NEVERMIND",0,0         
Entry3:		dc.b	"3SMOOTHTALKER",0,1         
Entry4:		dc.b	"4RELEGATION",0,0
Entry5:		dc.b	"5SIDEWAYSPIRAL",0,0         
Entry6:		dc.b	"6MYMUMISELVIS",0,0
Entry7:		dc.b	"7PANDABLUES",0,0         
Entry8:		dc.b	"8LEGENSNEVERSLEEP",0,0
Entry9:		dc.b	"9SCREWER",0,0       
Entry10:	dc.b	"10BLUESDOODLE",0,1             
Entry11:	dc.b	"11RANDOMPARTS",0,0
Entry12:	dc.b	"12RUINE",0,0         
Entry13:	dc.b	"13TRAVELLINGMINDS",0,0
Entry14:	dc.b	"14ELIMTRIBUTE",0,0
Entry15:	dc.b	"15GATES",0,1
Entry16:	dc.b	"16DEADCHIPPER",0,0
Entry17:	dc.b	"17DUSTINTHEWIND",0,0        
Entry18:	dc.b	"18PHOTOGRAPHIC",0,0
Entry19:	dc.b	"19BIZARDRY",0,0
Entry20:	dc.b	"20CHIPHEAVEN",0,0
Entry21:	dc.b	"21EVERYTHING",0,1
Entry22:	dc.b	"22SOLSKOGEN",0,0

ScrollText1:	dc.b	"FUNNY OLD FARTS BY TRIACE AND DASCON OF DESIRE     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 25146 BYTES"
		dc.b	"                    ",0     
		
ScrollText2:	dc.b	"NEVERMIND BY TRIACE OF DESIRE     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 35288 BYTES"
		dc.b	"                    ",0     

ScrollText3:	dc.b	"SMOOTH TALKER BY JERRY OF DESIRE     "
		dc.b	"FORMAT: AHX     "
		dc.b	"SIZE: 8569 BYTES"
		dc.b	"                    ",0     
		
ScrollText4:	dc.b	"RELEGATION BY DASCON OF DESIRE     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 26044"
		dc.b	"                    ",0     
		
ScrollText5:	dc.b	"SIDEWAY SPIRAL BY TECON OF DESIRE     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 53618 BYTES"
		dc.b	"                    ",0     

ScrollText6:	dc.b	"MY MUM IS ELVIS BY OPTIC     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 11892 BYTES"
		dc.b	"                    ",0     

ScrollText7:	dc.b	"PANDA BLUES BY JERRY OF DESIRE     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 51930 BYTES"
		dc.b	"                    ",0     

ScrollText8:	dc.b	"LEGENDS NEVER SLEEP BY WOTW     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 73480 BYTES"
		dc.b	"                    ",0     

ScrollText9:	dc.b	"SCREWER BY DEETROY     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 14834 BYTES"
		dc.b	"                    ",0     
		
ScrollText10:	dc.b	"2K BLUES DOODLE BY TECON OF DESIRE    "
		dc.b	"FORMAT: AHX     "
		dc.b	"SIZE: 1974 BYTES"
		dc.b	"                    ",0     

ScrollText11:	dc.b	"RANDOM PARTS BY ALPHA C     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 23200 BYTES"
		dc.b	"                    ",0     

ScrollText12:	dc.b	"RUINE BY XYCE OF TITAN     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 38760 BYTES"
		dc.b	"                    ",0     

ScrollText13:	dc.b	"TRAVELLING MINDS BY VEDDER AND SLASH OF INSANE     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 18980 BYTES"
		dc.b	"                    ",0     

ScrollText14:	dc.b	"ELIMINATOR TRIBUTE BY MOBY     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 142892 BYTES"
		dc.b	"                    ",0     
		
ScrollText15:	dc.b	"GATES BY VIRGILL     "
		dc.b	"FORMAT: AHX     "
		dc.b	"SIZE: 10255 BYTES"
		dc.b	"                    ",0     

ScrollText16:	dc.b	"DEAD CHIPPER BY BEX OF SLIPSTREAM     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 11844 BYTES"
		dc.b	"                    ",0     

ScrollText17:	dc.b	"DUST IN THE WIND BY NET     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 30232 BYTES"
		dc.b	"                    ",0     

ScrollText18:	dc.b	"PHOTOGRAPHIC CHIP BY OLLE OF PJZ     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 23864 BYTES"
		dc.b	"                    ",0     

ScrollText19:	dc.b	"BIZARDRY BY CURT COOL OF DEPTH     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 29060 BYTES"
		dc.b	"                    ",0     

ScrollText20:	dc.b	"CHIP HEAVEN BY DASCON OF DESIRE     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 80460 BYTES"
		dc.b	"                    ",0     
		
ScrollText21:	dc.b	"EVERYTHING IS CONNECTED BY SHINOBY OF PLANET JAZZ     "
		dc.b	"FORMAT: AHX     "
		dc.b	"SIZE: 30042 BYTES"
		dc.b	"                    ",0     

ScrollText22:	dc.b	"LETS CHIP CHOP BY MYGG OF INSANE     "
		dc.b	"FORMAT: PROTRACKER     "
		dc.b	"SIZE: 28512 BYTES"
		dc.b	"                    ",0     
		
		
SelectText1:	even
		dc.w	40
		dc.b	"FUNNY OLD FARTS",0
		even
		dc.w	40
		dc.b	"DASCON & TRIACE",0
		
SelectText2:	even
		dc.w	88
		dc.b	"NEVERMIND",0
		even	
		dc.w	112
		dc.b	"TRIACE",0
				
SelectText3:	even
		dc.w	56
		dc.b	"SMOOTH TALKER",0
		even
		dc.w	120
		dc.b	"JERRY",0
		
SelectText4:	even
		dc.w	80
		dc.b	"RELEGATION",0
		even
		dc.w	112
		dc.b	"DASCON",0
		
SelectText5:	even
		dc.w	48
		dc.b	"SIDEWAY SPIRAL",0
		even
		dc.w	120
		dc.b	"TECON",0
		
SelectText6:	even
		dc.w	40
		dc.b	"MY MUM IS ELVIS",0
		even
		dc.w	120
		dc.b	"OPTIC",0
		
SelectText7:	even
		dc.w	72
		dc.b	"PANDA BLUES",0
		even
		dc.w	120
		dc.b	"JERRY",0
		
SelectText8:	even
		dc.w	24
		dc.b	"LEGENDS NVR SLEEP",0
		even
		dc.w	128
		dc.b	"WOTW",0
		
SelectText9:	even
		dc.w	104
		dc.b	"SCREWER",0
		even
		dc.w	104
		dc.b	"DEETROY",0
		
SelectText10:	even
		dc.w	40
		dc.b	"2K BLUES DOODLE",0
		even
		dc.w	120
		dc.b	"TECON",0
		
SelectText11:	even
		dc.w	64
		dc.b	"RANDOM PARTS",0
		even
		dc.w	104
		dc.b	"ALPHA C",0
		
SelectText12:	even
		dc.w	120
		dc.b	"RUINE",0
		even
		dc.w	128
		dc.b	"XYCE",0
		
SelectText13:	even
		dc.w	32
		dc.b	"TRAVELLING MINDS",0
		even
		dc.w	48
		dc.b	"VEDDER & SLASH",0
				
SelectText14:	even
		dc.w	16
		dc.b	"ELIMINATOR TRIBUTE",0
		even
		dc.w	128
		dc.b	"MOBY",0
		
SelectText15:	even
		dc.w	120
		dc.b	"GATES",0
		even
		dc.w	104
		dc.b	"VIRGILL",0
		
SelectText16:	even
		dc.w	64
		dc.b	"DEAD CHIPPER",0
		even
		dc.w	136
		dc.b	"BEX",0
		
SelectText17:	even
		dc.w	32
		dc.b	"DUST IN THE WIND",0
		even
		dc.w	136
		dc.b	"NET",0
		
SelectText18:	even
		dc.w	24
		dc.b	"PHOTOGRAPHIC CHIP",0
		even
		dc.w	128
		dc.b	"OLLE",0

SelectText19:	even
		dc.w	96
		dc.b	"BIZARDRY",0
		even
		dc.w	88
		dc.b	"CURT COOL",0
		
SelectText20:	even
		dc.w	72
		dc.b	"CHIP HEAVEN",0
		even
		dc.w	112
		dc.b	"DASCON",0
		
SelectText21:	even
		dc.w	16
		dc.b	"E'THINGS CONNECTED",0
		even
		dc.w	104
		dc.b	"SHINOBI",0
		
SelectText22:	even
		dc.w	48
		dc.b	"LETS CHIP CHOP",0
		even
		dc.w	128
		dc.b	"MYGG",0
		even
*/

