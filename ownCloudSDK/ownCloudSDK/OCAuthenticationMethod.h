//
//  OCAuthenticationMethod.h
//  ownCloudSDK
//
//  Created by Felix Schwarz on 05.02.18.
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

@class OCConnection;
@class OCConnectionRequest;

typedef NSString* OCAuthenticationMethodIdentifier; //!< NSString identifier for an authentication method, f.ex. "owncloud.oauth2" for OAuth2
typedef NSString* OCAuthenticationMethodKey NS_TYPED_ENUM; //!< NSString key used in the options dictionary used to generate the authentication data for a bookmark.
typedef NSDictionary<OCAuthenticationMethodKey,id>* OCAuthenticationMethodBookmarkAuthenticationDataGenerationOptions; //!< Dictionary with options used to generate the authentication data for a bookmark. F.ex. passwords or the view controller to

typedef void(^OCAuthenticationMethodAuthenticationCompletionHandler)(NSError *error);

typedef NS_ENUM(NSUInteger, OCAuthenticationMethodType)
{
	OCAuthenticationMethodTypePassphrase,	//!< Authentication method is password based (=> UI should show username and password entry field)
	OCAuthenticationMethodTypeToken		//!< Authentication method is token based (=> UI should show no username and password entry field)
};

@interface OCAuthenticationMethod : NSObject
{
	@private
	id _cachedAuthenticationSecret;
}

#pragma mark - Registration
+ (void)registerAuthenticationMethodClass:(Class)authenticationMethodClass; //!< Add an authentication method to the core
+ (void)unregisterAuthenticationMethodClass:(Class)authenticationMethodClass; //!< Add an authentication method to the core
+ (NSArray <Class> *)registeredAuthenticationMethodClasses; //!< Array of registered authentication method classes
+ (Class)registeredAuthenticationMethodForIdentifier:(OCAuthenticationMethodIdentifier)identifier; //!< Returns the OCAuthenticationMethod class for identifier

#pragma mark - Identification
+ (OCAuthenticationMethodType)type;
+ (OCAuthenticationMethodIdentifier)identifier;

#pragma mark - Passphrase-based Authentication Only
+ (BOOL)usesUserName; //!< This authentication method uses a user name
+ (NSString *)userNameFromAuthenticationData:(NSData *)authenticationData; //!< Returns the user name stored inside authenticationData

#pragma mark - Authentication / Deauthentication ("Login / Logout")
- (void)authenticateConnection:(OCConnection *)connection withCompletionHandler:(OCAuthenticationMethodAuthenticationCompletionHandler)completionHandler; //!< Authenticates the connection.
- (void)deauthenticateConnection:(OCConnection *)connection withCompletionHandler:(OCAuthenticationMethodAuthenticationCompletionHandler)completionHandler; //!< Deauthenticates the connection.

- (OCConnectionRequest *)authorizeRequest:(OCConnectionRequest *)request forConnection:(OCConnection *)connection; //!< Applies all necessary modifications to a request so that it authorized against using this authentication method. This can be adding tokens, passwords, etc. to the headers. The request returned by this method is sent.

#pragma mark - Generate bookmark authentication data
- (void)generateBookmarkAuthenticationDataWithConnection:(OCConnection *)connection options:(OCAuthenticationMethodBookmarkAuthenticationDataGenerationOptions)options completionHandler:(void(^)(NSError *error, OCAuthenticationMethodIdentifier authenticationMethodIdentifier, NSData *authenticationData))completionHandler; //!< Generates the authenticationData for a connection's bookmark and returns the result via the completionHandler. It is not directly stored in the bookmark so that an app can decide on its own when to overwrite existing data - or save the result. The authentication method is obligated to return an error in the completionHandler if authentication is not possible (f.ex. rejected token request, wrong username/passphrase).

#pragma mark - Authentication Secret Caching
- (id)cachedAuthenticationSecretForConnection:(OCConnection *)connection; //!< Method that allows an authentication method to cache a secret in memory. If none is present in memory, -loadCachedAuthenticationSecretForConnection: is called.
- (id)loadCachedAuthenticationSecretForConnection:(OCConnection *)connection; //!< Called by -cachedAuthenticationSecretForConnection: if no authentication secret is stored in memory. Should retrieve and return the authentication secret for the connection.
- (void)flushCachedAuthenticationSecret; //!< Flushes the cached authentication secret. Called f.ex. if the device is locked or the user switches to another app.

@end

extern OCAuthenticationMethodKey OCAuthenticationMethodUsernameKey; //!< For passphrase-based authentication methods: the user name (value type: NSString*)
extern OCAuthenticationMethodKey OCAuthenticationMethodPassphraseKey; //!< For passphrase-based authentication methods: the passphrase (value type: NSString*)
extern OCAuthenticationMethodKey OCAuthenticationMethodPresentingViewControllerKey; //!< The UIViewController to use when presenting a view controller (for f.ex. token-based authentication mechanisms like OAuth2) (value type: UIViewController*)

#define OCAuthenticationMethodAutoRegister +(void)load{ \
						[OCAuthenticationMethod registerAuthenticationMethodClass:self]; \
				       	   }

