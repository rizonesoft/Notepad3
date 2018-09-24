#include "StyleLexers.h"

// ----------------------------------------------------------------------------

static int LexFunction(LexFunctionType type, int value)
{
  static bool bStyleChanged = false;

  switch (type) {
  case FCT_SETTING_CHANGE:
    if (value < 0)
      return (bStyleChanged ? 1 : 0);
    else {
      bStyleChanged = (value > 0);
      return 1;
    }

  default:
    break;
  }
  return 0;
};

// ----------------------------------------------------------------------------

KEYWORDLIST KeyWords_AVS = {
"true false return global",
"addborders alignedsplice amplify amplifydb animate applyrange assumebff assumefieldbased assumefps "
"assumeframebased assumesamplerate assumescaledfps assumetff audiodub audiodubex avifilesource "
"avisource bicubicresize bilinearresize blackmanresize blackness blankclip blur bob cache changefps "
"colorbars colorkeymask coloryuv compare complementparity conditionalfilter conditionalreader "
"convertaudio convertaudioto16bit convertaudioto24bit convertaudioto32bit convertaudioto8bit "
"convertaudiotofloat convertbacktoyuy2 convertfps converttobackyuy2 converttomono converttorgb "
"converttorgb24 converttorgb32 converttoy8 converttoyv16 converttoyv24 converttoyv411 converttoyuy2 "
"converttoyv12 crop cropbottom delayaudio deleteframe dissolve distributor doubleweave duplicateframe "
"ensurevbrmp3sync fadein fadein0 fadein2 fadeio fadeio0 fadeio2 fadeout fadeout0 fadeout2 fixbrokenchromaupsampling "
"fixluminance fliphorizontal flipvertical frameevaluate freezeframe gaussresize generalconvolution "
"getchannel getchannels getmtmode getparity grayscale greyscale histogram horizontalreduceby2 "
"imagereader imagesource imagewriter info interleave internalcache internalcachemt invert killaudio "
"killvideo lanczos4resize lanczosresize layer letterbox levels limiter loop mask maskhs max merge "
"mergeargb mergechannels mergechroma mergeluma mergergb messageclip min mixaudio monotostereo normalize "
"null opendmlsource overlay peculiarblend pointresize pulldown reduceby2 resampleaudio resetmask reverse "
"rgbadjust scriptclip segmentedavisource segmenteddirectshowsource selecteven selectevery selectodd "
"selectrangeevery separatefields setmtmode sharpen showalpha showblue showfiveversions showframenumber "
"showgreen showred showsmpte showtime sincresize skewrows spatialsoften spline16resize spline36resize "
"spline64resize ssrc stackhorizontal stackvertical subtitle subtract supereq swapfields swapuv "
"temporalsoften timestretch tone trim turn180 turnleft turnright tweak unalignedsplice utoy utoy8 "
"version verticalreduceby2 vtoy vtoy8 wavsource weave writefile writefileend writefileif writefilestart "
"ytouv",
"addgrain addgrainc agc_hdragc analyzelogo animeivtc asharp audiograph autocrop autoyuy2 avsrecursion "
"awarpsharp bassaudiosource bicublinresize bifrost binarize blendfields blindpp blockbuster bordercontrol "
"cfielddiff cframediff chromashift cnr2 colormatrix combmask contra convolution3d convolution3dyv12 "
"dctfilter ddcc deblendlogo deblock deblock_qed decimate decomb dedup deen deflate degrainmedian depan "
"depanestimate depaninterleave depanscenes depanstabilize descratch despot dfttest dgbob dgdecode_mpeg2source "
"dgsource directshowsource distancefunction dss2 dup dupmc edeen edgemask ediupsizer eedi2 eedi3 eedi3_rpow2 "
"expand faerydust fastbicubicresize fastbilinearresize fastediupsizer dedgemask fdecimate ffaudiosource "
"ffdshow ffindex ffmpegsource ffmpegsource2 fft3dfilter fft3dgpu ffvideosource fielddeinterlace fielddiff "
"fillmargins fity2uv fity2u fity2v fitu2y fitv2y fluxsmooth fluxsmoothst fluxsmootht framediff framenumber "
"frfun3b frfun7 gicocu golddust gradfun2db grapesmoother greedyhma grid guavacomb hqdn3d hybridfupp "
"hysteresymask ibob improvesceneswitch inflate inpand inpaintlogo interframe interlacedresize "
"interlacedwarpedresize interleaved2planar iscombed iscombedt iscombedtivtc kerneldeint leakkernelbob "
"leakkerneldeint limitedsharpen limitedsharpenfaster logic lsfmod lumafilter lumayv12 manalyse "
"maskeddeinterlace maskedmerge maskedmix mblockfps mcompensate mctemporaldenoise mctemporaldenoisepp "
"mdegrain1 mdegrain2 mdegrain3 mdepan medianblur mergehints mflow mflowblur mflowfps mflowinter minblur "
"mipsmooth mmask moderatesharpen monitorfilter motionmask mpasource mpeg2source mrecalculate mscdetection "
"msharpen mshow msmooth msu_fieldshiftfixer msu_frc msuper mt mt_adddiff mt_average mt_binarize mt_circle "
"mt_clamp mt_convolution mt_deflate mt_diamond mt_edge mt_ellipse mt_expand mt_freeellipse mt_freelosange "
"mt_freerectangle mt_hysteresis mt_infix mt_inflate mt_inpand mt_invert mt_logic mt_losange mt_lut mt_lutf "
"mt_luts mt_lutspa mt_lutsx mt_lutxy mt_lutxyz mt_makediff mt_mappedblur mt_merge mt_motion mt_polish "
"mt_rectangle mt_square mti mtsource multidecimate mvanalyse mvblockfps mvchangecompensate mvcompensate "
"mvdegrain1 mvdegrain2 mvdegrain3 mvdenoise mvdepan mvflow mvflowblur mvflowfps mvflowfps2 mvflowinter "
"mvincrease mvmask mvrecalculate mvscdetection mvshow nicac3source nicdtssource niclpcmsource nicmpasource "
"nicmpg123source nnedi nnedi2 nnedi2_rpow2 nnedi3 nnedi3_rpow2 nomosmooth overlaymask peachsmoother pixiedust "
"planar2interleaved qtgmc qtinput rawavsource rawsource reduceflicker reinterpolate411 removedirt removedust "
"removegrain removegrainhd removetemporalgrain repair requestlinear reversefielddominance rgb3dlut rgdeinterlace "
"rgsdeinterlace rgblut rotate sangnom seesaw sharpen2 showchannels showcombedtivtc smartdecimate smartdeinterlace "
"smdegrain smoothdeinterlace smoothuv soothess soxfilter spacedust sshiq ssim ssiq stmedianfilter t3dlut tanisotropic "
"tbilateral tcanny tcomb tcombmask tcpserver tcpsource tdecimate tdeint tedgemask telecide temporalcleaner "
"temporalrepair temporalsmoother tfieldblank tfm tisophote tivtc tmaskblank tmaskedmerge tmaskedmerge3 tmm "
"tmonitor tnlmeans tomsmocomp toon textsub ttempsmooth ttempsmoothf tunsharp unblock uncomb undot unfilter "
"unsharpmask vaguedenoiser variableblur verticalcleaner videoscope vinverse vobsub vqmcalc warpedresize warpsharp "
"xsharpen yadif yadifmod yuy2lut yv12convolution yv12interlacedreduceby2 yv12interlacedselecttopfields "
"yv12layer yv12lut yv12lutxy yv12substract yv12torgb24 yv12toyuy2",
"abs apply assert bool ceil chr clip continueddenominator continuednumerator cos default defined eval "
"averagechromau averagechromav averageluma chromaudifference chromavdifference lumadifference "
"exist exp findstr float floor frac hexvalue import int isbool isclip isfloat isint isstring lcase leftstr "
"load_stdcall_plugin loadcplugin loadplugin loadvfapiplugin loadvirtualdubplugin log midstr muldiv nop "
"opt_allowfloataudio opt_avipadscanlines opt_dwchannelmask opt_usewaveextensible opt_vdubplanarhack "
"pi pow rand revstr rightstr round scriptdir scriptfile scriptname select setmemorymax setplanarlegacyalignment "
"rgbdifference rgbdifferencefromprevious rgbdifferencetonext udifferencefromprevious udifferencetonext "
"setworkingdir sign sin spline sqrt string strlen time ucase undefined value versionnumber versionstring "
"uplanemax uplanemedian uplanemin uplaneminmaxdifference vdifferencefromprevious vdifferencetonext "
"vplanemax vplanemedian vplanemin vplaneminmaxdifference ydifferencefromprevious ydifferencetonext "
"yplanemax yplanemedian yplanemin yplaneminmaxdifference",
"audiobits audiochannels audiolength audiolengthf audiorate framecount framerate frameratedenominator "
"frameratenumerator getleftchannel getrightchannel hasaudio hasvideo height isaudiofloat isaudioint "
"isfieldbased isframebased isinterleaved isplanar isrgb isrgb24 isrgb32 isyuv isyuy2 isyv12 width",
"", "", "", "" };


EDITLEXER lexAVS = { 
SCLEX_AVS, IDS_LEX_AVI_SYNTH, L"AviSynth Script", L"avs; avsi", L"", 
&LexFunction, // static
&KeyWords_AVS, {
    { STYLE_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    //{ SCE_AVS_DEFAULT, IDS_LEX_STR_63126, L"Default", L"", L"" },
    { MULTI_STYLE(SCE_AVS_COMMENTLINE,SCE_AVS_COMMENTBLOCK,SCE_AVS_COMMENTBLOCKN,0), IDS_LEX_STR_63127, L"Comment", L"fore:#008000", L"" },
    { SCE_AVS_OPERATOR, IDS_LEX_STR_63132, L"Operator", L"", L"" },
    { MULTI_STYLE(SCE_AVS_STRING,SCE_AVS_TRIPLESTRING,0,0), IDS_LEX_STR_63131, L"String", L"fore:#7F007F", L"" },
    { SCE_AVS_NUMBER, IDS_LEX_STR_63130, L"Number", L"fore:#007F7F", L"" },
    { SCE_AVS_KEYWORD, IDS_LEX_STR_63128, L"Keyword", L"fore:#00007F; bold", L"" },
    { SCE_AVS_FILTER, IDS_LEX_STR_63314, L"Filter", L"fore:#00007F; bold", L"" },
    { SCE_AVS_PLUGIN, IDS_LEX_STR_63315, L"Plugin", L"fore:#0080C0; bold", L"" },
    { SCE_AVS_FUNCTION, IDS_LEX_STR_63277, L"Function", L"fore:#007F7F", L"" },
    { SCE_AVS_CLIPPROP, IDS_LEX_STR_63316, L"Clip Property", L"fore:#00007F", L"" },
    //{ SCE_AVS_USERDFN, IDS_LEX_STR_63106, L"User Defined", L"fore:#8000FF", L"" },
    EDITLEXER_SENTINEL } };
