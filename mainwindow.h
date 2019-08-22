#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QDir>
#include <QDebug>
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
    QVector<QCustomPlot> plotsVector;
private:
    Ui::MainWindow *ui;
    void setWidth(float w);
    void setLength(float l);
    void displayField(int timeMoment);
    void displayTRange();
    void getInitialInfoFromFile();
    void writeTemperatureToTecplot();
    void createCharData();
    void mousePressEvent(QMouseEvent *event);
    void processField(int pointX, int pointY, int methodFlag);
    void qCalc(int pointX, int pointY, int methodFlag);
    double erfc(double x);
    double fTheta(double x, double alpha);
    int getPropessingFlag();
    void setCutsRelation();
    void createSmoothField();
private slots:
    void redrawPlots();
    void on_valueChangedLeft();
    void on_valueChangedRight();
    void on_valueChangedUp();
    void on_valueChangedDown();
    void on_putMarks_clicked();
    void on_facilityName_changed();
    void on_processingMethodButton_clicked();
    void on_action_2_triggered();
    void on_qCalcButton_clicked();
    void on_flowParamsAction_triggered();
    void on_action_triggered();
    void on_action_3_triggered();
    void slotAboutProgram();
    void on_horizontalSlider_valueChanged(int position);
    void on_cutButton_clicked();
    void setRho( );
    void setCp( );
    void setLambda();
    void setT00();
    void setAlpha0();
};

#endif // MAINWINDOW_H
