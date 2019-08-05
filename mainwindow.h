#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include <QWidget>
#include <QFileSystemModel>
#include <QDir>
#include <QDebug>
//#include <QListWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <Qstring>
#include <QVector>
#include <QTime>
#include <QImage>
#include <QtWidgets>
#include <QWidget>
#include <QGraphicsSceneMouseEvent>
#include "doublesliderwidget.h"
#include "doublesliderwidgetvert.h"
#include "qcustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow//, QListWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);//{loadingTfieldOneMoreTime = 0;};
    QGraphicsScene *grScene;
    QLabel *rangeLabel;
    ~MainWindow();
    QString openFileName;
    int myLength;
    int myWidth;
    int cutLeft, cutRight, cutUp, cutDown;
    int initialNum;
    int polyPower;
    static const int frameThickness = 2;
    static const int LowPassFlag = 0;
    static const int PolynominalFlag = 1;
    static const int ERFCFlag = 2;
    QVector<float> Tfield;//x y t
    QVector<float> TfieldSmooth;//x y t
    QVector<float> q;//x y t
    QVector<double> timeArray;
    //double *timeArrayERFC;
    double timeArrayERFC[20];
    double *tau0;
    double *TFieldERFC;
    double rho, Cp, lambda, kCoef, deltaTau, qCalcCoef;
    float lambda1;
    float lambda2;
    int timeNum;
    float Tmax, Tmin, Tinit;
    float T00, alpha0;
    uchar *Tdata;
    bool loadingTfieldOneMoreTime;
    QVector<QPoint> pointsForPlots;
    QList<QCustomPlot*> plotsList;
    DoubleSliderWidget *hWid;
    DoubleSliderWidgetVert *vWid;
    QWidget *plotsWidget;
    QDialog *settingsDialog;
    QDialog *flowParamsDialog;
    QGridLayout *plotsLayout;
    //float ***Tfield;
    QLineEdit *rhoEdit;
    QLineEdit *CpEdit;
    QLineEdit *lambdaEdit;
    QLineEdit *T00Edit;
    QLineEdit *alpha0Edit;
    QLabel *rhoLabel;
    QLabel *CpLabel;
    QLabel *lambdaLabel;
    QLabel *T00Label;
    QLabel *alpha0Label;
    QWidget *plotsWidget2;
    QWidget *plotsWidget3;
    //QCustomPlot *customPlot;
    //QCustomPlot *customPlot2;
    QVector<QCustomPlot> plotsVector;
private slots:
    void on_action_triggered();

    void on_action_3_triggered();

    //void on_listView_doubleClicked(const QModelIndex &index);

    void slotAboutProgram();

    void setWidth(float w);

    void setLength(float l);

    void displayField(int timeMoment);
    void displayTRange();

    //void on_pushButton_2_clicked();

    //void on_Play_clicked();

    //void timerTick();

    void on_horizontalSlider_valueChanged(int position);
    void on_cutButton_clicked();
    void setCutsRelation();
    void createSmoothField();
    //void setMethodFlag(int m);
private:
    Ui::MainWindow *ui;
    //QFileSystemModel *model;
    void getInitialInfoFromFile();
    void writeTemperatureToTecplot();
    //void changeCharData(int l,int w,int Nt);
    void createCharData();
    void mousePressEvent(QMouseEvent *event);
    void processField(int i, int j, int methodFlag);
    void qCalc(int i, int j, int methodFlag);
    double erfc(double x);
    double fTheta(double x, double alpha);
signals:
    void replotNeeded();
private slots:
    void on_valueChangedLeft();
    void on_valueChangedRight();
    void on_valueChangedUp();
    void on_valueChangedDown();
    void on_putMarks_clicked();
    void on_facilityName_changed();
    void on_processingMethodButton_clicked();
    void on_action_2_triggered();
    void setRho( );
    void setCp( );
    void setLambda();
    void setT00();
    void setAlpha0();
    void replot();
    void on_qCalcButton_clicked();
    void on_flowParamsAction_triggered();
    int getPropessingFlag();
};

#endif // MAINWINDOW_H
