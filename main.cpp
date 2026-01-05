#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "datamanager.hpp"

void app_terminate(QGuiApplication& app, QQmlApplicationEngine& engine) {
    app.quit();
    QCoreApplication::exit();
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    DataManager dman;

    engine.rootContext()->setContextProperty("DataManager", &dman);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("hmon", "Main");

    return app.exec();
}
