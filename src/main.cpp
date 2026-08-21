#include "controller/AppController.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("SLF-EWN"));
    app.setApplicationName(QStringLiteral("SLF-EWN"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setApplicationDisplayName(QStringLiteral("SLF-EWN  ·  Subsurface Early Warning"));

    QQuickStyle::setStyle(QStringLiteral("Basic"));

    Slf::AppController controller;

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/"));
    engine.rootContext()->setContextProperty(QStringLiteral("App"), &controller);
    engine.rootContext()->setContextProperty(QStringLiteral("Auth"), controller.auth());

    const QUrl url(QStringLiteral("qrc:/SlfEwn/Main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
