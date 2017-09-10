/******************************************************************
*
*	CyberUtil for C
*
*	Copyright (C) Satoshi Konno 2005
*
*       Copyright (C) 2006-2007 Nokia Corporation. All rights reserved.
*
*       This is licensed under BSD-style license,
*       see file COPYING.
*
*	File: cstring.c
*
*	Revision:
*
*	01/25/05
*		- first revision
*	12/13/07 Aapo Makela
*		- Changed memory reallocation policy to more efficient
*		- Fix crashes in out-of-memory situations
*	05/20/09
*		- Improved cg_string_*() not to abort when the string object is null.
*
******************************************************************/

#include <cybergarage/util/cstring.h>
#include <cybergarage/util/clog.h>

#include <string.h>

/* Define amount of extra characters allocated on each realloc, with this we
   can avoid many small subsequent reallocs, which takes lots of time */
#define CG_STRING_REALLOC_EXTRA		16

/****************************************
* cg_string_new
****************************************/


CgMutex    *pGlobalCGMutex = NULL;


void cg_set_mutex(CgMutex    *pMutex)
{
    pGlobalCGMutex = pMutex; //55486
}

CgString *cg_string_new()
{
	CgString *str;

	cg_log_debug_l5("Entering...\n");

	str = (CgString *)malloc(sizeof(CgString));

	if (NULL != str) {
		str->value = NULL;
		str->memSize = 0;
		str->valueSize = 0;
	}

	cg_log_debug_l5("Leaving...\n");

	return str;
}

/****************************************
* cg_string_delete
****************************************/

void cg_string_delete(CgString *str)
{
	cg_log_debug_l5("Entering...\n");

    cg_mutex_lock(pGlobalCGMutex);
	if (NULL != str) {
		cg_string_clear(str);
        
		SAFE_FREE(str);
	}
	cg_mutex_unlock(pGlobalCGMutex);
	cg_log_debug_l5("Leaving...\n");
}

/****************************************
* cg_string_delete
****************************************/

void cg_string_clear(CgString *str)
{
	cg_log_debug_l5("Entering...\n");
	
	if (NULL != str)
    {
		if (str->value != NULL )
        {
            SAFE_FREE(str->value);
			str->value = NULL;
			str->memSize = 0;
			str->valueSize = 0;
		}
	}

	cg_log_debug_l5("Leaving...\n");
}


void cg_hexdump_setvalue(unsigned char *str, const unsigned char *value)
{
	cg_log_debug_l5("Entering...\n");
    
	if (NULL != str) {
        /*
		if (value != NULL)
			cg_hexdump_setnvalue(str, value, 87654);
        */
	}
	
	cg_log_debug_l5("Leaving...\n");
}

/****************************************
* cg_string_setvalue
****************************************/

void cg_string_setvalue(CgString *str, const char *value)
{
	cg_log_debug_l5("Entering...\n");

	if (NULL != str) {
		if (value != NULL)
			cg_string_setnvalue(str, value, cg_strlen(value));
	}
	
	cg_log_debug_l5("Leaving...\n");
}

/****************************************
* cg_string_setintvalue
****************************************/

void cg_string_setintvalue(CgString *str, int value)
{
	char buf[CG_STRING_INTEGER_BUFLEN];

	cg_log_debug_l5("Entering...\n");

	cg_string_setvalue(str, cg_int2str(value, buf, sizeof(buf)));

	cg_log_debug_l5("Leaving...\n");
}

/****************************************
* cg_string_setlongvalue
****************************************/

void cg_string_setlongvalue(CgString *str, long value)
{
	char buf[CG_STRING_LONG_BUFLEN];

	cg_log_debug_l5("Entering...\n");

	cg_string_setvalue(str, cg_long2str(value, buf, sizeof(buf)));

	cg_log_debug_l5("Leaving...\n");
}

/****************************************
* cg_string_setnvalue
****************************************/

void cg_string_setnvalue(CgString *str, const char *value, size_t len)
{
	cg_log_debug_l5("Entering...\n");

    cg_mutex_lock(pGlobalCGMutex);
	if (NULL != str) {
		cg_string_clear(str);
		if (value != NULL) {
			str->valueSize = len;
			str->memSize = str->valueSize + 1;
			str->value = (char *)malloc(str->memSize * sizeof(char));

			if ( NULL == str->value ) {
				cg_log_debug_s("Memory allocation failure!\n");
				cg_mutex_unlock(pGlobalCGMutex);
				return;
			}

			/* memcpy works better with non-zero-terminated data than strncpy */
			memcpy(str->value, value, len);
			str->value[len] = '\0';
		}
	}
    cg_mutex_unlock(pGlobalCGMutex);
	cg_log_debug_l5("Leaving...\n");
}

/****************************************
* cg_string_setpointervalue
****************************************/

void cg_string_setpointervalue(CgString *str, char *value, size_t len)
{
	cg_log_debug_l5("Entering...\n");
    cg_mutex_lock(pGlobalCGMutex);
	if (NULL != str) {
		cg_string_clear(str);
		str->value = value;
		str->valueSize = len;
		str->memSize = str->valueSize + 1;
	}
	cg_mutex_unlock(pGlobalCGMutex);
	
	cg_log_debug_l5("Leaving...\n");
}

/****************************************
* cg_string_getvalue
****************************************/

char *cg_string_getvalue(CgString *str)
{
	cg_log_debug_l5("Entering...\n");

	cg_log_debug_l5("Leaving...\n");

	return (NULL != str) ? str->value : NULL;
}

/****************************************
* cg_string_getmemorysize
****************************************/

size_t cg_string_getmemorysize(CgString *str)
{
	cg_log_debug_l5("Entering...\n");

	if (NULL == str)
		return 0;

	cg_log_debug_l5("Leaving...\n");

	return str->memSize;
}

/****************************************
* cg_string_length
****************************************/

size_t cg_string_length(CgString *str)
{
	cg_log_debug_l5("Entering...\n");

	if (NULL == str)
		return 0;

	if (str->value == NULL)
		return 0;

	cg_log_debug_l5("Leaving...\n");

	return str->valueSize;
}

/****************************************
* cg_string_add
****************************************/

char *cg_string_addvalue(CgString *str, const char *value)
{
	cg_log_debug_l5("Entering...\n");

	cg_log_debug_l5("Leaving...\n");

	return cg_string_naddvalue(str, value, cg_strlen(value));
}

void cg_hexdump_setnvalue(unsigned char *str, const unsigned char *value, int len)
{
	cg_log_debug_l5("Entering...\n");
    

    
	cg_log_debug_l5("Leaving...\n");
}

/****************************************
* cg_string_add
****************************************/

char *cg_string_naddvalue(CgString *str, const char *value, size_t valueLen)
{
	char *newValue = NULL;
	size_t newMemSize = 0;

	cg_log_debug_l5("Entering...\n");

	if (NULL == str)
		return NULL;

	if (value == NULL || valueLen <= 0)
	{
		/* Empty string, nothing to add */
		return cg_string_getvalue(str);
	}

	/* Check, if we need to allocate memory for the new data */
	newMemSize = str->valueSize + valueLen + 1;
	if (newMemSize > str->memSize || str->value == NULL)
	{
		/* realloc also some extra in order to avoid multiple reallocs */
		newMemSize += CG_STRING_REALLOC_EXTRA;
		newValue = realloc(str->value, newMemSize * sizeof(char));

		if (newValue == NULL)
		{
			/* Memory allocation failed, bail out */
			return NULL;
		}

		str->memSize = newMemSize;
		str->value = newValue;
	}

	/* memcpy works better with non-zero-terminated data
	   than strncpy */
	memcpy(str->value + str->valueSize, value, valueLen);

	str->valueSize += valueLen;
	
	/* In case this is a string, append a termination character */
	str->value[str->valueSize] = '\0';

	cg_log_debug_l5("Leaving...\n");

	return cg_string_getvalue(str);
}

/****************************************
* cg_string_addrep
****************************************/

char *cg_string_addrepvalue(CgString *str, const char *value, size_t repeatCnt)
{
	int n;
	cg_log_debug_l5("Entering...\n");

	for (n = 0; n < repeatCnt; n++)
		cg_string_addvalue(str, value);

	cg_log_debug_l5("Leaving...\n");

	return cg_string_getvalue(str);
}

/****************************************
* cg_string_naddrep
****************************************/

char *cg_string_naddrepvalue(CgString *str, const char *value, size_t valueLen, size_t repeatCnt)
{
	int n;

	cg_log_debug_l5("Entering...\n");

	for (n = 0; n < repeatCnt; n++)
		cg_string_naddvalue(str, value, valueLen);

	cg_log_debug_l5("Leaving...\n");

	return cg_string_getvalue(str);
}

/****************************************
* cg_string_replace
****************************************/

char *cg_string_replace(CgString *str, char *fromStr[], char *toStr[], size_t fromStrCnt)
{
	char *orgValue = NULL;
	size_t orgValueLen = 0;
	int n = 0;
	int copyPos = 0;
	size_t *fromStrLen = NULL;
	CgString *repValue = NULL;
	BOOL isReplaced = FALSE;

	cg_log_debug_l5("Entering...\n");

	if (NULL == str )
		return NULL;
	
	repValue = cg_string_new();
	
	fromStrLen = (size_t *)malloc(sizeof(size_t) * fromStrCnt);

	if ( NULL == fromStrLen )
	{
        cg_string_delete(repValue);
		cg_log_debug_s("Memory allocation failure!\n");
		return NULL;
	}
	
	for (n=0; n<fromStrCnt; n++)
		fromStrLen[n] = cg_strlen(fromStr[n]);
	
	orgValue = cg_string_getvalue(str);
	orgValueLen = cg_string_length(str);
	
	copyPos = 0;
	while (copyPos<orgValueLen) {
		isReplaced = FALSE;
		for (n=0; n<fromStrCnt; n++) {
			if (strncmp(fromStr[n], orgValue + copyPos,  fromStrLen[n]) == 0) {
				cg_string_addvalue(repValue, toStr[n]);
				copyPos += fromStrLen[n];
				isReplaced = TRUE;
				continue;
			}
		}
		if (isReplaced == TRUE)
			continue;
		cg_string_naddvalue(repValue, orgValue + copyPos, 1);
		copyPos++;
	}
	
	SAFE_FREE(fromStrLen);

	cg_string_setvalue(str, cg_string_getvalue(repValue));	

	cg_string_delete(repValue);
		
	cg_log_debug_l5("Leaving...\n");

	return cg_string_getvalue(str);
}
