#include "chart.h"
#include "series.h"
#include <QPainter>

using namespace Fftchart;

Chart::Chart(QQuickItem *parent)
    : PaintedItem(parent)
{
    typeMap[RTA]        = "RTA";
    typeMap[Magnitude]  = "Magnitude";
    typeMap[Phase]      = "Phase";
    typeMap[Scope]      = "Scope";
    typeMap[Impulse]    = "Impulse";

    axisX = new Axis(AxisDirection::horizontal, this);
    axisY = new Axis(AxisDirection::vertical, this);

    setType(RTA);
}
QString Chart::typeString() const
{
    return typeMap.at(_type);
}
void Chart::setTypeByString(const QString &type)
{
    for (auto it = typeMap.begin(); it != typeMap.end(); ++it) {
        if (it->second == type) {
            setType(it->first);
        }
    }
}
void Chart::setType(const Type type)
{
    if (_type != type) {
        _type = type;

        switch (_type) {
            case RTA:         axisY->configure(AxisType::linear, -90,    0,  9);break;
            case Magnitude:   axisY->configure(AxisType::linear, -18,   18, 13);break;
            case Phase:       axisY->configure(AxisType::linear, -180, 180,  9);break;
            case Scope:       axisY->configure(AxisType::linear, -1,     1, 11);break;
            case Impulse:     axisY->configure(AxisType::linear, -1,     1, 11);break;
            default: ;
        }

        switch (_type) {
            case RTA:
            case Magnitude:
            case Phase:
                axisX->configure(AxisType::logarithmic, 20, 20000);
                axisX->setISOLabels();
                break;

            case Impulse:
            case Scope:
                axisX->configure(AxisType::linear, -10, 10, 11);break;
            break;
            default: ;
        }

        axisX->needUpdate();
        axisY->needUpdate();
        needUpdate();
    }
}
void Chart::paint(QPainter *painter)
{
    //paint axiss & series
    foreach (QQuickItem *item, childItems()) {

        QPainterPath border;
        QPen pen(QColor::fromRgbF(0, 0, 0, 0.1), 1);
        painter->setPen(pen);
        border.moveTo(x()       + padding.left,  y()        + padding.top);
        border.lineTo(x()       + padding.left,  height()   - padding.bottom);
        border.lineTo(width()   - padding.right, height()   - padding.bottom);
        border.lineTo(width()   - padding.right, y()        + padding.top);
        border.lineTo(x()       + padding.left,  y()        + padding.top);
        painter->fillPath(border, QBrush(QColor(Qt::white)));
        painter->drawPath(border);

        QQuickPaintedItem *child = qobject_cast<QQuickPaintedItem*>(item);

        QString className(child->metaObject()->className());
        if (className.compare("Fftchart::Series") == 0) {
            child->setWidth(pwidth());
            child->setHeight(pheight());
            child->setX(x() + padding.left);
            child->setY(y() + padding.top);
        } else {
            child->setWidth(width());
            child->setHeight(height());
            child->setX(x());
            child->setY(y());
        }
    }
}
void Chart::appendDataSource(Chartable *source)
{
    new Series(source, this);
}
void Chart::needUpdate()
{
    update();
}