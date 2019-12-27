//
//  AWSInterface.h
//
//  Created by Nissim Hadar on 3 Oct 2018.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AWSInterface_h
#define hifi_AWSInterface_h

#include <QCheckBox>
#include <QLineEdit>
#include <QObject>
#include <QRadioButton>
#include <QTextStream>

#include "BusyWindow.h"

#include "PythonInterface.h"

class AWSInterface : public QObject {
    Q_OBJECT
public:
    explicit AWSInterface(QObject* parent = 0);

    void createWebPageFromResults(const QString& testResults,
                                  const QString& workingDirectory,
                                  QCheckBox* updateAWSCheckBox,
                                  QRadioButton* diffImageRadioButton,
                                  QRadioButton* ssimImageRadionButton,
                                  QLineEdit* urlLineEdit,
                                  const QString& branch,
                                  const QString& user
    );

    void extractTestFailuresFromZippedFolder(const QString& folderName);
    void createHTMLFile();

    void startHTMLpage(QTextStream& stream);
    void writeHead(QTextStream& stream);
    void writeBody(QTextStream& stream);
    void finishHTMLpage(QTextStream& stream);

    void writeTitle(QTextStream& stream, const QStringList& originalNamesFailures, const QStringList& originalNamesSuccesses);
    void writeTable(QTextStream& stream, const QStringList& originalNamesFailures, const QStringList& originalNamesSuccesses);
    void openTable(QTextStream& stream, const QString& testResult, const bool isFailure);
    void closeTable(QTextStream& stream);

    void createEntry(const int index, const QString& testResult, QTextStream& stream, const bool isFailure);

    void updateAWS();

private:
    QString _testResults;
    QString _workingDirectory;
    QString _resultsFolder;
    QString _htmlFailuresFolder;
    QString _htmlSuccessesFolder;
    QString _htmlFilename;

    const QString FAILURES_FOLDER{ "failures" };
    const QString SUCCESSES_FOLDER{ "successes" };
    const QString HTML_FILENAME{ "TestResults.html" };

    BusyWindow _busyWindow;

    PythonInterface* _pythonInterface;
    QString _pythonCommand;

    QString AWS_BUCKET{ "hifi-qa" };

    QLineEdit* _urlLineEdit;
    QString _user;
    QString _branch;

    QString _comparisonImageFilename;

    int _numberOfFailures;
    int _numberOfSuccesses;
};

#endif  // hifi_AWSInterface_h