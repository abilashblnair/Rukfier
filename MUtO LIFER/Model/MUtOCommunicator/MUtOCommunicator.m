//
//  MUtOCommunicator.m
//  MUtO LIFER
//
//  Created by Abilash Cumulations on 02/09/17.
//  Copyright © 2017 Abilash. All rights reserved.
//

#import "MUtOCommunicator.h"
#import "RequiredGlobally.h"

@interface MUtOCommunicator()
{
    dispatch_queue_t serverQueue;
}
@end

@implementation MUtOCommunicator
+ (MUtOCommunicator *)sharedCommunicator
{
    static MUtOCommunicator *communicator = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        communicator = [MUtOCommunicator new];
        [communicator createServer];
        communicator.connectedDevices = [NSMutableArray new];
    });
    return communicator;
}
- (void)createServer
{
    serverQueue = dispatch_queue_create("MUto server", nil);
    self.serverSocket = [[GCDAsyncSocket alloc]initWithDelegate:self delegateQueue:serverQueue];
    NSError *Error = nil;
    NSString *IPaddress = [RequiredGlobally getIPAddress];
    if(![self.serverSocket acceptOnPort:7070 error:&Error])
    {
        NSLog(@"Error while creating server socket on port == %d with error == %@",7070,[Error localizedDescription]);
    }else
    {
        NSLog(@"Successfully created socket for Port == 7070 my ip === %@",IPaddress);
    }
}

- (void)sendMessage:(NSString *)message
{
    [self.serverSocket writeData:[message dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:10];
}



#pragma mark -  GCDASYC DELEGATE
- (void)socket:(GCDAsyncSocket *)sock didAcceptNewSocket:(GCDAsyncSocket *)newSocket
{
    NSLog(@"Accepted new socket from %@:%hu with local host==%@", [newSocket connectedHost], [newSocket connectedPort],[newSocket localHost]);
    
    self.serverSocket = newSocket;
    NSString *welcomMessage = @"Hello\r\n";
    [self.serverSocket writeData:[welcomMessage dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:1];
    
    [self.serverSocket readDataWithTimeout:-1 tag:0];
    for (GCDAsyncSocket *Socket in self.connectedDevices) {
        if (![[Socket connectedHost] isEqualToString:[newSocket connectedHost]]) {
            [self.connectedDevices addObject:Socket];
        }
    }
    if ([self.delegate respondsToSelector:@selector(didConnectToHost:withGCDSocket:)]) {
        [self.delegate didConnectToHost:self withGCDSocket:newSocket];
    }
    
}

-(void)socket:(GCDAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag{
    NSString *msg = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    
    NSLog(@"MSG: %@",msg);
    
}

- (void)socketDidDisconnect:(GCDAsyncSocket *)sock withError:(NSError *)err
{
      NSLog(@"Socket disconnected with error === %@",[err localizedDescription]);
    if (err != nil) {
        if ([self.connectedDevices containsObject:sock]) {
            [self.connectedDevices removeObject:sock];
        }
    }
}

- (void)socket:(GCDAsyncSocket *)sock didWriteDataWithTag:(long)tag
{
    if (tag == 10) {
        if ([self.delegate respondsToSelector:@selector(didReceiveData)]) {
            [self.delegate didReceiveData];
        }
    }
    
    
    
}

@end
