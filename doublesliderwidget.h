#ifndef DOUBLESLIDERWIDGET_H
#define DOUBLESLIDERWIDGET_H

#include <QWidget>
class DoubleSliderWidget : public QWidget
{
public:
    Q_OBJECT
public:
    explicit DoubleSliderWidget(QWidget* parent);
    explicit DoubleSliderWidget();
    void setValueLeft(int value);
    void setValueRight(int value);
    void setRange(int min, int max);
    void setHandleOrientDown();
    void setHandleOrientUp();
    int getValueLeft() const;
    int getValueRight() const;

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
    void valueChangedLeft(int);
    void valueChangedRight(int);
private:
    bool orientDown;
    double valueLeft_;
    double valueRight_;
    bool isMouseOverLeft_;
    bool isMovingLeft_;
    bool isMouseOverRight_;
    bool isMovingRight_;

    QRect sliderRect_1;
    QRect sliderRect_2;
    int width_;
};

#endif // DOUBLESLIDERWIDGET_H
