#include "sliderwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>

namespace
{
    const int WIDTH = 128;
    const int HEIGHT = 24;
    const int SIDE = 4;
    const int BACKGROUND = 2;
}

/*SliderWidget::SliderWidget(QWidget* parent)
    : QWidget(parent){}

{
    SetRange(0,100);
    //Min_ = 0;
    //Max_ = 100;
    SetValue(0);
    //Value_ = 0;
    IsMouseOver_ = false;
    IsMoving_=false;
    Width_ = 0;
    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::ClickFocus);
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}*/
SliderWidget::SliderWidget(QWidget* parent)
    : QWidget(parent)
    , Min_(0)
    , Max_(100)
    , Value_(0)
    , IsMouseOver_(false)
    , IsMoving_(false)
    , Width_(0)
{
    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::ClickFocus);
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

SliderWidget::SliderWidget()
{

}

void SliderWidget::SetValue(double value)
{
    if (value < Min_)
    {
        value = Min_;
    }
    else if (value > Max_)
    {
        value = Max_;
    }

    if (value != Value_)
    {
        Value_ = value;
        this->repaint();
        emit ValueChanged(Value_);
    }
}

void SliderWidget::SetRange(double min, double max)
{
    //assert(min < max);

    Min_ = min;
    Max_ = max;

    SetValue(Value_);

    this->repaint();
}

double SliderWidget::Value() const
{
    return Value_;
}

QSize SliderWidget::minimumSizeHint() const
{
    return QSize(WIDTH, HEIGHT);
}

void SliderWidget::leaveEvent(QEvent* /*event*/)
{
    if (IsMouseOver_)
    {
        IsMouseOver_ = false;
        this->repaint();
    }
}

void SliderWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (IsMoving_)
    {
        const double value = PosToValue(event->x());
        SetValue(value);
    }
    else if (!IsMouseOver_ && SliderRect_.contains(event->pos()))
    {
        IsMouseOver_ = true;
        this->repaint();
    }
    else if (IsMouseOver_ && !SliderRect_.contains(event->pos()))
    {
        IsMouseOver_ = false;
        this->repaint();
    }
}

void SliderWidget::mousePressEvent(QMouseEvent* event)
{
    if (SliderRect_.contains(event->pos()))
    {
        IsMoving_ = true;
    }
    else
    {
        SetValue(PosToValue(event->x()));
    }
}

void SliderWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    IsMoving_ = false;
}

void SliderWidget::keyPressEvent(QKeyEvent* event)
{
    const double tick = (Max_ - Min_) / Width_;
    if (event->key() == Qt::Key_Left)
    {
        SetValue(Value_ - tick);
    }
    else if (event->key() == Qt::Key_Right)
    {
        SetValue(Value_ + tick);
    }
}

void SliderWidget::paintEvent(QPaintEvent* event)
{
    Width_ = event->rect().width();

    int x = Width_ / (Max_ - Min_) * (Value_ - Min_);
    if (x < SIDE)
    {
        x = SIDE;
    }
    else if (x + SIDE > Width_ - 1)
    {
        x = Width_ - SIDE - 2;
    }

    SliderRect_ = QRect(x - SIDE, 0, SIDE * 2, HEIGHT);

    QPainter painter(this);

    const QBrush back = palette().window();
    const QBrush shadow = palette().shadow();
    const QBrush line = Qt::black;
    const QBrush light = palette().light();
    const QBrush fill = IsMouseOver_ ? light : back;

    painter.setPen(shadow.color());
    painter.setBrush(back);
    painter.drawRect(0, HEIGHT / 2 - BACKGROUND,Width_ - 1, BACKGROUND * 2);

    painter.setBrush(shadow);
    const QPoint sliderShadow[] = {
        QPoint(x + 1, 1),
        QPoint(x + SIDE + 1, SIDE + 1),
        QPoint(x + SIDE + 1, HEIGHT - 1),
        QPoint(x - SIDE + 1, HEIGHT - 1),
        QPoint(x - SIDE + 1, SIDE + 1),
        QPoint(x + 1, 1),
    };
    painter.drawPolygon(sliderShadow, sizeof(sliderShadow) / sizeof(sliderShadow[0]));

    painter.setPen(line.color());
    painter.setBrush(fill);
    const QPoint slider[] = {
        QPoint(x, 0),
        QPoint(x + SIDE, SIDE),
        QPoint(x + SIDE, HEIGHT - 2),
        QPoint(x - SIDE, HEIGHT - 2),
        QPoint(x - SIDE, SIDE),
        QPoint(x, 0),
    };
    painter.drawPolygon(slider, sizeof(slider) / sizeof(slider[0]));

    painter.drawLine(x, 3, x, HEIGHT - 4);

    painter.setPen(light.color());
    painter.drawLine(x - SIDE + 1, SIDE + 1, x, 2);
    painter.drawLine(x + 1, 4, x + 1, HEIGHT - 3);
}

double SliderWidget::PosToValue(int x) const
{
    return (x * (Max_ - Min_) + Width_ * Min_) / static_cast<double>(Width_);
}
