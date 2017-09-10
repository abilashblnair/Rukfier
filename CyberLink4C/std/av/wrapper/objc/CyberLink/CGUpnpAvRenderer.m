//
//  CGUpnpAvServer.m
//  CyberLink for C
//
//  Created by Satoshi Konno on 08/07/02.
//  Copyright 2008 Satoshi Konno. All rights reserved.
//

#include <cybergarage/upnp/std/av/cmediarenderer.h>
//#include "../../../include/cybergarage/upnp/std/av/cmediarenderer.h"

#import "CGUpnpAvRenderer.h"
#import "CGUpnpAVPositionInfo.h"

@interface CGUpnpAvRenderer()
@property (assign) int currentPlayMode;
@end

enum {
	CGUpnpAvRendererPlayModePlay,
	CGUpnpAvRendererPlayModePause,
	CGUpnpAvRendererPlayModeStop,
};

@implementation CGUpnpAvRenderer

@synthesize cAvObject;
@synthesize currentPlayMode;

- (id)init
{
	if ((self = [super init]) == nil)
		return nil;

	cAvObject = cg_upnpav_dmr_new();
	[self setCObject:cg_upnpav_dmr_getdevice(cAvObject)];
	
	[self setCurrentPlayMode:CGUpnpAvRendererPlayModeStop];
	
	return self;
}

- (id) initWithCObject:(CgUpnpDevice *)cobj
{
	if ((self = [super initWithCObject:cobj]) == nil)
		return nil;

	cAvObject = NULL;

	return self;
}

- (CGUpnpAction *)actionOfTransportServiceForName:(NSString *)serviceName
{
	CGUpnpService *avTransService = [self getServiceForType:@"urn:schemas-upnp-org:service:AVTransport:1"];
	if (!avTransService)
		return nil;
	
	return [avTransService getActionForName:serviceName];
}

- (CGUpnpAction *)actionOfRenderingControlServiceForName:(NSString *)serviceName
{
	CGUpnpService *avRCService = [self getServiceForType:@"urn:schemas-upnp-org:service:RenderingControl:1"];
	if (!avRCService)
		return nil;
	
	return [avRCService getActionForName:serviceName];
}

- (CGUpnpStateVariable *)getStateVariablebyName:(NSString*)varName
{
	CGUpnpService *avTransService = [self getServiceForType:@"urn:schemas-upnp-org:service:AVTransport:1"];
	if (!avTransService)
		return nil;
    
    return [avTransService getStateVariableForName:varName];
    
}


//- (BOOL)setAVTransportURI:(NSString *)aURL;
- (BOOL)setAVTransportURI:(NSString *)aURL metadata:(NSString *)ametaData
{
	CGUpnpAction *action = [self actionOfTransportServiceForName:@"SetAVTransportURI"];
	if (!action)
		return NO;

	[action setArgumentValue:@"0" forName:@"InstanceID"];
	[action setArgumentValue:aURL forName:@"CurrentURI"];
	[action setArgumentValue:ametaData forName:@"CurrentURIMetaData"];
	
	if (![action post])
		return NO;
	
	return YES;
}

- (BOOL)play;
{
	CGUpnpAction *action = [self actionOfTransportServiceForName:@"Play"];
	if (!action)
		return NO;
	
	[action setArgumentValue:@"0" forName:@"InstanceID"];
	[action setArgumentValue:@"1" forName:@"Speed"];
	
	if (![action post])
		return NO;
	
	[self setCurrentPlayMode:CGUpnpAvRendererPlayModePlay];
	
	return YES;
}

- (BOOL)stop;
{
	CGUpnpAction *action = [self actionOfTransportServiceForName:@"Stop"];
	if (!action)
		return NO;
	
	[action setArgumentValue:@"0" forName:@"InstanceID"];
	
	if (![action post])
		return NO;
	
	[self setCurrentPlayMode:CGUpnpAvRendererPlayModeStop];
	
	return YES;
}

- (BOOL)pause
{
	CGUpnpAction *action = [self actionOfTransportServiceForName:@"Pause"];
	if (!action)
		return NO;
	
	[action setArgumentValue:@"0" forName:@"InstanceID"];
	
	if (![action post])
		return NO;
	
	[self setCurrentPlayMode:CGUpnpAvRendererPlayModePause];
	
	return YES;
}

//- (BOOL)seek:(float)absTime
//{
//	CGUpnpAction *action = [self actionOfTransportServiceForName:@"Seek"];
//	if (!action)
//		return NO;
//	
//	[action setArgumentValue:@"0" forName:@"InstanceID"];
//	[action setArgumentValue:@"ABS_TIME" forName:@"Unit"];
//	[action setArgumentValue:[NSString stringWithDurationTime:absTime] forName:@"Target"];
//	
//	if (![action post])
//		return NO;
//	
//	return YES;
//}

- (BOOL)seek:(int)realTime
{
    CGUpnpAction *action = [self actionOfTransportServiceForName:@"Seek"];
    if (!action)
        return NO;
    
    [action setArgumentValue:@"0" forName:@"InstanceID"];
    [action setArgumentValue:@"REL_TIME" forName:@"Unit"];
    //[action setArgumentValue:[NSString stringWithDurationTime:absTime] forName:@"Target"];
    // NSString *realTimeStr = [NSString stringWithFormat:@"%d", realTime];
    
    NSString *realTimeStr =[NSString stringWithFormat:@"%02d:%02d:%02d",realTime / 3600, realTime / 60,realTime % 60];
    [action setArgumentValue:realTimeStr forName:@"Target"];
    NSLog(@"Real time str %@",realTimeStr);
    
    if (![action post])
        return NO;
    
    return YES;
}

- (NSString *)GetCurrentTransportActions
{
	CGUpnpAction *action = [self actionOfTransportServiceForName:@"GetCurrentTransportActions"];
	if (!action)
		return NO;
	
	[action setArgumentValue:@"0" forName:@"InstanceID"];
	
	if (![action post])
		return @"";
	
	return [action argumentValueForName:@"Actions"];
}

- (BOOL)isPlaying
{
	if ([self currentPlayMode] == CGUpnpAvRendererPlayModePlay)
		return YES;
	return NO;
}

- (CGUpnpAVPositionInfo *)positionInfo
{
	CGUpnpAction *action = [self actionOfTransportServiceForName:@"GetPositionInfo"];
	if (!action)
		return NO;
	
	[action setArgumentValue:@"0" forName:@"InstanceID"];
	
	if (![action post])
		return nil;
	
	return [[[CGUpnpAVPositionInfo alloc] initWithAction:action] autorelease];
}

- (BOOL)setVolume:(NSString *)desiredVol
{
	CGUpnpAction *action = [self actionOfRenderingControlServiceForName:@"SetVolume"];
	if (!action)
		return NO;
    
	[action setArgumentValue:@"0" forName:@"InstanceID"];
	[action setArgumentValue:@"Master" forName:@"Channel"];
	[action setArgumentValue:desiredVol forName:@"DesiredVolume"];
	
	if (![action post])
		return NO;
	
	return YES;
}

- (BOOL)setMute:(BOOL)desiredMute
{
	CGUpnpAction *action = [self actionOfRenderingControlServiceForName:@"SetMute"];
	if (!action)
		return NO;
    NSString *mutestr;
    
    if(desiredMute)
        mutestr = @"True";
    else
        mutestr = @"False";
    
	[action setArgumentValue:@"0" forName:@"InstanceID"];
	[action setArgumentValue:@"Master" forName:@"Channel"];
	[action setArgumentValue:mutestr forName:@"DesiredMute"];
	
	if (![action post])
		return NO;
	
	return YES;
}

- (NSString *)getMute
{
	CGUpnpAction *action = [self actionOfRenderingControlServiceForName:@"GetMute"];
	if (!action)
		return @"";
    
	[action setArgumentValue:@"0" forName:@"InstanceID"];
	[action setArgumentValue:@"Master" forName:@"Channel"];
	
	if (![action post])
		return @"";
    
    return [action argumentValueForName:@"CurrentMute"];
	
}

- (NSString *)getVolume
{
	CGUpnpAction *action = [self actionOfRenderingControlServiceForName:@"GetVolume"];
	if (!action)
		return @"";
    
	[action setArgumentValue:@"0" forName:@"InstanceID"];
	[action setArgumentValue:@"Master" forName:@"Channel"];
	
	if (![action post])
		return @"";
	
    return [action argumentValueForName:@"CurrentVolume"];

}

/*
- (BOOL)start
{
	if (!cAvObject)
		return NO;
	return cg_upnpav_dms_start(cAvObject);
}

- (BOOL)stop
{
	if (!cAvObject)
		return NO;
	return cg_upnpav_dms_stop(cAvObject);
}
*/

@end
