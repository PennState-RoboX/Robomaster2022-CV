#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    QLocale locale = QLocale::system();

    if( locale.language() == QLocale::Chinese )
    {
        //ch:中文语言环境加载默认设计界面 | en:The Chinese language environment load the default design
    }
    else
    {
        //ch:其他语言环境加载英文界面 | en:Other language environments load the English design
        translator.load(QString(":/BasicDemoByGenTL_zh_EN.qm")); //ch:选择翻译文件 | en:Choose the translation file
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
