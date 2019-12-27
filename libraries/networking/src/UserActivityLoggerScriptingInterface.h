//
//  UserActivityLoggerScriptingInterface.h
//  libraries/networking/src
//
//  Created by Ryan Huffman on 6/06/16.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_UserActivityLoggerScriptingInterface_h
#define hifi_UserActivityLoggerScriptingInterface_h

#include <QObject>
#include <QJsonObject>

#include <DependencyManager.h>

class UserActivityLoggerScriptingInterface : public QObject, public Dependency {
    Q_OBJECT
public:
    Q_INVOKABLE void enabledEdit();
    Q_INVOKABLE void openedTablet(bool visibleToOthers);
    Q_INVOKABLE void closedTablet();
    Q_INVOKABLE void openedMarketplace();
    Q_INVOKABLE void toggledAway(bool isAway);
    Q_INVOKABLE void tutorialProgress(QString stepName, int stepNumber, float secondsToComplete,
        float tutorialElapsedTime, QString tutorialRunID = "", int tutorialVersion = 0, QString controllerType = "");
    Q_INVOKABLE void palAction(QString action, QString target);
    Q_INVOKABLE void palOpened(float secondsOpen);
    Q_INVOKABLE void makeUserConnection(QString otherUser, bool success, QString details = "");
    Q_INVOKABLE void privacyShieldToggled(bool newValue);
    Q_INVOKABLE void privacyShieldActivated();
    Q_INVOKABLE void logAction(QString action, QVariantMap details = QVariantMap{});
    Q_INVOKABLE void commercePurchaseSuccess(QString marketplaceID, QString contentCreator, int cost, bool firstPurchaseOfThisItem);
    Q_INVOKABLE void commercePurchaseFailure(QString marketplaceID, QString contentCreator, int cost, bool firstPurchaseOfThisItem, QString errorDetails);
    Q_INVOKABLE void commerceEntityRezzed(QString marketplaceID, QString source, QString type);
    Q_INVOKABLE void commerceWalletSetupStarted(int timestamp, QString setupAttemptID, int setupFlowVersion, QString referrer, QString currentDomain);
    Q_INVOKABLE void commerceWalletSetupProgress(int timestamp, QString setupAttemptID, int secondsElapsed, int currentStepNumber, QString currentStepName);
    Q_INVOKABLE void commerceWalletSetupFinished(int timestamp, QString setupAttemptID, int secondsToComplete);
    Q_INVOKABLE void commercePassphraseEntry(QString source);
    Q_INVOKABLE void commercePassphraseAuthenticationStatus(QString status);
private:
    void doLogAction(QString action, QJsonObject details = {});
};

#endif // hifi_UserActivityLoggerScriptingInterface_h
