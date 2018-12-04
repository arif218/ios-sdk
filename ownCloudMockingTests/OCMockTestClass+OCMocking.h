//
//  OCMockTestClass+OCMocking.h
//  ownCloudMockingTests
//
//  Created by Felix Schwarz on 11.07.18.
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

#import "OCMockTestClass.h"
#import "OCMockManager.h"
#import "OCMockTestClass.h"

@interface OCMockTestClass (OCMocking)

+ (BOOL)ocm_returnsTrue;

- (BOOL)ocm_returnsFalse;

@end

typedef BOOL(^OCMockMockTestClassReturnsTrueBlock)(void);
extern OCMockLocation OCMockLocationMockTestClassReturnsTrue;

typedef BOOL(^OCMockMockTestClassReturnsFalseBlock)(void);
extern OCMockLocation OCMockLocationMockTestClassReturnsFalse;
