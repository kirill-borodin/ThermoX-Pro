#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
//#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTranslator;

    if(qtTranslator.load(QLocale::system(),"qt","_",QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    {
        //qDebug() << "translator ok";
        a.installTranslator(&qtTranslator);
    }

    QTranslator qtBaseTranslator;
    if(qtBaseTranslator.load("qtbase_"+QLocale::system().name(),QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    {
        //qDebug() << "basetranslator ok";
        a.installTranslator(&qtBaseTranslator);
    }

    a.setApplicationName("ThermoX-Pro");
    a.setApplicationVersion(QString("%1.%2").arg(MAJOR_VERSION).arg(MINOR_VERSION));
    MainWindow w;
    //QScrollArea sa;
    //QWidget* pwgt = new QWidget;
    //QWidget* wgt = new QWidget;
    //QPixmap pix ("bur_gamma0.png") ;
    //QPalette pal;
    //pal.setBrush(pwgt->backgroundRole(), QBrush(pix) );
    //pwgt->setPalette(pal);
    //pwgt->setAutoFillBackground(true);
    //pwgt->setFixedSize(pix.width(), pix.height() ) ;
    //sa.setWidget(wgt);
    //sa.resize(400, 300);
    //sa.show();
    //w.setCentralWidget(wgt);
    //w.setStyleSheet("background-color: rgb(100,100,100)");

    //QScrollArea sa;
   /* QWidget pwgt = new QWidget(this);
    QPixmap pix ("bur_gamma0.png") ;
    QPalette pal;
    pal.setBrush(pwgt->backgroundRole(), QBrush(pix) );
    pwgt->setPalette(pal);
    pwgt->setAutoFillBackground(true);
    pwgt->setFixedSize(pix.width(), pix.height() ) ;
    ui->scrollArea->setWidget(pwgt);*/
    //sa.resize(400, 300);
    //sa.show();

    w.show();

    return a.exec();
}
