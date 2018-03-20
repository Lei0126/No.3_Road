#include "controller.h"
#include <QDebug>

#define Print()     qDebug() << " file:" << __FILE__ << " line:" <<__LINE__ <<" "

Controller::Controller(Camera *cam, QObject *parent) : QObject(parent), camera(cam)
{
    m_mode = NONE;
    m_routeTime =1000;
}

void Controller::setMode(const char m)
{
    if(m == m_mode) return;
    m_tSystem->stop();
    switch((Mode)m) {
    case AUTO:
        Print() << "mode changed to AUTO";
        connect(servo, SIGNAL(completed()), camera, SLOT(capture()));
        m_tSystem->start(500);
        break;
    case ROUTING:
        Print() << "mode changed to ROUTING";
        connect(servo, SIGNAL(completed()), camera, SLOT(capture()));
        m_tSystem->start(m_routeTime);
        break;
    case MANUAL:
        Print() << "mode changed to MANUAL";
        disconnect(servo, SIGNAL(completed()), camera, SLOT(capture()));

        break;
    }
    m_mode = (Mode)m;
    emit modeChanged(m_mode);
}

int Controller::nowAngle() const
{
    return servo->nowAngle();
}

void Controller::setAngle(const int degree)
{
    servo->setNowAngle(degree);
}

void Controller::changeAngle(const int degree)
{
    emit angleChanged(degree);
}

void Controller::changeRouteTime(const int value)
{
    m_routeTime = value*1000;
    if(m_mode == ROUTING){
        m_tSystem->stop();
        m_tSystem->start(m_routeTime);
    }
}

void Controller::init()
{
    servo = new Servo();
    infrared = new Infrared();
    m_tSystem = new QTimer();
    Print() << "control thread begin";
    connect(m_tSystem, SIGNAL(timeout()), SLOT(onSystem()));
    connect(servo, SIGNAL(nowAngleChanged(int)), SLOT(changeAngle(int)));
    setMode(AUTO);
}

void Controller::onSystem()
{
    Infrared::Direction dir;
    int degree;
    switch(m_mode) {
    case AUTO:
        dir = infrared->check();
        if(dir) {
            servo->setNowAngle(dir*90-90);
            Print() << (int)dir;
        }
        break;
    case ROUTING:
        degree = servo->nowAngle() + 30;
        degree = degree >= 180 ? degree -180 : degree;
        servo->setNowAngle(degree);
        break;
    }
}
