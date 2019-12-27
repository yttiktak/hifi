//
//  AvatarTraits.h
//  libraries/avatars/src
//
//  Created by Stephen Birarda on 7/30/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AvatarTraits_h
#define hifi_AvatarTraits_h

#include <algorithm>
#include <cstdint>
#include <array>
#include <vector>

#include <QtCore/QUuid>

class ExtendedIODevice;
class AvatarData;

namespace AvatarTraits {
    enum TraitType : int8_t {
        // Null trait
        NullTrait = -1,

        // Simple traits
        SkeletonModelURL = 0,
        SkeletonData,
        // Instanced traits
        FirstInstancedTrait,
        AvatarEntity = FirstInstancedTrait,
        Grab,

        // Traits count
        TotalTraitTypes
    };

    const int NUM_SIMPLE_TRAITS = (int)FirstInstancedTrait;
    const int NUM_INSTANCED_TRAITS = (int)TotalTraitTypes - (int)FirstInstancedTrait;
    const int NUM_TRAITS = (int)TotalTraitTypes;

    using TraitInstanceID = QUuid;

    inline bool isSimpleTrait(TraitType traitType) {
        return traitType > NullTrait && traitType < FirstInstancedTrait;
    }

    using TraitVersion = int32_t;
    const TraitVersion DEFAULT_TRAIT_VERSION = 0;
    const TraitVersion NULL_TRAIT_VERSION = -1;

    using TraitWireSize = int16_t;
    const TraitWireSize DELETED_TRAIT_SIZE = -1;
    const TraitWireSize MAXIMUM_TRAIT_SIZE = INT16_MAX;

    using TraitMessageSequence = int64_t;
    const TraitMessageSequence FIRST_TRAIT_SEQUENCE = 0;
    const TraitMessageSequence MAX_TRAIT_SEQUENCE = INT64_MAX;

    qint64 packTrait(TraitType traitType, ExtendedIODevice& destination, const AvatarData& avatar);
    qint64 packVersionedTrait(TraitType traitType, ExtendedIODevice& destination,
                              TraitVersion traitVersion, const AvatarData& avatar);

    qint64 packTraitInstance(TraitType traitType, TraitInstanceID traitInstanceID,
                             ExtendedIODevice& destination, AvatarData& avatar);
    qint64 packVersionedTraitInstance(TraitType traitType, TraitInstanceID traitInstanceID,
                                      ExtendedIODevice& destination, TraitVersion traitVersion,
                                      AvatarData& avatar);

    qint64 packInstancedTraitDelete(TraitType traitType, TraitInstanceID instanceID, ExtendedIODevice& destination,
                                           TraitVersion traitVersion = NULL_TRAIT_VERSION);

};

#endif // hifi_AvatarTraits_h
