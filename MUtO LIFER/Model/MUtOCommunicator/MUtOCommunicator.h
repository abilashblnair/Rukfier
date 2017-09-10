//
//  MUtOCommunicator.h
//  MUtO LIFER
//
//  Created by Abilash Cumulations on 02/09/17.
//  Copyright © 2017 Abilash. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GCDAsyncSocket.h"

@class MUtOCommunicator;
@protocol MUtOCommunicatorDelegate <NSObject>

- (void)didConnectToHost:(MUtOCommunicator *)communicator withGCDSocket:(GCDAsyncSocket *)socket;

- (void)didReceiveData;
@end

@interface MUtOCommunicator : NSObject<GCDAsyncSocketDelegate>

@property (nonatomic,strong) GCDAsyncSocket *serverSocket;
@property (nonatomic,strong) NSMutableArray *connectedDevices;
@property (nonatomic,weak) id<MUtOCommunicatorDelegate>delegate;

+ (MUtOCommunicator *)sharedCommunicator;

- (void)sendMessage:(NSString *)message;
@end
