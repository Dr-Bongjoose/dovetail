// Dovetail — main.cpp. Slice-1: launch, show the QML shell.
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName(QStringLiteral("Dovetail"));
    QGuiApplication::setApplicationName(QStringLiteral("Dovetail"));
    QGuiApplication::setApplicationVersion(QStringLiteral("0.0.1"));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("Dovetail", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;

    return QGuiApplication::exec();
}