//
//  BlendshapeConstants.h
//
//
//  Created by Clement on 1/23/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_BlendshapeConstants_h
#define hifi_BlendshapeConstants_h

/// The names of the blendshapes expected by Faceshift, terminated with an empty string.
extern const char* FACESHIFT_BLENDSHAPES[];

enum class Blendshapes : int {
    EyeBlink_L = 0,
    EyeBlink_R,
    EyeSquint_L,
    EyeSquint_R,
    EyeDown_L,
    EyeDown_R,
    EyeIn_L,
    EyeIn_R,
    EyeOpen_L,
    EyeOpen_R,
    EyeOut_L,
    EyeOut_R,
    EyeUp_L,
    EyeUp_R,
    BrowsD_L,
    BrowsD_R,
    BrowsU_C,
    BrowsU_L,
    BrowsU_R,
    JawFwd,
    JawLeft,
    JawOpen,
    JawRight,
    MouthLeft,
    MouthRight,
    MouthFrown_L,
    MouthFrown_R,
    MouthSmile_L,
    MouthSmile_R,
    MouthDimple_L,
    MouthDimple_R,
    LipsStretch_L,
    LipsStretch_R,
    LipsUpperClose,
    LipsLowerClose,
    LipsFunnel,
    LipsPucker,
    Puff,
    CheekSquint_L,
    CheekSquint_R,
    MouthClose,
    MouthUpperUp_L,
    MouthUpperUp_R,
    MouthLowerDown_L,
    MouthLowerDown_R,
    MouthPress_L,
    MouthPress_R,
    MouthShrugLower,
    MouthShrugUpper,
    NoseSneer_L,
    NoseSneer_R,
    TongueOut,
    UserBlendshape0,
    UserBlendshape1,
    UserBlendshape2,
    UserBlendshape3,
    UserBlendshape4,
    UserBlendshape5,
    UserBlendshape6,
    UserBlendshape7,
    UserBlendshape8,
    UserBlendshape9,
    BlendshapeCount
};

enum class LegacyBlendshpaes : int {
    JawChew,  // not in ARKit
    LipsUpperUp, // split in ARKit
    LipsLowerDown, // split in ARKit
    ChinLowerRaise, // not in ARKit
    ChinUpperRaise, // not in ARKit
    Sneer, // split in ARKit
    LegacyBlendshapeCount
};

// NEW in ARKit
// * MouthClose
// * MouthUpperUp_L
// * MouthUpperUp_R
// * MouthLowerDown_L
// * MouthLowerDown_R
// * MouthPress_L
// * MouthPress_R
// * MouthShrugLower
// * MouthShrugUpper
// * NoseSneer_L
// * NoseSneer_R
// * TongueOut

// Legacy shapes
// * JawChew (not in ARKit)
// * LipsUpperUp (split in ARKit)
// * LipsLowerDown (split in ARKit)
// * Sneer (split in ARKit)
// * ChinLowerRaise (not in ARKit)
// * ChinUpperRaise (not in ARKit)
// * LipsUpperOpen (not in ARKit)
// * LipsLowerOpen (not in ARKit)


#endif // hifi_BlendshapeConstants_h
