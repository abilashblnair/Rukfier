//
//  UPnPManager.h
//  DDMS
//
//  Created by libre on 17/09/14.
//  Copyright (c) 2014 Libre Wireless Technologies. All rights reserved.
//

#import "CyberLinkHeader.h"


@protocol UPnPManagerDelegate <NSObject>//add by Tony

@optional
- (void)renderAdded:(CGUpnpAvRenderer *)render;// new render added
- (void)renderRemoved:(CGUpnpAvRenderer *)render;//removed
- (void)renderInvalid:(CGUpnpAvRenderer *)render;

@end


@interface UPnPManager : NSObject<CGUpnpControlPointDelegate>
{
}

//@property(nonatomic, weak) id<UPnPManagerDelegate> CPdelegate;  //for dynamic add/del of control points


@property(nonatomic, weak) CGUpnpAvServer *currentServer;
@property(atomic, weak) CGUpnpAvRenderer *currentRenderer;
@property(nonatomic, assign) id<UPnPManagerDelegate>managerDelegate;

+ (UPnPManager*)getUPnPManagerInstance;

- (void)startControlPoint;
- (void)startLibreUPnPDevice;
- (CGUpnpAvController *)getControlPoint;
- (NSString *)returnCurrentPort;
- (BOOL)subscribe2ControlPoint:(CGUpnpAvRenderer*)renderer callback:(void*())func;
- (void)stopLibreUPnPDevice;
- (void)stopControlPoint;
-(void)makeControlPointNil;
- (BOOL)isCurrentRenderPlaying;
- (BOOL)pauseCurrentRender;
- (BOOL)playCurrentRender;
- (BOOL)stopCurrentRender;
-(void)deleteRenderer;

-(BOOL)networkChanged;

- (NSArray *)getRendersList;
- (NSArray *)getServersList;
-(void)searchRenderers;
-(void)searchServers;


@end