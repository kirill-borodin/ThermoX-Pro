#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>

class SliderWidget
    : public QWidget
{
    Q_OBJECT
public:
    explicit SliderWidget(QWidget* parent);
    explicit SliderWidget();
    void SetValue(double value);
    void SetRange(double min, double max);
    double Value() const;

protected:
    virtual QSize minimumSizeHint() const;

    virtual void leaveEvent(QEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void paintEvent(QPaintEvent* event);

signals:
    void ValueChanged(double);

private:
    double PosToValue(int x) const;

private:
    double Min_;
    double Max_;
    double Value_;

    bool IsMouseOver_;
    bool IsMoving_;

    QRect SliderRect_;
    int Width_;
};

#endif // SLIDERWIDGET_H
