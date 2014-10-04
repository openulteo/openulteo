/**
 * Copyright (C) 2007-2013 Ulteo SAS
 * http://www.ulteo.com
 * Author Julien LANGLOIS <julien@ulteo.com> 2007-2008
 * Author Guillaume SEMPE <guillaume.sempe@gmail.com> 2012
 * Author David PHAM-VAN <d.pham-van@ulteo.com> 2013
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/


#ifndef LOCALES_H
#define LOCALES_H

typedef struct _ULTEO_LOCAL {
  NSInteger keyboardID;
  NSString *keyboardName;
} ULTEO_LOCAL;

// Windows Locales ref (http://msdn2.microsoft.com/en-us/library/ms776294.aspx)
static const ULTEO_LOCAL _win_locales[] = {
  
  {0x0436, @"af_ZA"},	// Afrikaans South Africa
  {0x041c, @"sq_AL"},	// Albanian Albania
  {0x0484, @"gsw_FR"},	// Alsatian France
  {0x045e, @"am_ET"},	// Amharic Ethiopia

  // Arabic
  {0x1401, @"ar_DZ"},	// Arabic Algeria
  {0x3c01, @"ar_BH"},	// Arabic Bahrain
  {0x0c01, @"ar_EG"},	// Arabic Egypt
  {0x0801, @"ar_IQ"},	// Arabic Iraq
  {0x2c01, @"ar_JO"},	// Arabic Jordan
  {0x3401, @"ar_KW"},	// Arabic Kuwait
  {0x3001, @"ar_LB"},	// Arabic Lebanon
  {0x1001, @"ar_LY"},	// Arabic Libya
  {0x1801, @"ar_MA"},	// Arabic Morocco
  {0x2001, @"ar_OM"},	// Arabic Oman
  {0x4001, @"ar_QA"},	// Arabic Qatar
  {0x0401, @"ar_SA"},	// Arabic Saudi Arabia
  {0x2801, @"ar_SY"},	// Arabic Syria
  {0x1c01, @"ar_TN"},	// Arabic Tunisia
  {0x3801, @"ar_AE"},	// Arabic U.A.E.
  {0x2401, @"ar_YE"},	// Arabic Yemen

  {0x042b, @"hy_AM"},	// Armenian Armenia
  {0x044d, @"as_IN"},	// Assamese India

  // Azeri
  {0x082c, @"az_AZ"},	// Azeri Azerbaijan, Cyrillic
  {0x042c, @"az_AZ"},	// Azeri Azerbaijan, Latin

  {0x046d, @"ba_RU"},	// Bashkir Russia
  {0x042d, @"eu_ES"},	// Basque Basque

  // Belarusian
  {0x0423, @"be_BY"},	// Belarusian Belarus

  // Bosnian
  {0x201a, @"bs_BA"},	// Bosnian Bosnia and Herzegovina, Cyrillic
  {0x141a, @"bs_BA"},	// Bosnian Bosnia and Herzegovina, Latin

  {0x047e, @"br_FR"},	// Breton France
  {0x0402, @"bg_BG"},	// Bulgarian Bulgaria
  {0x0403, @"ca_ES"},	// Catalan Catalan

  // Chinese
  {0x0c04, @"zh_HK"},	// Chinese Hong Kong SAR, PRC
  {0x1404, @"zh_MO"},	// Chinese Macao SAR
  {0x1004, @"zh_SG"},	// Chinese Singapore

  {0x0804, @"zh_Hans"},	// Chinese Simplified
  {0x0404, @"zh_Hant"},	// Chinese Traditional
  {0x0483, @"co_FR"},	// Corsican France

  // Croatian
  {0x101a, @"hr_BA"},	// Croatian Bosnia and Herzegovina, Latin
  {0x041a, @"hr_HR"},	// Croatian Croatia

  {0x0405, @"cs_CZ"},	// Czech Czech Republic
  {0x0406, @"da_DK"},	// Danish Denmark
  {0x048c, @"gbz_AF"},	// Dari Afghanistan
  {0x0465, @"dv_MV"},	// Divehi Maldives

  // Dutch
  {0x0813, @"nl_BE"},	// Dutch Belgium
  {0x0413, @"nl_NL"},	// Dutch Netherlands

  // English
  {0x0c09, @"en_AU"},	// English Australia
  {0x2809, @"en_BE"},	// English Belize
  {0x1009, @"en_CA"},	// English Canada
  {0x2409, @"en_029"},	// English Caribbean
  {0x4009, @"en_IN"},	// English India
  {0x1809, @"en_IE"},	// English Ireland
  {0x1809, @"en_IE"},	// English Ireland
  {0x2009, @"en_JM"},	// English Jamaica
  {0x4409, @"en_MY"},	// English Malaysia
  {0x1409, @"en_NZ"},	// English New Zealand
  {0x3409, @"en_PH"},	// English Philippines
  {0x4809, @"en_SG"},	// English Singapore
  {0x1c09, @"en_ZA"},	// English South Africa
  {0x2c09, @"en_TT"},	// English Trinidad and Tobago
  {0x0809, @"en_GB"},	// English United Kingdom
  {0x0409, @"en_US"},	// English United States
  {0x3009, @"en_ZW"},	// English Zimbabwe

  {0x0425, @"et_EE"},	// Estonian Estonia
  {0x0438, @"fo_FO"},	// Faroese Faroe Islands
  {0x0464, @"fil_PH"},	// Filipino Philippines
  {0x040b, @"fi_FI"},	// Finnish Finland

  // French
  {0x080c, @"fr_BE"},	// French Belgium
  {0x0c0c, @"fr_CA"},	// French Canada
  {0x040c, @"fr_FR"},	// French France
  {0x140c, @"fr_LU"},	// French Luxembourg
  {0x180c, @"fr_MC"},	// French Monaco
  {0x100c, @"fr_CH"},	// French Switzerland

  {0x0462, @"fy_NL"},	// Frisian Netherlands
  {0x0456, @"gl_ES"},	// Galician Spain
  {0x0437, @"ka_GE"},	// Georgian Georgia

  // German
  {0x0c07, @"de_AT"},	// German Austria
  {0x0407, @"de_DE"},	// German Germany
  {0x1407, @"de_LI"},	// German Liechtenstein
  {0x1007, @"de_LU"},	// German Luxembourg
  {0x0807, @"de_CH"},	// German Switzerland

  {0x0408, @"el_GR"},	// Greek Greece
  {0x046f, @"kl_GL"},	// Greenlandic Greenland
  {0x0447, @"gu_IN"},	// Gujarati India
  {0x0468, @"ha_NG"},	// Hausa Nigeria
  {0x040d, @"he_IL"},	// Hebrew Israel
  {0x0439, @"hi_IN"},	// Hindi India
  {0x040e, @"hu_HU"},	// Hungarian Hungary
  {0x040f, @"is_IS"},	// Icelandic Iceland
  {0x0470, @"ig_NG"},	// Igbo Nigeria
  {0x0421, @"id_ID"},	// Indonesian Indonesia

  // Inuktitut
  {0x085d, @"iu_CA"},	// Inuktitut Canada
  {0x045d, @"iu_CA"},	// Inuktitut Canada

  {0x083c, @"ga_IE"},	// Irish Ireland

  // Italian
  {0x0410, @"it_IT"},	// Italian Italy
  {0x0810, @"it_CH"},	// Italian Switzerland

  {0x0411, @"ja_JP"},	// Japanese Japan

  // Kannada
  {0x044b, @"kn_IN"},	// Kannada India

  {0x043f, @"kk_KZ"},	// Kazakh Kazakhstan
  {0x0453, @"kh_KH"},	// Khmer Cambodia
  {0x0486, @"qut_GT"},	// K'iche Guatemala
  {0x0487, @"rw_RW"},	// Kinyarwanda Rwanda
  {0x0457, @"kok_IN"},	// Konkani India
  {0x0412, @"ko_KR"},	// Korean Korea
  {0x0440, @"ky_KG"},	// Kyrgyz Kyrgyzstan
  {0x0454, @"lo_LA"},	// Lao Lao PDR
  {0x0426, @"lv_LV"},	// Latvian Latvia
  {0x0427, @"lt_LT"},	// Lithuanian Lithuanian
  {0x082e, @"dsb_DE"},	// Lower Sorbian Germany
  {0x046e, @"lb_LU"},	// Luxembourgish Luxembourg
  {0x042f, @"mk_MK"},	// Macedonian Macedonia, FYROM

  // Malay
  {0x083e, @"ms_BN"},	// Malay Brunei Darassalam
  {0x043e, @"ms_MY"},	// Malay Malaysia

  {0x044c, @"ml_IN"},	// Malayalam India
  {0x043a, @"mt_MT"},	// Maltese Malta
  {0x0481, @"mi_NZ"},	// Maori New Zealand
  {0x047a, @"arn_CL"},	// Mapudungun Chile
  {0x044e, @"mr_IN"},	// Marathi India
  {0x047c, @"moh_CA"},	// Mohawk Canada

  // Mongolian
  {0x0450, @"mn_MN"},	// Mongolian Mongolia, Cyrillic
  {0x0850, @"mn_CN"},	// Mongolian Mongolia

  // Nepali
  {0x0461, @"ne_NP"},	// Nepali Nepal

  // Norwegian
  {0x0414, @"no_NO"},	// Norwegian Bokmål, Norway
  {0x0814, @"no_NO"},	// Norwegian Nynorsk, Norway

  {0x0482, @"oc_FR"},	// Occitan France
  {0x0448, @"or_IN"},	// Oriya India
  {0x0463, @"ps_AF"},	// Pashto Afghanistan
  {0x0429, @"fa_IR"},	// Persian Iran
  {0x0415, @"pl_PL"},	// Polish Poland

  // Portuguese
  {0x0416, @"pt_BR"},	// Portuguese Brazil
  {0x0816, @"pt_PT"},	// Portuguese Portugal
  {0x0816, @"pt_PT"},	// Portuguese Portugal

  {0x0446, @"pa_IN"},	// Punjabi India

  // Quechua
  {0x046b, @"quz_BO"},	// Quechua Bolivia
  {0x086b, @"quz_EC"},	// Quechua Ecuador
  {0x0c6b, @"quz_PE"},	// Quechua Peru

  {0x0418, @"ro_RO"},	// Romanian Romania
  {0x0417, @"rm_CH"},	// Romansh Switzerland
  {0x0419, @"ru_RU"},	// Russian Russia

  // Sami
  {0x243b, @"se_FI"},	// Sami Inari, Finland
  {0x103b, @"se_NO"},	// Sami Lule, Norway
  {0x143b, @"se_SE"},	// Sami Lule, Sweden
  {0x0c3b, @"se_FI"},	// Sami Northern, Finland
  {0x043b, @"se_NO"},	// Sami Northern, Norway
  {0x083b, @"se_SE"},	// Sami Northern, Sweden
  {0x203b, @"se_FI"},	// Sami Skolt, Finland
  {0x183b, @"se_NO"},	// Sami Southern, Norway
  {0x1c3b, @"se_SE"},	// Sami Southern, Sweden

  // Sanskrit
  {0x044f, @"sa_IN"},	// Sanskrit India
  {0x181a, @"sa_BA"},	// Sanskrit Bosnia and Herzegovina, Latin
  {0x0c1a, @"sa_CS"},	// Sanskrit Serbia, Cyrillic
  {0x081a, @"sa_CS"},	// Sanskrit Serbia, Latin

  {0x046c, @"ns_ZA"},	// Sesotho sa Leboa/Northern Sotho South Africa
  {0x0432, @"tn_ZA"},	// Setswana/Tswana South Africa
  {0x045b, @"si_LK"},	// Sinhala Sri Lanka
  {0x041b, @"sk_SK"},	// Slovak Slovakia
  {0x0424, @"sl_SI"},	// Slovenian Slovenia

  // Spanish
  {0x2c0a, @"es_AR"},	// Spanish Argentina
  {0x400a, @"es_BO"},	// Spanish Bolivia
  {0x340a, @"es_CL"},	// Spanish Chile
  {0x240a, @"es_CO"},	// Spanish Colombia
  {0x140a, @"es_CR"},	// Spanish Costa Rica
  {0x1c0a, @"es_DO"},	// Spanish Dominican Republic
  {0x300a, @"es_EC"},	// Spanish Ecuador
  {0x440a, @"es_SV"},	// Spanish El Salvador
  {0x100a, @"es_GT"},	// Spanish Guatemala
  {0x480a, @"es_HN"},	// Spanish Honduras
  {0x080a, @"es_MX"},	// Spanish Mexico
  {0x4c0a, @"es_NI"},	// Spanish Nicaragua
  {0x180a, @"es_PA"},	// Spanish Panama
  {0x3c0a, @"es_PY"},	// Spanish Paraguay
  {0x280a, @"es_PE"},	// Spanish Peru
  {0x500a, @"es_PR"},	// Spanish Puerto Rico
  {0x0c0a, @"es_ES"},	// Spanish Spain
  {0x380a, @"es_UY"},	// Spanish Uruguay
  {0x200a, @"es_VE"},	// Spanish Venezuela

  {0x0441, @"sw_KE"},	// Swahili Kenya

  // Swedish
  {0x081d, @"sv_FI"},	// Swedish Finland
  {0x041d, @"sv_SE"},	// Swedish Sweden
  {0x041d, @"sv_SE"},	// Swedish Sweden

  {0x045a, @"syr_SY"},	// Syriac Syria
  {0x0428, @"tg_TJ"},	// Tajik Tajikistan
  {0x085f, @"tmz_DZ"},	// Tamazight Algeria, Latin
  {0x0449, @"ta_IN"},	// Tamil India
  {0x0444, @"tt_RU"},	// Tatar Russia
  {0x044a, @"te_IN"},	// Telugu India
  {0x041e, @"th_TH"},	// Thai Thailand
  {0x0451, @"bo_CN"},	// Tibetan PRC
  {0x041f, @"tr_TR"},	// Turkish Turkey
  {0x0442, @"tk_TM"},	// Turkmen Turkmenistan
  {0x0480, @"ug_CN"},	// Uighur PRC
  {0x0422, @"uk_UA"},	// Ukrainian Ukraine

  // Upper Sorbian
  {0x042e, @"wen_DE"},	// Upper Sorbian Germany
  {0x0420, @"wen_PK"},	// Upper Sorbian Pakistan

  // Uzbek
  {0x0843, @"uz_UZ"},	// Uzbek Uzbekistan, Cyrillic
  {0x0443, @"uz_UZ"},	// Uzbek Uzbekistan, Latin

  {0x042a, @"vi_VN"},	// Vietnamese Vietnam
  {0x0452, @"cy_GB"},	// Welsh United Kingdom
  {0x0488, @"wo_SN"},	// Wolof Senegal
  {0x0434, @"xh_ZA"},	// Xhosa/isiXhosa South Africa
  {0x0485, @"sah_RU"},	// Yakut Russia
  {0x0478, @"ii_CN"},	// Yi PRC
  {0x046a, @"yo_NG"},	// Yoruba Nigeria
  {0x0435, @"zu_ZA"},	// Zulu/isiZulu South Africa

  {0, nil}
};

#endif //LOCALES_H

