"use strict";
//
//  audio.js
//  scripts/system/
//
//  Created by Gabriel Calero & Cristian Duarte on Jan 16, 2018
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(function() { // BEGIN LOCAL_SCOPE

var audiobar;
var audioButton;

var logEnabled = false;

function printd(str) {
    if (logEnabled)
        print("[audio.js] " + str);
}

function init() {
    audiobar = new QmlFragment({
        qml: "hifi/AudioBar.qml"
    });

    audioButton = audiobar.addButton({
        icon: "icons/mic-unmute-a.svg",
        activeIcon: "icons/mic-mute-a.svg",
        text: "",
        bgOpacity: 0.0,
        hoverBgOpacity: 0.0,
        activeHoverBgOpacity: 0.0,
        activeBgOpacity: 0.0
    });

    onMuteToggled();

    audioButton.clicked.connect(onMuteClicked);
    audioButton.entered.connect(onMutePressed);
    Audio.mutedChanged.connect(onMuteToggled);
}

function onMuteClicked() {
    Audio.muted = !Audio.muted;
}

function onMutePressed() {
    Controller.triggerHapticPulseOnDevice(Controller.findDevice("TouchscreenVirtualPad"), 0.1, 40.0, 0);
}

function onMuteToggled() {
    printd("On Mute Toggled");
    audioButton.isActive = Audio.muted; // Menu.isOptionChecked("Mute Microphone")
    printd("Audio button is active: " + audioButton.isActive);
}

Script.scriptEnding.connect(function () {
    if(audioButton) {
        audioButton.clicked.disconnect(onMuteClicked);
        audioButton.entered.disconnect(onMutePressed);
        Audio.mutedChanged.connect(onMuteToggled);
    }
});

init();

}()); // END LOCAL_SCOPE
