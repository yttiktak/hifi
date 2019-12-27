//
//  AssetResourceRequest.h
//  libraries/networking/src
//
//  Created by Ryan Huffman on 2015/07/23
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AssetResourceRequest_h
#define hifi_AssetResourceRequest_h

#include <QUrl>

#include <PortableHighResolutionClock.h>

#include "AssetRequest.h"
#include "ResourceRequest.h"

class AssetResourceRequest : public ResourceRequest {
    Q_OBJECT
public:
    AssetResourceRequest(
        const QUrl& url,
        const bool isObservable = true,
        const qint64 callerId = -1,
        const QString& extra = "");
    virtual ~AssetResourceRequest() override;

protected:
    virtual void doSend() override;

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    static bool urlIsAssetHash(const QUrl& url);

    void requestMappingForPath(const AssetUtils::AssetPath& path);
    void requestHash(const AssetUtils::AssetHash& hash);

    GetMappingRequest* _assetMappingRequest { nullptr };
    AssetRequest* _assetRequest { nullptr };

    p_high_resolution_clock::time_point _lastProgressDebug;

    int64_t _lastRecordedBytesDownloaded { 0 };
};

#endif
