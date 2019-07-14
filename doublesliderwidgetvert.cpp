#include "doublesliderwidgetvert.h"
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

DoubleSliderWidgetVert::DoubleSliderWidgetVert(QWidget* parent)
    : QWidget(parent)
    , min_(0)
    , max_(100)
    , valueUp_(50)
    , valueDown_(100)
    , isMouseOverUp_(false)
    , isMovingUp_(false)
    , isMouseOverDown_(false)
    , isMovingDown_(false)
    , width_(0)
    , orientLeft(true)
{
    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::ClickFocus);
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

DoubleSliderWidgetVert::DoubleSliderWidgetVert()
{
}

void DoubleSliderWidgetVert::setValueUp(int value)
{
    if (value < min_)
    {
        value = min_;
    }

    if (value != valueUp_)
    {
        valueUp_ = value;
        this->repaint();
        emit valueChangedUp(valueUp_);
    }
}

void DoubleSliderWidgetVert::setValueDown(int value)
{
    if (value > max_)
    {
        value = max_;
    }

    if (value != valueDown_)
    {
        valueDown_ = value;
        this->repaint();
        emit valueChangedDown(valueDown_);
    }
}

void DoubleSliderWidgetVert::setRange(int min, int max)
{
    min_ = min;
    max_ = max;

    setValueUp(valueUp_);
    setValueDown(valueDown_);

    this->repaint();
}

int DoubleSliderWidgetVert::getValueDown() const
{
    return valueDown_;
}

void DoubleSliderWidgetVert::setHandleOrientLeft()
{
    orientLeft = true;
}

void DoubleSliderWidgetVert::setHandleOrientRight()
{
    orientLeft = false;
}

int DoubleSliderWidgetVert::getValueUp() const
{
    return valueUp_;
}

QSize DoubleSliderWidgetVert::minimumSizeHint() const
{
    return QSize(WIDTH, HEIGHT);
}

void DoubleSliderWidgetVert::leaveEvent(QEvent* /*event*/)
{
    if (isMouseOverUp_)
    {
        isMouseOverUp_ = false;
        this->repaint();
    }

    if (isMouseOverDown_)
    {
        isMouseOverDown_ = false;
        this->repaint();
    }
}

void DoubleSliderWidgetVert::mouseMoveEvent(QMouseEvent* event)
{
    if (isMovingUp_)
        {
            if(event->y() <valueDown_)
                setValueUp(event->y());
        }
        else if (!isMouseOverUp_ && sliderRect_1.contains(event->pos()))
        {
            isMouseOverUp_ = true;
            this->repaint();
        }
        else if (isMouseOverUp_ && !sliderRect_1.contains(event->pos()))
        {
            isMouseOverUp_ = false;
            this->repaint();
        }

    if (isMovingDown_)
        {
            if(event->y() >valueUp_)
                setValueDown(event->y());
        }
        else if (!isMouseOverDown_ && sliderRect_2.contains(event->pos()))
        {
            isMouseOverDown_ = true;
            this->repaint();
        }
        else if (isMouseOverDown_ && !sliderRect_2.contains(event->pos()))
        {
            isMouseOverDown_ = false;
            this->repaint();
        }
}

void DoubleSliderWidgetVert::mousePressEvent(QMouseEvent* event)
{
    if (sliderRect_1.contains(event->pos()))
    {
        isMovingUp_ = true;
    }
    else if (sliderRect_2.contains(event->pos()))
    {
        isMovingDown_ = true;
    }
    else
    {
        if(event->y() >=valueDown_)
            setValueDown(event->y());
        else
        {
            if(event->y() <=valueUp_)
                setValueUp(event->y());
            else
            {
                int deltaY1 = event->y() - getValueUp();
                int deltaY2 = getValueDown() - event->y();
                setValueUp(valueUp_ + deltaY1/5*2);
                setValueDown(valueDown_ - deltaY2/5*2);
            }
        }
    }
}

void DoubleSliderWidgetVert::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    isMovingUp_ = false;
    isMovingDown_ = false;
}

void DoubleSliderWidgetVert::paintEvent(QPaintEvent* event)
{
    width_ = event->rect().height();

    int y = valueUp_ - min_;
    if (y < SIDE)
    {
        y = SIDE;
    }
    else if (y + SIDE > width_ - 1)
    {
        y = width_ - SIDE - 2;
    }

    sliderRect_1 = QRect(0, y - SIDE, HEIGHT, SIDE * 2);

    QPainter painter(this);

    const QBrush back = palette().window();
    const QBrush shadow = palette().shadow();
    const QBrush line = Qt::black;
    const QBrush light = palette().light();
    const QBrush fill = light;

    painter.setPen(shadow.color());
    painter.setBrush(back);
    painter.drawRect(HEIGHT / 2 - BACKGROUND, 0, BACKGROUND * 2, width_ - 1);

    painter.setBrush(shadow);

    QPoint sliderShadow[6];
    if(orientLeft == true)
    {
        sliderShadow[0] = QPoint( 1, y +1);
        sliderShadow[1] = QPoint(SIDE + 1,y - SIDE + 1);
        sliderShadow[2] = QPoint(HEIGHT - 1, y - SIDE + 1);
        sliderShadow[3] = QPoint(HEIGHT - 1, y + SIDE + 1);
        sliderShadow[4] = QPoint(SIDE + 1, y + SIDE + 1);
        sliderShadow[5] = QPoint(1, y + 1);
    }
    else
    {
        sliderShadow[0] = QPoint(HEIGHT - 1, y +1);
        sliderShadow[1] = QPoint(HEIGHT - 1 - SIDE,y - SIDE + 1);
        sliderShadow[2] = QPoint(1, y - SIDE + 1);
        sliderShadow[3] = QPoint(1, y + SIDE + 1);
        sliderShadow[4] = QPoint(HEIGHT - 1 - SIDE, y + SIDE + 1);
        sliderShadow[5] = QPoint(HEIGHT - 1, y  +1);
    }

    painter.drawPolygon(sliderShadow, sizeof(sliderShadow) / sizeof(sliderShadow[0]));

    painter.setPen(line.color());
    painter.setBrush(fill);

    QPoint slider[6];
    if(orientLeft == true)
    {
        slider[0] = QPoint(0, y);
        slider[1] = QPoint(SIDE, y - SIDE);
        slider[2] = QPoint(HEIGHT - 2, y - SIDE);
        slider[3] = QPoint(HEIGHT - 2, y + SIDE);
        slider[4] = QPoint(SIDE, y + SIDE);
        slider[5] = QPoint(0, y);
        painter.drawPolygon(slider, sizeof(slider) / sizeof(slider[0]));

        painter.drawLine(3, y, HEIGHT - 4, y);

        painter.setPen(light.color());
        painter.drawLine(SIDE + 1, y + SIDE - 1, 2, y);
        painter.drawLine(4, y - 1, HEIGHT - 3, y - 1);
    }
    else
    {
        slider[0] = QPoint(HEIGHT - 2, y);
        slider[1] = QPoint(HEIGHT - 2 - SIDE,y - SIDE);
        slider[2] = QPoint(0, y - SIDE);
        slider[3] = QPoint(0, y + SIDE);
        slider[4] = QPoint(HEIGHT - 2 - SIDE,y + SIDE);
        slider[5] = QPoint(HEIGHT - 2, y);
        painter.drawPolygon(slider, sizeof(slider) / sizeof(slider[0]));
        painter.drawLine(HEIGHT - 6, y, 3, y);

        painter.setPen(light.color());
        //painter.drawLine(HEIGHT - SIDE - 1, y + SIDE - 1, HEIGHT - 2, y);
        painter.drawLine(3, y - 1, HEIGHT - 4, y - 1);
    }

    y = valueDown_ - min_;
    if (y < SIDE)
    {
        y = SIDE;
    }
    else if (y + SIDE > width_ - 1)
    {
        y = width_ - SIDE - 2;
    }

    sliderRect_2 = QRect(0, y - SIDE, HEIGHT, SIDE * 2);

    painter.setPen(shadow.color());
    painter.setBrush(back);

    painter.setBrush(shadow);
    QPoint sliderShadow2[6];
    if(orientLeft == true)
    {
        sliderShadow2[0] = QPoint( 1, y -1);
        sliderShadow2[1] = QPoint(SIDE + 1,y - (SIDE - 1));
        sliderShadow2[2] = QPoint(HEIGHT - 1, y - SIDE + 1);
        sliderShadow2[3] = QPoint(HEIGHT - 1, y + SIDE + 1);
        sliderShadow2[4] = QPoint(SIDE + 1, y + SIDE + 1);
        sliderShadow2[5] = QPoint(1, y - 1);
    }
    else
    {
        sliderShadow2[0] = QPoint(HEIGHT - 1, y +1);
        sliderShadow2[1] = QPoint(HEIGHT - 1 - SIDE,y - SIDE + 1);
        sliderShadow2[2] = QPoint(1, y - SIDE + 1);
        sliderShadow2[3] = QPoint(1, y + SIDE + 1);
        sliderShadow2[4] = QPoint(HEIGHT - 1 - SIDE, y + SIDE + 1);
        sliderShadow2[5] = QPoint(HEIGHT - 1, y  +1);
    }
    painter.drawPolygon(sliderShadow2, sizeof(sliderShadow2) / sizeof(sliderShadow2[0]));

    painter.setPen(line.color());
    painter.setBrush(fill);
    QPoint slider2[6];
    if(orientLeft == true)
    {
        slider2[0] = QPoint(0, y);
        slider2[1] = QPoint(SIDE, y - SIDE);
        slider2[2] = QPoint(HEIGHT - 2, y - SIDE);
        slider2[3] = QPoint(HEIGHT - 2, y + SIDE);
        slider2[4] = QPoint(SIDE, y + SIDE);
        slider2[5] = QPoint(0, y);
        painter.drawPolygon(slider2, sizeof(slider2) / sizeof(slider2[0]));

        painter.drawLine(3, y, HEIGHT - 4, y);

        painter.setPen(light.color());
        painter.drawLine(SIDE + 1, y + SIDE - 1, 2, y);
        painter.drawLine(4, y - 1, HEIGHT - 3, y - 1);
    }
    else
    {
        slider2[0] = QPoint(HEIGHT - 2, y);
        slider2[1] = QPoint(HEIGHT - 2 - SIDE,y - SIDE);
        slider2[2] = QPoint(0, y - SIDE);
        slider2[3] = QPoint(0, y + SIDE);
        slider2[4] = QPoint(HEIGHT - 2 - SIDE,y + SIDE);
        slider2[5] = QPoint(HEIGHT - 2, y);
        painter.drawPolygon(slider2, sizeof(slider2) / sizeof(slider2[0]));
        painter.drawLine(HEIGHT - 6, y, 3, y);

        painter.setPen(light.color());
        //painter.drawLine(HEIGHT - SIDE - 1, y + SIDE - 1, HEIGHT - 2, y);
        painter.drawLine(3, y - 1, HEIGHT - 4, y - 1);
    }
}
