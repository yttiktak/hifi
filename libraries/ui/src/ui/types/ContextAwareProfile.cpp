//
//  FileTypeProfile.cpp
//  interface/src/networking
//
//  Created by Kunal Gosar on 2017-03-10.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ContextAwareProfile.h"

#include <cassert>
#include <QtCore/QThread>
#include <QtQml/QQmlContext>

#include <shared/QtHelpers.h>
#include <SharedUtil.h>

static const QString RESTRICTED_FLAG_PROPERTY = "RestrictFileAccess";

ContextAwareProfile::ContextAwareProfile(QQmlContext* context) :
    ContextAwareProfileParent(context), _context(context) { 
    assert(context);
}


void ContextAwareProfile::restrictContext(QQmlContext* context, bool restrict) {
    context->setContextProperty(RESTRICTED_FLAG_PROPERTY, restrict);
}

bool ContextAwareProfile::isRestrictedInternal() {
    if (QThread::currentThread() != thread()) {
        bool restrictedResult = false;
        BLOCKING_INVOKE_METHOD(this, "isRestrictedInternal", Q_RETURN_ARG(bool, restrictedResult));
        return restrictedResult;
    }

    QVariant variant = _context->contextProperty(RESTRICTED_FLAG_PROPERTY);
    if (variant.isValid()) {
        return variant.toBool();
    }

    // BUGZ-1365 - we MUST defalut to restricted mode in the absence of a flag, or it's too easy for someone to make 
    // a new mechanism for loading web content that fails to restrict access to local files
    return true;
}

bool ContextAwareProfile::isRestricted() {
    auto now = usecTimestampNow();
    if (now > _cacheExpiry) {
        _cachedValue = isRestrictedInternal();
        _cacheExpiry = now + MAX_CACHE_AGE;
    }
    return _cachedValue;
}
