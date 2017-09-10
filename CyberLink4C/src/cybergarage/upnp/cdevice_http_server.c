/******************************************************************
*
*	CyberLink for C
*
*	Copyright (C) Satoshi Konno 2005
*
*       Copyright (C) 2006 Nokia Corporation. All rights reserved.
*
*       This is licensed under BSD-style license,
*       see file COPYING.
*
*	File: cdevice_http_server.c
*
*	Revision:
*
*	03/22/05
*		- first revision
*
*	18-Jan-06 Aapo Makela
*		- Added HEAD-method support
*		- Fixed to handle "Not Found" case for GET-request
*		- Fixed to unescape URIs
******************************************************************/

#include <cybergarage/upnp/cdevice.h>
#include <cybergarage/upnp/cupnp.h>
#include <cybergarage/http/chttp.h>
#include <cybergarage/net/curl.h>
#include <cybergarage/upnp/control/ccontrol.h>
#include <cybergarage/upnp/event/cevent.h>
#include <cybergarage/util/clog.h>
#import <AudioToolbox/AudioFile.h>
#import <AudioToolbox/ExtendedAudioFile.h>



/****************************************
* prototype define for static functions
****************************************/




static void cg_upnp_device_getrequestrecieved(CgUpnpDevice *dev, CgHttpRequest *httpReq);
static void cg_upnp_device_getStreamingrequestrecieved(CgUpnpDevice *dev, CgHttpRequest *httpReq);
static void cg_upnp_device_postrequestrecieved(CgUpnpDevice *dev, CgHttpRequest *httpReq);
static void cg_upnp_device_soapactionrecieved(CgUpnpDevice *dev, CgSoapRequest *soapReq);

static void cg_upnp_device_controlrequestrecieved(CgUpnpService *service, CgSoapRequest *soapReq);
#if !defined(CG_UPNP_NOUSE_ACTIONCTRL)
static void cg_upnp_device_actioncontrolrequestrecieved(CgUpnpService *service, CgUpnpActionRequest *actionReq);
#endif
#if !defined(CG_UPNP_NOUSE_QUERYCTRL)
static void cg_upnp_device_querycontrolrequestrecieved(CgUpnpService *service, CgUpnpQueryRequest *queryReq);
#endif

#if !defined(CG_UPNP_NOUSE_SUBSCRIPTION)
static void cg_upnp_device_subscriptionrecieved(CgUpnpDevice *dev, CgUpnpSubscriptionRequest *subReq);
static void cg_upnp_device_newsubscriptionrecieved(CgUpnpService *service, CgUpnpSubscriptionRequest *subReq);
static void cg_upnp_device_renewsubscriptionrecieved(CgUpnpService *service, CgUpnpSubscriptionRequest *subReq);
static void cg_upnp_device_unsubscriptionrecieved(CgUpnpService *service, CgUpnpSubscriptionRequest *subReq);
#endif

/****************************************
* cg_upnp_device_httprequestrecieved
****************************************/

void cg_upnp_device_httpStreamingrequestrecieved(CgHttpRequest *httpReq)
{
    //return;
    printf("\n\n   cg_upnp_device_httprequestrecieved ");
    CgUpnpDevice *dev;
    CgString *unescapedUrl;
    char *url;
    
    cg_log_debug_l4("Entering...\n");
    
    dev = (CgUpnpDevice *)cg_http_request_getuserdata(httpReq);
    
    /* Unescape URI */
    url = cg_http_request_geturi(httpReq);
    printf("\n Requested url AKSHAY %s",url);
    if (0 < cg_strlen(url)) {
        unescapedUrl = cg_string_new();
        cg_net_uri_unescapestring(url, 0, unescapedUrl);
        if (0 < cg_string_length(unescapedUrl))
            cg_http_request_seturi(httpReq, cg_string_getvalue(unescapedUrl));
        cg_string_delete(unescapedUrl);
    }
    if (cg_http_request_isgetrequest(httpReq) == TRUE ||
        cg_http_request_isheadrequest(httpReq) == TRUE) {
        printf("\n Is a get request");
        
        cg_upnp_device_getStreamingrequestrecieved(dev, httpReq);
        
        
        return;
    }
    
    if (cg_http_request_ispostrequest(httpReq) == TRUE) {
        cg_upnp_device_postrequestrecieved(dev, httpReq);
        return;
    }
    
#if !defined(CG_UPNP_NOUSE_SUBSCRIPTION)
    if (cg_http_request_issubscriberequest(httpReq) == TRUE || cg_http_request_isunsubscriberequest(httpReq) == TRUE) {
        cg_upnp_device_subscriptionrecieved(dev, httpReq);
        return;
    }
#endif
    
    cg_http_request_postbadrequest(httpReq);
    
    cg_log_debug_l4("Leaving...\n");
}


void cg_upnp_device_httprequestrecieved(CgHttpRequest *httpReq)
{
	CgUpnpDevice *dev;
	CgString *unescapedUrl;
	char *url;
	
	cg_log_debug_l4("Entering...\n");

	dev = (CgUpnpDevice *)cg_http_request_getuserdata(httpReq);
	
	/* Unescape URI */
	url = cg_http_request_geturi(httpReq);
	if (0 < cg_strlen(url)) {
		unescapedUrl = cg_string_new();
		cg_net_uri_unescapestring(url, 0, unescapedUrl);
		if (0 < cg_string_length(unescapedUrl))
			cg_http_request_seturi(httpReq, cg_string_getvalue(unescapedUrl));
		cg_string_delete(unescapedUrl);
	}
	
	if (cg_http_request_isgetrequest(httpReq) == TRUE ||
	    cg_http_request_isheadrequest(httpReq) == TRUE) {
		cg_upnp_device_getrequestrecieved(dev, httpReq);
		return;
	}

	if (cg_http_request_ispostrequest(httpReq) == TRUE) {
		cg_upnp_device_postrequestrecieved(dev, httpReq);
		return;
	}

#if !defined(CG_UPNP_NOUSE_SUBSCRIPTION)
	if (cg_http_request_issubscriberequest(httpReq) == TRUE || cg_http_request_isunsubscriberequest(httpReq) == TRUE) {
		cg_upnp_device_subscriptionrecieved(dev, httpReq);
		return;
	}
#endif

	cg_http_request_postbadrequest(httpReq);

	cg_log_debug_l4("Leaving...\n");
}

/****************************************
* 
* HTTP GET REQUEST
*
****************************************/

/****************************************
* cg_upnp_device_updateurlbase
****************************************/

void cg_upnp_device_seturlbase(CgUpnpDevice *dev, char *value)
{
	CgXmlNode *rootNode;
	CgXmlNode *node;

	cg_log_debug_l4("Entering...\n");

	if (cg_upnp_device_isrootdevice(dev) == FALSE)
		return;
	
	rootNode = cg_upnp_device_getrootnode(dev);
	if (rootNode == NULL)
		return;

	node = cg_xml_node_getchildnode(rootNode, CG_UPNP_DEVICE_URLBASE_NAME);
	if (node != NULL) {
		cg_xml_node_setvalue(node, value);
		return;
	}

	node = cg_xml_node_new();
	cg_xml_node_setname(node, CG_UPNP_DEVICE_URLBASE_NAME);
	cg_xml_node_setvalue(node, value);

	cg_xml_node_addchildnode(rootNode, node);

	cg_log_debug_l4("Leaving...\n");
}

static void cg_upnp_device_updateurlbase(CgUpnpDevice *dev, char *host)
{
	char urlBase[CG_UPNP_DEVICE_URLBASE_MAXLEN];

	cg_log_debug_l4("Entering...\n");

	cg_net_gethosturl(host, cg_upnp_device_gethttpport(dev), "", urlBase, sizeof(urlBase));
	cg_upnp_device_seturlbase(dev, urlBase);

	cg_log_debug_l4("Leaving...\n");
}

static char *cg_upnp_device_getdescription(CgUpnpDevice *dev, char *ifAddr, CgString *descStr)
{
	CgXmlNode *rootNode;
	
	cg_log_debug_l4("Entering...\n");

	cg_upnp_device_lock(dev);
	
	if (cg_upnp_isnmprmode() == FALSE)
		cg_upnp_device_updateurlbase(dev, ifAddr);
	
	rootNode = cg_upnp_device_getrootnode(dev);
	
	if (rootNode != NULL) {
		cg_string_addvalue(descStr, CG_UPNP_XML_DECLARATION);
		cg_string_addvalue(descStr, "\n");
		cg_xml_node_tostring(rootNode, TRUE, descStr);	
	}
	
	cg_upnp_device_unlock(dev);
	
	return cg_string_getvalue(descStr);

	cg_log_debug_l4("Leaving...\n");
}

void removeSubstring(char *s,const char *toremove)
{
    while( (s=strstr(s,toremove)) )
        memmove(s,s+strlen(toremove),1+strlen(s+strlen(toremove)));
}

int URIdecode (char *str, char *copy) {  //url decode to remove %20 --> ' ' etc
    int len = strlen(str), i, j = 0;
    char hex[3] = {0};
    
    for (i = 0; i < len; i++) {
        if (str[i] == '%' && i < len-2) {
            i++;
            strncpy(hex, &str[i++], 2);
            copy[j] = strtol(hex, NULL, 16);
        } else if (str[i] == '+') copy[j] = ' ';
        else copy[j] = str[i];
        j++;
    }
    copy[j] = '\0';
    printf("\n decode url %s",copy);
    return j;
}
//Remove special character inbetween the url
void str_replace(char *target, const char *needle, const char *replacement)
{
    char buffer[1024] = { 0 };
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);
    
    while (1) {
        const char *p = strstr(tmp, needle);
        
        // walked past last occurrence of needle; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }
        
        // copy part before needle
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;
        
        // copy replacement string
        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;
        
        // adjust pointers, move on
        tmp = p + needle_len;
    }
    
    // write altered string back to target
    strcpy(target, buffer);
}


bool isclosesocket = false;

static void cg_upnp_device_getStreamingrequestrecieved(CgUpnpDevice *dev, CgHttpRequest *httpReq)
{
    //return;
    isclosesocket = false;
    printf("\n cg_upnp_device_getStreamingrequestrecieved");
    
    cg_http_request_print(httpReq);
    CgString *descStr;
    char *url;
    char *ifAddr;
    CgHttpResponse *httpRes;
    
    char decodeURL[200];  //Hopefully this allocation is enough
    //URIdecode(dec,copy);
    
    url = cg_http_request_geturi(httpReq);
    if (cg_strlen(url) <= 0) {
        cg_http_request_postbadrequest(httpReq);
        return;
    }
    
    //cg_http_packet_getheadervalue();
    
    descStr = cg_string_new();
    ifAddr = cg_http_request_getlocaladdress(httpReq);
    
    
    
    if(strstr(url,"/akshayv") != NULL && strstr(url,"!") != NULL){  //Futsal //Indicate that it is a non mp3 song hence convert lpcm
        removeSubstring(url,"/akshayv");  //strip off the /spurlclint
        
        str_replace(url, "%20", " ");
        
        printf("\n url after replacing special character %s",url);
        
        URIdecode(url,decodeURL);
        printf("\n URL  after decoding %s",decodeURL);
        
        
        char *tok = strtok(url,"!");
        
        char tempurl[255];
        strcpy(tempurl,tok);
        
        char dur[20];
        strcpy(dur, strtok(NULL,"!"));
        
        
        strcpy(url,tempurl);
        
        
        //TimeSeekRange.dlna.org
        char *rangeHeaderReqval = cg_http_request_getheadervalue(httpReq,"Range");
        
        descStr = cg_string_new();
        ifAddr = cg_http_request_getlocaladdress(httpReq);
        
        //Range: bytes =135545-  //This is the request format for Range
        char *subString = strtok(rangeHeaderReqval,"="); // find the first double quote
        subString=strtok(NULL,"-");
        
        int byteoffset;
        if(rangeHeaderReqval==NULL)
            byteoffset = 0;
        else
            byteoffset = atoi(subString);
        
        httpRes = cg_http_response_new();
        cg_http_response_setstatuscode(httpRes, CG_HTTP_STATUS_PARTIAL_CONTENT);// Fix for seek issue :Akshay
        //cg_http_response_setstatuscode(httpRes, CG_HTTP_STATUS_OK);
        cg_http_response_setcontenttype(httpRes, CG_AUDIO_CONTENTL16_TYPE);
        
        cg_http_response_setacceptranges(httpRes, "bytes");
        cg_http_response_settransfermode(httpRes, "Streaming");
        
        
        // char *urlhardcode = "/var/mobile/Applications/14D04BC8-7DE5-4A63-BC75-B1D52DE384C4/Documents/ARR.m4a";
        //        char *urlhardcode = "var/mobile/Applications/14D04BC8-7DE5-4A63-BC75-B1D52DE384C4/Documents/ARR Dheeme 1.mp3";
        CFURLRef urlAsset = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                                          CFStringCreateWithCString(kCFAllocatorMallocZone, url, kCFStringEncodingASCII),
                                                          kCFURLPOSIXPathStyle,
                                                          false);
        
        
        
        OSStatus openErr = noErr;
        AudioFileID audioFile = NULL;
        
        AudioBufferList fillBufList;
        
        
        UInt32 size;
        ExtAudioFileRef sourceAudioFile = 0;
        ExtAudioFileRef destinationFile = 0;
        
        OSStatus result = noErr;
        
        //        if(sourceAudioFile){
        //            isclosesocket = true;
        //        result = ExtAudioFileDispose(sourceAudioFile);
        //        }
        
        result = ExtAudioFileOpenURL(urlAsset, &sourceAudioFile);
        
        
        AudioStreamBasicDescription sourceFormat;
        size = sizeof( sourceFormat );
        result = ExtAudioFileGetProperty( sourceAudioFile, kExtAudioFileProperty_FileDataFormat, &size, &sourceFormat );
        
        AudioStreamBasicDescription clientFormat;
        int songDur = atoi(dur);  //TODO: change this duration
        clientFormat.mSampleRate = 44100;
        clientFormat.mFormatID = kAudioFormatLinearPCM;
        clientFormat.mFormatFlags = kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger;
        clientFormat.mChannelsPerFrame = 2;
        clientFormat.mBitsPerChannel = 16;  //kAudioFormatFlagIsPacked
        clientFormat.mBytesPerFrame = 4;
        clientFormat.mBytesPerPacket = 4;
        clientFormat.mFramesPerPacket = 1;
        
        int contentLen = clientFormat.mSampleRate*clientFormat.mBitsPerChannel*clientFormat.mChannelsPerFrame*songDur/8;
        
        // Set the client format in source and destination file.
        size = sizeof( clientFormat );
        result = ExtAudioFileSetProperty( sourceAudioFile, kExtAudioFileProperty_ClientDataFormat, size, &clientFormat );
        
        AudioStreamBasicDescription dstFormat;
        dstFormat = sourceFormat;
        
        
        
        size = sizeof( clientFormat );
        
        //       AudioBufferList fillBufList;
        UInt32 bufferByteSize = 32768;  //byesperframe*frames; //ARR song
        char srcBuffer[bufferByteSize];
        fillBufList.mNumberBuffers = 1;
        fillBufList.mBuffers[0].mNumberChannels = 2;
        fillBufList.mBuffers[0].mDataByteSize = bufferByteSize;
        fillBufList.mBuffers[0].mData = srcBuffer;
        UInt32 numFrames = 8192;  //8192
        
        bool isbyteseek = false;
        
        //           if(theData)
        //             free(theData);
        
        if(byteoffset == 0)//Read entire file
            isbyteseek = false;
        else
            isbyteseek = true;
        
        UInt32 offsetbytesize;
        if(isbyteseek)   //Write the Range headers  // TODO implement byteseek logic
        {
            
            offsetbytesize = contentLen-byteoffset;
            
            
            char contentRange[100] = "bytes ";
            char strtemp[15];
            sprintf(strtemp, "%d", offsetbytesize);
            printf( "Offset SEEK %d", offsetbytesize);
            strcat(contentRange,strtemp);
            strcat(contentRange,"-");
            sprintf(strtemp, "%d", contentLen-1);
            strcat(contentRange,strtemp);
            strcat(contentRange,"/");
            sprintf(strtemp, "%d", contentLen);
            strcat(contentRange,strtemp);
            printf("\n Content-Range: %s",contentRange);
            cg_http_response_setcontentrange(httpRes, contentRange);
            cg_http_response_setcontentlength(httpRes, offsetbytesize);
            
            cg_http_request_postresponse_user(httpReq, httpRes);  //UNCOMMENT
            CgSocket* sock = cg_http_request_getsocket(httpReq); //5884865
            
            //calculate frame to seek = offset/framesize
            UInt32 frametoseek=byteoffset/4;
            result = ExtAudioFileSeek(sourceAudioFile, frametoseek);
            
            while(1){
                if(isclosesocket) break;
                frametoseek = frametoseek+numFrames;
                result = ExtAudioFileRead(sourceAudioFile, &numFrames, &fillBufList);
                
                if(numFrames == 0) break;
                
                result = ExtAudioFileSeek(sourceAudioFile, frametoseek+1);
                cg_socket_write(sock,fillBufList.mBuffers[0].mData,fillBufList.mBuffers[0].mDataByteSize);
            }
        }
        else
        {
            char contentRange[100] = "bytes ";
            strcat(contentRange,"1-");
            char strtemp[15];
            sprintf(strtemp, "%d", contentLen-1);
            strcat(contentRange,strtemp);
            strcat(contentRange,"/");
            sprintf(strtemp, "%d", contentLen-1);
            strcat(contentRange,strtemp);
            printf("\n Content-Range: %s",contentRange);
            cg_http_response_setcontentrange(httpRes, contentRange);
            cg_http_response_setcontentlength(httpRes, contentLen);
            
            cg_http_request_postresponse_user(httpReq, httpRes);  //UNCOMMENT
            CgSocket* sock = cg_http_request_getsocket(httpReq); //5884865
            
            UInt32 frametoseek=0;
            while(1){
                frametoseek = frametoseek+numFrames;
                result = ExtAudioFileRead(sourceAudioFile, &numFrames, &fillBufList);
                
                if(numFrames == 0) break;
                
                result = ExtAudioFileSeek(sourceAudioFile, frametoseek+1);
                cg_socket_write(sock,fillBufList.mBuffers[0].mData,fillBufList.mBuffers[0].mDataByteSize);
            }
        }
        
        if (cg_http_request_isheadrequest(httpReq) == TRUE)
        {
            
            cg_http_response_setcontent(httpRes, NULL);
        }
        
        
        
        
        // free(fillBufList.mBuffers[0].mData);
        cg_http_response_delete(httpRes);
        
        return;
    }
    
    if(strstr(url,"/akshayv") != NULL )
    {
        removeSubstring(url,"/akshayv");  //strip off the /spurlclint
        
        str_replace(url, "%20", " ");
        
        printf("\n url after replacing special character %s",url);
        
        URIdecode(url,decodeURL);
        printf("\n URL  after decoding %s",decodeURL);
        
        
        //TimeSeekRange.dlna.org
        char *rangeHeaderReqval = cg_http_request_getheadervalue(httpReq,"Range");
        
        descStr = cg_string_new();
        ifAddr = cg_http_request_getlocaladdress(httpReq);
        
        //Range: bytes =135545-  //This is the request format for Range
        char *subString = strtok(rangeHeaderReqval,"="); // find the first double quote
        subString=strtok(NULL,"-");
        
        int byteoffset;
        if(rangeHeaderReqval==NULL)
            byteoffset = 0;
        else
            byteoffset = atoi(subString);
        
        
        printf("\n#########a<--byteoffset=%d\n",byteoffset);
        
        httpRes = cg_http_response_new();
        
        if(strstr(url,".mp3") != NULL)
            cg_http_response_setcontenttype(httpRes, CG_AUDIO_CONTENT_TYPE_MP3);
        else if(strstr(url,".m4a") != NULL)
            cg_http_response_setcontenttype(httpRes, CG_AUDIO_CONTENT_TYPE_AAC);//Akshay
        else if(strstr(url,".wav") != NULL)
            cg_http_response_setcontenttype(httpRes, CG_AUDIO_CONTENT_TYPE_WAV);//Akshay
        
        cg_http_response_setacceptranges(httpRes, "bytes");
        cg_http_response_settransfermode(httpRes, "Streaming");
        
        FILE *outFile = fopen(url, "rb");
        
        if(outFile ==  NULL)
        {
            return;
        }
        
        unsigned  int contentLen  = 0;
        fseek(outFile, 0L, SEEK_END);
        contentLen = ftell(outFile);
        
        fseek(outFile, 0L, SEEK_SET);
        
        
        
        bool isbyteseek = false;
        if(byteoffset == 0)
            isbyteseek = false;
        else
            isbyteseek = true;
        
        unsigned long nTotalSent = 0;
        unsigned long cmdPos = 0;
        unsigned long nSent = 0;
        
        UInt32 offsetbytesize;
        if(isbyteseek)   //Write the Range headers  // TODO implement byteseek logic
        {
            
            offsetbytesize = contentLen-byteoffset;
            
            
            char contentRange[100] = "bytes ";
            char strtemp[15];
            sprintf(strtemp, "%d", offsetbytesize);
            printf( "Offset SEEK %d", offsetbytesize);
            strcat(contentRange,strtemp);
            strcat(contentRange,"-");
            sprintf(strtemp, "%d", contentLen-1);
            strcat(contentRange,strtemp);
            strcat(contentRange,"/");
            sprintf(strtemp, "%d", contentLen);
            strcat(contentRange,strtemp);
            printf("\n Content-Range: %s",contentRange);
            cg_http_response_setcontentrange(httpRes, contentRange);
            cg_http_response_setcontentlength(httpRes, offsetbytesize);
            
            cg_http_response_setstatuscode(httpRes, CG_HTTP_STATUS_PARTIAL_CONTENT);
            
            cg_http_request_postresponse_user(httpReq, httpRes);  //UNCOMMENT
            CgSocket* sock = cg_http_request_getsocket(httpReq); //5884865
            
            fseek(outFile, byteoffset, SEEK_SET);
            
            char *buffer = malloc(offsetbytesize);
            int bytesRead = fread(buffer, 1, offsetbytesize, outFile);
            
            printf("\n V-->BUFFER1 \n\n\n = %s",buffer);
            
            nSent = cg_socket_write(sock,buffer + cmdPos,offsetbytesize);
            printf("\na<--nSent=%lu,nTotalSent=%lu\n",nSent,nTotalSent);
            
            //            if(buffer != NULL)
            //            {
            SAFE_FREE(buffer);
            //            }
            
        }
        else
        {
            char contentRange[100] = "bytes ";
            strcat(contentRange,"1-");
            char strtemp[15];
            sprintf(strtemp, "%d", contentLen-1);
            strcat(contentRange,strtemp);
            strcat(contentRange,"/");
            sprintf(strtemp, "%d", contentLen-1);
            strcat(contentRange,strtemp);
            printf("\n Content-Range: %s",contentRange);
            cg_http_response_setcontentrange(httpRes, contentRange);
            cg_http_response_setcontentlength(httpRes, contentLen);
            cg_http_response_setstatuscode(httpRes, CG_HTTP_STATUS_OK);
            
            cg_http_request_postresponse_user(httpReq, httpRes);  //UNCOMMENT
            CgSocket* sock = cg_http_request_getsocket(httpReq); //5884865
            
            unsigned long bytesRead=0;
            char *buffer = malloc(contentLen);
            bytesRead = fread(buffer, 1, contentLen, outFile);
            
            printf("\n V-->BUFFER \n\n\n = %s",buffer);
            
            nSent = cg_socket_write(sock,buffer + cmdPos,contentLen);
            printf("\na<--nSent=%lu,nTotalSent=%lu\n",nSent,nTotalSent);
            
            SAFE_FREE(buffer);
        }
        
        if (cg_http_request_isheadrequest(httpReq) == TRUE)
        {
            
            cg_http_response_setcontent(httpRes, NULL);
        }
        
        cg_http_response_delete(httpRes);
        
        fclose(outFile);
        
        return;
    }
    
    
    
    /*if(strstr(url,"/spurlclint") != NULL && strstr(url,".m4a")){
     removeSubstring(url,"/spurlclint");  //strip off the /spurlclint
     URIdecode(url,decodeURL);
     printf("\n This is an spurlclint  akki after decode %s",decodeURL);
     
     char *rangeHeaderReqval = cg_http_request_getheadervalue(httpReq,"Range");
     
     descStr = cg_string_new();
     ifAddr = cg_http_request_getlocaladdress(httpReq);
     
     //Range: bytes =135545-  //This is the request format for Range
     char *subString = strtok(rangeHeaderReqval,"="); // find the first double quote
     subString=strtok(NULL,"-");
     
     int byteoffset;
     if(rangeHeaderReqval==NULL)
     byteoffset = 0;
     else
     byteoffset = atoi(subString);
     
     printf("\na<--byteoffset=%d\n",byteoffset);
     
     
     CFURLRef urlAsset = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
     CFStringCreateWithCString(kCFAllocatorMallocZone, decodeURL,kCFStringEncodingASCII),
     kCFURLPOSIXPathStyle,
     false);
     
     
     
     
     OSStatus openErr = noErr;
     AudioFileID audioFile = NULL;
     openErr = AudioFileOpenURL(urlAsset,
     kAudioFileReadPermission ,
     0,
     &audioFile);
     printf("\n @@@@FILE open status status%d\n",(int)openErr);
     
     
     UInt64 fileDataSize = 0;
     AudioStreamBasicDescription theFileFormat;
     UInt32 thePropertySize = sizeof(theFileFormat);
     thePropertySize = sizeof(fileDataSize);
     
     
     openErr = AudioFileGetProperty(audioFile, kAudioFilePropertyAudioDataByteCount, &thePropertySize, &fileDataSize);
     printf("\n @@@@AudioFileGetProperty status%d fileDataSize =%d\n",(int)openErr,fileDataSize);
     
     
     // UInt32 dataSize = fileDataSize;
     //        void* theData = malloc(dataSize); //allocate inside if loop
     
     UInt32 dataSize;
     UInt32 offsetbytesize;
     void* theData;  //allocate inside if loop
     
     OSStatus openErr11;
     
     bool isbyteseek = false;
     
     //           if(theData)
     //             free(theData);
     
     if(byteoffset == 0)
     {//Read entire file
     dataSize = fileDataSize;
     theData = malloc(dataSize);
     
     if(theData)
     openErr11 = AudioFileReadBytes(audioFile, false, 0, &dataSize, theData);
     
     printf("\n @@@@AudioFileReadBytes status%d",(int)openErr);
     isbyteseek = false;
     }
     else
     {
     offsetbytesize = fileDataSize-byteoffset;
     dataSize = offsetbytesize;
     theData = malloc(dataSize);
     
     if(theData)
     openErr11 = AudioFileReadBytes(audioFile, false,byteoffset, &offsetbytesize, theData);
     printf("\n @@@@AudioFileReadBytes status%d",(int)openErr);
     isbyteseek = true;
     }
     
     //}
     //http://192.168.1.101:45000/spurlclint/var/mobile/Applications/7EBB2ABB-7354-4F7D-A3F2-7EDA243FDF8A/Documents/BNEJ.mp3
     
     openErr = AudioFileClose(audioFile);
     printf("\n @@@@AudioFileClose status%d",(int)openErr);
     printf("\n This is an sprul Audio file close status%d",(int)openErr);
     printf("\n This is an sprul ");
     
     httpRes = cg_http_response_new();
     cg_http_response_setstatuscode(httpRes, CG_HTTP_STATUS_PARTIAL_CONTENT);// Fix for seek issue :Akshay
     //cg_http_response_setstatuscode(httpRes, CG_HTTP_STATUS_OK);
     
     if(strstr(url,".mp3") != NULL)
     cg_http_response_setcontenttype(httpRes, CG_AUDIO_CONTENT_TYPE_MP3);
     else if(strstr(url,".m4a") != NULL)
     cg_http_response_setcontenttype(httpRes, CG_AUDIO_CONTENT_TYPE_AAC);//Akshay
     
     //        cg_http_response_setcontentfeaturedlnaorg(httpRes, "DLNA.ORG_PN=AAC_ISO;DLNA_OP=01;DLNA_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000");
     
     cg_http_response_setacceptranges(httpRes, "bytes");
     cg_http_response_settransfermode(httpRes, "Streaming");
     
     //char *contentRange = "bytes 1-5884864/5884865";
     if(isbyteseek)   //Write the Range headers
     {
     char contentRange[100] = "bytes ";
     char strtemp[15];
     sprintf(strtemp, "%d", byteoffset);
     strcat(contentRange,strtemp);
     strcat(contentRange,"-");
     sprintf(strtemp, "%d", (dataSize-1));
     strcat(contentRange,strtemp);
     strcat(contentRange,"/");
     sprintf(strtemp, "%d", dataSize);
     strcat(contentRange,strtemp);
     printf("\n Content-Range: %s",contentRange);
     
     cg_http_response_setcontentrange(httpRes, contentRange);
     cg_http_response_setcontentlength(httpRes, offsetbytesize);
     }
     else
     {
     char contentRange[100] = "bytes ";
     strcat(contentRange,"1-");
     char strtemp[15];
     sprintf(strtemp, "%d", (dataSize-1));
     strcat(contentRange,strtemp);
     strcat(contentRange,"/");
     sprintf(strtemp, "%d", dataSize);
     strcat(contentRange,strtemp);
     printf("\n Content-Range: %s",contentRange);
     
     cg_http_response_setcontentrange(httpRes, contentRange);
     cg_http_response_setcontentlength(httpRes, dataSize);
     }
     
     if (cg_http_request_isheadrequest(httpReq) == TRUE)
     {
     
     cg_http_response_setcontent(httpRes, NULL);
     }
     
     //        cg_http_response_setcontent(httpRes, theData);
     
     //cg_http_response_print(httpRes);
     
     
     //        cg_http_request_postresponse(httpReq, httpRes);
     //        cg_http_response_delete(httpRes);
     
     printf("\na<--STREAMING REQ:cg_http_request_postresponse_user =%d\n",cg_http_request_postresponse_user(httpReq, httpRes));  //UNCOMMENT
     CgSocket* sock = cg_http_request_getsocket(httpReq); //5884865
     //cg_http_request_postresponse(httpReq, httpRes);
     
     unsigned long nTotalSent = 0;
     unsigned long cmdPos = 0;
     unsigned long nSent = 0;
     
     while(1)
     {
     nSent = cg_socket_write(sock,theData + cmdPos,8192);
     
     
     nTotalSent += nSent;
     cmdPos += nSent;
     
     printf("\na<--nSent=%lu,nTotalSent=%lu\n",nSent,nTotalSent);
     
     if((nSent == 0 ) || (nTotalSent == dataSize))
     break;
     
     }
     //printf("\na<--STREAMING cg_socket_write =%zu,sock =%d,theData=%d,dataSize=%d\n",cg_socket_write(sock,theData,dataSize),sock,(int)theData,dataSize);
     //cg_socket_write(sock,theData,dataSize);
     //cg_socket_write(sock,theData,5184670);  //7984670
     free(theData);
     cg_http_response_delete(httpRes);
     
     return;
     }*/
}



static void cg_upnp_device_getrequestrecieved(CgUpnpDevice *dev, CgHttpRequest *httpReq)
{
    CgString *descStr;
    char *url;
    char *ifAddr;
    CgUpnpService *embService;
    CgUpnpDevice *embDev;
    CgHttpResponse *httpRes;
    //CgHttpResponse1 *httpRes1;
    
    cg_log_debug_l4("Entering...\n");
    
    url = cg_http_request_geturi(httpReq);
    if (cg_strlen(url) <= 0) {
        cg_http_request_postbadrequest(httpReq);
        return;
    }
    
    //cg_http_packet_getheadervalue();
    
    descStr = cg_string_new();
    ifAddr = cg_http_request_getlocaladdress(httpReq);
    
    
    
    
    if(strstr(url,"/akshayv") != NULL){
        removeSubstring(url,"/akshayv");  //strip off the /spurlclint
        printf("\n This is an spurlclint  %s",url);
        
        // char *urlhardcode = "/var/mobile/Applications/14D04BC8-7DE5-4A63-BC75-B1D52DE384C4/Documents/ARR.m4a";
        char *urlhardcode = "var/mobile/Applications/14D04BC8-7DE5-4A63-BC75-B1D52DE384C4/Documents/ARR Dheeme 1.mp3";
        CFURLRef urlAsset = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                                          CFStringCreateWithCString(kCFAllocatorMallocZone, url, kCFStringEncodingASCII),
                                                          kCFURLPOSIXPathStyle,
                                                          false);
        OSStatus openErr = noErr;
        AudioFileID audioFile = NULL;
        openErr = AudioFileOpenURL(urlAsset,
                                   kAudioFileReadPermission ,
                                   0,
                                   &audioFile);
        
        UInt64 fileDataSize = 0;
        AudioStreamBasicDescription theFileFormat;
        UInt32 thePropertySize = sizeof(theFileFormat);
        thePropertySize = sizeof(fileDataSize);
        
        
        openErr = AudioFileGetProperty(audioFile, kAudioFilePropertyAudioDataByteCount, &thePropertySize, &fileDataSize);
        
        
        UInt32 dataSize = fileDataSize;
        void* theData = malloc(dataSize);
        OSStatus openErr11;
        
        
        if (theData) {
            openErr11 = AudioFileReadBytes(audioFile, false, 0, &dataSize, theData);
        }
        
        printf("\n This is an sprul ");
        
        httpRes = cg_http_response_new();
        cg_http_response_setstatuscode(httpRes, CG_HTTP_STATUS_OK);
        cg_http_response_setcontenttype(httpRes, CG_AUDIO_CONTENT_TYPE_MP3);
        
        cg_http_response_setacceptranges(httpRes, "bytes");
        cg_http_response_settransfermode(httpRes, "Streaming");
        
        char *contentRange = "bytes 1-5884864/5884865";
        /*
         strcat(contentRange,"1-");
         // strcat(contentRange,itoa(dataSize-1));
         strcat(contentRange,"/");
         //  strcat(contentRange,itoa(dataSize));
         printf("\n Content-Range: %s",contentRange);
         */
        cg_http_response_setcontentrange(httpRes, contentRange);
        cg_http_response_setcontentlength(httpRes, dataSize);
        
        if (cg_http_request_isheadrequest(httpReq) == TRUE)
        {
            
            cg_http_response_setcontent(httpRes, NULL);
        }
        
        cg_http_response_print(httpRes);
        cg_http_request_postresponse_user(httpReq, httpRes);
        CgSocket* sock = cg_http_request_getsocket(httpReq); //5884865
        cg_socket_write(sock,theData,dataSize);
        //cg_socket_write(sock,theData,5184670);  //7984670
        cg_http_response_delete(httpRes);
        
        return;
    }
    
    cg_log_debug_s("Requested: |%s|, description: |%s|\n", url, cg_string_getvalue(dev->descriptionURI));
    if (cg_upnp_device_isdescriptionuri(dev, url) == TRUE) {
        
        cg_upnp_device_getdescription(dev, ifAddr, descStr);
    }
    else if ((embService = cg_upnp_device_getservicebyscpdurl(dev, url)) != NULL) {
        cg_upnp_service_getdescription(embService, descStr);
    }
    else if ((embDev = cg_upnp_device_getdevicebydescriptionuri(dev, url)) != NULL) {
        cg_upnp_device_getdescription(embDev, ifAddr, descStr);
        //} else if(strcmp(url,"/spurl.mp3") == 0){
    } else if(strstr(url,"/akshayv") == 0){
        printf("\n This is an spurlclint");
        
        CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                                     CFStringCreateWithCString(kCFAllocatorMallocZone, "var/mobile/Applications/1B5260F2-E9F4-4A28-B975-234A4A030A51/Documents/BNEJ.mp3", kCFStringEncodingASCII),
                                                     kCFURLPOSIXPathStyle,
                                                     false);
        OSStatus openErr = noErr;
        AudioFileID audioFile = NULL;
        openErr = AudioFileOpenURL(url,
                                   kAudioFileReadPermission ,
                                   0,
                                   &audioFile);
        
        UInt64 fileDataSize = 0;
        AudioStreamBasicDescription theFileFormat;
        UInt32 thePropertySize = sizeof(theFileFormat);
        thePropertySize = sizeof(fileDataSize);
        
        
        openErr = AudioFileGetProperty(audioFile, kAudioFilePropertyAudioDataByteCount, &thePropertySize, &fileDataSize);
        
        
        //Read data into buffer
        UInt32 dataSize = fileDataSize;
        //UInt32 dataSize = 53691465;
        void* theData = malloc(dataSize);
        OSStatus openErr11;
        
        
        if (theData) {
            openErr11 = AudioFileReadBytes(audioFile, false, 0, &dataSize, theData);
        }
        
        
        
        printf("\n This is an sprul ");
        
        httpRes = cg_http_response_new();
        cg_http_response_setstatuscode(httpRes, CG_HTTP_STATUS_OK);
        cg_http_response_setcontenttype(httpRes, CG_AUDIO_CONTENT_TYPE_MP3);
        
        
        if (cg_http_request_isheadrequest(httpReq) == TRUE)
        {
            
            cg_http_response_setcontent(httpRes, NULL);
        }
        
        cg_http_response_print(httpRes);
        
        cg_http_request_postresponse_user(httpReq, httpRes);
        CgSocket* sock = cg_http_request_getsocket(httpReq);
        cg_socket_write(sock,theData,dataSize);
        cg_http_response_delete(httpRes);
        
        return;
        
        
    }
    
    else
    {
        /* Here we should handle Not Found case */
        cg_http_request_poststatuscode(httpReq, CG_HTTP_STATUS_NOT_FOUND);
        cg_string_delete(descStr);
        return;
    }
    
    
    
    
    
    httpRes = cg_http_response_new();
    cg_http_response_setstatuscode(httpRes, CG_HTTP_STATUS_OK);
    cg_http_response_setcontenttype(httpRes, CG_XML_CONTENT_TYPE);
    cg_http_response_setcontent(httpRes, cg_string_getvalue(descStr));
    //printf("\n Content %d",descStr);
    cg_http_response_setcontentlength(httpRes, cg_string_length(descStr));
    
    if (cg_http_request_isheadrequest(httpReq) == TRUE)
    {
        /* If the request is head request, then clear message body */
        cg_http_response_setcontent(httpRes, NULL);
    }
    
    cg_http_response_print(httpRes);
    
    cg_http_request_postresponse(httpReq, httpRes);
    cg_http_response_delete(httpRes);
    
    cg_string_delete(descStr);	
    
    cg_log_debug_l4("Leaving...\n");
}


static void cg_upnp_device_postrequestrecieved(CgUpnpDevice *dev, CgHttpRequest *httpReq)
{
	CgSoapRequest *soapReq;
	cg_log_debug_l4("Entering...\n");

	if (cg_http_request_issoapaction(httpReq) == TRUE) {
        soapReq = cg_soap_request_new();
		cg_soap_request_sethttprequest(soapReq, httpReq);
		cg_upnp_device_soapactionrecieved(dev, soapReq);
		cg_soap_request_delete(soapReq);
		return;
	}
	cg_http_request_postbadrequest(httpReq);

	cg_log_debug_l4("Leaving...\n");
}


/****************************************
* 
* SOAP REQUEST
*
****************************************/

static void cg_upnp_device_badsoapactionrecieved(CgHttpRequest *httpReq)
{
	cg_log_debug_l4("Entering...\n");

	cg_http_request_postbadrequest(httpReq);

	cg_log_debug_l4("Leaving...\n");
}

static void cg_upnp_device_invalidcontrolrecieved(CgSoapRequest *soapReq, int code)
{
	CgHttpRequest *httpReq;
	CgSoapResponse *soapRes;
	CgHttpResponse *httpRes;

	cg_log_debug_l4("Entering...\n");

	httpReq = cg_soap_request_gethttprequest(soapReq);
	
	soapRes = cg_soap_response_new();
	cg_upnp_control_soap_response_setfaultresponse(soapRes, code, cg_upnp_status_code2string(code));
	httpRes = cg_soap_response_gethttpresponse(soapRes);
	cg_http_request_postresponse(httpReq, httpRes);
	cg_soap_response_delete(soapRes);

	cg_log_debug_l4("Leaving...\n");
}

static void cg_upnp_device_soapactionrecieved(CgUpnpDevice *dev, CgSoapRequest *soapReq)
{
	CgHttpRequest *httpReq;
	CgUpnpService *ctrlService;
	char *url;
	
	cg_log_debug_l4("Entering...\n");

	httpReq = cg_soap_request_gethttprequest(soapReq);
	
	url = cg_http_request_geturi(httpReq);
	cg_log_debug_s("Control url in device: %s\n", url);
	ctrlService = cg_upnp_device_getservicebycontrolurl(dev, url);
	
	if (ctrlService != NULL) {
		cg_upnp_device_controlrequestrecieved(ctrlService, soapReq);
		return;
	}
	
	cg_upnp_device_badsoapactionrecieved(httpReq);

	cg_log_debug_l4("Leaving...\n");
}

static void cg_upnp_device_controlrequestrecieved(CgUpnpService *service, CgSoapRequest *soapReq)
{
	CgHttpRequest *httpReq;
	CgUpnpActionRequest *actionReq;
	CgUpnpQueryRequest *queryReq;
	
	cg_log_debug_l4("Entering...\n");

	httpReq = cg_soap_request_gethttprequest(soapReq);
	
#if !defined(CG_UPNP_NOUSE_QUERYCTRL)
	if (cg_upnp_control_isqueryrequest(soapReq) == TRUE) {
		queryReq = cg_upnp_control_query_request_new();
		cg_upnp_control_query_request_setsoaprequest(queryReq, soapReq);
		cg_upnp_device_querycontrolrequestrecieved(service, queryReq);
		cg_upnp_control_query_request_delete(queryReq);
		return;
	}
#endif
	
#if !defined(CG_UPNP_NOUSE_ACTIONCTRL)
	if (cg_upnp_control_isactionrequest(soapReq) == TRUE) {
        actionReq = cg_upnp_control_action_request_new();
		cg_upnp_control_action_request_setsoaprequest(actionReq, soapReq);
		cg_upnp_device_actioncontrolrequestrecieved(service, actionReq);
		cg_upnp_control_action_request_delete(actionReq);
		return;
	}
#endif
	
	cg_upnp_device_badsoapactionrecieved(httpReq);

	cg_log_debug_l4("Leaving...\n");
}

/****************************************
* 
* SOAP REQUEST (Action)
*
****************************************/

#if !defined(CG_UPNP_NOUSE_ACTIONCTRL)

#define cg_upnp_device_invalidactioncontrolrecieved(actionReq) cg_upnp_device_invalidcontrolrecieved(cg_upnp_control_action_request_getsoaprequest(actionReq), CG_UPNP_STATUS_INVALID_ACTION)

static void cg_upnp_device_actioncontrolrequestrecieved(CgUpnpService *service, CgUpnpActionRequest *actionReq)
{
	char *actionName;
	CgUpnpAction *action;
	CgUpnpArgumentList *actionArgList;
	CgUpnpArgumentList *actionReqArgList;
	
	cg_log_debug_l4("Entering...\n");

	actionName = cg_upnp_control_action_request_getactionname(actionReq);
    action = cg_upnp_service_getactionbyname(service, actionName);
	if (action == NULL) {
		cg_upnp_device_invalidactioncontrolrecieved(actionReq);
		return;
	}
	
	actionArgList = cg_upnp_action_getargumentlist(action);
	actionReqArgList = cg_upnp_control_action_request_getargumentlist(actionReq);
	cg_upnp_argumentlist_set(actionArgList, actionReqArgList);
	if (cg_upnp_action_performlistner(action, actionReq) == FALSE)
		cg_upnp_device_invalidactioncontrolrecieved(actionReq);

	cg_log_debug_l4("Leaving...\n");
}

#endif

/****************************************
* 
* SOAP REQUEST (Query)
*
****************************************/

#if !defined(CG_UPNP_NOUSE_QUERYCTRL)

#define cg_upnp_device_invalidquerycontrolrecieved(queryReq) cg_upnp_device_invalidcontrolrecieved(cg_upnp_control_query_request_getsoaprequest(queryReq), CG_UPNP_STATUS_INVALID_VAR)

static void cg_upnp_device_querycontrolrequestrecieved(CgUpnpService *service, CgUpnpQueryRequest *queryReq)
{
	char *varName;
	CgUpnpStateVariable *stateVar;
	
	cg_log_debug_l4("Entering...\n");

	varName = cg_upnp_control_query_request_getvarname(queryReq);
	if (cg_upnp_service_hasstatevariablebyname(service, varName) == FALSE) {
		cg_upnp_device_invalidquerycontrolrecieved(queryReq);
		return;
	}

	stateVar = cg_upnp_service_getstatevariablebyname(service, varName);
	if (cg_upnp_statevariable_performlistner(stateVar, queryReq) == FALSE)
		cg_upnp_device_invalidquerycontrolrecieved(queryReq);

	cg_log_debug_l4("Leaving...\n");
}

#endif

/****************************************
* 
* SOAP REQUEST (SubScribe)
*
****************************************/

#if !defined(CG_UPNP_NOUSE_SUBSCRIPTION)

static void cg_upnp_device_badsubscriptionrecieved(CgUpnpSubscriptionRequest *subReq, int code)
{
	CgUpnpSubscriptionResponse *subRes;
	
	cg_log_debug_l4("Entering...\n");

	subRes = cg_upnp_event_subscription_response_new();
	cg_upnp_event_subscription_response_setstatuscode(subRes, code);
	cg_upnp_event_subscription_request_postresponse(subReq, subRes);
	cg_upnp_event_subscription_response_delete(subRes);

	cg_log_debug_l4("Leaving...\n");
}

static void cg_upnp_device_subscriptionrecieved(CgUpnpDevice *dev, CgUpnpSubscriptionRequest *subReq)
{
	char *uri;
	CgUpnpService *service;
	
	cg_log_debug_l4("Entering...\n");

	uri = cg_http_request_geturi(subReq);
	if (cg_strlen(uri) <= 0) {
		cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_PRECONDITION_FAILED);
		return;
	}
	service = cg_upnp_device_getservicebyeventsuburl(dev, uri);
	if (service == NULL) {
		cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_PRECONDITION_FAILED);
		return;
	}
		
	if (cg_upnp_event_subscription_request_hascallback(subReq) == FALSE && cg_upnp_event_subscription_request_hassid(subReq) == FALSE) {
		cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_PRECONDITION_FAILED);
		return;
	}

        if (cg_upnp_event_subscription_request_hascallback(subReq) &&
            cg_strlen(cg_upnp_event_subscription_request_getcallback(subReq)) <= 0) {
		cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_PRECONDITION_FAILED);
		return;
	}

        if (cg_upnp_event_subscription_request_hassid(subReq) && 
            (cg_upnp_event_subscription_request_hascallback(subReq) ||
             cg_upnp_event_subscription_request_hasnt(subReq))) {
                cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_BAD_REQUEST);
		return;
        }

	if (cg_upnp_event_subscription_request_hasnt(subReq) &&
            (cg_strcmp(cg_upnp_event_subscription_request_getnt(subReq), CG_UPNP_NT_EVENT) != 0)) {
                cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_PRECONDITION_FAILED);
		return;
        }

	/**** UNSUBSCRIBE ****/
	if (cg_upnp_event_subscription_isunsubscriberequest(subReq) == TRUE) {
		cg_upnp_device_unsubscriptionrecieved(service, subReq);
		return;
	}

	/**** SUBSCRIBE (NEW) ****/
	if (cg_upnp_event_subscription_request_hascallback(subReq) == TRUE) {
		cg_upnp_device_newsubscriptionrecieved(service, subReq);
		return;
	}
		
	/**** SUBSCRIBE (RENEW) ****/
	if (cg_upnp_event_subscription_request_hassid(subReq) == TRUE) {
		cg_upnp_device_renewsubscriptionrecieved(service, subReq);
		return;
	}

	cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_PRECONDITION_FAILED);

	cg_log_debug_l4("Leaving...\n");
}

static void cg_upnp_device_newsubscriptionrecieved(CgUpnpService *service, CgUpnpSubscriptionRequest *subReq)
{
	char *callback;
	char *aux;
	CgTime timeout;
	char sid[CG_UPNP_SUBSCRIPTION_SID_SIZE];
	CgUpnpSubscriber *sub;
	CgUpnpSubscriptionResponse *subRes;
	
	cg_log_debug_l4("Entering...\n");

	aux = cg_strdup(cg_upnp_event_subscription_request_getcallback(subReq));
	if (aux == NULL)
		return;

	if(cg_strstr(aux,"http") == -1 )
	{
	  cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_PRECONDITION_FAILED);
	  SAFE_FREE(aux);
	  return;
	}
	callback = cg_strltrim(aux, CG_UPNP_SUBSCRIPTION_CALLBACK_START_WITH, 1);
	cg_strrtrim(callback, CG_UPNP_SUBSCRIPTION_CALLBACK_END_WITH, 1);
	
	timeout = cg_upnp_event_subscription_request_gettimeout(subReq);
	/* Limit timeout to the given maximum */
	if (CG_UPNP_SUBSCRIPTION_MAX_TIMEOUT > 0)
	{
		if (timeout > CG_UPNP_SUBSCRIPTION_MAX_TIMEOUT ||
		    timeout == CG_UPNP_SUBSCRIPTION_INFINITE_VALUE)
		{
			timeout = CG_UPNP_SUBSCRIPTION_MAX_TIMEOUT;
		}
	}
	
	cg_upnp_event_subscription_createsid(sid, sizeof(sid));

	sub = cg_upnp_subscriber_new();
	cg_upnp_subscriber_setdeliveryurl(sub, callback);
	SAFE_FREE(aux);
	cg_upnp_subscriber_settimeout(sub, timeout);
	cg_upnp_subscriber_setsid(sub, sid);
	cg_upnp_service_addsubscriber(service, sub);

	subRes = cg_upnp_event_subscription_response_new();
	cg_upnp_event_subscription_subscriberesponse_setresponse(subRes, CG_HTTP_STATUS_OK);
	cg_upnp_event_subscription_response_setsid(subRes, sid);
	cg_upnp_event_subscription_response_settimeout(subRes, timeout);
	cg_upnp_event_subscription_request_postresponse(subReq, subRes);
	cg_upnp_event_subscription_response_delete(subRes);

	cg_upnp_service_createnotifyallthread(service, CG_UPNP_SERVICE_NOTIFY_WAITTIME);
       
     //ALOGE ("-----subscriber func %u ---- \n",&subscribeListener);

	cg_log_debug_l4("Leaving...\n");
}

static void cg_upnp_device_renewsubscriptionrecieved(CgUpnpService *service, CgUpnpSubscriptionRequest *subReq)
{
	CgUpnpSubscriber *sub;
	const char *sid;
	long timeout;
	CgUpnpSubscriptionResponse *subRes;
	
	cg_log_debug_l4("Entering...\n");

	sid = cg_upnp_event_subscription_request_getsid(subReq);
	sub = cg_upnp_service_getsubscriberbysid(service, sid);

	if (sub == NULL) {
		cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_PRECONDITION_FAILED);
		return;
	}

	if(cg_upnp_subscriber_isexpired(sub) == TRUE)
	{
	    cg_upnp_service_removesubscriber(service, sub);
	   	cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_PRECONDITION_FAILED);
		return;
	}
	
	timeout = cg_upnp_event_subscription_request_gettimeout(subReq);
	/* Limit timeout to the given maximum */
	if (CG_UPNP_SUBSCRIPTION_MAX_TIMEOUT > 0)
	{
		if (timeout > CG_UPNP_SUBSCRIPTION_MAX_TIMEOUT ||
		    timeout == CG_UPNP_SUBSCRIPTION_INFINITE_VALUE)
		{
			timeout = CG_UPNP_SUBSCRIPTION_MAX_TIMEOUT;
		}
	}
	
	cg_upnp_subscriber_settimeout(sub, timeout);
	cg_upnp_subscriber_renew(sub);

	subRes = cg_upnp_event_subscription_response_new();
	cg_upnp_event_subscription_subscriberesponse_setresponse(subRes, CG_HTTP_STATUS_OK);
	cg_upnp_event_subscription_response_setsid(subRes, sid);
	cg_upnp_event_subscription_response_settimeout(subRes, timeout);
	cg_upnp_event_subscription_request_postresponse(subReq, subRes);
	cg_upnp_event_subscription_response_delete(subRes);

	cg_log_debug_l4("Leaving...\n");
}		

static void cg_upnp_device_unsubscriptionrecieved(CgUpnpService *service, CgUpnpSubscriptionRequest *subReq)
{
    CgUpnpSubscriber *sub;
    const char *sid;
    CgUpnpSubscriptionResponse *subRes;
    
    cg_log_debug_l4("Entering...\n");
    
    sid = cg_upnp_event_subscription_request_getsid(subReq);
    cg_upnp_service_lock(service);
    sub = cg_upnp_service_getsubscriberbysid(service, sid);
    
    if (sub == NULL) {
        cg_upnp_device_badsubscriptionrecieved(subReq, CG_HTTP_STATUS_PRECONDITION_FAILED);
        cg_upnp_service_unlock(service);
        return;
    }
    
    cg_upnp_service_removesubscriber(service, sub);
    cg_upnp_service_unlock(service);
    
    subRes = cg_upnp_event_subscription_response_new();
    cg_upnp_event_subscription_subscriberesponse_setresponse(subRes, CG_HTTP_STATUS_OK);
    cg_upnp_event_subscription_request_postresponse(subReq, subRes);
    cg_upnp_event_subscription_response_delete(subRes);
    
    cg_log_debug_l4("Leaving...\n");
}

#endif
