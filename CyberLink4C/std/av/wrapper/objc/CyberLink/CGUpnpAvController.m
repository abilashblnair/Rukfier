//
//  CGUpnpAvController.m
//  CyberLink for C
//
//  Created by Satoshi Konno on 08/06/25.
//  Copyright 2008 Satoshi Konno. All rights reserved.
//

#include <cybergarage/upnp/ssdp/cssdp.h>
#include <cybergarage/upnp/std/av/cmediaserver.h>

#import "CGUpnpAvController.h"
#import "CGUpnpAvServer.h"
#import "CGUpnpAvConstants.h"
#import "CGXml.h"
#import "CGXmlNode.h"
#import "CGUpnpAvObject.h"
#import "CGUpnpAvContainer.h"
#import "CGUpnpAvRenderer.h"


#define CG_UPNPAV_DMR_DIDL_NAME "DIDL-Lite"
#define CG_UPNPAV_DMR_DIDL_XMLNS "xmlns"
#define CG_UPNPAV_DMR_DIDL_XMLNS_URL "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
#define CG_UPNPAV_DMR_DIDL_XMLNS_UPNP "xmlns:upnp"
#define CG_UPNPAV_DMR_DIDL_XMLNS_UPNP_URL "urn:schemas-upnp-org:metadata-1-0/upnp/"
#define CG_UPNPAV_DMR_DIDL_XMLNS_DC "xmlns:dc"
#define CG_UPNPAV_DMR_DIDL_XMLNS_DC_URL "http://purl.org/dc/elements/1.1/"
#define CG_UPNPAV_DMR_DIDL_XMLNS_DLNA "xmlns:dlna"
#define CG_UPNPAV_DMR_DIDL_XMLNS_DLNA_URL "urn:schemas-dlna-org:metadata-1-0/"
#define CG_UPNPAV_DMR_DIDL_XMLNS_SEC "xmlns:sec"
#define CG_UPNPAV_DMR_DIDL_XMLNS_SEC_URL "http://www.sec.co.kr/"
#define CG_UPNPAV_DMR_AUDIO_UPNP_CLASS "object.item.audioItem.musicTrack"

@implementation CGUpnpAvController

- (id)init
{
	if ((self = [super init]) == nil)
		return nil;
	
	[self setSsdpSearchMX:1];
	
	return self;
}

- (void)dealloc
{
	[super dealloc];
}


////////////////////////////////////////////////////////////
// Media Server
////////////////////////////////////////////////////////////

- (NSArray *)servers;
{
	NSArray *devices = [self devices];
	NSMutableArray *serverArray = [[[NSMutableArray alloc] init] autorelease];

	for (CGUpnpDevice *dev in devices) {
		if (![dev isDeviceType:@CG_UPNPAV_DMS_DEVICE_TYPE])
			continue;
		CGUpnpAvServer *server = nil;		
		void *devData = [dev userData];
		if (!devData) {
			CgUpnpDevice *cDevice = [dev cObject];
			if (!cDevice)
				continue;
			server = [[[CGUpnpAvServer alloc] initWithCObject:cDevice] autorelease];
			[server setUserObject:server];
		}
		else 
			server = (CGUpnpAvServer *)((id)devData);
		if (server == nil)
			continue;

//        if ([[server friendlyName] isEqualToString:@"Cyber Garage Media Server (MediaServer) : 1"])
//			continue; //Dont add Cyber garage local server into the list
        
		[serverArray addObject:server];
	}
	return serverArray;
}

- (CGUpnpAvServer *)serverForUDN:(NSString *)aUdn
{
	if (aUdn == nil)
		return nil;
	NSArray *servers = [self servers];
	for (CGUpnpAvServer *server in servers) {
		if ([server isUDN:aUdn])
			return [[server retain] autorelease];
	}
	return nil;
}

- (CGUpnpAvServer *)serverForFriendlyName:(NSString *)aFriendlyName
{
	if (aFriendlyName == nil)
		return nil;
	NSArray *servers = [self servers];
	for (CGUpnpAvServer *server in servers) {
		if ([server isFriendlyName:aFriendlyName])
			return [[server retain] autorelease];
	}
	return nil;
}

- (CGUpnpAvServer *)serverForPath:(NSString *)aPath;
{
	NSArray *srvAndObjPathArray = [aPath pathComponents];
	if ([srvAndObjPathArray count] <= 0)
		return nil;
	if ([aPath isAbsolutePath] && ([srvAndObjPathArray count] <= 1))
		return nil;
	NSString *avSrvName = [CGXml unescapestring:[aPath isAbsolutePath] ? [srvAndObjPathArray objectAtIndex:1] : [srvAndObjPathArray objectAtIndex:0]];
	return [self serverForFriendlyName:avSrvName];
}

- (CGUpnpAvServer *)serverForIndexPath:(NSIndexPath *)aIndexPath
{
	NSUInteger idxCnt = [aIndexPath length];
	if (idxCnt < 1)
		return nil;
	
	NSArray *servers = [self servers];
	NSUInteger serverNum = [aIndexPath indexAtPosition:0];
	if ([servers count] <= serverNum)
		return nil;
	
	return [servers objectAtIndex:serverNum];
}

- (CGUpnpAvObject *)objectForTitlePath:(NSString *)aServerAndTitlePath
{
	CGUpnpAvServer *avSrv = [self serverForPath:aServerAndTitlePath];
	if (avSrv == nil)
		return nil;

	NSMutableArray *titlePathArray = [NSMutableArray arrayWithArray:[aServerAndTitlePath pathComponents]];
	[titlePathArray removeObjectAtIndex:0];
	if (aServerAndTitlePath.isAbsolutePath)
		[titlePathArray removeObjectAtIndex:0];

	NSString *titlePath = [NSString pathWithComponents:titlePathArray];
	CGUpnpAvObject *avObj = [avSrv objectForTitlePath:titlePath];
	
	return [[avObj retain] autorelease];
}

- (CGUpnpAvObject *)objectForIndexPath:(NSIndexPath *)aServerAndTitleIndexPath
{
	CGUpnpAvServer *avSrv = [self serverForIndexPath:aServerAndTitleIndexPath];
	if (!avSrv)
		return nil;
	
	CGUpnpAvContainer*rootObj = [avSrv rootObject];
	if (!rootObj)
		return nil;

	CGUpnpAvObject *avObj = rootObj;
	NSUInteger idxPathLength = [aServerAndTitleIndexPath length];
	NSUInteger n;
	for (n=1; n<idxPathLength; n++) {
		if ([avObj isItem])
			return nil;
		CGUpnpAvContainer *avCon = (CGUpnpAvContainer *)avObj;
		NSUInteger avObjIdx = [aServerAndTitleIndexPath indexAtPosition:n];
		avObj = [avCon childAtIndex:avObjIdx];
		NSLog(@"objectForIndexPath = [%tu/%tu][%tu/%tu] %@ (%@)", n, idxPathLength, avObjIdx,  [avCon childCount], [avObj title], [avObj objectId]);
		if (!avObj)
			return nil;
	}
	
	
	return [[avObj retain] autorelease];
}

- (NSArray *)browseDirectChildrenWithTitlePath:(NSString *)aServerAndTitlePath
{
	CGUpnpAvServer *avSrv = [self serverForPath:aServerAndTitlePath];
	if (avSrv == nil)
		return nil;
	
	CGUpnpAvObject *avObj = [self objectForTitlePath:aServerAndTitlePath];
	if (avObj == nil)
		return nil;
	
	return [avSrv browseDirectChildren:[avObj objectId]];
}

- (NSArray *)browseDirectChildrenWithIndexPath:(NSIndexPath *)aServerAndTitleIndexPath
{
	CGUpnpAvServer *avSrv = [self serverForIndexPath:aServerAndTitleIndexPath];
	if (avSrv == nil)
		return nil;
	
	CGUpnpAvObject *avObj = [self objectForIndexPath:aServerAndTitleIndexPath];
	if (avObj == nil)
		return nil;

	return [avSrv browseDirectChildren:[avObj objectId]];
}

////////////////////////////////////////////////////////////
// Media Renderer
////////////////////////////////////////////////////////////

- (NSArray *)renderers;
{
	NSArray *devices = [self devices];
	NSMutableArray *rendererrArray = [[[NSMutableArray alloc] init] autorelease];
	
	for (CGUpnpDevice *dev in devices) {
		if (![dev isDeviceType:@CG_UPNPAV_DMR_DEVICE_TYPE])
			continue;
		CgUpnpDevice *cDevice = [dev cObject];
		if (!cDevice)
			continue;
		CGUpnpAvRenderer *renderer = [[[CGUpnpAvRenderer alloc] initWithCObject:cDevice] autorelease];
		if (renderer == nil)
			continue;
		[rendererrArray addObject:renderer];
	}
	return rendererrArray;
}

- (CGUpnpAvRenderer *)rendererForUDN:(NSString *)aUdn
{
	if (aUdn == nil)
		return nil;
	NSArray *renderers = [self renderers];
	for (CGUpnpAvRenderer *renderer in renderers) {
		if ([renderer isUDN:aUdn])
			return [[renderer retain] autorelease];
	}
	return nil;
}

////////////////////////////////////////////////////////////
// Search
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Search
////////////////////////////////////////////////////////////

- (void)search
{
    [super search];
    /*#if defined(CG_UPNPAVCONTROLLER_SERCH_DEVICETYPES)
     [super searchWithST:[NSString stringWithUTF8String:CG_UPNPAV_DMS_DEVICE_TYPE]];
     [super searchWithST:[NSString stringWithUTF8String:CG_UPNPAV_DMR_DEVICE_TYPE]];
     #endif*/
}

- (void)searchRenderers
{
    [super searchRenderers];
}

- (void)searchServers
{
    [super searchServers];
}

///// previous code /////

//- (void)search
//{
//	[super search];
//#if defined(CG_UPNPAVCONTROLLER_SERCH_DEVICETYPES)
//	[super searchWithST:[NSString stringWithUTF8String:CG_UPNPAV_DMS_DEVICE_TYPE]];
//	[super searchWithST:[NSString stringWithUTF8String:CG_UPNPAV_DMR_DEVICE_TYPE]];
//#endif
//}
//
//- (void)mSearch
//{
//#if defined(CG_UPNPAVCONTROLLER_SERCH_DEVICETYPES)
//    [super searchWithST:[NSString stringWithUTF8String:CG_UPNPAV_DMR_DEVICE_TYPE]];
//#endif
//
//}

////// previous code ////

/*- (NSString *) getDIDLXml4AVT:(NSString *)songtilestr upnpclass:(NSString *)upnpclassstr format:(NSString *)formatstr size:(NSString*)sizestr duration:(NSString*)durationstr url:(NSString*)urlstr
{
    CgXmlNode *node;
    CgXmlNode *childnode;
 	CgXmlNode *leafnode;
	
	node = cg_xml_node_new();
    cg_xml_node_setname(node, CG_UPNPAV_DMR_DIDL_NAME);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS, CG_UPNPAV_DMR_DIDL_XMLNS_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_UPNP, CG_UPNPAV_DMR_DIDL_XMLNS_UPNP_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_DC, CG_UPNPAV_DMR_DIDL_XMLNS_DC_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_DLNA, CG_UPNPAV_DMR_DIDL_XMLNS_DLNA_URL);
    cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_SEC, CG_UPNPAV_DMR_DIDL_XMLNS_SEC_URL);
    
    childnode = cg_xml_node_new();
	cg_xml_node_setname(childnode, "item");
	cg_xml_node_setattribute(childnode, "id", "4");
    cg_xml_node_setattribute(childnode, "parentID", "1");
    cg_xml_node_setattribute(childnode, "restricted", "1");
    cg_xml_node_addchildnode(node, childnode);
    
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "upnp:class");
    cg_xml_node_setvalue(leafnode, CG_UPNPAV_DMR_AUDIO_UPNP_CLASS);
    cg_xml_node_addchildnode(childnode, leafnode);
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "dc:title");
    cg_xml_node_setvalue(leafnode, [songtilestr UTF8String]);
    cg_xml_node_addchildnode(childnode, leafnode);
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "res");
    cg_xml_node_setvalue(leafnode, [urlstr UTF8String]);
    
    if ([formatstr rangeOfString:@"DLNA.ORG_OP=" options:NSCaseInsensitiveSearch].location == NSNotFound)
    {
        NSString *test = [formatstr stringByAppendingString:@";DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01500000000000000000000000000000"];
        //formatstr append "DLNA.ORG_PN=MP3;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01500000000000000000000000000000"
        NSLog(@"string does not contain DLNA.ORG_OP");
        cg_xml_node_setattribute(leafnode, "protocolInfo",[test UTF8String]);
        
    }
    else
    {
        cg_xml_node_setattribute(leafnode, "protocolInfo",[formatstr UTF8String]);
        NSLog(@"string contains DLNA.ORG_OP flag!");
    }
    
    //	cg_xml_node_setattribute(leafnode, "protocolInfo",[formatstr UTF8String]);
    cg_xml_node_setattribute(leafnode, "size", [sizestr UTF8String]);
    cg_xml_node_setattribute(leafnode, "duration", [durationstr UTF8String]);
    cg_xml_node_addchildnode(childnode, leafnode);
    
	CgString *str = cg_string_new();
	cg_xml_node_tostring(node, TRUE, str);
	printf("a<---DIDLMetadata %s", cg_string_getvalue(str));
    
    return[[NSString alloc] initWithCString:cg_string_getvalue(str) encoding:NSUTF8StringEncoding];
    //return (NSString*)cg_string_getvalue(str);
	//cg_string_delete(str);
}*/

- (NSString *) getDIDLXml4SongTitle:(NSString *)songtilestr Artist:(NSString *)artiststr Album:(NSString*)albumstr AlbumartUrl:(NSString*)albArturlStr upnpclass:(NSString *)upnpclassstr format:(NSString *)formatstr size:(NSString*)sizestr duration:(NSString*)durationstr url:(NSString*)urlstr
{
    CgXmlNode *node;
    CgXmlNode *childnode;
    CgXmlNode *leafnode;
    
    node = cg_xml_node_new();
    cg_xml_node_setname(node, CG_UPNPAV_DMR_DIDL_NAME);
    cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS, CG_UPNPAV_DMR_DIDL_XMLNS_URL);
    cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_UPNP, CG_UPNPAV_DMR_DIDL_XMLNS_UPNP_URL);
    cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_DC, CG_UPNPAV_DMR_DIDL_XMLNS_DC_URL);
    cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_DLNA, CG_UPNPAV_DMR_DIDL_XMLNS_DLNA_URL);
    cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_SEC, CG_UPNPAV_DMR_DIDL_XMLNS_SEC_URL);
    
    childnode = cg_xml_node_new();
    cg_xml_node_setname(childnode, "item");
    cg_xml_node_setattribute(childnode, "id", "4");
    cg_xml_node_setattribute(childnode, "parentID", "1");
    cg_xml_node_setattribute(childnode, "restricted", "1");
    cg_xml_node_addchildnode(node, childnode);
    
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "upnp:class");
    cg_xml_node_setvalue(leafnode, CG_UPNPAV_DMR_AUDIO_UPNP_CLASS);
    cg_xml_node_addchildnode(childnode, leafnode);
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "dc:title");
    cg_xml_node_setvalue(leafnode, [songtilestr UTF8String]);
    cg_xml_node_addchildnode(childnode, leafnode);
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "upnp:artist");
    cg_xml_node_setvalue(leafnode, [artiststr UTF8String]);
    cg_xml_node_addchildnode(childnode, leafnode);
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "upnp:album");
    cg_xml_node_setvalue(leafnode, [albumstr UTF8String]);
    cg_xml_node_addchildnode(childnode, leafnode);
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "upnp:albumArtURI");
    cg_xml_node_setvalue(leafnode, [albArturlStr UTF8String]);
    cg_xml_node_addchildnode(childnode, leafnode);
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "res");
    cg_xml_node_setvalue(leafnode, [urlstr UTF8String]);
    
    if ([formatstr rangeOfString:@"DLNA.ORG_OP=" options:NSCaseInsensitiveSearch].location == NSNotFound)
    {
        NSString *test = [formatstr stringByAppendingString:@";DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01500000000000000000000000000000"];
        //formatstr append "DLNA.ORG_PN=MP3;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01500000000000000000000000000000"
        NSLog(@"string does not contain DLNA.ORG_OP");
        cg_xml_node_setattribute(leafnode, "protocolInfo",[test UTF8String]);
        
    }
    else
    {
        cg_xml_node_setattribute(leafnode, "protocolInfo",[formatstr UTF8String]);
        NSLog(@"string contains DLNA.ORG_OP flag!");
    }
    
    //	cg_xml_node_setattribute(leafnode, "protocolInfo",[formatstr UTF8String]);
    cg_xml_node_setattribute(leafnode, "size", [sizestr UTF8String]);
    cg_xml_node_setattribute(leafnode, "duration", [durationstr UTF8String]);
    cg_xml_node_addchildnode(childnode, leafnode);
    
    CgString *str = cg_string_new();
    cg_xml_node_tostring(node, TRUE, str);
    printf("a<---DIDLMetadata %s", cg_string_getvalue(str));
    
    return[[NSString alloc] initWithCString:cg_string_getvalue(str) encoding:NSUTF8StringEncoding];
    //return (NSString*)cg_string_getvalue(str);
    //cg_string_delete(str);
}


- (CGUpnpAvResource *)getresourceNode:(NSString *)xmlstr
{
    char *resultXml;
	CgXmlParser *xmlParser;
	CgXmlNodeList *rootNode;
	CgXmlNode *didlNode;
	CgXmlNode *resNode;
	CgXmlNode *cnode;
	CgXmlNode *rnode;
    CGUpnpAvResource *avRes;
	
	resultXml = (char *)[xmlstr UTF8String];
	if (cg_strlen(resultXml) <= 0)
		return nil;
	
	rootNode = cg_xml_nodelist_new();
	xmlParser = cg_xml_parser_new();
    NSString *url;
	if (cg_xml_parse(xmlParser, rootNode, resultXml, cg_strlen(resultXml))) {
		didlNode = cg_xml_nodelist_getbyname(rootNode, "DIDL-Lite");
		if (didlNode) {
 
            for (cnode=cg_xml_node_getchildnodes(didlNode); cnode; cnode=cg_xml_node_next(cnode)) {
                if (cg_xml_node_isname(cnode, "item")) {
					CGUpnpAvItem *avItem = [[CGUpnpAvItem alloc] initWithXMLNode:cnode];
					for (rnode=cg_xml_node_getchildnodes(cnode); rnode; rnode=cg_xml_node_next(rnode)) {
						if (cg_xml_node_isname(rnode, "res")) {
							avRes = [[CGUpnpAvResource alloc] initWithXMLNode:rnode];
                            return avRes;
                             //url = [avRes url];
						}
					}
				}
            }
            
        //return url;

    }
}
}




//- (NSString *)getattribute:(NSString *)xmlstring: attributename:(NSString *) attname
//{
//    char *resultXml;
//	CgXmlParser *xmlParser;
//	CgXmlNodeList *rootNode;
//	CgXmlNode *didlNode;
//	CgXmlNode *resNode;
//	CgXmlNode *cnode;
//	CgXmlNode *rnode;
//    
//    CgXmlAttribute *attr;
//    char *name;
//	
//	resultXml = (char *)[xmlstring UTF8String];
//	if (cg_strlen(resultXml) <= 0)
//		return nil;
//	
//	NSMutableArray *avObjArray = [NSMutableArray array];
//	
//	rootNode = cg_xml_nodelist_new();
//	xmlParser = cg_xml_parser_new();
//    
//	if (cg_xml_parse(xmlParser, rootNode, resultXml, cg_strlen(resultXml))) {
//		resNode = cg_xml_nodelist_getbyname(rootNode, "res");
//		if (resNode) {
//            attr = cg_xml_node_getattribute(resNode,"duration");
//            name = cg_strdup( cg_xml_attribute_getname(attr) );
//        }
//        
//    return[[NSString alloc] initWithCString:name encoding:NSUTF8StringEncoding];
//    
//
//}
//}

@end

