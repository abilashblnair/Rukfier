//
//  ViewController.m
//  MUtO LIFER
//
//  Created by Abilash Cumulations on 02/09/17.
//  Copyright © 2017 Abilash. All rights reserved.
//

#import "ViewController.h"
#import "MUtOCommunicator.h"
#import "UPnPManager.h"
#import "RequiredGlobally.h"
#import <AVFoundation/AVFoundation.h>

@interface ViewController ()<MUtOCommunicatorDelegate>
{
    NSString *songurl;
        CGUpnpAvRenderer *deviceRenderer;
}
@property (nonatomic,strong)  AVPlayer *player;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    [[MUtOCommunicator sharedCommunicator]setDelegate:self];
}



#pragma mark -  MUTO communicator delegate
- (void)didConnectToHost:(MUtOCommunicator *)communicator withGCDSocket:(GCDAsyncSocket *)socket
{
    UPnPManager *upnpmnr = [UPnPManager getUPnPManagerInstance];
    
    [upnpmnr startLibreUPnPDevice];
    
    [upnpmnr startControlPoint];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        //deviceRenderer = [self getRendererForIP:[socket connectedHost]];
        deviceRenderer = [[CGUpnpAvRenderer alloc]init];
        [self prepareServer4IpodPlayback];
    });
}


-(CGUpnpAvRenderer*) getRendererForIP:(NSString*) IP
{
    NSArray *allRenderers = [[UPnPManager getUPnPManagerInstance] getRendersList];
    for(CGUpnpAvRenderer *rendr in allRenderers)
    {
        if([rendr.ipaddress isEqualToString:IP] )
        {
            return rendr;
        }
    }
    return nil;
}

-(void) prepareServer4IpodPlayback
{
    // Step1 export the item to destination folder
    NSURL *assetURL = [NSURL fileURLWithPath:[[NSBundle mainBundle]pathForResource:@"Kaatru" ofType:@"mp3"]];
    
    
    // we're responsible for making sure the destination url doesn't already exist
    // create the import object
            NSData *outData = [NSData dataWithContentsOfFile:[assetURL path]];
    
            if([outData bytes] > (void*)0)
            {
                //Vishnu - Exporting to DLNA
                    [self exportToDlna:assetURL];
                    

            }
            else
            {
                
                }

}
- (NSString *)convert2HMMSSF3:(NSString*) dur
{
    float num_seconds = [dur floatValue];
    int hours, minutes, seconds;
    hours = num_seconds / (60 * 60);
    num_seconds -= hours * (60 * 60);
    minutes = num_seconds / 60;
    num_seconds -= minutes * 60;
    seconds = num_seconds;
    
    NSString * oduration = @"";
    NSString *hrFromInt = [NSString stringWithFormat:@"%02d", hours];
    NSString *mnFromInt = [NSString stringWithFormat:@"%02d", minutes];
    NSString *secFromInt = [NSString stringWithFormat:@"%02d", seconds];
    oduration = [oduration stringByAppendingString:hrFromInt];
    oduration = [oduration stringByAppendingString:@":"];
    oduration = [oduration stringByAppendingString:mnFromInt];
    oduration = [oduration stringByAppendingString:@":"];
    oduration = [oduration stringByAppendingString:secFromInt];
    
    return oduration;
    
}



-(void)exportToDlna:(NSURL*) outURL
{
    
    NSString *songTitleTemp = @"Kathru";
    NSString *artistTemp = @"abi";
    NSString *albumTemp = @"";
    
    //localSrcArtWork = [item valueForProperty: MPMediaItemPropertyArtwork];
    //NSLog(@"a<--albumarturl=%@",albumarturl);
    NSString *upnpClass = @"object.item.audioItem.musicTrack";
    
    AVURLAsset* audioAsset = [AVURLAsset URLAssetWithURL:outURL options:nil];
    CMTime audioDuration = audioAsset.duration;
    float audioDurationSeconds = CMTimeGetSeconds(audioDuration);
    
    NSString *duration = [NSString stringWithFormat:@"%.0f",audioDurationSeconds];
    //char size = [item valueForProperty:mpmedi]
    
    //NSString *outurl;
   NSString *exportURL = [outURL absoluteString];
    
    NSString * absPath = @"http://";
    absPath = [absPath stringByAppendingString:[RequiredGlobally getIPAddress]];
    
    NSString *httpPort = [[UPnPManager getUPnPManagerInstance] returnCurrentPort];
    
    absPath = [absPath stringByAppendingString:@":"];
    absPath = [absPath stringByAppendingString:httpPort];
    absPath = [absPath stringByAppendingString:@"/akshayv"];  //insert special url
    NSString *durHMMSS;
    durHMMSS = [self convert2HMMSSF3:duration];
    
    
    exportURL = [exportURL stringByRemovingPercentEncoding];
    NSString *struri = [absPath stringByAppendingString:[exportURL substringFromIndex:7]];
    
    NSString *protocolStr;
    bool isKnownFormat = false;
    if(([exportURL rangeOfString:@".mp3"].location != NSNotFound))
    {
        protocolStr = @"http-get:*:audio/mpeg:DLNA.ORG_PN=MP3;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01700000000000000000000000000000";
        isKnownFormat = true;
    }
    else if(([exportURL rangeOfString:@".m4a"].location != NSNotFound))
    {
        protocolStr = @"http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01700000000000000000000000000000";
        isKnownFormat = true;
    }
    else if(([exportURL rangeOfString:@".wav"].location != NSNotFound))
    {
        protocolStr = @"http-get:*:audio/wav:DLNA.ORG_PN=WAV;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01700000000000000000000000000000";
        isKnownFormat = true;
    }

    
    if (!isKnownFormat)  //Futsal
    {
        //protocolStr = @"http-get:*:audio/L16:DLNA.ORG_PN=LPCM;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01700000000000000000000000000000";
        protocolStr = @"http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=01700000000000000000000000000000";
        
        struri = [struri stringByAppendingString:@"!"];
        NSNumber *durations = [NSNumber numberWithInteger:duration.integerValue];
        //       NSString *myString = [NSNumber *durations];
        struri = [struri stringByAppendingString:[durations stringValue]];
    }
    
    
    NSString *songURL = struri;
    CGUpnpAvController *upnpAvController = [[UPnPManager getUPnPManagerInstance] getControlPoint];
    NSLog(@"a<--upnpAvController=%@, %@",upnpAvController,[[UPnPManager getUPnPManagerInstance] getControlPoint]);
    
    NSString * DIDLXml = [upnpAvController getDIDLXml4SongTitle:songTitleTemp Artist:artistTemp Album:albumTemp AlbumartUrl:nil upnpclass:upnpClass format:protocolStr size:@"" duration:durHMMSS url:struri];
    
    
   NSString *metaData = DIDLXml;
    
    
    if (deviceRenderer == nil)
        deviceRenderer = [[CGUpnpAvRenderer alloc]init];
    
    
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        [self pushSongtoRenderer:songURL withMetaData:metaData];
        
        
    });
    
}

-(void) pushSongtoRenderer:(NSString *)songURL withMetaData:(NSString *)metadata
{

   
    if(deviceRenderer==nil)
        return;
    
    if ([deviceRenderer stop]) //seturi
    {
        NSLog(@"StopAVT post success");
    }
    else
        NSLog(@"STOPAVT post failure");
    NSLog(@"a<--friendly name=%@",songURL);
    NSLog(@"song URL PUSHING TO  RENDERER %@",songURL);
    if ([deviceRenderer setAVTransportURI:songURL metadata:metadata]) //seturi
    {
        NSLog(@"AVTranport post success");
    }
    else
        NSLog(@"AVTranport post failure");
    
    if ([deviceRenderer play]) //seturi
    {
        NSLog(@"AVT:Play post success");
    }
    else
    {
        NSLog(@"AVT:Play post failure");
    }
    [[MUtOCommunicator sharedCommunicator]sendMessage:songURL];
            songURL =  [songURL stringByReplacingOccurrencesOfString:@" " withString:@"%20"];
     songurl = songURL;
    

    
    
}

- (void)didReceiveData
{
    self.player = [AVPlayer playerWithURL:[NSURL URLWithString:songurl]];
    [self.player play];
}



@end
