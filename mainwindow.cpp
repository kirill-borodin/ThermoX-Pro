#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sliderwidget.h"
#include "doublesliderwidget.h"
#include "doublesliderwidgetvert.h"
#include <sstream>
#include <string>
#include <iostream>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QtWidgets>
#include <QFrame>
#include <QGraphicsSceneMouseEvent>
#include "qcustomplot.h"
#include <math.h>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
  , cutUp(0)
  , cutDown(0)
  , cutLeft(0)
  , cutRight(0)
  ,  ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    hWid = new DoubleSliderWidget(this);
    vWid = new DoubleSliderWidgetVert(this);
    hWid->setVisible(false);
    vWid->setVisible(false);
    connect(ui->action_4,SIGNAL(triggered()),this,SLOT(slotAboutProgram()),Qt::UniqueConnection);
    connect(ui->action,SIGNAL(triggered()),this,SLOT(on_action_triggered()),Qt::UniqueConnection);
    connect(hWid,SIGNAL(valueChangedLeft(int)),this,SLOT(on_valueChangedLeft()),Qt::UniqueConnection);
    connect(hWid,SIGNAL(valueChangedRight(int)),this,SLOT(on_valueChangedRight()),Qt::UniqueConnection);
    connect(vWid,SIGNAL(valueChangedUp(int)),this,SLOT(on_valueChangedUp()),Qt::UniqueConnection);
    connect(vWid,SIGNAL(valueChangedDown(int)),this,SLOT(on_valueChangedDown()),Qt::UniqueConnection);
    connect(ui->facilityBox,SIGNAL(currentIndexChanged(int)),this,SLOT(on_facilityName_changed()));
    connect(ui->methodsBox,SIGNAL(currentIndexChanged(int)),this,SLOT(redrawPlots()));
    ui->line->setVisible(false);
    ui->line_2->setVisible(false);
    ui->line_3->setVisible(false);
    ui->line_4->setVisible(false);
    ui->Play->setVisible(false);
    ui->scrollArea->setVisible(false);
    plotsWidget = new QWidget;
    plotsLayout = new QGridLayout;
    QStringList facilityNames;
    facilityNames << "ПГУ-11" << "У-306-3";
    ui->facilityBox->addItems(facilityNames);
    ui->facilityBox->setVisible(false);
    ui->methodsBox->setVisible(false);
    ui->labelFacility->setVisible(false);
    ui->labelMethod->setVisible(false);
    T00 = 483.0;
    alpha0 = 0.6;
    polyPower = 5;
    settingsDialog = new QDialog(this);
    flowParamsDialog = new QDialog(this);
    rhoEdit = new QLineEdit(settingsDialog);
    CpEdit = new QLineEdit(settingsDialog);
    lambdaEdit = new QLineEdit(settingsDialog);
    rhoLabel = new QLabel(settingsDialog);
    CpLabel = new QLabel(settingsDialog);
    lambdaLabel = new QLabel(settingsDialog);
    T00Label = new QLabel(flowParamsDialog);
    alpha0Label = new QLabel(flowParamsDialog);
    T00Edit = new QLineEdit(flowParamsDialog);
    alpha0Edit = new QLineEdit(flowParamsDialog);
    rhoEdit->setText("1745");
    CpEdit->setText("1330");
    lambdaEdit->setText("0.52");
    T00Edit->setText("483.0");
    alpha0Edit->setText("0.6");
    rho = 1745.0;
    Cp = 1330.0;
    lambda = 0.52;
    kCoef = rho*Cp*lambda;
    qCalcCoef = 2*sqrt(kCoef/M_PI);
    plotsWidget2 = new QWidget;
    plotsWidget3 = new QWidget;
    Tinit = 11.0;
    deltaTau = 5.0;
    grScene = new QGraphicsScene(ui->graphicsView);
    rangeLabel = new QLabel(this);
    rangeLabel->setVisible(false);
}

MainWindow::~MainWindow()
{
    for(auto &obj: plotsList)
        delete obj;

    if(!Tfield.empty())
    {
        delete [] Tdata;
        delete plotsWidget;
        delete plotsLayout;
        delete hWid;
        delete vWid;
        delete plotsWidget2;
        delete plotsWidget3;
        delete grScene;
        delete rangeLabel;
        delete flowParamsDialog;
        delete settingsDialog;
        delete [] TFieldERFC;
        delete [] tau0;
    }

    delete ui;
}

void MainWindow::on_action_triggered()
{
    QString strFilter;
    MainWindow::openFileName = QFileDialog::getOpenFileName(0,tr("Open file"),"*.asc",strFilter);
    if(!openFileName.isEmpty())
        getInitialInfoFromFile();
}

void MainWindow::on_action_3_triggered()
{
    int result = QMessageBox::question(this,tr("Выход из программы" ), QString(tr("Вы уверены, что хотите выйти?")), QMessageBox::Yes, QMessageBox::No);

    if(QMessageBox::Yes == result)
        close();
    else
        return;
}

void MainWindow::slotAboutProgram()
{
    QMessageBox::about(this,"О программе",QString("%1 v.%2 \n\n"
    "Данная программа разработана для обработки экспериментальных данных "
    "по теплообмену моделей в установках ЦНИИмаш при воздействии на них "
    "сверхзвукового течения с помощью поля температур, записанного тепловизором."
    "\n\nВ текущей версии существует возможность работы "
    "с переменным шагом по времени, переменной температурой "
    "торможения и коэффициентом теплообмена.").arg(qApp->applicationName()).arg(qApp->applicationVersion()));
}

void MainWindow::getInitialInfoFromFile()
{
    QFile file;
    QTime time0, time1;
    QString str;
    QStringList lst;
    int i = 0, j = 0, k = 0, l = 0;
    float min = 1000.0, max = 0.0;
    float tmp = 0.0;
    file.setFileName(openFileName);

    if(file.open(QIODevice::ReadOnly))
    {
        if(!Tfield.empty())
        {
            delete[] Tdata;
            Tfield.clear();
            TfieldSmooth.clear();
            timeArray.clear();
            pointsForPlots.clear();
            plotsList.clear();
            q.clear();
            cutUp = 0;
            cutLeft = 0;
            cutRight = 0;
            cutDown = 0;
            ui->scrollArea->setVisible(false);
            plotsWidget = new QWidget;
            plotsLayout = new QGridLayout;
            ui->facilityBox->setVisible(false);
            ui->methodsBox->setVisible(false);
            ui->labelFacility->setVisible(false);
            ui->labelMethod->setVisible(false);
            ui->processingMethodButton->setEnabled(false);
        }

        for(i=0;i<24;i++)
        {
            str = file.readLine(0);
            str.replace("\t"," ");
            lst = str.split(" ");

            if(lst.at(0) == "Format")
            {
             setLength(lst.at(1).toInt());
             setWidth(lst.at(2).toInt());
            }

            if(lst.at(0) == "Cut" && lst.at(1) == "on")
            {
                lambda1 = lst.at(2).toFloat()*1.0E-6;
            }

            if(lst.at(0) == "Cut" && lst.at(1) == "off")
            {
                lambda2 = lst.at(2).toFloat()*1.0E-6;
            }
        }

        while(!file.atEnd())
        {
            for(i=0;i<5;i++)
            {
                str = file.readLine();
                str.replace(","," ");
                str.replace(":"," ");
                str.replace("\t"," ");
                lst = str.split(" ");

                if(lst.at(0) == "Time")
                {
                    if(k==0)
                    {
                        timeArray.push_back(0.0);
                        time0.setHMS(lst.at(1).toInt(),lst.at(2).toInt(),lst.at(3).toInt(),lst.at(4).toInt());
                    }
                    else
                    {
                        time1.setHMS(lst.at(1).toInt(),lst.at(2).toInt(),lst.at(3).toInt(),lst.at(4).toInt());
                        timeArray.push_back(timeArray.at(k-1) + time0.msecsTo(time1)*0.001);
                        time0 = time1;
                    }
                        k++;
                }
             }
            for(j=0; j<myWidth; j++)
            {
                str = file.readLine();
                str.replace("\t"," ");
                lst = str.split(" ");
                for(l=j*myLength; l<(j+1)*myLength; l++)
                {
                    tmp = lst.at(l-j*(myLength)).toFloat();

                    tmp > 0? Tfield.push_back(tmp) : Tfield.push_back(0.0);
                }
            }
        }
    }

    timeNum = timeArray.length();

    for(k = 0; k < timeNum; k++)
    {
        for(j = 0; j < myWidth; j++)
        {
            for(i = 0; i < myLength; i++)
            {
                if(Tfield[i+(j+k*myWidth)*myLength] <= min)
                    min = Tfield[i+(j+k*myWidth)*myLength];
                if(Tfield[i+(j+k*myWidth)*myLength] >= max)
                    max = Tfield[i+(j+k*myWidth)*myLength];
            }
        }
    }

    Tmin = min;
    Tmax = max;
    ui->horizontalSlider->setEnabled(true);
    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(timeNum-1);

    createCharData();
    ui->graphicsView->setFrameStyle(QFrame::StyledPanel);
    ui->graphicsView->setFixedHeight(myWidth+frameThickness);
    ui->graphicsView->setFixedWidth(myLength+frameThickness);
    ui->horizontalSlider->setGeometry(ui->graphicsView->x(),ui->graphicsView->y()+ui->graphicsView->height()+5,ui->graphicsView->width(),ui->horizontalSlider->height());

    displayField(0);
    ui->labelTimeDisplay->setText("Time: 0 s");
    ui->labelNumDisplay->setText("  1/"+QString::number(timeNum));
    ui->labelNumDisplay->setGeometry(ui->graphicsView->x()+115,ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height(),ui->labelNumDisplay->width(),ui->labelNumDisplay->height());
    ui->labelTimeDisplay->setGeometry(ui->graphicsView->x()+158,ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height(),ui->labelTimeDisplay->width(),ui->labelTimeDisplay->height());
    ui->cutButton->setEnabled(true);
    ui->putMarks->setEnabled(true);
    ui->Play->setEnabled(false);
    ui->processingMethodButton->setEnabled(true);
    ui->TminDisplay->setGeometry(ui->graphicsView->x()+ui->graphicsView->width()+25,ui->graphicsView->y()+ui->graphicsView->height()-20,35,35);
    ui->TmaxDisplay->setGeometry(ui->graphicsView->x()+ui->graphicsView->width()+25,ui->graphicsView->y()-15,35,35);
    ui->TmidDisplay->setGeometry(ui->graphicsView->x()+ui->graphicsView->width()+25,ui->graphicsView->y()+(ui->graphicsView->height()-20-15)/2,35,35);
    ui->TminDisplay->setText(QString::number((int)Tmin));
    ui->TmaxDisplay->setText(QString::number((int)Tmax));
    ui->TmidDisplay->setText(QString::number((int)((Tmax+Tmin)/2)));

    displayTRange();

    if(ui->TminDisplay->x() + ui->TminDisplay->width() > MainWindow::width())
        MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),ui->TminDisplay->x() + ui->TminDisplay->width() + 20,MainWindow::height());

    if(ui->labelNumDisplay->y() + ui->labelNumDisplay->height() + 50 > MainWindow::height())
        MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),MainWindow::width(),ui->labelNumDisplay->y() + ui->labelNumDisplay->height() + 50);

    createSmoothField();
}

void MainWindow::writeTemperatureToTecplot()
{
    QFile file;

    std::stringstream str;
    std::string s;
    int i = 0, j = 0, k = 0;

    file.setFileName("T_Result.txt");

    if(file.open(QIODevice::WriteOnly))
    {
        file.write("TITLE=\"T\"");
        file.write(" VARIABLES=\"X\" \"Y\" \"T\"");
        for(k = 0;k<timeNum;k++)
        {
            str.str("");
            str << "\r\nZONE T=\"t=" << timeArray.at(k) << " s\" I=" << myLength-cutLeft-cutRight << " J=" << myWidth-cutUp-cutDown << " F=POINT\n";
            s = str.str();
            file.write(s.c_str());
            file.write("\r\n");

            for(j = cutUp; j < myWidth-cutDown; j++)
            {
                for(i = cutLeft; i < myLength-cutRight; i++)
                {
                    str.str("");
                    str << i+1 << " " << myWidth-j << " " << Tfield[i+(j+k*myWidth)*myLength] << "\r\n";
                    s = str.str();
                    file.write(s.c_str());
                }
            }
        }
    }

    file.close();
}

void MainWindow::createCharData()
{
    int i, j, k, kk=0;
    float deltaColor = 255/(Tmax - Tmin);
    Tdata = new uchar[myWidth*myLength*timeNum];

    for(k = 0; k < timeNum; k++)
    {
        for(j = 0; j < myWidth; j++)
        {
            for(i = 0; i < myLength; i++)
            {
                Tdata[kk] = (int)((Tfield[i+(j+k*myWidth)*myLength] - Tmin)*deltaColor);
                kk++;
            }
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)//Ставим точки на картинке, рисуем графики T(t) и q(t) в этих точках
{
    if (event->x() > ui->graphicsView->x() + 1 && event->x() < ui->graphicsView->x()+ui->graphicsView->width() - 1 && event->y() > ui->graphicsView->y() + 1 && event->y() < ui->graphicsView->y() - 1+ui->graphicsView->height() && (ui->putMarks->text() == "Завершить"))
    {
        QCustomPlot *customPlot = new QCustomPlot(plotsWidget2);
        QCustomPlot *customPlot2 = new QCustomPlot(plotsWidget3);
        QPoint p;
        QPen pen(Qt::red);
        QPen pen2(Qt::blue);
        int viewWidth = 300;
        int viewHeight = 200;
        double min=1000.0, max=-100.0;
        double minq=1000.0, maxq=-100.0;
        QVector<double> x, y, y2, qSet;

        p.setX(event->pos().x()-ui->graphicsView->x()-2+cutLeft);
        p.setY(event->pos().y()-ui->graphicsView->y()-22+cutUp);

        //if(p.x() > 1 && p.x()<myLength -1 - cutLeft - cutRight && p.y() > 1 && p.y() < myWidth-1 - cutUp - cutDown)
        if(p.x() > cutLeft + 1 && p.x()<myLength -1 - cutRight && p.y() > cutUp + 1 && p.y() < myWidth-1 - cutDown)
        {
            pointsForPlots.push_back(p);
            double rad = 1.4;
            ui->graphicsView->scene()->addEllipse(p.x()-cutLeft-rad, p.y()-cutUp-rad, rad*2.0, rad*2.0, QPen(), QBrush(Qt::SolidPattern));

            int i = p.x();
            int j = p.y();

            for (int k=0; k<timeArray.length(); k++)
            {
              x.push_back(timeArray.at(k));
              y.push_back(Tfield[i+(j+k*myWidth)*myLength]);

              if(min >= y.at(k))
                  min = y.at(k);
              if(max <= y.at(k))
                  max = y.at(k);
            }

            customPlot->removeGraph(0);
            customPlot->removeGraph(0);
            customPlot->addGraph();
            customPlot->graph(0)->setData(x, y);
            customPlot->graph(0)->setPen(pen);
            customPlot->graph(0)->setLineStyle(QCPGraph::lsNone);
            customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossSquare, 4));
            std::stringstream str;
            str << "Point №" << (plotsList.size()+1)/2+1;
            customPlot->graph(0)->setName(str.str().c_str());

            switch(ui->methodsBox->currentIndex())
            {
                case(LowPassFlag):
                    str << " LowPass";
                    break;
                case(PolynominalFlag):
                    str << " Poly";
                    break;
                case(ERFCFlag):
                    str << " ERFC";
                    break;
            }

            customPlot->addGraph();

            processField(i,j,ui->methodsBox->currentIndex());

            qCalc(i,j,ui->methodsBox->currentIndex());
            for (int k=0; k<timeArray.size(); k++)
            {
                y2.push_back(TfieldSmooth[i+(j+k*(myWidth))*(myLength)]);
                qSet.push_back(q[i+(j+k*(myWidth))*(myLength)]);
                if(minq >= qSet.at(k))
                    minq = qSet.at(k);
                if(maxq <= qSet.at(k))
                    maxq = qSet.at(k);
            }
            customPlot->graph(1)->setData(x, y2);
            customPlot->graph(1)->setPen(pen2);
            customPlot->graph(1)->setName(str.str().c_str());
            customPlot2->removeGraph(0);
            customPlot2->addGraph();
            customPlot2->graph(0)->setData(x, qSet);
            customPlot2->graph(0)->setPen(pen);
            customPlot2->graph(0)->setName(str.str().c_str());
            customPlot2->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::AlignBottom|Qt::AlignRight));
            customPlot2->legend->setVisible(true);
            customPlot2->xAxis->setLabel("t, с");
            customPlot2->yAxis->setLabel("q, Вт/м2");
            customPlot2->xAxis->setRange(timeArray.first(), timeArray.last());
            customPlot2->yAxis->setRange(minq, maxq);
            customPlot2->setGeometry(10+ (10+viewWidth)*((pointsForPlots.size()-1)%2),10+ (10+viewHeight)*((pointsForPlots.size()-1) - ((pointsForPlots.size()-1)%2))/2,viewWidth,viewHeight);
            customPlot2->replot();

            customPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::AlignBottom|Qt::AlignRight));
            customPlot->legend->setVisible(true);
            customPlot->xAxis->setLabel("t, с");
            customPlot->yAxis->setLabel("T, °C");
            customPlot->xAxis->setRange(timeArray.first(), timeArray.last());
            customPlot->yAxis->setRange(min-5, max+5);
            customPlot->setGeometry(10+ (10+viewWidth)*((pointsForPlots.size()-1)%2),10+ (10+viewHeight)*((pointsForPlots.size()-1) - ((pointsForPlots.size()-1)%2))/2,viewWidth,viewHeight);
            customPlot->replot();
            plotsList.append(customPlot);
            plotsList.append(customPlot2);

            if(plotsList.length() == 2)
            {
                if(ui->graphicsView->y() + ui->graphicsView->height() + ui->horizontalSlider->height() + ui->labelTimeDisplay->height() + 10 > ui->qCalcButton->y()+ui->qCalcButton->height())
                {
                    ui->scrollArea->setGeometry(ui->scrollArea->x(),40+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),2*viewWidth+50,10+viewHeight);
                    ui->facilityBox->setGeometry(ui->scrollArea->x()+ui->scrollArea->width()*22/50,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->facilityBox->width(),ui->facilityBox->height());
                    ui->methodsBox->setGeometry(ui->scrollArea->x()+ui->scrollArea->width()*69/100,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->methodsBox->width(),ui->methodsBox->height());
                    ui->labelFacility->setGeometry(ui->facilityBox->x()-63,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->methodsBox->width(),ui->methodsBox->height());
                    ui->labelMethod->setGeometry(ui->methodsBox->x()-42,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->methodsBox->width(),ui->methodsBox->height());
                    if(ui->TminDisplay->x() + ui->TminDisplay->width() + 15 < plotsWidget->x() + plotsWidget->width())
                        //MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),1.1*(plotsWidget->x() + plotsWidget->width()),40+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height()+2*viewHeight+90);
                        MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),1.1*(plotsWidget->x() + plotsWidget->width()),40+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height()+2*viewHeight+90);
                    else
                        MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),ui->TminDisplay->x() + ui->TminDisplay->width() + 15,40+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height()+2*viewHeight+90);
                }
                else
                {
                    //MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),2.3*viewWidth,ui->qCalcButton->y()+ui->qCalcButton->height()+2*viewHeight+150);
                    if(ui->TminDisplay->x() + ui->TminDisplay->width() + 15 < plotsWidget->x() + plotsWidget->width())
                        //MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),1.1*(plotsWidget->x() + plotsWidget->width()),40+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height()+2*viewHeight+90);
                        MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),1.1*(plotsWidget->x() + plotsWidget->width()),40+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height()+2*viewHeight+90);
                    else
                        MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),ui->TminDisplay->x() + ui->TminDisplay->width() + 15,40+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height()+2*viewHeight+90);
                    ui->scrollArea->setGeometry(20,300,2*viewWidth+50,10+viewHeight);
                    ui->facilityBox->setGeometry(210,260,ui->facilityBox->width(),ui->facilityBox->height());
                    ui->methodsBox->setGeometry(370,260,ui->methodsBox->width(),ui->methodsBox->height());
                    ui->labelFacility->setGeometry(150,260,ui->labelFacility->width(),ui->labelFacility->height());
                    ui->labelMethod->setGeometry(330,260,ui->labelMethod->width(),ui->labelMethod->height());
                }

                ui->facilityBox->setVisible(true);
                ui->methodsBox->setVisible(true);
                ui->labelFacility->setVisible(true);
                ui->labelMethod->setVisible(true);
                ui->scrollArea->setVisible(true);
            }

            plotsList.last()->setMinimumSize(viewWidth,viewHeight);
            plotsList.at(plotsList.size()-2)->setMinimumSize(viewWidth,viewHeight);
            plotsLayout->addWidget(plotsList.at(plotsList.size()-2),(plotsList.size()-1)/2,0);
            plotsLayout->addWidget(plotsList.last(),(plotsList.size()-1)/2,1);

            if(plotsList.size()<5)
                ui->scrollArea->setFixedHeight((10+viewHeight)*((plotsList.size()+1)-(plotsList.size()-1)%2)/2+20);

            plotsWidget->setLayout(plotsLayout);
            ui->scrollArea->setWidget(plotsWidget);
       }
    }
}

void MainWindow::processField(int pointX, int pointY, int methodFlag)//Сглаживаем погрешности температуры разными методами (LowPass - ряд Фурье, обрезание высших гармоник, Polynominal - приближение полиномом, ERFC - приближение готовым решением задачи одномерного прогрева с условием 3 рода)
{
    QVector<double> x, y;
    int i, j, k;
    double temp;
    double A[6][6];//Матрицы и столбцы для поиска коэффициентов полинома методом наименьших квадратов
    double left[6][6];
    double b[6];
    double a[6];

    for (int k=0; k<timeArray.size(); k++)
    {
      x.push_back(timeArray[k]);
      y.push_back(Tfield[pointX+(pointY+k*myWidth)*myLength]);
    }

    switch(methodFlag)
    {

    case(LowPassFlag):
        {
            double lengthFourier = x.last() - x.first();//Максимальная длина волны в разложении по Фурье
            double v0 = (x.size()-1)/2.0/lengthFourier;//Минимальная частота в наборе гармоник
            double kLinear, bLinear;//Приближаем линейно нашу зависимость внутри отрезка [x_i, x_i+1]
            QVector<double> aFourier;

            for(int kFreq = 0; kFreq < 2*(x.length()-1); kFreq++)//Разлагаем в ряд Фурье
            {
                aFourier.push_back(0.0);
                for(int kTime = 0; kTime < x.length()-1; kTime++)
                {
                    if(kFreq == 0)
                    {
                        aFourier[kFreq] = aFourier[kFreq] + (y.at(kTime+1) + y.at(kTime))*(x.at(kTime+1) - x.at(kTime));
                    }
                    else
                    {
                        kLinear = (y.at(kTime+1) - y.at(kTime))/(x.at(kTime+1) - x.at(kTime));
                        bLinear = y.at(kTime) - kLinear*x.at(kTime);
                        aFourier[kFreq] = aFourier[kFreq] + (kLinear*x.at(kTime+1) + bLinear)*sin(M_PI*kFreq*x.at(kTime+1)/lengthFourier) - (kLinear*x.at(kTime) + bLinear)*sin(M_PI*kFreq*x.at(kTime)/lengthFourier) + (cos(M_PI*kFreq*x.at(kTime+1)/lengthFourier) - cos(M_PI*kFreq*x.at(kTime)/lengthFourier))*kLinear*lengthFourier/M_PI/kFreq;
                    }
                }

                if(kFreq == 0)
                    aFourier[kFreq] = aFourier[kFreq]/lengthFourier;
                else
                    aFourier[kFreq] = aFourier[kFreq]*2.0/M_PI/kFreq;
            }

            for (k=0; k<timeArray.length(); k++)//Гасим высшие гармоники
            {
                i = pointX+(pointY+k*(myWidth))*(myLength);
                TfieldSmooth[i] = aFourier.at(0)/2;

                for(int kFreq = 1; kFreq < 2*(x.length()-1); kFreq++)
                    TfieldSmooth[i] = TfieldSmooth[i] + aFourier.at(kFreq)*cos(M_PI*kFreq*x.at(k)/lengthFourier)*exp(-kFreq/lengthFourier/v0*kFreq/lengthFourier/v0/1.1);
            }

            break;
        }
        case(PolynominalFlag):
        {
            //Метод наименьших квадратов для поиска коэффициентов полинома

            for(i = 0; i < polyPower; i++)
            {
                for(j = 0; j < polyPower; j++)
                {
                    temp = 0.0;
                    for (k=0; k<timeArray.size(); k++)
                        //temp = temp + (x.at(k)^((double)i))*(x.at(k)^((double)j));
                        temp = temp + pow(x.at(k),(double)i)*pow(x.at(k),(double)j);
                    A[i][j] = temp;
                }

                b[i] = 0.0;
                for (k=0; k<timeArray.size(); k++)
                    b[i] = b[i] + pow(x.at(k),(double)i)*y[k];
            }

            //Gauss forward marching method

            for (k=0; k<polyPower; k++)
            {
                for (i=k+1; i<polyPower; i++)
                {
                    left[i][k] = A[i][k]/A[k][k];
                    b[i] = b[i] - left[i][k]*b[k];
                }

                for(j = k + 1; j < polyPower; j++)
                    for (i=k+1; i<polyPower; i++)
                        A[j][i] = A[j][i] - left[j][k]*A[k][i];
            }

            //Backward marching

            a[polyPower] = b[polyPower]/A[polyPower][polyPower];

            for(i = polyPower - 1; i > -1; i--)
            {
                a[i] = b[i];
                for(j = i+1; j<polyPower; j++)
                    a[i] = a[i] - A[i][j]*a[j];
                a[i] = a[i]/A[i][i];
            }

            for (k=0; k<timeArray.size(); k++)
            {
                temp = 0.0;
                for(j = 0; j < polyPower; j++)
                    temp = temp + a[j]*pow(x.at(k),(double)j);

                i = pointX+(pointY+k*(myWidth))*(myLength);

                TfieldSmooth[i] = temp;
            }

            break;
        }

        case(ERFCFlag):
        {
            polyPower=2;//Зависимость ERFC похожа на корень, поэтому приблизим сначала им и найдем значение функции и ее производной в начальный момент времени
            y.clear();
            double theta0, theta0der;
            double delta = 10.0;//Иногда целесообразно сместить все вверх на 10-30 градусов
            double alpha;//Альфа и тау и есть параметры, по которым мы приближаем исходную зависимость
            double tau0_;

            for (k=0; k< timeArray.length(); k++)
                y.push_back(pow((Tfield[pointX+(pointY+k*myWidth)*myLength]+delta),2));

            for(i = 0; i < polyPower; i++)
            {
                for(j = 0; j < polyPower; j++)
                {
                    temp = 0.0;
                    for (k=0; k< timeArray.length(); k++)
                        temp = temp + pow(x.at(k),i)*pow(x.at(k),j);
                    A[i][j] = temp;
                }

                for (k=0; k< timeArray.length(); k++)
                    b[i] = 0.0;
                    b[i] = b[i] + pow(x.at(k),i)*y[k];
            }

            qDebug() << "We are here0";

            //Gauss forward marching method

            for (k=0; k<polyPower-1; k++)
            {
                for (i=k+1; i<polyPower; i++)
                {
                    left[i][k] = A[i][k]/A[k][k];
                    b[i] = b[i] - left[i][k]*b[k];
                }

                for(j = k + 1; j < polyPower; j++)
                    for (i=k+1; i<polyPower; i++)
                        A[j][i] = A[j][i] - left[j][k]*A[k][i];
            }

            //Backward marching

            a[polyPower] = b[polyPower]/A[polyPower][polyPower];

            for(i = polyPower - 1; i > -1; i--)
            {
                a[i] = b[i];
                for(j = i+1; j<polyPower; j++)
                    a[i] = a[i] - A[i][j]*a[j];
                a[i] = a[i]/A[i][i];
            }

            qDebug() << "We are here1";

            theta0 = (sqrt(a[0]) - delta - Tinit)/(T00 - Tinit - 273.15);//Наша функция в начальный момент
            theta0der = a[1]/sqrt(a[0])/(T00 - Tinit - 273.15)/2;//Ее производная

            if(theta0 < 0)
                theta0 = -theta0;

            if(theta0der < 0)
                theta0der = -theta0der;

            double deltaGamma = 1.0;//Гаммы нужны для решения уравнения методом деления пополам
            double leftGamma = 1.0e-7;
            double rightGamma = 0.4;
            double middleGamma, sum1, sum2;

            qDebug() << "We are here2";

            while(deltaGamma > 1.0e-4)
            {
                middleGamma = (rightGamma + leftGamma)/2;
                deltaGamma = (rightGamma - leftGamma)/middleGamma;
                sum1 = 1.0 - exp(rightGamma)*erfc(sqrt(rightGamma)) - theta0;
                sum2 = 1.0 - exp(middleGamma)*erfc(sqrt(middleGamma)) - theta0;

                if(sum1*sum2 > 0)
                    rightGamma = middleGamma;
                else
                    leftGamma = middleGamma;
            }

            qDebug() << "We are here3";

            middleGamma = (rightGamma + leftGamma)/2;

            tau0_ = (sqrt(middleGamma/M_PI) - middleGamma*exp(middleGamma)*erfc(sqrt(middleGamma)))/theta0der;
            alpha=sqrt(kCoef*middleGamma/tau0_);

            if(tau0_ > 50*timeArray[1])//Слишком низкий тепловой поток, пропишем дефолтное маленькое значение
            {
                tau0_ = 50*timeArray[1];
                alpha = 1.0;
            }
            else
            {
                y.clear();

                //Немного подправим значение альфы, используя метод наименьших квадратов

                for (k=0; k<timeArray.length(); k++)
                {
                    i = pointX+(pointY+k*myWidth)*myLength;
                    y.push_back((Tfield[i] - Tinit)/(T00 - Tinit - 273.15));
                }

                double xLeft = 0.0, xRight = 0.1;
                double xMiddle, sum1, sum2;

                delta = xRight - xLeft;
                for (k=0; k<timeArray.size(); k++)
                {
                  x[k] += tau0_;
                }
                while(delta>1.0e-4)
                {
                    xMiddle = (xLeft+xRight)/2.0;
                    delta = (xRight - xLeft)/xMiddle;

                    sum1 = 0.0;
                    sum2 = 0.0;
                    for(i = 0; i < timeNum; i++)
                    {
                        sum1 = sum1 + (y[i] - (1-exp(xMiddle*x[i])*erfc(sqrt(xMiddle*x[i])))) * (sqrt(x[i]/M_PI/xMiddle) - x[i]*exp(xMiddle*x[i])*erfc(sqrt(xMiddle*x[i])));
                        sum2 = sum2 + (y[i] - (1-exp(xRight*x[i])*erfc(sqrt(xRight*x[i])))) * (sqrt(x[i]/M_PI/xRight) - x[i]*exp(xRight*x[i])*erfc(sqrt(xRight*x[i])));
                    }

                    if(sum1*sum2 > 0)
                        xRight = xMiddle;
                    else
                        xLeft = xMiddle;
                }

                alpha = sqrt(xMiddle*kCoef);

                for (k=0; k<timeArray.length(); k++)
                {
                  i = pointX+(pointY+k*(myWidth))*(myLength);
                  TfieldSmooth[i] = Tinit + (T00 - Tinit - 273.15)*fTheta(x[k],alpha);
                }
            }

            qDebug() << "We are here4";

            //Запоминаем предысторию (потому что начальный момент и зависимость температуры от времени в каждоый точке термограммы свои)

            initialNum = -tau0_/timeArray[1];
            temp = -tau0_/initialNum;

            if(initialNum <-20)
            {
                initialNum = -20;
                temp = tau0_/initialNum;
            }
            for (k=0; k<timeArray.length(); k++)
                x[k] -= tau0_;

            for (k=initialNum; k<0; k++)
            {
                i = pointX+(pointY+(k-initialNum)*(myWidth))*(myLength);
                TFieldERFC[i] = Tinit + (T00 - Tinit - 273.15)*fTheta(fabs(k*temp + tau0_),alpha);//Здесь храним предысторию температуры
            }

            tau0[pointX+pointY*myLength] = tau0_;//Здесь храним начальный момент времени

            break;
        }
    }
}

void MainWindow::qCalc(int pointX, int pointY, int methodFlag)//По сглаженной температуре восстановим тепловой поток
{
    int k, i1, i2;
    double qAccum;//Вместо интеграла используем суммирование

    switch(methodFlag)
    {

    case(LowPassFlag)://В обоих случаях метод вычисления потоков одинаков, потому что не нужно учитывать предысторию
    case(PolynominalFlag):
    {
        i1 = pointX+(pointY+0*(myWidth))*(myLength);
        for(k = 1; k<timeArray.size(); k++)
        {
            qAccum = 0.0;
            for(int l = 1; l < k+1; l++)
            {
                i2 = pointX+(pointY+l*(myWidth))*(myLength);
                i1 = pointX+(pointY+(l-1)*(myWidth))*(myLength);
                qAccum = qAccum + (TfieldSmooth[i2] - TfieldSmooth[i1])/(sqrt(timeArray.last() - timeArray[l]) + sqrt(timeArray.last() - timeArray[l-1]));
            }

            i1 = pointX+(pointY+k*(myWidth))*(myLength);
            q[i1] = qAccum*qCalcCoef;
        }
        break;
    }

    case(ERFCFlag):
    {
        double tmp;
        int i, l;

        initialNum = tau0[pointX+pointY*myLength]/timeArray[1];
        tmp = -tau0[pointX+pointY*myLength]/initialNum;

        i1 = pointX+(pointY+0*(myWidth))*(myLength);

        //Предыстория в каждой точке своя, поэтому длина массива в общем случае разная. Заполним массивы с учетом предыстории
        double* localTarray = new double[initialNum+timeArray.length()];
        double* localTimeArray = new double[initialNum+timeArray.length()];

        for(k = 0; k<initialNum; k++)
        {
            localTimeArray[k] = (initialNum - k)*tmp;
        }

        for(k = -initialNum; k<0; k++)
        {
            i = pointX+(pointY+(k+initialNum)*(myWidth))*(myLength);
            localTarray[k+initialNum] = TFieldERFC[i];
        }

        for(k = 0; k<timeArray.length(); k++)
        {
            i2 = pointX+(pointY+k*(myWidth))*(myLength);
            localTimeArray[k+initialNum] = timeArray[k];
            localTarray[k+initialNum] = TfieldSmooth[i2];
        }

        for(k = 1; k<initialNum+timeArray.length(); k++)
        {
            qAccum = 0.0;
            for(l = 1; l < k+1; l++)
            {
                qAccum = qAccum + (localTarray[l] - localTarray[l-1])/(sqrt(localTimeArray[k] - localTimeArray[l]) + sqrt(localTimeArray[k] - localTimeArray[l-1]));
            }

            if((k-initialNum)>=0)
            {
                i1 = pointX+(pointY+(k-initialNum)*(myWidth))*(myLength);
                q[i1] = qAccum*qCalcCoef;
            }
        }

        delete [] localTarray;
        delete [] localTimeArray;
        break;
    }

    }
}

double MainWindow::erfc(double x)
{
    double sum = x, mult = 1, memb, eps = 1e-20;
    int n = 0;

    if(x>4.0)
        eps=1e-25;

    do
    {
        n++;
        mult *= -x*x/n;
        memb = x/(2*n+1)*mult;
        sum += memb;
    }while(fabs(memb) >= eps);

    return 1- 2/sqrt(M_PI)*sum;

}

double MainWindow::fTheta(double x, double alpha)
{
    return 1.0 - exp(alpha*alpha*x/kCoef)*erfc(alpha*sqrt(x/kCoef));
}

void MainWindow::on_valueChangedLeft()
{
    cutLeft = hWid->getValueLeft();
    ui->line->setGeometry(hWid->x()+hWid->getValueLeft(),hWid->y()+frameThickness,2,myWidth);
}

void MainWindow::on_valueChangedRight()
{
    cutRight = myLength - hWid->getValueRight();
    ui->line_3->setGeometry(hWid->x()+hWid->getValueRight(),hWid->y()+frameThickness,2,myWidth);
}

void MainWindow::on_valueChangedUp()
{
    cutUp = vWid->getValueUp();
    ui->line_2->setGeometry(hWid->x(),vWid->getValueUp()+hWid->y()+frameThickness-2,myLength,2);
}

void MainWindow::on_valueChangedDown()
{
    cutDown = myWidth - vWid->getValueDown();
    ui->line_4->setGeometry(hWid->x(),vWid->getValueDown()+hWid->y()+frameThickness-2,myLength,2);
}

void MainWindow::setLength(float l)
{
    myLength = l;
}

void MainWindow::setWidth(float w)
{
    myWidth = w;
}

void MainWindow::displayField(int timeMoment)
{
    delete grScene;
    grScene = new QGraphicsScene(ui->graphicsView);
    QColor color;
    QPixmap pic;
    QVector<QRgb> color_table;
    int i=0, j=0, k=0;
    uchar *localTdata;
    double rad = 1.4;

    if(!(ui->cutButton->text() == "Обрезать"))
    {
        localTdata = new uchar[myWidth*myLength];

        for(j = 0; j < myWidth; j++)
        {
            for(i = 0; i < myLength; i++)
            {
                localTdata[k] = Tdata[i+(j+timeMoment*myWidth)*myLength];
                k++;
            }
        }

        QImage img(localTdata,myLength, myWidth, QImage::Format_Indexed8);
        for(i = 0; i < 256; i++)
        {
            color.setHsv(255-i,255,255);
            color_table.append(qRgb(color.red(),color.green(),color.blue()));
        }
        img.setColorTable(color_table);
        pic.convertFromImage(img);
    }
    else
    {
        localTdata = new uchar[(myLength - cutLeft - cutRight)*(myWidth - cutUp - cutDown)];
        for(j = cutUp; j < myWidth - cutDown; j++)
        {
            for(i = cutLeft; i < myLength - cutRight; i++)
            {
                localTdata[k] = Tdata[i+(j+timeMoment*myWidth)*myLength];
                k++;
            }
        }

        QImage img(localTdata,myLength - cutLeft - cutRight, myWidth - cutUp - cutDown, QImage::Format_Indexed8);
        for(i = 0; i < 256; i++)
        {
            color.setHsv(255-i,255,255);
            color_table.append(qRgb(color.red(),color.green(),color.blue()));
        }
        img.setColorTable(color_table);
        pic.convertFromImage(img);
    }
    grScene->addPixmap(pic);

    if(!pointsForPlots.isEmpty())
    {
        for(i=0; i<pointsForPlots.size(); i++)
        {
            grScene->addEllipse(pointsForPlots.at(i).x()-rad-cutLeft, pointsForPlots.at(i).y()-rad-cutUp, rad*2.0, rad*2.0, QPen(), QBrush(Qt::SolidPattern));
        }
    }

    ui->graphicsView->setScene(grScene);
    delete [] localTdata;
}

void MainWindow::displayTRange()
{
    int i,j,k;
    QColor color;
    uchar *tempRange;
    QVector<QRgb> color_table2;
    tempRange = new uchar[ui->graphicsView->height()*15];

    k = 0;
    int h = ui->graphicsView->height()-3;
    int w = 15-3;

    for(j = 0; j < h; ++j)
    {
        for(i = 0; i < w; ++i)
        {
            tempRange[k] = (h-j)*255/h;
            k++;
        }
    }

    QPixmap pic2;
    QImage img2(tempRange,w,h, QImage::Format_Indexed8);
    for(i = 0; i < 256; i++)
    {
        color.setHsv(255-i,255,255);
        color_table2.append(qRgb(color.red(),color.green(),color.blue()));
    }
    img2.setColorTable(color_table2);
    pic2.convertFromImage(img2);
    rangeLabel->setGeometry(ui->graphicsView->x() + ui->graphicsView->width()+4, ui->graphicsView->y()+20, w, h+4);
    rangeLabel->setPixmap(pic2.scaledToHeight(h+4));
    rangeLabel->setVisible(true);
    delete [] tempRange;
}

void MainWindow::on_qCalcButton_clicked()
{
    writeTemperatureToTecplot();
}

void MainWindow::on_horizontalSlider_valueChanged(int position)
{
    if(!Tfield.isEmpty())
    {
        std::stringstream str;
        displayField(position);
        str.str("");
        str << "Time: " << timeArray.at(position) << " s";

        ui->labelTimeDisplay->setText(str.str().c_str());
        str.str("");
        if(position < 10)
            str << "  " << position + 1 << "/" << timeNum;
        else
        {
            if(position < 100)
                str << " " << position + 1 << "/" << timeNum+1;
                else
                {
                    if(position < 1000)
                        str << position + 1 << "/" << timeNum+1;
                }
        }
        ui->labelNumDisplay->setText(str.str().c_str());
    }
}

void MainWindow::on_cutButton_clicked()
{
    if(ui->cutButton->text() == "Обрезать")
    {
        ui->cutButton->setText("Завершить");
        rangeLabel->setVisible(false);
        ui->TmaxDisplay->setVisible(false);
        ui->TminDisplay->setVisible(false);
        ui->TmidDisplay->setVisible(false);
        ui->putMarks->setDisabled(true);
        ui->qCalcButton->setDisabled(true);
        hWid->setGeometry(ui->graphicsView->x()+frameThickness/2,ui->graphicsView->y()-frameThickness/2,myLength,24);
        hWid->setRange(0,myLength);
        hWid->setValueLeft(0);
        hWid->setValueRight(myLength);
        hWid->show();
        vWid->setGeometry(ui->graphicsView->x()+frameThickness+myLength,ui->graphicsView->y()+21,24,myWidth);
        vWid->setRange(0,myWidth);
        vWid->setValueUp(0);
        vWid->setValueDown(myWidth);
        vWid->show();
        vWid->setDisabled(false);
        vWid->setVisible(true);
        hWid->setDisabled(false);
        hWid->setVisible(true);
        ui->line->setVisible(true);
        ui->line_2->setVisible(true);
        ui->line_3->setVisible(true);
        ui->line_4->setVisible(true);
    }
    else
    {
        ui->cutButton->setText("Обрезать");
        ui->putMarks->setDisabled(false);
        ui->line->setVisible(false);
        ui->line_2->setVisible(false);
        ui->line_3->setVisible(false);
        ui->line_4->setVisible(false);
        vWid->setDisabled(true);
        vWid->setVisible(false);
        hWid->setDisabled(true);
        hWid->setVisible(false);
        setCutsRelation();
        ui->graphicsView->setFixedWidth(myLength-cutRight-cutLeft+frameThickness);
        ui->graphicsView->setFixedHeight(myWidth-cutUp-cutDown+frameThickness);
        ui->cutButton->setEnabled(false);
        ui->horizontalSlider->setValue(ui->horizontalSlider->value());
        ui->horizontalSlider->setGeometry(ui->graphicsView->x(),ui->graphicsView->y()+ui->graphicsView->height()+5,ui->graphicsView->width(),ui->horizontalSlider->height());
        ui->labelNumDisplay->setGeometry(ui->graphicsView->x()+ui->graphicsView->width()*45/100-43,ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height(),ui->labelNumDisplay->width(),ui->labelNumDisplay->height());
        ui->labelTimeDisplay->setGeometry(ui->graphicsView->x()+ui->graphicsView->width()*45/100,ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height(),ui->labelTimeDisplay->width(),ui->labelTimeDisplay->height());

        int i, j, k, kk=0;
        float min = 1000.0;
        float max = 0.0;

        for(k = 0; k < timeNum; k++)
        {
            for(j = cutUp; j < myWidth - cutDown; j++)
            {
                for(i = cutLeft; i < myLength - cutRight; i++)
                {
                    if(Tfield[i+(j+k*myWidth)*myLength] <= min)
                        min = Tfield[i+(j+k*myWidth)*myLength];
                    if(Tfield[i+(j+k*myWidth)*myLength] >= max)
                        max = Tfield[i+(j+k*myWidth)*myLength];
                }
            }
        }

        Tmin = min;
        Tmax = max;

        float deltaColor = 255/(Tmax - Tmin);

        for(k = 0; k < timeNum; k++)
        {
            for(j = 0; j < myWidth; j++)
            {
                for(i = 0; i < myLength; i++)
                {
                    Tdata[kk] = (int)((Tfield[i+(j+k*myWidth)*myLength] - Tmin)*deltaColor);
                    kk++;
                }
            }
        }
        ui->TmaxDisplay->setVisible(true);
        ui->TminDisplay->setVisible(true);
        ui->TmidDisplay->setVisible(true);
        ui->TminDisplay->setGeometry(ui->graphicsView->x()+ui->graphicsView->width()+25,ui->graphicsView->y()+ui->graphicsView->height()-20,35,35);
        ui->TmaxDisplay->setGeometry(ui->graphicsView->x()+ui->graphicsView->width()+25,ui->graphicsView->y()-15,35,35);
        ui->TmidDisplay->setGeometry(ui->graphicsView->x()+ui->graphicsView->width()+25,ui->graphicsView->y()+(ui->graphicsView->height()-20-15)/2,35,35);
        ui->TminDisplay->setText(QString::number((int)Tmin));
        ui->TmaxDisplay->setText(QString::number((int)Tmax));
        ui->TmidDisplay->setText(QString::number((int)((Tmax+Tmin)/2)));
        displayTRange();
    }
    displayField(ui->horizontalSlider->value());
}

void MainWindow::setCutsRelation()
{
    int d = (cutLeft + cutRight)%4;
    if(d != 0)
    {
        switch(d)
        {
            case(1):
                {
                    if(cutLeft > 0)
                    {
                        cutLeft-=1;
                        return;
                    }
                    else
                    {
                        cutRight-=1;
                        return;
                    }
                }
            case(2):
                {
                    if(cutLeft >=1 && cutRight >=1)
                    {
                        cutLeft -=1;
                        cutRight -=1;
                        return;
                    }
                    else
                    {
                        if(cutLeft == 0)
                        {
                            cutRight -= 2;
                            return;
                        }
                        else
                        {
                            cutLeft-=2;
                            return;
                        }
                    }
                }
            case(3):
                {

                    if(cutLeft == 0)
                    {
                        cutRight-=3;
                        return;
                    }
                    else if(cutLeft == 1)
                        {
                            cutRight-=2;
                            cutLeft-=1;
                            return;
                         }
                    else if(cutLeft == 2)
                        {
                            cutRight-=1;
                            cutLeft-=2;
                            return;
                         }
                    else
                        if(cutRight == 0)
                        {
                            cutLeft-=3;
                            return;
                        }
                    else
                        if(cutRight == 1)
                        {
                            cutLeft-=2;
                            cutRight-=1;
                            return;
                         }
                    else
                        if(cutRight == 2)
                        {
                            cutLeft-=1;
                            cutRight-=2;
                            return;
                         }
                    else
                        {
                            cutRight-=2;
                            cutLeft-=1;
                            return;
                         }
                }
        }

        if(cutLeft < 0)
            cutLeft = 0;
        if(cutRight < 0)
            cutRight = 0;
    }
}

void MainWindow::createSmoothField()
{
    int i = 0, j = 0, k = 0;

    for(k = 0; k < timeNum; k++)
    {
        //for(j = cutUp; j < myWidth - cutDown; j++)
        for(j = 0; j < myWidth; j++)
        {
            //for(i = cutLeft; i < myLength - cutRight; i++)
            for(i = 0; i < myLength; i++)
            {
                TfieldSmooth.push_back(Tfield[i+(j+k*myWidth)*myLength]);
                q.push_back(0.0);
            }
        }
    }

    TFieldERFC = new double[(myWidth)*(myLength)*20];
    tau0 = new double[(myWidth)*(myLength)];
}

void MainWindow::on_putMarks_clicked()
{
    if(ui->putMarks->text() == "Поставить метки")
    {
        ui->putMarks->setText("Завершить");
        ui->cutButton->setDisabled(true);
        ui->qCalcButton->setDisabled(true);
        displayField(ui->horizontalSlider->value());
        ui->graphicsView->setCursor(Qt::CrossCursor);
        ui->action_2->setDisabled(true);
        ui->action_4->setDisabled(true);
        ui->action->setDisabled(true);
        ui->facilityBox->setDisabled(true);
        ui->methodsBox->setDisabled(true);
    }
    else
    {
        ui->putMarks->setText("Поставить метки");
        ui->graphicsView->setCursor(Qt::ArrowCursor);
        ui->action->setDisabled(false);
        ui->action_2->setDisabled(false);
        ui->action_4->setDisabled(false);
        ui->putMarks->setDisabled(true);
        ui->processingMethodButton->setEnabled(true);
    }
}

void MainWindow::on_facilityName_changed()
{
    if(ui->facilityBox->currentIndex() == 0)
    {
        ui->methodsBox->clear();
        QStringList lst;
        lst << "LowPass" << "Polynominal";
        ui->methodsBox->addItems(lst);
    }
    else
    {
        ui->methodsBox->clear();
        QStringList lst("ERFC");
        ui->methodsBox->addItems(lst);
    }
}

void MainWindow::on_processingMethodButton_clicked()
{
    if(ui->processingMethodButton->text() == "Выбрать метод обработки")
    {
        ui->facilityBox->setDisabled(false);
        ui->methodsBox->setDisabled(false);
        ui->processingMethodButton->setText("Завершить");
        //timeArrayERFC = new double[(myWidth-cutUp-cutDown)*(myLength-cutRight-cutLeft)*20];
        //TFieldERFC = new double[(myWidth-cutUp-cutDown)*(myLength-cutRight-cutLeft)*20];
        //tau0 = new double[(myWidth-cutUp-cutDown)*(myLength-cutRight-cutLeft)];

        if(!ui->scrollArea->isVisible())
        {
            ui->facilityBox->setGeometry(ui->scrollArea->x()+ui->scrollArea->width()*22/50,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->facilityBox->width(),ui->facilityBox->height());
            ui->methodsBox->setGeometry(ui->scrollArea->x()+ui->scrollArea->width()*69/100,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->methodsBox->width(),ui->methodsBox->height());
            ui->labelFacility->setGeometry(ui->facilityBox->x()-63,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->methodsBox->width(),ui->methodsBox->height());
            ui->labelMethod->setGeometry(ui->methodsBox->x()-42,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->methodsBox->width(),ui->methodsBox->height());
        }

        ui->facilityBox->setVisible(true);
        ui->methodsBox->setVisible(true);
        ui->labelFacility->setVisible(true);
        ui->labelMethod->setVisible(true);
        ui->cutButton->setDisabled(true);
        ui->putMarks->setDisabled(true);
        ui->qCalcButton->setDisabled(true);
    }
    else
    {
        ui->facilityBox->setDisabled(true);
        ui->methodsBox->setDisabled(true);
        ui->processingMethodButton->setText("Выбрать метод обработки");
        ui->qCalcButton->setEnabled(true);
        ui->qCalcButton->setDisabled(false);
    }
}

void MainWindow::on_action_2_triggered()
{
    settingsDialog->setGeometry(ui->graphicsView->x()+MainWindow::x(),ui->graphicsView->y()+MainWindow::y()+50,200,180);
    settingsDialog->show();
    rhoLabel->show();
    CpLabel->show();
    lambdaLabel->show();

    rhoLabel->setText("ρ");
    CpLabel->setText("Cp");
    lambdaLabel->setText("λ");

    rhoLabel->setGeometry(30,30,20,20);
    CpLabel->setGeometry(30,70,20,20);
    lambdaLabel->setGeometry(30,110,20,20);

    rhoEdit->show();
    CpEdit->show();
    lambdaEdit->show();

    rhoEdit->setGeometry(50,30,47,27);
    CpEdit->setGeometry(50,70,47,27);
    lambdaEdit->setGeometry(50,110,47,27);

    connect(rhoEdit,SIGNAL(editingFinished()),this,SLOT(setRho( )),Qt::UniqueConnection);
    connect(CpEdit,SIGNAL(editingFinished()),this,SLOT(setCp( )),Qt::UniqueConnection);
    connect(lambdaEdit,SIGNAL(editingFinished()),this,SLOT(setLambda( )),Qt::UniqueConnection);
    if(!plotsLayout->isEmpty())
        redrawPlots();
}

void MainWindow::setRho( )
{
    QString str = rhoEdit->text();
    if(str.toDouble() > 0)
        rho = str.toDouble();
    kCoef = rho*Cp*lambda;
    qCalcCoef = 2*sqrt(kCoef/M_PI);
    if(!plotsLayout->isEmpty())
        redrawPlots();
}

void MainWindow::setCp()
{
    QString str = CpEdit->text();
    if(str.toDouble() > 0)
        Cp = str.toDouble();
    kCoef = rho*Cp*lambda;
    qCalcCoef = 2*sqrt(kCoef/M_PI);
    if(!plotsLayout->isEmpty())
        redrawPlots();
}

void MainWindow::setLambda( )
{
    QString str = lambdaEdit->text();
    if(str.toDouble() > 0)
        lambda = str.toDouble();
    kCoef = rho*Cp*lambda;
    qCalcCoef = 2*sqrt(kCoef/M_PI);
    if(!plotsLayout->isEmpty())
        redrawPlots();
}

void MainWindow::setT00()
{
    QString str = T00Edit->text();
    if(str.toDouble() > 0)
        T00 = str.toDouble();

    if(!plotsLayout->isEmpty())
        redrawPlots();
}

void MainWindow::setAlpha0()
{
    QString str = alpha0Edit->text();
    if(str.toDouble() > 0)
        alpha0 = str.toDouble();

    if(!plotsLayout->isEmpty())
        redrawPlots();
}

void MainWindow::redrawPlots()
{
    QPen pen(Qt::red);
    QPen pen2(Qt::blue);
    int viewWidth = 300;
    int viewHeight = 200;
    double min=1000.0, max=-100.0;
    double minq=1000.0, maxq=-100.0;
    QVector<double> x, y, y2, qSet;

    int iMethod = getPropessingFlag();

    for(int ii = 0; ii<pointsForPlots.length(); ii++)
    {
        min=1000.0, max=-100.0;
        minq=1000.0, maxq=-100.0;
        int i1 = 2*ii;
        int i2 = 2*ii + 1;
        plotsList[i1]->clearGraphs();
        plotsList[i2]->clearGraphs();
        x.clear();
        y.clear();
        y2.clear();
        qSet.clear();
        int i = pointsForPlots[ii].x();
        int j = pointsForPlots[ii].y();
        for (int k=0; k<timeArray.size(); k++)
        {
          x.push_back(timeArray[k]);
          y.push_back(Tfield[i+(j+k*myWidth)*myLength]);

          if(min >= y.at(k))
              min = y.at(k);
          if(max <= y.at(k))
              max = y.at(k);
        }

        plotsList[i1]->addGraph();
        plotsList[i1]->graph(0)->setData(x, y);
        plotsList[i1]->graph(0)->setPen(pen);
        plotsList[i1]->graph(0)->setLineStyle(QCPGraph::lsNone);
        plotsList[i1]->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossSquare, 4));
        std::stringstream str;
        str << "Point №" << ii+1;
        plotsList[i1]->graph(0)->setName(str.str().c_str());

        switch(iMethod)
        {
            case(LowPassFlag):
                str << " LowPass";
                break;
            case(PolynominalFlag):
                str << " Poly";
                break;
            case(ERFCFlag):
                str << " ERFC";
                break;
        }

        plotsList[i1]->addGraph();
        processField(i,j,iMethod);
        qCalc(i,j,iMethod);

        for (int k=0; k<timeArray.size(); k++)
        {
            y2.push_back(TfieldSmooth[i+(j+k*(myWidth))*(myLength)]);
            qSet.push_back(q[i+(j+k*(myWidth))*(myLength)]);
            if(minq >= qSet.at(k))
                minq = qSet.at(k);
            if(maxq <= qSet.at(k))
                maxq = qSet.at(k);
        }

        plotsList[i1]->graph(1)->setData(x, y2);
        plotsList[i1]->graph(1)->setData(x, y2);
        plotsList[i1]->graph(1)->setPen(pen2);
        plotsList[i1]->graph(1)->setName(str.str().c_str());
        plotsList[i2]->addGraph();
        plotsList[i2]->graph(0)->setData(x, qSet);
        plotsList[i2]->graph(0)->setPen(pen);
        plotsList[i2]->graph(0)->setName(str.str().c_str());
        plotsList[i2]->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::AlignBottom|Qt::AlignRight));
        plotsList[i2]->legend->setVisible(true);
        plotsList[i2]->xAxis->setLabel("t, с");
        plotsList[i2]->yAxis->setLabel("q, Вт/м2");
        plotsList[i2]->xAxis->setRange(timeArray.first(), timeArray.last());
        plotsList[i2]->yAxis->setRange(minq, maxq);
        plotsList[i2]->replot();
        plotsList[i1]->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::AlignBottom|Qt::AlignRight));
        plotsList[i1]->legend->setVisible(true);
        plotsList[i1]->xAxis->setLabel("t, с");
        plotsList[i1]->yAxis->setLabel("T, °C");
        plotsList[i1]->xAxis->setRange(timeArray.first(), timeArray.last());
        plotsList[i1]->yAxis->setRange(min-5, max+5);
        plotsList[i1]->replot();
        plotsList[i1]->setMinimumSize(viewWidth,viewHeight);
        plotsList[i2]->setMinimumSize(viewWidth,viewHeight);
        plotsList[i1]->show();
        plotsList[i2]->show();
    }
}

void MainWindow::on_flowParamsAction_triggered()
{
    flowParamsDialog->setGeometry(ui->graphicsView->x()+MainWindow::x(),ui->graphicsView->y()+MainWindow::y()+50,200,180);
    flowParamsDialog->show();
    T00Label->show();
    alpha0Label->show();

    T00Label->setText("T00");
    alpha0Label->setText("α0");

    T00Label->setGeometry(30,30,20,20);
    alpha0Label->setGeometry(30,70,20,20);

    T00Edit->show();
    alpha0Edit->show();

    T00Edit->setGeometry(50,30,47,27);
    alpha0Edit->setGeometry(50,70,47,27);

    connect(T00Edit,SIGNAL(editingFinished()),this,SLOT(setT00( )),Qt::UniqueConnection);
    connect(alpha0Edit,SIGNAL(editingFinished()),this,SLOT(setAlpha0( )),Qt::UniqueConnection);
}

int MainWindow::getPropessingFlag()
{
    return ui->facilityBox->currentIndex() == 1? ERFCFlag : ui->methodsBox->currentIndex();
}
