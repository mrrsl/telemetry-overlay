#include "traymenu.h"

#include <QAction>

TrayMenu::TrayMenu(QWidget *parent):
    QMainWindow{parent},
    tray_icon(new QSystemTrayIcon(this))
{
    tray_menu = new QMenu(parent);
}

TrayMenu::~TrayMenu() {
    delete tray_icon;
    delete tray_menu;
}

QMenu* TrayMenu::create_menu() {

}
