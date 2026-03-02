#ifndef TRAYMENU_H
#define TRAYMENU_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>

class TrayMenu : public QMainWindow
{
    Q_OBJECT

    QSystemTrayIcon* tray_icon;
    QMenu* tray_menu;

    QMenu* create_menu();

public:
    explicit TrayMenu(QWidget *parent = nullptr);
    ~TrayMenu();

signals:
};

#endif // TRAYMENU_H
