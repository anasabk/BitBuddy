#include "Joystick.h"

#include <QMouseEvent>
#include <limits>

Joystick::Joystick(int size, QWidget *parent) :
    QWidget(parent),
    stick(new QWidget(parent)),
    r(size / 2.0f),
    sr(size / 6.0f)
{
    char styleSheet[300];

    setFixedSize(size, size);
    sprintf(styleSheet, "background: rgba(0, 0, 0, 0.3); border: 2px solid #e0e0e0; border-radius: %dpx;", static_cast<int>(r));
    setStyleSheet(styleSheet);

    stick->setFixedSize(sr * 2, sr * 2);
    sprintf(styleSheet, "background: #e0e0e0; border-radius: %dpx;", static_cast<int>(sr));
    stick->setStyleSheet(styleSheet);

    stick->setAttribute(Qt::WA_TransparentForMouseEvents);

    stackUnder(stick);

    moveStick(QPoint(r, r));
}

void Joystick::mousePressEvent(QMouseEvent *event)
{
    isPressed = true;

    moveStick(event->pos());
}

void Joystick::mouseReleaseEvent(QMouseEvent *event)
{
    isPressed = false;

    moveStick(QPoint(r, r));
}

void Joystick::mouseMoveEvent(QMouseEvent *event)
{
    if (isPressed)
        moveStick(event->pos());
}

void Joystick::moveEvent(QMoveEvent *event)
{
    moveStick(QPoint(r, r));
}

void Joystick::moveStick(QPoint pos)
{
    float x = pos.x() - r;
    float y = -pos.y() + r;
    float m = x != 0 ? y / x : std::sqrt(std::numeric_limits<float>::max() - 1.0f);

    float cx = std::sqrt(std::pow(r, 2.0f) / (std::pow(m, 2.0f) + 1.0f)) * std::copysign(1.0f, x);
    float cy = m * cx;

    if (std::pow(x, 2.0f) + std::pow(y, 2.0f) > std::pow(r, 2.0f))
    {
        x = cx;
        y = cy;
    }

    qDebug() << "Joystick:" << "x:" << x / r << "y:" << y / r;

    pos.rx() = x + r - sr;
    pos.ry() = -(y - r) - sr;
    pos += this->pos();

    stick->move(pos);
}
