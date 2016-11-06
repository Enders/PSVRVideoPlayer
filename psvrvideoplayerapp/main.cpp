#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QPixmap>>
#include <QCursor>

#include <QtQml>
#include <QWindow>
#include <QScreen>

#include <QDebug>

#include "qmlpsvr.h"
#include "psvrsensorsmanager.h"

void forceFullScreen(const QGuiApplication &app) {
    QWindow* window = app.allWindows().at(0);
    window->setVisibility(QWindow::Visibility::FullScreen);
}

void hideCursor(const QGuiApplication &app) {
    QPixmap nullCursor(16, 16);
    nullCursor.fill(Qt::transparent);
    app.setOverrideCursor(QCursor(nullCursor));
}

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(("PSVRVideoPlayer"));
    QGuiApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("Simple CLI PSVR Video Player");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("filename", "Path to the video file");
    QCommandLineOption ipdOption(QStringList() << "i" << "ipd", "IPD in mm, 40 to 80 (default to 64mm)", "ipd", "64");
    parser.addOption(ipdOption);
    QCommandLineOption formatOption(QStringList() << "f" << "format", "Video format: 180sbs, 360sbs (default to 360 sbs)", "format", "360sbs");
    parser.addOption(formatOption);

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.size() != 1) {
        fprintf(stderr, "%s\n", qPrintable("Error: must specify only one file argument."));
        parser.showHelp(1);
    }
    QString filename = args.at(0);

    // Convert to absolute path for QML Video player
    QFileInfo info(filename);
    if (info.isRelative()) {
        filename = info.absoluteFilePath();
    }

    int ipd = parser.value(ipdOption).toInt();
    if (ipd < 40 || ipd > 80) {
       fprintf(stderr, "%s\n", qPrintable("Error: specified IPD is invalid."));
       parser.showHelp(1);
    }

    QString format = parser.value(formatOption);
    if (!(QStringList() << "180sbs" << "360sbs").contains(format)){
        fprintf(stderr, "%s\n", qPrintable("Error: unknown specified format."));
        parser.showHelp(1);
    }

    qmlRegisterType<QMLPSVR>("QMLPSVR", 0, 1, "QMLPSVR");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("filename", filename);
    engine.rootContext()->setContextProperty("videoType", format);
    engine.rootContext()->setContextProperty("eyeSeparation", (float) ipd / 1000.0f);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    forceFullScreen(app);
    hideCursor(app);

    return app.exec();
}
