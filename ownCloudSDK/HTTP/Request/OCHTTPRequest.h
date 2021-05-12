//
//  OCHTTPRequest.h
//  ownCloudSDK
//
//  Created by Felix Schwarz on 06.02.18.
//  Copyright © 2018 ownCloud GmbH. All rights reserved.
//

/*
 * Copyright (C) 2018, ownCloud GmbH.
 *
 * This code is covered by the GNU Public License Version 3.
 *
 * For distribution utilizing Apple mechanisms please see https://owncloud.org/contribute/iOS-license-exception/
 * You should have received a copy of this license along with this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.en.html>.
 *
 */

#import <Foundation/Foundation.h>
#import "OCEventTarget.h"
#import "OCBookmark.h"
#import "OCCertificate.h"
#import "OCHTTPTypes.h"
#import "OCProgress.h"
#import "OCCellularSwitch.h"

@class OCHTTPPipelineTask;

typedef SEL OCHTTPRequestResultHandlerAction; //!< Selector following the format -handleResultForRequest:(OCHTTPRequest *)request error:(NSError *)error;

typedef NS_ENUM(NSUInteger, OCHTTPRequestObserverEvent)
{
	OCHTTPRequestObserverEventTaskResume	//!< Return YES if the observer takes care of resuming the URL session task, NO if the observer doesn't.
};

typedef NS_ENUM(NSUInteger, OCHTTPRequestRedirectPolicy)
{
	OCHTTPRequestRedirectPolicyDefault,		//!< Default redirection policy, defaults to OCHTTPRequestRedirectPolicyValidateConnection (OCHTTPRequestRedirectPolicyAllowSameHost if OCConnectionTransparentTemporaryRedirect is active)

	OCHTTPRequestRedirectPolicyHandleLocally,	//!< The HTTP layer should not handle redirects, and return the original (redirection) response for local handling instead
	OCHTTPRequestRedirectPolicyAllowSameHost,	//!< The HTTP layer should handle redirects transparently, but only to targets on the same host and scheme, otherwise return the response instead
	OCHTTPRequestRedirectPolicyAllowAnyHost,	//!< The HTTP layer should handle redirects transparently, to any target, even on other hosts
	OCHTTPRequestRedirectPolicyValidateConnection	//!< The HTTP layer should handle redirects as global exception, triggering a connection validation and - upon success - a retry of the original request
};

typedef BOOL(^OCHTTPRequestObserver)(OCHTTPPipelineTask *task, OCHTTPRequest *request, OCHTTPRequestObserverEvent event);

typedef NSString* OCHTTPRequestResumeInfoKey;
typedef NSDictionary<OCHTTPRequestResumeInfoKey,id>* OCHTTPRequestResumeInfo;

@interface OCHTTPRequest : NSObject <NSSecureCoding>
{
	NSData *_bodyData;
	NSInputStream *_bodyURLInputStream;
}

@property(strong,readonly) OCHTTPRequestID identifier; //!< Unique ID (auto-generated) for every request

@property(strong) OCProgress *progress;			//!< Resolvable progress object that tracks progress and provides cancellation ability/status

@property(strong) OCHTTPMethod method;			//!< The HTTP method to use to request the URL

@property(strong) NSURL *url;				//!< The URL to request
@property(strong) NSURL *effectiveURL;			//!< The URL that's effectively requested (generated by -prepareForScheduling)
@property(strong) OCHTTPRequestParameters parameters;	//!< The parameters to send as part of the URL (GET) or as the request's body (POST)
@property(strong) OCHTTPHeaderFields headerFields;//!< The HTTP headerfields to send alongside the request
@property(strong,nonatomic) NSData *bodyData;		//!< The HTTP body to send (as body data). Ignored / overwritten if .method is POST and .parameters has key-value pairs.
@property(strong) NSURL *bodyURL;			//!< The HTTP body to send (from a file). Ignored if .method is POST and .parameters has key-value pairs.

@property(assign,nonatomic) OCHTTPRequestRedirectPolicy redirectPolicy; //!< Controls redirect handling for this request. Defaults to OCHTTPRequestRedirectPolicyDefault.
@property(assign,nonatomic) NSUInteger maximumRedirectionDepth; //!< Maximum number of redirects to follow before failing. Defaults to 5.
@property(strong,nonatomic) NSArray<NSURL *> *redirectionHistory; //!< URLs visited during redirection, starting with the original URL.

@property(strong) NSDate *earliestBeginDate;		//!< The earliest this request should be sent.

@property(strong) NSSet<OCConnectionSignalID> *requiredSignals; //!< Set of signals that need to be set before scheduling this request. This may change the order in which requests are sent - unless a groupID is set.

@property(strong) OCCellularSwitchIdentifier requiredCellularSwitch; //!< Switch that needs to be allowed before scheduling this request. This may change the order in which requests are sent - unless a groupID is set.

@property(assign) BOOL autoResume;				//!< For GET requests, try to automatically resume transfer if connection is cut off
@property(strong) OCHTTPRequestResumeInfo autoResumeInfo;	//!< Resume information for auto resume

@property(assign) OCHTTPRequestResultHandlerAction resultHandlerAction;	//!< The selector to invoke on OCConnection when the request has concluded.
@property(copy)   OCHTTPRequestEphermalResultHandler ephermalResultHandler;	//!< The resultHandler to invoke if resultHandlerAction==NULL. Ephermal [not serialized].
@property(copy)   OCConnectionEphermalRequestCertificateProceedHandler ephermalRequestCertificateProceedHandler; //!< The certificateProceedHandler to invoke for certificates that need user approval. [not serialized]
@property(assign) BOOL forceCertificateDecisionDelegation; //!< YES if certificateProceedHandler and the connection (delegate) should be consulted even if the certificate has no issues or was previously approved by the user. [not serialized]

@property(strong) OCEventTarget *eventTarget;		//!< The target the parsed result should be delivered to as an event.
@property(strong) NSDictionary *userInfo;		//!< User-info for free use. All contents should be serializable.

@property(assign) OCHTTPRequestPriority priority; //!< Priority of the request from 0.0 (lowest priority) to 1.0 (highest priority). Defaults to NSURLSessionTaskPriorityDefault (= 0.5).
@property(strong) OCHTTPRequestGroupID groupID; 	//!< ID of the Group the request belongs to (if any). Requests in the same group are executed serially, whereas requests that belong to no group are executed as soon as possible.

@property(copy) OCHTTPRequestObserver requestObserver; //!< OCHTTPRequestObserver block called as the request encounters various events

@property(assign) BOOL downloadRequest;			//!< If the request is for the download of a file and the response body should be written to a file.
@property(strong) NSURL *downloadedFileURL;		//!< If downloadRequest is YES, location of the downloaded file. It's possible to pre-occupy this field, in which case the temporary file will be copied to that URL when the download completes.
@property(assign) BOOL downloadedFileIsTemporary;	//!< If YES, the downloadedFileURL points to a temporary file that will be removed automatically. If NO, downloadedFileURL points to a file that won't be removed automatically (== if downloadedFileURL was set before starting the download).

@property(assign) BOOL avoidCellular;			//!< Request is not allowed to be sent over a cellular connection. Defaults to NO.

@property(assign) BOOL isNonCritial;			//!< Request that are marked non-critical are allowed to be cancelled to speed up shutting down the connection queue

@property(assign) BOOL cancelled;

@property(strong,readonly,nonatomic) NSError *error;	//!< Convenience accessor for .httpResponse.error

#pragma mark - Init
+ (instancetype)requestWithURL:(NSURL *)url;

#pragma mark - Queue scheduling support
- (void)prepareForScheduling; //!< Called directly before scheduling of a request begins.
- (NSMutableURLRequest *)generateURLRequest; //!< Returns an NSURLRequest for this request.
- (void)scrubForRescheduling;

- (OCHTTPRequestID)recreateRequestID; //!< Creates and sets a new request ID on .identifier and the X-Request-ID header (for internal use only!)

#pragma mark - Cancel support
- (void)cancel;

#pragma mark - Access
- (NSString *)valueForParameter:(NSString *)parameter;
- (void)setValue:(NSString *)value forParameter:(NSString *)parameter;
- (void)setValueArray:(NSArray *)valueArray apply:(NSString *(^)(id value))applyBlock forParameter:(NSString *)parameter;

- (void)addParameters:(NSDictionary<NSString*,NSString*> *)parameters;

- (NSString *)valueForHeaderField:(OCHTTPHeaderFieldName)headerField;
- (void)setValue:(NSString *)value forHeaderField:(OCHTTPHeaderFieldName)headerField;

- (void)addHeaderFields:(NSDictionary<OCHTTPHeaderFieldName,NSString*> *)headerFields;

#pragma mark - Response
@property(strong) OCHTTPResponse *httpResponse;

#pragma mark - Description
+ (NSString *)bodyDescriptionForURL:(NSURL *)url data:(NSData *)data headers:(NSDictionary<NSString *, NSString *> *)headers prefixed:(BOOL)prefixed;
+ (NSString *)formattedHeaders:(NSDictionary<NSString *, NSString *> *)headers withLinePrefix:(NSString *)linePrefix;

- (NSString *)requestDescriptionPrefixed:(BOOL)prefixed;

@end

extern OCHTTPMethod OCHTTPMethodGET;
extern OCHTTPMethod OCHTTPMethodPOST;
extern OCHTTPMethod OCHTTPMethodPATCH;
extern OCHTTPMethod OCHTTPMethodHEAD;
extern OCHTTPMethod OCHTTPMethodPUT;
extern OCHTTPMethod OCHTTPMethodDELETE;
extern OCHTTPMethod OCHTTPMethodMKCOL;
extern OCHTTPMethod OCHTTPMethodOPTIONS;
extern OCHTTPMethod OCHTTPMethodMOVE;
extern OCHTTPMethod OCHTTPMethodCOPY;
extern OCHTTPMethod OCHTTPMethodPROPFIND;
extern OCHTTPMethod OCHTTPMethodPROPPATCH;
extern OCHTTPMethod OCHTTPMethodREPORT;
extern OCHTTPMethod OCHTTPMethodLOCK;
extern OCHTTPMethod OCHTTPMethodUNLOCK;

extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameLocation;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameAuthorization;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameXRequestID;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameOriginalRequestID;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameContentType;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameContentLength;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameDepth;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameDestination;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameOverwrite;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameIfMatch;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameIfNoneMatch;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameUserAgent;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameXOCMTime;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameOCChecksum;
extern OCHTTPHeaderFieldName OCHTTPHeaderFieldNameOCConnectionValidator;

extern OCProgressPathElementIdentifier OCHTTPRequestGlobalPath;

extern OCHTTPRequestResumeInfoKey OCHTTPRequestResumeInfoKeySystemResumeData; //!< NSURLSessionDownloadTaskResumeData data provided by the OS
