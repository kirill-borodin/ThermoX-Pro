#ifndef DOUBLESLIDERWIDGETVERT_H
#define DOUBLESLIDERWIDGETVERT_H
#include <QWidget>

class DoubleSliderWidgetVert : public QWidget
{
public:
    Q_OBJECT
public:
    DoubleSliderWidgetVert(QWidget* parent);
    DoubleSliderWidgetVert();
    void setValueUp(int value);
    void setValueDown(int value);
    void setRange(int min, int max);
    int getValueUp() const;
    int getValueDown() const;
    void setHandleOrientLeft();
    void setHandleOrientRight();
    double min_;
    double max_;
protected:
    virtual QSize minimumSizeHint() const;

    virtual void leaveEvent(QEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);

signals:
    void valueChangedUp(int);
    void valueChangedDown(int);
private:
    bool orientLeft;
    double valueUp_;
    double valueDown_;
    bool isMouseOverUp_;
    bool isMovingUp_;
    bool isMouseOverDown_;
    bool isMovingDown_;

    QRect sliderRect_1;
    QRect sliderRect_2;
    int width_;
};

#endif // DOUBLESLIDERWIDGETVERT_H
