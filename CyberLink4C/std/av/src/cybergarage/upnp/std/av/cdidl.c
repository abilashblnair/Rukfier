/************************************************************
*
*	CyberLink for C
*
*	Copyright (C) Satoshi Konno 2005
*
*	File: cdidl.c
*
*	Revision:
*       05/11/05
*               - first release.
*
************************************************************/

#include <cybergarage/upnp/std/av/cdidl.h>

/****************************************
* Constants
****************************************/

#define CG_UPNPAV_DMS_DIDL_NAME "DIDL-Lite"
#define CG_UPNPAV_DMS_DIDL_XMLNS "xmlns"
#define CG_UPNPAV_DMS_DIDL_XMLNS_URL "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/"
#define CG_UPNPAV_DMS_DIDL_XMLNS_DC "xmlns:dc"
#define CG_UPNPAV_DMS_DIDL_XMLNS_DC_URL "http://purl.org/dc/elements/1.1/"
#define CG_UPNPAV_DMS_DIDL_XMLNS_UPNP "xmlns:upnp"
#define CG_UPNPAV_DMS_DIDL_XMLNS_UPNP_URL "urn:schemas-upnp-org:metadata-1-0/upnp/"
#define CG_UPNPAV_DMS_DIDL_XMLNS_DLNA "xmlns:dlna"
#define CG_UPNPAV_DMS_DIDL_XMLNS_DLNA_URL "urn:schemas-dlna-org:metadata-1-0/"

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

/****************************************
* cg_upnpav_didl_node_new
****************************************/

CgXmlNode *cg_upnpav_didl_node_new()
{
	CgXmlNode *node;
	
	node = cg_xml_node_new();
/*
	cg_xml_node_setname(node, CG_UPNPAV_DMS_DIDL_NAME);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMS_DIDL_XMLNS, CG_UPNPAV_DMS_DIDL_XMLNS_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMS_DIDL_XMLNS_DC, CG_UPNPAV_DMS_DIDL_XMLNS_DC_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMS_DIDL_XMLNS_UPNP, CG_UPNPAV_DMS_DIDL_XMLNS_UPNP_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMS_DIDL_XMLNS_DLNA, CG_UPNPAV_DMS_DIDL_XMLNS_DLNA_URL);
    */
	cg_xml_node_setname(node, CG_UPNPAV_DMR_DIDL_NAME);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS, CG_UPNPAV_DMR_DIDL_XMLNS_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_UPNP, CG_UPNPAV_DMR_DIDL_XMLNS_UPNP_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_DC, CG_UPNPAV_DMR_DIDL_XMLNS_DC_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_DLNA, CG_UPNPAV_DMR_DIDL_XMLNS_DLNA_URL);
    cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_SEC, CG_UPNPAV_DMR_DIDL_XMLNS_SEC_URL);

	return node;
}


CgXmlNode *cg_upnpav_didl_node_new1()
{
	CgXmlNode *node;
    CgXmlNode *childnode;
 	CgXmlNode *leafnode;
	
	node = cg_xml_node_new();
    /*
     cg_xml_node_setname(node, CG_UPNPAV_DMS_DIDL_NAME);
     cg_xml_node_setattribute(node, CG_UPNPAV_DMS_DIDL_XMLNS, CG_UPNPAV_DMS_DIDL_XMLNS_URL);
     cg_xml_node_setattribute(node, CG_UPNPAV_DMS_DIDL_XMLNS_DC, CG_UPNPAV_DMS_DIDL_XMLNS_DC_URL);
     cg_xml_node_setattribute(node, CG_UPNPAV_DMS_DIDL_XMLNS_UPNP, CG_UPNPAV_DMS_DIDL_XMLNS_UPNP_URL);
     cg_xml_node_setattribute(node, CG_UPNPAV_DMS_DIDL_XMLNS_DLNA, CG_UPNPAV_DMS_DIDL_XMLNS_DLNA_URL);
     */
	cg_xml_node_setname(node, CG_UPNPAV_DMR_DIDL_NAME);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS, CG_UPNPAV_DMR_DIDL_XMLNS_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_UPNP, CG_UPNPAV_DMR_DIDL_XMLNS_UPNP_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_DC, CG_UPNPAV_DMR_DIDL_XMLNS_DC_URL);
	cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_DLNA, CG_UPNPAV_DMR_DIDL_XMLNS_DLNA_URL);
    cg_xml_node_setattribute(node, CG_UPNPAV_DMR_DIDL_XMLNS_SEC, CG_UPNPAV_DMR_DIDL_XMLNS_SEC_URL);
    
    childnode = cg_xml_node_new();
	cg_xml_node_setname(childnode, "item");
	cg_xml_node_setattribute(childnode, "id", "45");
    cg_xml_node_setattribute(childnode, "parentID", "45");
    cg_xml_node_setattribute(childnode, "restricted", "1");
    cg_xml_node_addchildnode(node, childnode);
    
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "upnp:class");
    cg_xml_node_setvalue(leafnode, "object.item.audioItem.musicTrack");
    cg_xml_node_addchildnode(childnode, leafnode);
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "dc:title");
    cg_xml_node_setvalue(leafnode, "title");
    cg_xml_node_addchildnode(childnode, leafnode);
    
    leafnode = cg_xml_node_new();
    cg_xml_node_setname(leafnode, "res");
    cg_xml_node_setvalue(leafnode, "http://");
	cg_xml_node_setattribute(leafnode, "protocolInfo","http protocolinfo");
    cg_xml_node_setattribute(leafnode, "size", "size");
    cg_xml_node_setattribute(leafnode, "duration", "10:10:10");
    cg_xml_node_addchildnode(leafnode, leafnode);
    
    
	return node;
}

