#include "datamanager.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QQmlContext *main_root_ctx = engine.rootContext();

    DataManager dataman;

    main_root_ctx->setContextProperty("data_manager", &dataman);

    // Terminate on app engine failure
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    
    engine.loadFromModule("hw_overlay", "Overlay");
    engine.loadFromModule("hw_overlay", "TrayButton");

    return app.exec();
}
