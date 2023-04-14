/*
	libconfigini - an ini formatted configuration parser library
	Copyright (C) 2013-present Taner YILMAZ <taner44@gmail.com>

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:
	 1. Redistributions of source code must retain the above copyright
		notice, this list of conditions and the following disclaimer.
	 2. Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.
	 3. Neither the name of copyright holders nor the names of its
		contributors may be used to endorse or promote products derived
		from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
	TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
	BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CONFIGINI_H_
#define CONFIGINI_H_

#include <stdio.h>


#ifndef __cplusplus
#include <stdbool.h>
//typedef unsigned char bool;
// #undef  false
// #define false 	0
// #undef  true
// #define true	(!false)

#endif


typedef struct Config Config;


#define CONFIG_SECTION_FLAT		NULL	/* config is flat data (has no section) */


/**
 * \brief Return types
 */
typedef enum
{
	CONFIG_OK,                    /* ok (no error) */
	CONFIG_ERR_FILE,              /* file io error (file not exists, cannot open file, ...) */
	CONFIG_ERR_NO_SECTION,        /* section does not exist */
	CONFIG_ERR_NO_KEY,            /* key does not exist */
	CONFIG_ERR_MEMALLOC,          /* memory allocation failed */
	CONFIG_ERR_INVALID_PARAM,     /* invalid parametrs (as NULL) */
	CONFIG_ERR_INVALID_VALUE,     /* value of key is invalid (inconsistent data, empty data) */
	CONFIG_ERR_PARSING,           /* parsing error of data (does not fit to config format) */
} ConfigRet;



#ifdef __cplusplus
extern "C" {
#endif



Config*     ConfigNew              (void);
void        ConfigFree             (Config *cfg);

const char *ConfigRetToString      (ConfigRet ret);

ConfigRet   ConfigRead             (FILE *fp, Config **cfg);
ConfigRet   ConfigReadFile         (const char *filename, Config **cfg);

ConfigRet   ConfigPrint            (const Config *cfg, FILE *stream);
ConfigRet   ConfigPrintToFile      (const Config *cfg, char *filename);
ConfigRet   ConfigPrintSettings    (const Config *cfg, FILE *stream);

int         ConfigGetSectionCount  (const Config *cfg);
int         ConfigGetKeyCount      (const Config *cfg, const char *sect);

ConfigRet   ConfigSetCommentCharset(Config *cfg, const char *comment_ch);
ConfigRet   ConfigSetKeyValSepChar (Config *cfg, char ch);
ConfigRet   ConfigSetBoolString    (Config *cfg, const char *true_str, const char *false_str);

ConfigRet   ConfigReadString       (const Config *cfg, const char *sect, const char *key, char *        val, int size, const char * dfl_val);
ConfigRet   ConfigReadInt          (const Config *cfg, const char *sect, const char *key, int *         val,           int          dfl_val);
ConfigRet   ConfigReadUnsignedInt  (const Config *cfg, const char *sect, const char *key, unsigned int *val,           unsigned int dfl_val);
ConfigRet   ConfigReadFloat        (const Config *cfg, const char *sect, const char *key, float *       val,           float        dfl_val);
ConfigRet   ConfigReadDouble       (const Config *cfg, const char *sect, const char *key, double *      val,           double       dfl_val);
ConfigRet   ConfigReadBool         (const Config *cfg, const char *sect, const char *key, bool *        val,           bool         dfl_val);

ConfigRet   ConfigAddString        (Config *cfg, const char *sect, const char *key, const char  *val);
ConfigRet   ConfigAddInt           (Config *cfg, const char *sect, const char *key, int          val);
ConfigRet   ConfigAddUnsignedInt   (Config *cfg, const char *sect, const char *key, unsigned int val);
ConfigRet   ConfigAddFloat         (Config *cfg, const char *sect, const char *key, float        val);
ConfigRet   ConfigAddDouble        (Config *cfg, const char *sect, const char *key, double       val);
ConfigRet   ConfigAddBool          (Config *cfg, const char *sect, const char *key, bool         val);

bool        ConfigHasSection       (const Config *cfg, const char *sect);

ConfigRet   ConfigRemoveSection    (Config *cfg, const char *sect);
ConfigRet   ConfigRemoveKey        (Config *cfg, const char *sect, const char *key);


#ifdef __cplusplus
}
#endif

// #define ENTER_TEST_FUNC														\
// 	do {																	\
// 		LOG_INFO("%s", "\n-----------------------------------------------");\
// 		LOG_INFO("<TEST: %s>\n", __FUNCTION__);								\
// 	} while (0)


// /*
//  * Read Config file
//  */
// static void Test1()
// {
// 	Config *cfg = NULL;

// 	ENTER_TEST_FUNC;

// 	if (ConfigReadFile(CONFIGREADFILE, &cfg) != CONFIG_OK) {
// 		LOG_ERR("ConfigOpenFile failed for %s", CONFIGREADFILE);
// 		return;
// 	}

// 	ConfigPrintSettings(cfg, stdout);
// 	ConfigPrint(cfg, stdout);

// 	ConfigFree(cfg);
// }

// /*
//  * Create Config handle, read Config file, edit and save to new file
//  */
// static void Test2()
// {
// 	Config *cfg = NULL;

// 	ENTER_TEST_FUNC;

// 	/* set settings */
// 	cfg = ConfigNew();
// 	ConfigSetBoolString(cfg, "yes", "no");

// 	/* we can give initialized handle (rules has been set) */
// 	if (ConfigReadFile(CONFIGREADFILE, &cfg) != CONFIG_OK) {
// 		LOG_ERR("ConfigOpenFile failed for %s", CONFIGREADFILE);
// 		return;
// 	}

// 	ConfigRemoveKey(cfg, "SECT1", "a");
// 	ConfigRemoveKey(cfg, "SECT2", "aa");
// 	ConfigRemoveKey(cfg, "owner", "title");
// 	ConfigRemoveKey(cfg, "database", "file");

// 	ConfigAddBool  (cfg, "SECT1", "isModified", true);
// 	ConfigAddString(cfg, "owner", "country", "Turkey");

// 	ConfigPrintSettings(cfg, stdout);
// 	ConfigPrint(cfg, stdout);
// 	ConfigPrintToFile(cfg, CONFIGSAVEFILE);

// 	ConfigFree(cfg);
// }

// /*
//  * Create Config handle and add sections & key-values
//  */
// static void Test3()
// {
// 	Config *cfg = NULL;

// 	ENTER_TEST_FUNC;

// 	cfg = ConfigNew();

// 	ConfigSetBoolString(cfg, "true", "false");

// 	ConfigAddString(cfg, "SECTION1", "Istanbul", "34");
// 	ConfigAddInt   (cfg, "SECTION1", "Malatya", 44);

// 	ConfigAddBool  (cfg, "SECTION2", "enable", true);
// 	ConfigAddDouble(cfg, "SECTION2", "Lira", 100);

// 	ConfigPrintSettings(cfg, stdout);
// 	ConfigPrint(cfg, stdout);

// 	ConfigFree(cfg);
// }

// /*
//  * Create Config without any section
//  */
// static void Test4()
// {
// 	Config *cfg = NULL;
// 	char s[1024];
// 	bool b;
// 	float f;

// 	ENTER_TEST_FUNC;

// 	cfg = ConfigNew();

// 	ConfigAddString(cfg, CONFIG_SECTION_FLAT, "Mehmet Akif ERSOY", "Safahat");
// 	ConfigAddString(cfg, CONFIG_SECTION_FLAT, "Necip Fazil KISAKUREK", "Cile");
// 	ConfigAddBool  (cfg, CONFIG_SECTION_FLAT, "isset", true);
// 	ConfigAddFloat (cfg, CONFIG_SECTION_FLAT, "degree", 35.0);

// 	ConfigPrint(cfg, stdout);

// 	///////////////////////////////////////////////////////////////////////////////////////////////

// 	ConfigReadString(cfg, CONFIG_SECTION_FLAT, "Mehmet Akif Ersoy", s, sizeof(s), "Poet");
// 	LOG_INFO("Mehmet Akif Ersoy = %s", s);

// 	ConfigReadString(cfg, CONFIG_SECTION_FLAT, "Mehmet Akif ERSOY", s, sizeof(s), "Poet");
// 	LOG_INFO("Mehmet Akif ERSOY = %s", s);

// 	ConfigReadBool(cfg, CONFIG_SECTION_FLAT, "isset", &b, false);
// 	LOG_INFO("isset = %s", b ? "true" : "false");

// 	ConfigReadFloat(cfg, CONFIG_SECTION_FLAT, "degree", &f, 1.5);
// 	LOG_INFO("degree = %f", f);

// 	///////////////////////////////////////////////////////////////////////////////////////////////

// 	ConfigFree(cfg);
// }


#endif /* CONFIGINI_H_ */
