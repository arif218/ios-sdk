//
//  OCConnectionRequest.h
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
#import "OCConnection.h"
#import "OCHTTPStatus.h"
#import "OCCertificate.h"

typedef NSString* OCConnectionRequestMethod NS_TYPED_ENUM;
typedef NSMutableDictionary<NSString*,NSString*>* OCConnectionHeaderFields;
typedef NSMutableDictionary<NSString*,NSString*>* OCConnectionParameters;

typedef float OCConnectionRequestPriority;
typedef NSString* OCConnectionRequestGroupID;

typedef SEL OCConnectionRequestResultHandlerAction; //!< Selector following the format -handleResultForRequest:(OCConnectionRequest *)request error:(NSError *)error;

@interface OCConnectionRequest : NSObject <NSSecureCoding>
{
	NSURLSessionTask *_urlSessionTask;
	NSProgress *_progress;
	
	NSNumber *_urlSessionTaskIdentifier;

	OCBookmarkUUID _bookmarkUUID;

	OCConnectionRequestMethod _method;

	NSURL *_url;
	NSURL *_effectiveURL;
	OCConnectionParameters _parameters;
	OCConnectionHeaderFields _headerFields;
	NSData *_bodyData;
	NSURL *_bodyURL;
	NSInputStream *_bodyURLInputStream;

	OCConnectionRequestResultHandlerAction _resultHandlerAction;
	OCConnectionEphermalResultHandler _ephermalResultHandler;
	OCConnectionEphermalRequestCertificateProceedHandler _ephermalRequestCertificateProceedHandler;
	BOOL _forceCertificateDecisionDelegation;

	OCEventTarget *_eventTarget;

 	OCConnectionRequestPriority _priority;
	OCConnectionRequestGroupID _groupID;
	BOOL _skipAuthorization;

	BOOL _downloadRequest;
	NSURL *_downloadedFile;

	OCHTTPStatus *_responseHTTPStatus;
	NSHTTPURLResponse *_injectedResponse;
	NSMutableData *_responseBodyData;
	OCCertificate *_responseCertificate;
	
	BOOL _cancelled;

	NSError *_error;
}

@property(strong) NSURLSessionTask *urlSessionTask;	//!< NSURLSessionTask used to perform the request [not serialized]
@property(strong) NSProgress *progress;			//!< Progress object that tracks progress and provides cancellation ability/status [not serialized]

@property(strong) NSNumber *urlSessionTaskIdentifier;	//!< Value of NSURLSessionTask.taskIdentifier

@property(strong) OCBookmarkUUID bookmarkUUID;		//!< UUID identifying the Bookmark that this request's connection is related to. Identifies the connection so that background queue results can be delivered correctly.

@property(strong) OCConnectionRequestMethod method;	//!< The HTTP method to use to request the URL

@property(strong) NSURL *url;				//!< The URL to request
@property(strong) NSURL *effectiveURL;			//!< The URL that's effectively requested (generated by -prepareForSchedulingInQueue:)
@property(strong) OCConnectionParameters parameters;	//!< The parameters to send as part of the URL (GET) or as the request's body (POST)
@property(strong) OCConnectionHeaderFields headerFields;//!< The HTTP headerfields to send alongside the request
@property(strong,nonatomic) NSData *bodyData;			//!< The HTTP body to send (as body data). Ignored / overwritten if .method is POST and .parameters has key-value pairs.
@property(strong) NSURL *bodyURL;			//!< The HTTP body to send (from a file). Ignored if .method is POST and .parameters has key-value pairs.

@property(assign) OCConnectionRequestResultHandlerAction resultHandlerAction;	//!< The selector to invoke on OCConnection when the request has concluded.
@property(copy)   OCConnectionEphermalResultHandler ephermalResultHandler;	//!< The resultHandler to invoke if resultHandlerAction==NULL. Ephermal [not serialized].
@property(copy)   OCConnectionEphermalRequestCertificateProceedHandler ephermalRequestCertificateProceedHandler; //!< The certificateProceedHandler to invoke for certificates that need user approval. [not serialized]
@property(assign) BOOL forceCertificateDecisionDelegation; //!< YES if certificateProceedHandler and the connection (delegate) should be consulted even if the certificate has no issues or was previously approved by the user. [not serialized]

@property(strong) OCEventTarget *eventTarget;		//!< The target the parsed result should be delivered to as an event.

@property(assign,nonatomic) OCConnectionRequestPriority priority; //!< Priority of the request from 0.0 (lowest priority) to 1.0 (highest priority)
@property(strong) OCConnectionRequestGroupID groupID; 	//!< ID of the Group the request belongs to (if any). Requests in the same group are executed serially, whereas requests that belong to no group are executed as soon as possible.
@property(assign) BOOL skipAuthorization;		//!< YES if the connection should not perform authorization on the request during scheduling [not serialized]

@property(assign) BOOL downloadRequest;			//!< If the request is for the download of a file and the response body should be written to a file.
@property(strong) NSURL *downloadedFile;		//!< If downloadRequest is YES, location of the downloaded file

@property(strong) NSMutableData *responseBodyData;	//!< If downloadRequest is NO, any body data received in response is stored here. [not serialized]
@property(strong) OCCertificate *responseCertificate;	//!< If HTTPS is used, the certificate of the server from which the response was served

@property(readonly) BOOL cancelled;

@property(strong) NSError *error;

#pragma mark - Init
+ (instancetype)requestWithURL:(NSURL *)url;

#pragma mark - Queue scheduling support
- (void)prepareForSchedulingInQueue:(OCConnectionQueue *)queue; //!< Called directly before scheduling of a request begins.
- (NSMutableURLRequest *)generateURLRequestForQueue:(OCConnectionQueue *)queue; //!< Returns an NSURLRequest for this request.

#pragma mark - Cancel support
- (void)cancel;

#pragma mark - Access
- (NSString *)valueForParameter:(NSString *)parameter;
- (void)setValue:(NSString *)value forParameter:(NSString *)parameter;

- (void)addParameters:(NSDictionary<NSString*,NSString*> *)parameters;

- (NSString *)valueForHeaderField:(NSString *)headerField;
- (void)setValue:(NSString *)value forHeaderField:(NSString *)headerField;

- (void)addHeaderFields:(NSDictionary<NSString*,NSString*> *)headerFields;

#pragma mark - Response
@property(strong,nonatomic) OCHTTPStatus *responseHTTPStatus; //!< HTTP Status delivered with response

- (NSHTTPURLResponse *)response; //!< Convenience accessor for urlSessionTask.response

- (NSURL *)responseRedirectURL; //!< URL contained in the response's Location header field

- (void)appendDataToResponseBody:(NSData *)appendResponseBodyData;

- (NSString *)responseBodyAsString; //!< Returns the response body as a string formatted using the text encoding provided by the server. If no text encoding is provided, ISO-8859-1 is used.

- (NSDictionary *)responseBodyConvertedDictionaryFromJSONWithError:(NSError **)outError; //!< Returns the response body as dictionary as converted by the JSON deserializer
- (NSArray *)responseBodyConvertedArrayFromJSONWithError:(NSError **)error; //!< Returns the response body as array as converted by the JSON deserializer

@end

extern OCConnectionRequestMethod OCConnectionRequestMethodGET;
extern OCConnectionRequestMethod OCConnectionRequestMethodPOST;
extern OCConnectionRequestMethod OCConnectionRequestMethodHEAD;
extern OCConnectionRequestMethod OCConnectionRequestMethodPUT;
extern OCConnectionRequestMethod OCConnectionRequestMethodDELETE;
extern OCConnectionRequestMethod OCConnectionRequestMethodMKCOL;
extern OCConnectionRequestMethod OCConnectionRequestMethodOPTIONS;
extern OCConnectionRequestMethod OCConnectionRequestMethodMOVE;
extern OCConnectionRequestMethod OCConnectionRequestMethodCOPY;
extern OCConnectionRequestMethod OCConnectionRequestMethodPROPFIND;
extern OCConnectionRequestMethod OCConnectionRequestMethodPROPPATCH;
extern OCConnectionRequestMethod OCConnectionRequestMethodLOCK;
extern OCConnectionRequestMethod OCConnectionRequestMethodUNLOCK;

