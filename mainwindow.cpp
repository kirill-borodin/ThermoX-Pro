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
#include <cassert>

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
    //plotsList = new QList<QCustomPlot*>();
    hWid->setVisible(false);
    vWid->setVisible(false);
    connect(ui->action_4,SIGNAL(triggered()),this,SLOT(slotAboutProgram()),Qt::UniqueConnection);
    connect(ui->action,SIGNAL(triggered()),this,SLOT(on_action_triggered()),Qt::UniqueConnection);
    connect(hWid,SIGNAL(valueChangedLeft(int)),this,SLOT(on_valueChangedLeft()),Qt::UniqueConnection);
    connect(hWid,SIGNAL(valueChangedRight(int)),this,SLOT(on_valueChangedRight()),Qt::UniqueConnection);
    connect(vWid,SIGNAL(valueChangedUp(int)),this,SLOT(on_valueChangedUp()),Qt::UniqueConnection);
    connect(vWid,SIGNAL(valueChangedDown(int)),this,SLOT(on_valueChangedDown()),Qt::UniqueConnection);
    connect(ui->facilityBox,SIGNAL(currentIndexChanged(int)),this,SLOT(on_facilityName_changed()));
    connect(ui->methodsBox,SIGNAL(currentIndexChanged(int)),this,SLOT(replot()));
    //connect(this,SIGNAL(replotNeeded()),this,SLOT(replot()));
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
    T00.push_back(483.0);
    polyPower = 5;
    settingsDialog = new QDialog(this);
    rhoEdit = new QLineEdit(settingsDialog);
    CpEdit = new QLineEdit(settingsDialog);
    lambdaEdit = new QLineEdit(settingsDialog);
    rhoLabel = new QLabel(settingsDialog);
    CpLabel = new QLabel(settingsDialog);
    lambdaLabel = new QLabel(settingsDialog);
    rhoEdit->setText("1745");
    CpEdit->setText("1330");
    lambdaEdit->setText("0.52");
    rho = 1745.0;
    Cp = 1330.0;
    lambda = 0.52;
    kCoef = rho*Cp*lambda;
    qCalcCoef = 2*sqrt(kCoef/M_PI);
    plotsWidget2 = new QWidget;
    plotsWidget3 = new QWidget;
    Tinit = 3.0;
    deltaTau = 5.0;
    displayScene = new QGraphicsScene(ui->graphicsView);
    //customPlot = new QCustomPlot(plotsWidget2);
    //customPlot2 = new QCustomPlot(plotsWidget3);
}

MainWindow::~MainWindow()
{
    delete Tdata;
    delete plotsWidget;
    delete plotsLayout;
    delete hWid;
    delete vWid;
    //delete(TfieldSmooth);
    //delete(Tfield);
    //delete plotsList;
    //delete(q);
    TfieldSmooth.clear();
    Tfield.clear();
    q.clear();
    delete plotsWidget2;
    delete plotsWidget3;
    //delete customPlot;
    //delete customPlot2;
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
            //delete plotsWidget;
            //delete plotsLayout;
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
                    Tfield.push_back(lst.at(l-j*(myLength)).toFloat());
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
    //MainWindow::setWidth(ui->graphicsView->x()+ui->graphicsView->width()+10);
    MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),ui->graphicsView->x()+ui->graphicsView->width()+25,ui->graphicsView->y()+ui->graphicsView->height()+100);
    ui->cutButton->setEnabled(true);
    ui->putMarks->setEnabled(true);
    //ui->pushButton_2->setEnabled(true);
    ui->Play->setEnabled(false);
    ui->qCalcButton->setEnabled(false);
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
    assert(Tmax != Tmin);
    float deltaColor = 256/(Tmax - Tmin);
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

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    //if (event->x() > 130 && event->x() < 130+ui->graphicsView->width() && event->y() > 30 && event->y() < 50+ui->graphicsView->height() && (ui->putMarks->text() == "Завершить"))
    if (event->x() > ui->graphicsView->x() && event->x() < ui->graphicsView->x()+ui->graphicsView->width() && event->y() > 30 && event->y() < 50+ui->graphicsView->height() && (ui->putMarks->text() == "Завершить"))
    {
        //QCustomPlot *customPlot;
        //QCustomPlot *customPlot2;
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
        p.setX(event->pos().x()-ui->graphicsView->x()-2);
        p.setY(event->pos().y()-ui->graphicsView->y()-22);
        if(p.x() > 1 && p.x()<myLength -1 - cutLeft - cutRight && p.y() > 1 && p.y() < myWidth-1 - cutUp - cutDown)
        {
            pointsForPlots.push_back(p);
            double rad = 1.4;
            ui->graphicsView->scene()->addEllipse(p.x()-rad, p.y()-rad, rad*2.0, rad*2.0, QPen(), QBrush(Qt::SolidPattern));

            int i = p.x();
            int j = p.y();

            for (int k=0; k<timeArray.size(); k++)
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

            //if(ui->methodsBox->currentIndex() == 0)
            //{
                //str << " LowPass";
                customPlot->addGraph();
                //qDebug() << "we are here" << i << j;

                processField(i,j,ui->methodsBox->currentIndex());
                //for (int k=0; k<timeArray.size(); k++)
                     //qDebug() << k <<  TfieldSmooth[i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft)];

                qCalc(i,j);
                for (int k=0; k<timeArray.size(); k++)
                {
                    y2.push_back(TfieldSmooth[i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft)]);
                    qSet.push_back(q[i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft)]);
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
            //}

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
            plotsList.at(plotsList.size()-1)->show();
            plotsList.last()->show();

            if(plotsList.size() == 2)
            {
                if(ui->graphicsView->y() + ui->graphicsView->height() + ui->horizontalSlider->height() + ui->labelTimeDisplay->height() + 10 > ui->pushButton_2->y()+ui->pushButton_2->height())
                {
                    ui->scrollArea->setGeometry(ui->scrollArea->x(),40+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),2*viewWidth+50,10+viewHeight);
                    ui->facilityBox->setGeometry(ui->scrollArea->x()+ui->scrollArea->width()*22/50,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->facilityBox->width(),ui->facilityBox->height());
                    ui->methodsBox->setGeometry(ui->scrollArea->x()+ui->scrollArea->width()*69/100,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->methodsBox->width(),ui->methodsBox->height());
                    ui->labelFacility->setGeometry(ui->facilityBox->x()-63,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->methodsBox->width(),ui->methodsBox->height());
                    ui->labelMethod->setGeometry(ui->methodsBox->x()-42,10+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height(),ui->methodsBox->width(),ui->methodsBox->height());
                    if(3*viewWidth > ui->graphicsView->x()+ui->graphicsView->width())
                        MainWindow::setGeometry(350,40,ui->graphicsView->x()+ui->graphicsView->width()+10,40+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height()+2*viewHeight+90);
                    else
                        MainWindow::setGeometry(350,40,2.3*viewWidth,40+ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height()+ui->labelNumDisplay->height()+2*viewHeight+90);
                }
                else
                {
                    MainWindow::setGeometry(350,40,2.3*viewWidth,ui->pushButton_2->y()+ui->pushButton_2->height()+2*viewHeight+150);
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
            //qDebug() << plotsList.length() << plotsLayout->columnCount() << plotsLayout->rowCount();
       }

        //delete plotsWidget2;
        //delete plotsWidget3;
        //delete customPlot;
        //delete customPlot2;
    }
}

void MainWindow::processField(int i, int j, int methodFlag)
{
    QVector<double> x, y;
    int ii, jj, k;
    double temp, temp0, temp1;
    double A[6][6];
    double left[6][6];
    double b[6];
    double a[6];

    for (int k=0; k<timeArray.size(); k++)
    {
      x.push_back(timeArray.at(k));
      y.push_back(Tfield[i+(j+k*myWidth)*myLength]);
    }
    switch(methodFlag)
    {

    case(LowPassFlag):
        {
            double lengthFourier = x.last() - x.first();
            double v0 = (x.size()-1)/2.0/lengthFourier;
            double kkk, bbb;
            QVector<double> aFourier;

            for(int kFreq = 0; kFreq < 2*(x.size()-1); kFreq++)
            {
                aFourier.push_back(0.0);
                for(int kTime = 0; kTime < x.size()-1; kTime++)
                {
                    if(kFreq == 0)
                    {
                        aFourier[kFreq] = aFourier[kFreq] + (y.at(kTime+1) + y.at(kTime))*(x.at(kTime+1) - x.at(kTime));
                    }
                    else
                    {
                        kkk = (y.at(kTime+1) - y.at(kTime))/(x.at(kTime+1) - x.at(kTime));
                        bbb = y.at(kTime) - (y.at(kTime+1) - y.at(kTime))/(x.at(kTime+1) - x.at(kTime))*x.at(kTime);
                        aFourier[kFreq] = aFourier[kFreq] + (kkk*x.at(kTime+1) + bbb)*sin(M_PI*kFreq*x.at(kTime+1)/lengthFourier) - (kkk*x.at(kTime) + bbb)*sin(M_PI*kFreq*x.at(kTime)/lengthFourier) + (cos(M_PI*kFreq*x.at(kTime+1)/lengthFourier) - cos(M_PI*kFreq*x.at(kTime)/lengthFourier))*kkk*lengthFourier/M_PI/kFreq;
                    }
                }

                if(kFreq == 0)
                    aFourier[kFreq] = aFourier[kFreq]/lengthFourier;
                else
                    aFourier[kFreq] = aFourier[kFreq]*2.0/M_PI/kFreq;
            }

            for (k=0; k<timeArray.size(); k++)
            {
                ii = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                TfieldSmooth[ii] = aFourier.at(0)/2;
                for(int kFreq = 1; kFreq < 2*(x.size()-1); kFreq++)
                    TfieldSmooth[ii] = TfieldSmooth[ii] + aFourier.at(kFreq)*cos(M_PI*kFreq*x.at(k)/lengthFourier)*exp(-kFreq/lengthFourier/v0*kFreq/lengthFourier/v0/1.1);
            }
            break;
        }
        case(PolynominalFlag):
        {
            for(ii = 0; ii < polyPower; ii++)
            {
                for(jj = 0; jj < polyPower; jj++)
                {
                    temp = 0.0;
                    for (k=0; k<timeArray.size(); k++)
                        //temp = temp + (x.at(k)^((double)ii))*(x.at(k)^((double)jj));
                        temp = temp + pow(x.at(k),(double)ii)*pow(x.at(k),(double)jj);
                    A[ii][jj] = temp;
                }

                b[ii] = 0.0;
                for (k=0; k<timeArray.size(); k++)
                    b[ii] = b[ii] + pow(x.at(k),(double)ii)*y[k];
            }

            //Gauss forward marching method

            for (k=0; k<polyPower; k++)
            {
                for (ii=k+1; ii<polyPower; ii++)
                {
                    left[ii][k] = A[ii][k]/A[k][k];
                    b[ii] = b[ii] - left[ii][k]*b[k];
                }

                for(jj = k + 1; jj < polyPower; jj++)
                    for (ii=k+1; ii<polyPower; ii++)
                        A[jj][ii] = A[jj][ii] - left[jj][k]*A[k][ii];
            }

            //Backward marching

            a[polyPower] = b[polyPower]/A[polyPower][polyPower];

            for(ii = polyPower - 1; ii > -1; ii--)
            {
                a[ii] = b[ii];
                for(jj = ii+1; jj<polyPower; jj++)
                    a[ii] = a[ii] - A[ii][jj]*a[jj];
                a[ii] = a[ii]/A[ii][ii];
            }

            for (k=0; k<timeArray.size(); k++)
            {
                temp = 0.0;
                for(jj = 0; jj < polyPower; jj++)
                    temp = temp + a[jj]*pow(x.at(k),(double)jj);

                ii = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);

                TfieldSmooth[ii] = temp;

                //ii = i+(j+k*(myWidth))*(myLength);
                //qDebug() << Tfield[ii] << temp;
            }

            break;
        }

        case(ERFCFlag):
        {
            polyPower=2;
            y.clear();
            double delta = 30.0;
            double alpha;
            int end = 10;
            for (k=0; k<end; k++)
            {
              //x.push_back(timeArray.at(k));
              //y.push_back((Tfield[i+(j+k*myWidth)*myLength]+30.0)^2);
              y.push_back(pow((Tfield[i+(j+k*myWidth)*myLength]+delta),2));
            }

            for(ii = 0; ii < polyPower; ii++)
            {
                for(jj = 0; jj < polyPower; jj++)
                {
                    temp = 0.0;
                    for (k=0; k<end; k++)
                        temp = temp + pow(x.at(k),ii)*pow(x.at(k),jj);
                    A[ii][jj] = temp;
                }

                b[ii] = 0.0;
                for (k=0; k<end; k++)
                    b[ii] = b[ii] + pow(x.at(k),ii)*y[k];
            }

            //Gauss forward marching method

            for (k=0; k<polyPower-1; k++)
            {
                for (ii=k+1; ii<polyPower; ii++)
                {
                    left[ii][k] = A[ii][k]/A[k][k];
                    b[ii] = b[ii] - left[ii][k]*b[k];
                }

                for(jj = k + 1; jj < polyPower; jj++)
                    for (ii=k+1; ii<polyPower; ii++)
                        A[jj][ii] = A[jj][ii] - left[jj][k]*A[k][ii];
            }

            //Backward marching

            a[polyPower] = b[polyPower]/A[polyPower][polyPower];

            for(ii = polyPower - 1; ii > -1; ii--)
            {
                a[ii] = b[ii];
                for(jj = ii+1; jj<polyPower; jj++)
                    a[ii] = a[ii] - A[ii][jj]*a[jj];
                a[ii] = a[ii]/A[ii][ii];
            }

            temp0 = (sqrt(a[0]) - delta - Tinit)/(T00[0] - Tinit - 273.15);
            temp1 = a[1]/sqrt(a[0])/(T00[0] - Tinit - 273.15)/2;

            if(temp0 < 0)
                temp0 = -temp0;

            if(temp1 < 0)
                temp1 = -temp1;

            double deltaGamma = 1.0;
            double leftGamma = 1.0e-7;
            double rightGamma = 0.4;
            double middleGamma, sum1, sum2;
            double tau0;

            while(deltaGamma > 1.0e-5)
            {
                middleGamma = (rightGamma + leftGamma)/2;
                deltaGamma = (rightGamma - leftGamma)/middleGamma;
                sum1 = 1.0 - exp(rightGamma)*erfc(sqrt(rightGamma)) - temp0;
                sum2 = 1.0 - exp(leftGamma)*erfc(sqrt(leftGamma)) - temp0;

                if(sum1*sum2 < 0)
                    rightGamma = middleGamma;
                else
                    leftGamma = middleGamma;

                //qDebug() << middleGamma << deltaGamma;
            }

            //for(double iii = 0.0; iii < 100; iii+=0.01)
            //{
               // qDebug() << iii << erfc(iii);
            //}

            middleGamma = (rightGamma + leftGamma)/2;

            tau0 = (sqrt(middleGamma/M_PI) - middleGamma*exp(middleGamma)*erfc(sqrt(middleGamma)))/temp1;

            alpha=sqrt(kCoef*middleGamma/tau0);

            qDebug() << "t0 =" << tau0 << ", a =" << alpha;
            qDebug() << "temp0 =" << temp0 << ", temp1 =" << temp1;
            //qDebug() << erfc(-1.0) << erfc(-0.1) << erfc(0.1) << erfc(1.1) << erfc(2.1) << erfc(5.0) << erfc(10.0);
            if(tau0 > 500*timeArray[1])
            {
                tau0 = 500*timeArray[1];
                alpha = 10.0;
            }
            else
            {
                y.clear();
                for (k=0; k<timeArray.size(); k++)
                {
                  //ii = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                  //ii = i+(j+k*(myWidth))*(myLength);
                  //x[k] += tau0;
                  y.push_back((Tfield[i+(j+k*myWidth)*myLength] - Tinit)/(T00[0] - Tinit - 273.15));
                }

                double xLeft = 0.0, xRight = 0.1;
                double xMiddle, sum1, sum2;

                delta = xRight - xLeft;
                for (k=0; k<timeArray.size(); k++)
                {
                  //ii = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                  //ii = i+(j+k*(myWidth))*(myLength);
                  x[k] += tau0;
                  //y.push_back((Tfield[i+(j+k*myWidth)*myLength] - Tinit)/(T00[0] - Tinit - 273.15));
                }
                while(delta>1.0e-4)
                {
                    xMiddle = (xLeft+xRight)/2.0;
                    delta = (xRight - xLeft)/xMiddle;

                    sum1 = 0.0;
                    sum2 = 0.0;
                    for(ii = 0; ii < timeNum; ii++)
                    {
                        sum1 = sum1 + (y[ii] - (1-exp(xMiddle*x[ii])*erfc(sqrt(xMiddle*x[ii])))) * (sqrt(x[ii]/M_PI/xMiddle) - x[ii]*exp(xMiddle*x[ii])*erfc(sqrt(xMiddle*x[ii])));
                        sum2 = sum2 + (y[ii] - (1-exp(xRight*x[ii])*erfc(sqrt(xRight*x[ii])))) * (sqrt(x[ii]/M_PI/xRight) - x[ii]*exp(xRight*x[ii])*erfc(sqrt(xRight*x[ii])));
                        //sum1 += (y[ii] - (1-exp(xMiddle*x[ii])*erfc(sqrt(xMiddle*x[ii])))) * (sqrt(x[ii]/M_PI/xMiddle) - x[ii]*exp(xMiddle*x[ii])*erfc(sqrt(xMiddle*x[ii])));
                        //sum2 += (y[ii] - (1-exp(xRight*x[ii])*erfc(sqrt(xRight*x[ii])))) * (sqrt(x[ii]/M_PI/xRight) - x[ii]*exp(xRight*x[ii])*erfc(sqrt(xRight*x[ii])));
                        //qDebug() << xMiddle*x[ii] << erfc(xMiddle*x[ii]);
                        //qDebug() << xRight*x[ii] << erfc(xRight*x[ii]);
                    }

                    if(sum1*sum2 > 0)
                        xRight = xMiddle;
                    else
                        xLeft = xMiddle;

                    //qDebug() << xMiddle << delta;
                }

                alpha = sqrt(xMiddle*kCoef);

                qDebug() << "\r\nt0 =" << tau0 << ", a =" << alpha;
               // for (k=0; k<timeArray.size(); k++)
                //{
                  //ii = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                  //ii = i+(j+k*(myWidth))*(myLength);
                 // x[k] -= tau0;
                  //y.push_back((Tfield[i+(j+k*myWidth)*myLength] - Tinit)/(T00[0] - Tinit - 273.15));
                //}
                for (k=0; k<timeArray.size(); k++)
                {
                  ii = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                  TfieldSmooth[ii] = Tinit + (T00[0] - Tinit - 273.15)*fTheta(x[k],alpha);
                  temp=TfieldSmooth[ii];
                  ii = i+(j+k*(myWidth))*(myLength);
                  //qDebug() << x[k] - tau0 << Tfield[ii] << temp;
                }
            }

            initialNum = tau0/timeArray[1];
            temp = tau0/initialNum;

            //qDebug() << "initialNum =" << initialNum << "temp =" << temp;

            if(initialNum > 10)
            {
                initialNum = 10;
                temp = tau0/initialNum;
            }

            //int ii1;

            /*for (k=0; k<timeNum; k++)
            {
                ii = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                ii1 = i+j*(myLength-cutRight-cutLeft);
                TFieldERFC[k+initialNum] = TfieldSmooth[ii];// = Tinit + (T00[0] - Tinit - 273.15)*fTheta(x[k] + tau0,alpha);
                //localTimeArray[k+initialNum] = timearray[k];
            }*/
            for (k=0; k<timeArray.size(); k++)
            {
              //ii = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
              //ii = i+(j+k*(myWidth))*(myLength);
              x[k] -= tau0;
              //y.push_back((Tfield[i+(j+k*myWidth)*myLength] - Tinit)/(T00[0] - Tinit - 273.15));
            }
            //qDebug() << "we are here";

            for (k=-initialNum; k<0; k++)
            {
                //qDebug() << "we are here";
                //ii = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                //ii1 = i+j*(myLength-cutRight-cutLeft);
                //ii = i+j*(myLength-cutRight-cutLeft);
                //ii1 = ii*(myWidth-cutUp-cutDown) + k;
                ii = i+(j+(k+initialNum)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                //qDebug() << ii;
                //qDebug() << (myWidth-cutUp-cutDown)*(myLength-cutRight-cutLeft)*10;
                //qDebug() << temp;
                //timeArrayERFC[0] = 0.5;
                timeArrayERFC[ii] = k*temp;
                //qDebug() << Tinit;
                //qDebug() << k*temp + tau0 << fTheta(fabs(k*temp + tau0),alpha);
                TFieldERFC[ii] = Tinit + (T00[0] - Tinit - 273.15)*fTheta(fabs(k*temp + tau0),alpha);
                //qDebug() << k*temp << TFieldERFC[ii];
            }

            //qDebug() << "we are here";

            if(initialNum < 10)
                for (k=10; k>initialNum; k--)
                {
                    ii = i+(j+(k)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                    timeArrayERFC[ii] = 0.0;
                }

            qDebug() << "tau0 =" << tau0 << "initNum =" << initialNum;

            break;
        }
    }
}

void MainWindow::qCalc(int i, int j)
{
    int k, i1, i2;
    double qq;

    switch(ui->methodsBox->currentIndex())
    {

    case(LowPassFlag):
    {
        if(ui->facilityBox->currentIndex() == 0)
            for(k = 1; k<timeArray.size(); k++)
            {
                qq = 0.0;
                for(int l = 1; l < k+1; l++)
                {
                    i2 = i+(j+l*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                    i1 = i+(j+(l-1)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                    qq = qq + (TfieldSmooth[i2] - TfieldSmooth[i1])/(sqrt(timeArray.last() - timeArray[l]) + sqrt(timeArray.last() - timeArray[l-1]));
                }

                i1 = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                q[i1] = qq*qCalcCoef;
            }

            //break;
        else
        {
            i1 = i+(j+0*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
            double b3 = pow(TfieldSmooth[i1],3.0);
            //double k3 = (b3-pow(Tinit,3.0))/pow(deltaTau,3);
            double k3 = (b3-pow(Tinit,3.0))/deltaTau;

            int tNum;
            double temp;
            tNum = deltaTau/timeArray[1];
            temp = timeArray[1]/tNum;

            double *localTarray = new double[tNum+timeNum];
            double *localTimeArray = new double[tNum+timeNum];

            for(k = -tNum; k<0; k++)
            {
                localTimeArray[k+tNum] = k*temp;
                localTarray[k+tNum] = pow(k3*localTimeArray[k]+b3,1.0/3.0);
            }

            for(k = 0; k<timeNum; k++)
            {
                i2 = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                localTimeArray[k] = timeArray[k];
                localTarray[k] = TfieldSmooth[i2];
            }

            for(k = -tNum; k<timeNum; k++)
            {
                qq = 0.0;
                for(int l = 1; l < k+1; l++)
                {
                    i2 = i+(j+(l+tNum)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                    i1 = i+(j+(l+tNum-1)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                    qq = qq + (localTarray[k+tNum] - localTarray[k+tNum-1])/(sqrt(localTimeArray[timeNum+tNum-1] - localTimeArray[l+tNum]) + sqrt(localTimeArray[timeNum+tNum-1] - localTimeArray[l+tNum-1]));
                }

                if(k>0)
                {
                    i1 = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                    q[i1] = qq*qCalcCoef;
                }
            }

            delete localTarray;
            delete localTimeArray;
            //break;
        }
        break;
    }

    case(PolynominalFlag):
    {
        i1 = i+(j+0*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
        double b3 = pow(TfieldSmooth[i1],3.0);
        //double k3 = (b3-pow(Tinit,3.0))/pow(deltaTau,3);
        double k3 = (b3-pow(Tinit,3.0))/deltaTau;

        int tNum;

        tNum = deltaTau/timeArray[1];
        double temp = timeArray[1]/tNum;

        double *localTarray = new double[tNum+timeNum];
        double *localTimeArray = new double[tNum+timeNum];

        for(k = -tNum; k<0; k++)
        {
            localTimeArray[k+tNum] = k*temp;
            localTarray[k+tNum] = pow(k3*localTimeArray[k]+b3,1.0/3.0);
        }

        for(k = 0; k<timeNum; k++)
        {
            i2 = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
            localTimeArray[k] = timeArray[k];
            localTarray[k] = TfieldSmooth[i2];
        }

        for(k = -tNum; k<timeNum; k++)
        {
            qq = 0.0;
            for(int l = 1; l < k+1; l++)
            {
                i2 = i+(j+(l+tNum)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                i1 = i+(j+(l+tNum-1)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                qq = qq + (localTarray[k+tNum] - localTarray[k+tNum-1])/(sqrt(localTimeArray[timeNum+tNum-1] - localTimeArray[l+tNum]) + sqrt(localTimeArray[timeNum+tNum-1] - localTimeArray[l+tNum-1]));
            }

            if(k>0)
            {
                i1 = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                q[i1] = qq*qCalcCoef;
            }
        }

        delete localTarray;
        delete localTimeArray;
        break;
    }

    case(ERFCFlag):
    {

        i1 = i+(j+0*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
        //double b3 = pow(TfieldSmooth[i1],3.0);
        //double k3 = (b3-pow(Tinit,3.0))/pow(deltaTau,3);
        //double k3 = (b3-pow(Tinit,3.0))/deltaTau;
        int ii;
        int tNum=10;

        for(k = 0; k<10; k++)
        {
            //ii = i+j*(myLength-cutRight-cutLeft);
            //ii1 = ii*(myWidth-cutUp-cutDown) + k;
            ii = i+(j+(k)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
            if(timeArrayERFC[ii] == 0.0)
                tNum--;
            //qDebug() << timeArrayERFC[ii];
        }

        double *localTarray = new double[tNum+timeNum];
        double *localTimeArray = new double[tNum+timeNum];

        //for(k = -tNum; k<0; k++)
        for(k = -tNum; k<0; k++)
        {
            //ii = i+j*(myLength-cutRight-cutLeft);
            //ii1 = ii*(myWidth-cutUp-cutDown) + k;
            ii = i+(j+(k+tNum)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
            localTimeArray[k+tNum] = timeArrayERFC[ii];
            localTarray[k+tNum] = TFieldERFC[ii];

        }

        for(k = 0; k<timeNum; k++)
        {
            i2 = i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
            localTimeArray[k+tNum] = timeArray[k];
            localTarray[k+tNum] = TfieldSmooth[i2];
            //qDebug() << localTimeArray[k+tNum] << localTarray[k+tNum];
        }
        //qDebug() << "we are here";
        int l;
        for(k = 1; k<timeNum+tNum; k++)
        {
            qq = 0.0;
            for(l = 1; l < k+1; l++)
            {
                //i2 = i+(j+(l+tNum)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                //i1 = i+(j+(l+tNum-1)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                qq = qq + (localTarray[l] - localTarray[l-1])/(sqrt(localTimeArray[k] - localTimeArray[l]) + sqrt(localTimeArray[k] - localTimeArray[l-1]));
                //qDebug() << k << l << localTarray[l] << localTarray[l-1] << localTimeArray[k] << localTimeArray[l] << localTimeArray[l-1];
            }

            if((k-tNum)>=0)
            {
                i1 = i+(j+(k-tNum)*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft);
                q[i1] = qq*qCalcCoef;//(T00[0] - localTarray[k]-273.15);
                //qDebug() << k << q[i1];
            }
            //qDebug() << k <<
        }



        delete localTarray;
        delete localTimeArray;
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
        mult *= -x*x/++n;
        memb = x/(2*n+1)*mult;
        sum += memb;
    }while(fabs(memb) >= eps);

    return 1.- 2./sqrt(M_PI)*sum;
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
    //QGraphicsScene *scene = new QGraphicsScene(ui->graphicsView);
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
    displayScene->addPixmap(pic);

    if(!pointsForPlots.isEmpty())
    {
        for(i=0; i<pointsForPlots.size(); i++)
        {
            displayScene->addEllipse(pointsForPlots.at(i).x()-rad, pointsForPlots.at(i).y()-rad, rad*2.0, rad*2.0, QPen(), QBrush(Qt::SolidPattern));
        }
    }

    ui->graphicsView->setScene(displayScene);
    delete localTdata;
}

void MainWindow::on_pushButton_2_clicked()
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
        ui->putMarks->setDisabled(true);
        ui->pushButton_2->setDisabled(true);
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
        ui->putMarks->setDisabled(false);
        //ui->pushButton_2->setDisabled(false);
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
        ui->cutButton->setText("Обрезать");
        ui->cutButton->setEnabled(false);
        displayField(ui->horizontalSlider->value());
        ui->horizontalSlider->setValue(ui->horizontalSlider->value());
        ui->horizontalSlider->setGeometry(ui->graphicsView->x(),ui->graphicsView->y()+ui->graphicsView->height()+5,ui->graphicsView->width(),ui->horizontalSlider->height());
        ui->labelNumDisplay->setGeometry(ui->graphicsView->x()+ui->graphicsView->width()*45/100-43,ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height(),ui->labelNumDisplay->width(),ui->labelNumDisplay->height());
        ui->labelTimeDisplay->setGeometry(ui->graphicsView->x()+ui->graphicsView->width()*45/100,ui->graphicsView->y()+ui->graphicsView->height()+ui->horizontalSlider->height(),ui->labelTimeDisplay->width(),ui->labelTimeDisplay->height());
    }
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
        for(j = cutUp; j < myWidth - cutDown; j++)
        {
            for(i = cutLeft; i < myLength - cutRight; i++)
            {
                TfieldSmooth.push_back(Tfield[i+(j+k*myWidth)*myLength]);
                q.push_back(0.0);
                //TfieldSmooth.push_back(Tfield[i+(j+k*(myWidth-cutDown-cutUp))*(myLength-cutLeft-cutRight)]);
                //kk++;
            }
        }
    }
}

void MainWindow::on_putMarks_clicked()
{
    if(ui->putMarks->text() == "Поставить метки")
    {
        ui->putMarks->setText("Завершить");
        createSmoothField();
        ui->cutButton->setDisabled(true);
        ui->pushButton_2->setDisabled(true);
        pointsForPlots.clear();
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
        //ui->pushButton_2->setDisabled(false);
        ui->action->setDisabled(false);
        ui->action_2->setDisabled(false);
        ui->action_4->setDisabled(false);
        ui->putMarks->setDisabled(true);
        //ui->qCalcButton->setEnabled(true);
        ui->processingMethodButton->setEnabled(true);
        //ui->facilityBox->setDisabled(false);
        //ui->methodsBox->setDisabled(false);
    }
}

void MainWindow::on_facilityName_changed()
{
    if(ui->facilityBox->currentIndex() == 0)
    {
        ui->methodsBox->clear();
        QStringList lst("LowPass");
        ui->methodsBox->addItems(lst);
    }
    else
    {
        ui->methodsBox->clear();
        QStringList lst;
        lst << "LowPass" << "Polynominal" << "ERFC";
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
        timeArrayERFC = new double[(myWidth-cutUp-cutDown)*(myLength-cutRight-cutLeft)*10];
        TFieldERFC = new double[(myWidth-cutUp-cutDown)*(myLength-cutRight-cutLeft)*10];
    }
    else
    {
        ui->facilityBox->setDisabled(true);
        ui->methodsBox->setDisabled(true);
        ui->processingMethodButton->setText("Выбрать метод обработки");
        ui->processingMethodButton->setDisabled(true);
        ui->qCalcButton->setEnabled(true);
    }
}

void MainWindow::on_action_2_triggered()
{
    settingsDialog->setGeometry(550,400,200,180);
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
}

void MainWindow::setRho( )
{
    QString str = rhoEdit->text();
    if(str.toDouble() > 0)
        rho = str.toDouble();
    kCoef = rho*Cp*lambda;
    qCalcCoef = 2*sqrt(kCoef/M_PI);
}

void MainWindow::setCp()
{
    QString str = CpEdit->text();
    if(str.toDouble() > 0)
        Cp = str.toDouble();
    kCoef = rho*Cp*lambda;
    qCalcCoef = 2*sqrt(kCoef/M_PI);
}

void MainWindow::setLambda( )
{
    QString str = lambdaEdit->text();
    if(str.toDouble() > 0)
        lambda = str.toDouble();
    kCoef = rho*Cp*lambda;
    qCalcCoef = 2*sqrt(kCoef/M_PI);
}

void MainWindow::on_methodsBox_currentIndexChanged(int index)
{
    //emit replotNeeded();
}

void MainWindow::replot()
{
    QPen pen(Qt::red);
    QPen pen2(Qt::blue);
    int viewWidth = 300;
    int viewHeight = 200;
    double min=1000.0, max=-100.0;
    double minq=1000.0, maxq=-100.0;
    QVector<double> x, y, y2, qSet;

    //plotsList.clear();
    for(int ii = 1; ii<pointsForPlots.length(); ii+=2)
    //for(auto &ii : pointsForPlots)
    {
        //QCustomPlot *customPlot = new QCustomPlot;
        //QCustomPlot *customPlot2 = new QCustomPlot;
        //pointsForPlots[ii].
        x.clear();
        y.clear();
        y2.clear();
        qSet.clear();
        int i = pointsForPlots[ii-1].x();
        int j = pointsForPlots[ii-1].y();
        //qDebug() << "we are here";
        for (int k=0; k<timeArray.size(); k++)
        {
          x.push_back(timeArray.at(k));
          y.push_back(Tfield[i+(j+k*myWidth)*myLength]);

          if(min >= y.at(k))
              min = y.at(k);
          if(max <= y.at(k))
              max = y.at(k);
        }

        plotsList[ii-1]->removeGraph(0);
        plotsList[ii-1]->removeGraph(0);
        plotsList[ii-1]->addGraph();
        plotsList[ii-1]->graph(0)->setData(x, y);
        plotsList[ii-1]->graph(0)->setPen(pen);
        plotsList[ii-1]->graph(0)->setLineStyle(QCPGraph::lsNone);
        plotsList[ii-1]->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossSquare, 4));
        std::stringstream str;
        str << "Point №" << ii-1;
        plotsList[ii-1]->graph(0)->setName(str.str().c_str());

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

        plotsList[ii-1]->addGraph();
        processField(i,j,ui->methodsBox->currentIndex());
        qCalc(i,j);

        for (int k=0; k<timeArray.size(); k++)
        {
            y2.push_back(TfieldSmooth[i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft)]);
            qSet.push_back(q[i+(j+k*(myWidth-cutUp-cutDown))*(myLength-cutRight-cutLeft)]);
            if(minq >= qSet.at(k))
                minq = qSet.at(k);
            if(maxq <= qSet.at(k))
                maxq = qSet.at(k);
        }

        plotsList[ii-1]->graph(1)->setData(x, y2);
        plotsList[ii-1]->graph(1)->setPen(pen2);
        plotsList[ii-1]->graph(1)->setName(str.str().c_str());
        plotsList[ii]->removeGraph(0);
        plotsList[ii]->addGraph();
        plotsList[ii]->graph(0)->setData(x, qSet);
        plotsList[ii]->graph(0)->setPen(pen);
        plotsList[ii]->graph(0)->setName(str.str().c_str());
        plotsList[ii]->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::AlignBottom|Qt::AlignRight));
        plotsList[ii]->legend->setVisible(true);
        plotsList[ii]->xAxis->setLabel("t, с");
        plotsList[ii]->yAxis->setLabel("q, Вт/м2");
        plotsList[ii]->xAxis->setRange(timeArray.first(), timeArray.last());
        plotsList[ii]->yAxis->setRange(minq, maxq);
        //customPlot2->setGeometry(10+ (10+viewWidth)*((pointsForPlots.size()-1)%2),10+ (10+viewHeight)*((pointsForPlots.size()-1) - ((pointsForPlots.size()-1)%2))/2,viewWidth,viewHeight);
        plotsList[ii]->replot();

        plotsList[ii-1]->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::AlignBottom|Qt::AlignRight));
        plotsList[ii-1]->legend->setVisible(true);
        plotsList[ii-1]->xAxis->setLabel("t, с");
        plotsList[ii-1]->yAxis->setLabel("T, °C");
        plotsList[ii-1]->xAxis->setRange(timeArray.first(), timeArray.last());
        plotsList[ii-1]->yAxis->setRange(min-5, max+5);
        //customPlot->setGeometry(10+ (10+viewWidth)*((pointsForPlots.size()-1)%2),10+ (10+viewHeight)*((pointsForPlots.size()-1) - ((pointsForPlots.size()-1)%2))/2,viewWidth,viewHeight);
        plotsList[ii-1]->replot();
        plotsList[ii]->replot();
        plotsList[ii-1]->setMinimumSize(viewWidth,viewHeight);
        plotsList[ii]->setMinimumSize(viewWidth,viewHeight);
        plotsList[ii-1]->show();
        plotsList[ii]->show();

        //plotsLayout->addWidget(customPlot,ii,0);
        //plotsLayout->addWidget(customPlot2,ii,1);
        //plotsWidget->setLayout(plotsLayout);
        //ui->scrollArea->setWidget(plotsWidget);

       // for(ii = 0; ii < pointsForPlots.size(); ii++)
       // {
       //     qDebug() << pointsForPlots.at(ii).x() << 512-pointsForPlots.at(ii).y();
        //}

        //if(ii == pointsForPlots.length()-1)
            //for (int k=0; k<timeArray.size(); k++)
            //{
            //    qDebug() << y[k] << y2[k];
            //}
    }
}