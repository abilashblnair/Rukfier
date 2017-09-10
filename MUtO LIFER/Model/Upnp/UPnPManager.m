//
//  UPnPManager.h
//  DDMS
//
//  Created by libre on 17/09/14.
//  Copyright (c) 2014 Libre Wireless Technologies. All rights reserved.
//

#import "UPnPManager.h"


@interface UPnPManager()
{
    CGUpnpAvServer *upnpAvServer;
    CGUpnpAvController *upnpAvController;
    NSString *currentPort;
}

@end


@implementation UPnPManager

@synthesize currentRenderer;
@synthesize currentServer;

static UPnPManager* _upnpManagerIns = nil;

+ (UPnPManager *)getUPnPManagerInstance
{
    @synchronized([UPnPManager class])
    {
        if (!_upnpManagerIns)
        {
            _upnpManagerIns = [[self alloc] init];
        }
        
        return _upnpManagerIns;
    }
    
    return nil;
}

+ (id)alloc
{
    @synchronized([UPnPManager class])
    {
        NSAssert(_upnpManagerIns == nil,
                 @"Attempted to allocate a second instance of a singleton.");
        _upnpManagerIns = [super alloc];
        return _upnpManagerIns;
    }
    
    return nil;
}

- (id)init
{
    self = [super init];
    if (self != nil)
    {
        //[self startControlPoint];  //start control point here
        // initialize stuff here
    }
    
    return self;
}

BOOL cg_upnp_controlpoint_subscribeallservices(CgUpnpControlPoint *ctrlPoint, CgUpnpDevice *dev, long timeout, void* (*functionPtr)(void) );

- (BOOL)subscribe2ControlPoint:(CGUpnpAvRenderer*)renderer callback:(void*())func;
{
    NSLog(@"a<--subscribe2ControlPoint\n");
    //renderer getServiceForType:
    return cg_upnp_controlpoint_subscribeallservices(upnpAvController.cObject, renderer.cObject, 200, func);
    //return YES;
}

-(void)makeControlPointNil
{
    //[upnpAvController deleteObject];
   //_upnpManagerIns = nil;
    upnpAvController = nil;
}

-(void)startControlPoint
{
    NSLog(@"###### Starting Control Point #####");
    if (nil == upnpAvController)
    {
        upnpAvController = [[CGUpnpAvController alloc] init ];
        [upnpAvController setDelegate:self];
        [upnpAvController setSsdpSearchMX:5];
    }
    [upnpAvController search];
}

-(void)searchRenderers
{
//        if (nil == upnpAvController)
//        {
//            upnpAvController = [[CGUpnpAvController alloc] init ];
//            [upnpAvController setDelegate:self];
//            [upnpAvController setSsdpSearchMX:5];
//    
//        }
//        else
//        {
//            [upnpAvController stop];
            //[upnpAvController start];
//            [upnpAvController delete:upnpAvController];
////            upnpAvController = nil;
//    
//        }
//
    
//        [upnpAvController searchRenderers];
    
    //////////JAVED --> Library Crash Temp Fix ////////////////////////
//    if (nil != upnpAvController)
//    {
        //[upnpAvController delete:upnpAvController];
    //[upnpAvController stop];
//    }
//    upnpAvController = [[CGUpnpAvController alloc] init ];
//    [upnpAvController setDelegate:self];
//    [upnpAvController setSsdpSearchMX:5];
   // [upnpAvController start];
//    [upnpAvController searchRenderers];
    
    
    
     if(nil == upnpAvController)
     {
       upnpAvController = [[CGUpnpAvController alloc] init ];
       [upnpAvController setDelegate:self];
       [upnpAvController setSsdpSearchMX:5];
     }
     
     [upnpAvController searchRenderers];
    
    
    //////////Library Crash Temp Fix ////////////////////////
}

-(void)searchServers
{
    
    if (nil == upnpAvController)
    {
        upnpAvController = [[CGUpnpAvController alloc] init ];
        [upnpAvController setDelegate:self];
        [upnpAvController setSsdpSearchMX:5];
    }
    [upnpAvController searchServers];
}

-(void)stopControlPoint
{
    NSLog(@"##### Stop Control Point ######");
    //implement if needed. Just dealloc control point
    CGUpnpAvController *controlPoint = [self getControlPoint];
   [controlPoint stop];
    
}

-(BOOL)networkChanged
{
    NSLog(@"##### networkChanged ######");
    //implement if needed. Just dealloc control point
    CGUpnpAvController *controlPoint = [self getControlPoint];
    return [controlPoint networkChanged];
    
}



-(void)deleteRenderer
{
    CGUpnpAvController *controlPoint = [self getControlPoint];
    [controlPoint deleteObject];
    
}

- (void)startLibreUPnPDevice
{
    NSLog(@"##### Starting device UPnP Device ######");
    if (nil == upnpAvServer)
    {
        upnpAvServer = [[CGUpnpAvServer alloc] init ];

    }
    
    if([upnpAvServer start])
    {
        NSLog(@" device Server Started successfully");
    }
    else
    {
        NSLog(@"Not able to start device Server");
    }
    
    currentPort = [upnpAvServer returnDevHTTPPORT];
}

- (void)stopLibreUPnPDevice
{
    NSLog(@"###### stopLibre UPnPDevice UPnP Device #####");
    if([upnpAvServer stop])
    {
        NSLog(@"Device Server Stopped successfully");
    }
    else
    {
        NSLog(@"Not able to Stopped device Server");
    }
    
}

- (NSString *)returnCurrentPort
{
    return currentPort;
}

- (CGUpnpAvController *)getControlPoint
{
    return upnpAvController;
}

#pragma mark
#pragma mark CGUpnpControlPointDelegate

- (void)controlPoint:(CGUpnpControlPoint *)controlPoint deviceAdded:(NSString *)deviceUdn
{
    CGUpnpAvRenderer *render = [upnpAvController rendererForUDN:deviceUdn];
    if (nil != render)
    {
        if ([self.managerDelegate respondsToSelector:@selector(renderAdded:)]) {
             [self.managerDelegate renderAdded:render];
        }
       
        [[NSNotificationCenter defaultCenter] postNotificationName:@"NewUPNPDeviceAdded" object:render];
        NSLog(@"a<--New Renderer =%@",[render friendlyName]);
    }
    
    NSLog(@"Device Added with udn %@, %@", deviceUdn, [render friendlyName]);
}

- (void)controlPoint:(CGUpnpControlPoint *)controlPoint deviceRemoved:(NSString *)deviceUdn
{
    CGUpnpAvRenderer *render = [upnpAvController rendererForUDN:deviceUdn];
    if (nil != render)
    {
     
        if ([self.managerDelegate respondsToSelector:@selector(renderRemoved:)]) {
           [self.managerDelegate renderRemoved:render];
        }
        
        
        [[NSNotificationCenter defaultCenter] postNotificationName:@"UPNPDeviceRemoved" object:render];
    }
    

    NSLog(@"a<--Device Removed with udn %@", deviceUdn);
}

- (void)controlPoint:(CGUpnpControlPoint *)controlPoint deviceUpdated:(NSString *)deviceUdn
{
    NSLog(@"Device Updated with udn %@", deviceUdn);
}

- (void)controlPoint:(CGUpnpControlPoint *)controlPoint deviceInvalid:(NSString *)deviceUdn
{
    CGUpnpAvRenderer *render = [upnpAvController rendererForUDN:deviceUdn];
    if (nil != render)
    {
        //[self.managerDelegate renderInvalid:render];
    }
    
    NSLog(@"Device Invalid");
}

- (BOOL)isCurrentRenderPlaying
{
    return [self.currentRenderer isPlaying];
}

- (BOOL)pauseCurrentRender
{
    return [self.currentRenderer pause];
}

- (BOOL)playCurrentRender
{
    return [self.currentRenderer play];
}

- (BOOL)stopCurrentRender
{
    return [self.currentRenderer stop];
}

- (NSArray *)getRendersList
{
    if (nil == upnpAvController)
    {
        return nil;
    }
    return [upnpAvController renderers];
}

- (NSArray *)getServersList
{
    if (nil == upnpAvController)
    {
        return nil;
    }
    return [upnpAvController servers];
}



@end