#include "doublesliderwidget.h"
#include <QMouseEvent>
#include <QPainter>
//#include <QWidget>

namespace
{
    const int WIDTH = 128;
    const int HEIGHT = 24;
    const int SIDE = 4;
    const int BACKGROUND = 2;
}

DoubleSliderWidget::DoubleSliderWidget(QWidget* parent)
    : QWidget(parent)
    , min_(0)
    , max_(100)
    , valueLeft_(50)
    , valueRight_(100)
    , isMouseOverLeft_(false)
    , isMovingLeft_(false)
    , isMouseOverRight_(false)
    , isMovingRight_(false)
    , width_(0)
    , orientDown(true)
{
    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::ClickFocus);
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

DoubleSliderWidget::DoubleSliderWidget()
{

}

void DoubleSliderWidget::setValueLeft(int value)
{
    if (value < min_)
    {
        value = min_;
    }
    //else if (value > max_)
    //{
    //    value = max_;
    //}

    if (value != valueLeft_)
    {
        valueLeft_ = value;
        this->repaint();
        emit valueChangedLeft(valueLeft_);
    }
}

void DoubleSliderWidget::setValueRight(int value)
{
    //if (value < min_)
    //{
    //    value = min_;
    //}
    if (value > max_)
    {
        value = max_;
    }

    if (value != valueRight_)
    {
        valueRight_ = value;
        this->repaint();
        emit valueChangedRight(valueRight_);
    }
}

void DoubleSliderWidget::setRange(int min, int max)
{
    //assert(min < max);

    min_ = min;
    max_ = max;

    setValueLeft(valueLeft_);
    setValueRight(valueRight_);

    this->repaint();
}

void DoubleSliderWidget::setHandleOrientUp()
{
    orientDown = false;
}

void DoubleSliderWidget::setHandleOrientDown()
{
    orientDown = true;
}

int DoubleSliderWidget::getValueRight() const
{
    return valueRight_;
}

int DoubleSliderWidget::getValueLeft() const
{
    return valueLeft_;
}

QSize DoubleSliderWidget::minimumSizeHint() const
{
    return QSize(WIDTH, HEIGHT);
}

void DoubleSliderWidget::leaveEvent(QEvent* /*event*/)
{
    if (isMouseOverLeft_)
    {
        isMouseOverLeft_ = false;
        this->repaint();
    }

    if (isMouseOverRight_)
    {
        isMouseOverRight_ = false;
        this->repaint();
    }
}

void DoubleSliderWidget::mouseMoveEvent(QMouseEvent* event)
{
    /*if (isMoving_)
    {
        //const int value = posToValue(event->x());
        //setValue(value);

        //setValue(event->x());
        if(event->x() >=valueLeft_)
            setValueRight(event->x());
        else {
            setValueLeft(event->x());
        }

        if(event->x() >=valueRight_)
            setValueRight(event->x());
        else
            if(event->x() <=valueLeft_)
                {
                    setValueLeft(event->x());
                }



    }
    else if (!isMouseOver_ && sliderRect_1.contains(event->pos()) || !isMouseOver_ && sliderRect_2.contains(event->pos()))
    {
        isMouseOver_ = true;
        this->repaint();
    }
    else if (isMouseOver_ && !sliderRect_1.contains(event->pos()) || isMouseOver_ && !sliderRect_2.contains(event->pos()))
    {
        isMouseOver_ = false;
        this->repaint();
    }*/

    if (isMovingLeft_)
        {
            if(event->x() <valueRight_)
                setValueLeft(event->x());
        }
        else if (!isMouseOverLeft_ && sliderRect_1.contains(event->pos()))
        {
            isMouseOverLeft_ = true;
            this->repaint();
        }
        else if (isMouseOverLeft_ && !sliderRect_1.contains(event->pos()))
        {
            isMouseOverLeft_ = false;
            this->repaint();
        }

    if (isMovingRight_)
        {
            if(event->x() >valueLeft_)
                setValueRight(event->x());
        }
        else if (!isMouseOverRight_ && sliderRect_2.contains(event->pos()))
        {
            isMouseOverRight_ = true;
            this->repaint();
        }
        else if (isMouseOverRight_ && !sliderRect_2.contains(event->pos()))
        {
            isMouseOverRight_ = false;
            this->repaint();
        }


}

void DoubleSliderWidget::mousePressEvent(QMouseEvent* event)
{
    if (sliderRect_1.contains(event->pos()))
    {
        isMovingLeft_ = true;
    }
    else if (sliderRect_2.contains(event->pos()))
    {
        isMovingRight_ = true;
    }
    else
    {
        //setValue(posToValue(event->x()));
        if(event->x() >=valueRight_)
            setValueRight(event->x());
        else
        {
            if(event->x() <=valueLeft_)
                setValueLeft(event->x());
            else
            {
                int deltaX1 = event->x() - getValueLeft();
                int deltaX2 = getValueRight() - event->x();
                setValueLeft(valueLeft_ + deltaX1*2/5);
                setValueRight(valueRight_ - deltaX2*2/5);
            }
        }
    }


}

void DoubleSliderWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    isMovingLeft_ = false;
    isMovingRight_ = false;
}

void DoubleSliderWidget::paintEvent(QPaintEvent* event)
{
    width_ = event->rect().width();

    int x = valueLeft_ - min_;
    if (x < SIDE)
    {
        x = SIDE;
    }
    else if (x + SIDE > width_ - 1)
    {
        x = width_ - SIDE - 2;
    }

    sliderRect_1 = QRect(x - SIDE, 0, SIDE * 2, HEIGHT);

    QPainter painter(this);

    const QBrush back = palette().window();
    const QBrush shadow = palette().shadow();
    const QBrush line = Qt::black;
    const QBrush light = palette().light();
    const QBrush fill = light;

    painter.setPen(shadow.color());
    painter.setBrush(back);
    painter.drawRect(0, HEIGHT / 2 - BACKGROUND,
        width_ - 1, BACKGROUND * 2);

    painter.setBrush(shadow);

    QPoint sliderShadow[6];

    if(orientDown == false)
    {
            sliderShadow[0] = QPoint(x + 1, 1);
            sliderShadow[1] = QPoint(x + SIDE + 1, SIDE + 1);
            sliderShadow[2] = QPoint(x + SIDE + 1, HEIGHT - 1);
            sliderShadow[3] = QPoint(x - SIDE + 1, HEIGHT - 1);
            sliderShadow[4] = QPoint(x - SIDE + 1, SIDE + 1);
            sliderShadow[5] = QPoint(x + 1, 1);
    }
    else
    {
            sliderShadow[0] = QPoint(x + 1, HEIGHT - 1);
            sliderShadow[1] = QPoint(x + SIDE + 1, HEIGHT - SIDE - 1);
            sliderShadow[2] = QPoint(x + SIDE + 1, 1);
            sliderShadow[3] = QPoint(x - SIDE + 1, 1);
            sliderShadow[4] = QPoint(x - SIDE + 1, HEIGHT - SIDE - 1);
            sliderShadow[5] = QPoint(x + 1, HEIGHT - 1);
    }
    painter.drawPolygon(sliderShadow, sizeof(sliderShadow) / sizeof(sliderShadow[0]));

    painter.setPen(line.color());
    painter.setBrush(fill);

    QPoint slider[6];

    if(orientDown == false)
    {
            slider[0] = QPoint(x, 0);
            slider[1] = QPoint(x + SIDE, SIDE);
            slider[2] = QPoint(x + SIDE, HEIGHT - 2);
            slider[3] = QPoint(x - SIDE, HEIGHT - 2);
            slider[4] = QPoint(x - SIDE, SIDE);
            slider[5] = QPoint(x, 0);
            painter.drawPolygon(slider, sizeof(slider) / sizeof(slider[0]));
            painter.drawLine(x, 3, x, HEIGHT - 4);

            painter.setPen(light.color());
            painter.drawLine(x - SIDE + 1, SIDE + 1, x, 2);
            painter.drawLine(x + 1, 4, x + 1, HEIGHT - 3);
    }
    else
    {
            slider[0] = QPoint(x, HEIGHT - 2);
            slider[1] = QPoint(x + SIDE, HEIGHT - 2 - SIDE);
            slider[2] = QPoint(x + SIDE, 0);
            slider[3] = QPoint(x - SIDE, 0);
            slider[4] = QPoint(x - SIDE, HEIGHT - 2 - SIDE);
            slider[5] = QPoint(x, HEIGHT - 2);
            painter.drawPolygon(slider, sizeof(slider) / sizeof(slider[0]));
            painter.drawLine(x, 2, x, HEIGHT - 5);

            painter.setPen(light.color());
            //painter.drawLine(x - SIDE + 1, SIDE + 1, x, 2);
            //painter.drawLine(x + SIDE - 1, HEIGHT - SIDE - 1, x, HEIGHT - 2);
            painter.drawLine(x + 1, 3, x + 1, HEIGHT - 4);
    }

    x = valueRight_ - min_;
    if (x < SIDE)
    {
        x = SIDE;
    }
    else if (x + SIDE > width_ - 1)
    {
        x = width_ - SIDE - 2;
    }

    sliderRect_2 = QRect(x - SIDE, 0, SIDE * 2, HEIGHT);

    //QPainter painter(this);

    //const QBrush back = palette().window();
    //const QBrush shadow = palette().shadow();
    //const QBrush line = Qt::black;
    //const QBrush light = palette().light();
    //const QBrush fill = isMouseOver_ ? light : back;

    painter.setPen(shadow.color());
    painter.setBrush(back);
    //painter.drawRect(0, HEIGHT / 2 - BACKGROUND,
        //width_ - 1, BACKGROUND * 2);

    painter.setBrush(shadow);
    QPoint sliderShadow2[6];
    if(orientDown == false)
    {
            sliderShadow2[0] = QPoint(x + 1, 1);
            sliderShadow2[1] = QPoint(x + SIDE + 1, SIDE + 1);
            sliderShadow2[2] = QPoint(x + SIDE + 1, HEIGHT - 1);
            sliderShadow2[3] = QPoint(x - SIDE + 1, HEIGHT - 1);
            sliderShadow2[4] = QPoint(x - SIDE + 1, SIDE + 1);
            sliderShadow2[5] = QPoint(x + 1, 1);
    }
    else
    {
            sliderShadow2[0] = QPoint(x + 1, HEIGHT - 1);
            sliderShadow2[1] = QPoint(x + SIDE + 1, HEIGHT - SIDE - 1);
            sliderShadow2[2] = QPoint(x + SIDE + 1, 1);
            sliderShadow2[3] = QPoint(x - SIDE + 1, 1);
            sliderShadow2[4] = QPoint(x - SIDE + 1, HEIGHT - SIDE - 1);
            sliderShadow2[5] = QPoint(x + 1, HEIGHT - 1);
    }
    painter.drawPolygon(sliderShadow2, sizeof(sliderShadow2) / sizeof(sliderShadow2[0]));

    painter.setPen(line.color());
    painter.setBrush(fill);
    QPoint slider2[6];

    if(orientDown == false)
    {
            slider2[0] = QPoint(x, 0);
            slider2[1] = QPoint(x + SIDE, SIDE);
            slider2[2] = QPoint(x + SIDE, HEIGHT - 2);
            slider2[3] = QPoint(x - SIDE, HEIGHT - 2);
            slider2[4] = QPoint(x - SIDE, SIDE);
            slider2[5] = QPoint(x, 0);
            painter.drawPolygon(slider2, sizeof(slider2) / sizeof(slider2[0]));
            painter.drawLine(x, 3, x, HEIGHT - 4);

            painter.setPen(light.color());
            painter.drawLine(x - SIDE + 1, SIDE + 1, x, 2);
            painter.drawLine(x + 1, 4, x + 1, HEIGHT - 3);
    }
    else
    {
            slider2[0] = QPoint(x, HEIGHT - 2);
            slider2[1] = QPoint(x + SIDE, HEIGHT - 2 - SIDE);
            slider2[2] = QPoint(x + SIDE, 0);
            slider2[3] = QPoint(x - SIDE, 0);
            slider2[4] = QPoint(x - SIDE, HEIGHT - 2 - SIDE);
            slider2[5] = QPoint(x, HEIGHT - 2);
            painter.drawPolygon(slider2, sizeof(slider2) / sizeof(slider2[0]));
            painter.drawLine(x, 2, x, HEIGHT - 5);

            painter.setPen(light.color());
            //painter.drawLine(x - SIDE + 1, SIDE + 1, x, 2);
            //painter.drawLine(x + SIDE - 1, HEIGHT - SIDE - 1, x, HEIGHT - 2);
            painter.drawLine(x + 1, 3, x + 1, HEIGHT - 4);
    }
}
