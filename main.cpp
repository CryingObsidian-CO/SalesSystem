#include <QApplication>
#include "mainwindow.h"
#include "sqlite/database.h"
#include <iostream>

int main(int argc, char* argv[])
{
    // 初始化数据库
    if (!init_db())
    {
        std::cerr << "数据库初始化失败\n";
        return 1;
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
