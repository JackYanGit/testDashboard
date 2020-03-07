#include "dashboard.h"
#include "ui_dashboard.h"
#define PropertyBuilderByName(type,name,verPermission,setPermission,getPermission)\
    verPermission:\
    type m_##name;\
    setPermission:\
    inline void set##name(type v)\
    {\
        m_##name=v;\
    }\
    getPermission:\
    inline type get##name(void)\
    {\
        return m_##name;\
    }\

Dashboard::Dashboard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dashboard)
{
    ui->setupUi(this);
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
    timer->start(100);
    minValue=0;                //最小值
    maxValue=100;                //最大值
    value=60;                   //目标值
    precision=0;                  //精确度,小数点后几位

    scaleMajor=10;                 //大刻度数量
    scaleMinor=10;                 //小刻度数量
    startAngle=45;                 //开始旋转角度
    endAngle=45;                   //结束旋转角度
    outerCircleColor=QColor(162,209,88);        //外圆背景颜色
    innerCircleColor=QColor(26,211,16);        //内圆背景颜色

    pieColorStart=QColor(162,188,8);           //饼圆开始颜色
    pieColorMid=QColor(162,88,88);             //饼圆中间颜色
    pieColorEnd=QColor(16,8,88);             //饼圆结束颜色

    coverCircleColor=QColor(10,88,100);        //覆盖圆背景颜色
    scaleColor=QColor(88,166,200);              //刻度尺颜色
    pointerColor=QColor(62,128,88);            //指针颜色
    centerCircleColor=QColor(162,88,218);       //中心圆颜色
    textColor=QColor(171,188,88);               //文字颜色

    showOverlay=1;               //显示遮罩层
    overlayColor=QColor(212,28,38);            //遮罩层颜色

    pieStyle=(Dashboard::PieStyle)0;              //饼图样式
    pointerStyle=(Dashboard::PointerStyle)0;      //指针样式

    reverse=1;                   //是否往回走
    currentValue=10;            //当前值
}

Dashboard::~Dashboard()
{
    delete ui;
}
void Dashboard::paintEvent(QPaintEvent *)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);

    //绘制准备工作,启用反锯齿,平移坐标轴中心,等比例缩放
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.translate(width / 2, height / 2);
    painter.scale(side / 200.0, side / 200.0);

    //绘制外圆
    drawOuterCircle(&painter);
    //绘制内圆
    drawInnerCircle(&painter);
    //绘制饼圆
    drawColorPie(&painter);
    //绘制覆盖圆 用以遮住饼圆多余部分
    drawCoverCircle(&painter);
    //绘制刻度线
    drawScale(&painter);
    //绘制刻度值
    drawScaleNum(&painter);

    //根据指示器形状绘制指示器
    if (pointerStyle == PointerStyle_Circle) {
        drawPointerCircle(&painter);
    } else if (pointerStyle == PointerStyle_Indicator) {
        drawPointerIndicator(&painter);
    } else if (pointerStyle == PointerStyle_IndicatorR) {
        drawPointerIndicatorR(&painter);
    } else if (pointerStyle == PointerStyle_Triangle) {
        drawPointerTriangle(&painter);
    }

    //绘制指针中心圆外边框
    drawRoundCircle(&painter);
    //绘制指针中心圆
    drawCenterCircle(&painter);
    //绘制当前值
    drawValue(&painter);
    //绘制遮罩层
    drawOverlay(&painter);
}

void Dashboard::drawOuterCircle(QPainter *painter)
{
    int radius = 99;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(outerCircleColor);
    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
    painter->restore();
}

void Dashboard::drawInnerCircle(QPainter *painter)
{
    int radius = 90;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(innerCircleColor);
    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
    painter->restore();
}

void Dashboard::drawColorPie(QPainter *painter)
{
    int radius = 60;
    painter->save();
    painter->setPen(Qt::NoPen);

    QRectF rect(-radius, -radius, radius * 2, radius * 2);

    if (pieStyle == PieStyle_Three) {
        //计算总范围角度,根据占比例自动计算三色圆环范围角度
        //可以更改比例
        double angleAll = 360.0 - startAngle - endAngle;
        double angleStart = angleAll * 0.7;
        double angleMid = angleAll * 0.15;
        double angleEnd = angleAll * 0.15;

        //增加偏移量使得看起来没有脱节
        int offset = 3;

        //绘制开始饼圆
        painter->setBrush(pieColorStart);
        painter->drawPie(rect, (270 - startAngle - angleStart) * 16, angleStart * 16);

        //绘制中间饼圆
        painter->setBrush(pieColorMid);
        painter->drawPie(rect, (270 - startAngle - angleStart - angleMid) * 16 + offset, angleMid * 16);

        //绘制结束饼圆
        painter->setBrush(pieColorEnd);
        painter->drawPie(rect, (270 - startAngle - angleStart - angleMid - angleEnd) * 16 + offset * 2, angleEnd * 16);
    } else if (pieStyle == PieStyle_Current) {
        //计算总范围角度,当前值范围角度,剩余值范围角度
        double angleAll = 360.0 - startAngle - endAngle;
        double angleCurrent = angleAll * ((currentValue - minValue) / (maxValue - minValue));
        double angleOther = angleAll - angleCurrent;

        //绘制当前值饼圆
        painter->setBrush(pieColorStart);
        painter->drawPie(rect, (270 - startAngle - angleCurrent) * 16, angleCurrent * 16);

        //绘制剩余值饼圆
        painter->setBrush(pieColorEnd);
        painter->drawPie(rect, (270 - startAngle - angleCurrent - angleOther) * 16, angleOther * 16);
    }

    painter->restore();
}

void Dashboard::drawCoverCircle(QPainter *painter)
{
    int radius = 50;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(coverCircleColor);
    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
    painter->restore();
}

void Dashboard::drawScale(QPainter *painter)
{
    int radius = 72;
    painter->save();

    painter->rotate(startAngle);
    int steps = (scaleMajor * scaleMinor);
    double angleStep = (360.0 - startAngle - endAngle) / steps;

    QPen pen;
    pen.setColor(scaleColor);
    pen.setCapStyle(Qt::RoundCap);

    for (int i = 0; i <= steps; i++) {
        if (i % scaleMinor == 0) {
            pen.setWidthF(1.5);
            painter->setPen(pen);
            painter->drawLine(0, radius - 10, 0, radius);
        } else {
            pen.setWidthF(0.5);
            painter->setPen(pen);
            painter->drawLine(0, radius - 5, 0, radius);
        }

        painter->rotate(angleStep);
    }

    painter->restore();
}

void Dashboard::drawScaleNum(QPainter *painter)
{
    int radius = 82;
    painter->save();
    painter->setPen(scaleColor);

    double startRad = (360 - startAngle - 90) * (M_PI / 180);
    double deltaRad = (360 - startAngle - endAngle) * (M_PI / 180) / scaleMajor;

    for (int i = 0; i <= scaleMajor; i++) {
        double sina = qSin(startRad - i * deltaRad);
        double cosa = qCos(startRad - i * deltaRad);
        double value = 1.0 * i * ((maxValue - minValue) / scaleMajor) + minValue;

        QString strValue = QString("%1").arg((double)value, 0, 'f', precision);
        double textWidth = fontMetrics().width(strValue);
        double textHeight = fontMetrics().height();
        int x = radius * cosa - textWidth / 2;
        int y = -radius * sina + textHeight / 4;
        painter->drawText(x, y, strValue);
    }

    painter->restore();
}

void Dashboard::drawPointerCircle(QPainter *painter)
{
    int radius = 6;
    int offset = 30;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(pointerColor);

    painter->rotate(startAngle);
    double degRotate = (360.0 - startAngle - endAngle) / (maxValue - minValue) * (currentValue - minValue);
    painter->rotate(degRotate);
    painter->drawEllipse(-radius, radius + offset, radius * 2, radius * 2);

    painter->restore();
}

void Dashboard::drawPointerIndicator(QPainter *painter)
{
    int radius = 75;
    painter->save();
    painter->setOpacity(0.8);
    painter->setPen(Qt::NoPen);
    painter->setBrush(pointerColor);

    QPolygon pts;
    pts.setPoints(3, -5, 0, 5, 0, 0, radius);

    painter->rotate(startAngle);
    double degRotate = (360.0 - startAngle - endAngle) / (maxValue - minValue) * (currentValue - minValue);
    painter->rotate(degRotate);
    painter->drawConvexPolygon(pts);

    painter->restore();
}

void Dashboard::drawPointerIndicatorR(QPainter *painter)
{
    int radius = 75;
    painter->save();
    painter->setOpacity(1.0);

    QPen pen;
    pen.setWidth(1);
    pen.setColor(pointerColor);
    painter->setPen(pen);
    painter->setBrush(pointerColor);

    QPolygon pts;
    pts.setPoints(3, -5, 0, 5, 0, 0, radius);

    painter->rotate(startAngle);
    double degRotate = (360.0 - startAngle - endAngle) / (maxValue - minValue) * (currentValue - minValue);
    painter->rotate(degRotate);
    painter->drawConvexPolygon(pts);

    //增加绘制圆角直线,与之前三角形重叠,形成圆角指针
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidthF(4);
    painter->setPen(pen);
    painter->drawLine(0, 0, 0, radius);

    painter->restore();
}

void Dashboard::drawPointerTriangle(QPainter *painter)
{
    int radius = 10;
    int offset = 38;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(pointerColor);

    QPolygon pts;
    pts.setPoints(3, -5, 0 + offset, 5, 0 + offset, 0, radius + offset);

    painter->rotate(startAngle);
    double degRotate = (360.0 - startAngle - endAngle) / (maxValue - minValue) * (currentValue - minValue);
    painter->rotate(degRotate);
    painter->drawConvexPolygon(pts);

    painter->restore();
}

void Dashboard::drawRoundCircle(QPainter *painter)
{
    int radius = 18;
    painter->save();
    painter->setOpacity(0.8);
    painter->setPen(Qt::NoPen);
    painter->setBrush(pointerColor);
    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
    painter->restore();
}

void Dashboard::drawCenterCircle(QPainter *painter)
{
    int radius = 15;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(centerCircleColor);
    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
    painter->restore();
}

void Dashboard::drawValue(QPainter *painter)
{
    int radius = 100;
    painter->save();
    painter->setPen(textColor);

    QFont font;
    font.setPixelSize(18);
    painter->setFont(font);

    QRectF textRect(-radius, -radius, radius * 2, radius * 2);
    QString strValue = QString("%1").arg((double)currentValue, 0, 'f', precision);
    painter->drawText(textRect, Qt::AlignCenter, strValue);

    painter->restore();
}

void Dashboard::drawOverlay(QPainter *painter)
{
    if (!showOverlay) {
        return;
    }

    int radius = 90;
    painter->save();
    painter->setPen(Qt::NoPen);

    QPainterPath smallCircle;
    QPainterPath bigCircle;
    radius -= 1;
    smallCircle.addEllipse(-radius, -radius, radius * 2, radius * 2);
    radius *= 2;
    bigCircle.addEllipse(-radius, -radius + 140, radius * 2, radius * 2);

    //高光的形状为小圆扣掉大圆的部分
    QPainterPath highlight = smallCircle - bigCircle;

    QLinearGradient linearGradient(0, -radius / 2, 0, 0);
    overlayColor.setAlpha(100);
    linearGradient.setColorAt(0.0, overlayColor);
    overlayColor.setAlpha(30);
    linearGradient.setColorAt(1.0, overlayColor);
    painter->setBrush(linearGradient);
    painter->rotate(-20);
    painter->drawPath(highlight);

    painter->restore();
}

double Dashboard::getMinValue() const
{
    return this->minValue;
}

double Dashboard::getMaxValue() const
{
    return this->maxValue;
}

double Dashboard::getValue() const
{
    return this->value;
}

int Dashboard::getPrecision() const
{
    return this->precision;
}

int Dashboard::getScaleMajor() const
{
    return this->scaleMajor;
}

int Dashboard::getScaleMinor() const
{
    return this->scaleMinor;
}

int Dashboard::getStartAngle() const
{
    return this->startAngle;
}

int Dashboard::getEndAngle() const
{
    return this->endAngle;
}

bool Dashboard::getAnimation() const
{
    return this->animation;
}
double Dashboard::getAnimationStep()       const
{
    return this->animationStep;
}

QColor Dashboard::getOuterCircleColor()    const
{
    return this->outerCircleColor;
}
QColor Dashboard::getInnerCircleColor()    const
{
    return this->innerCircleColor;
}

QColor Dashboard::getPieColorStart()       const
{
    return this->pieColorStart;
}
QColor Dashboard::getPieColorMid()         const
{
    return this->pieColorMid;
}
QColor Dashboard::getPieColorEnd()         const
{
    return this->pieColorEnd;
}

QColor Dashboard::getCoverCircleColor()    const
{
    return this->coverCircleColor;
}
QColor Dashboard::getScaleColor()          const
{
    return this->scaleColor;
}
QColor Dashboard::getPointerColor()        const
{
    return this->pointerColor;
}
QColor Dashboard::getCenterCircleColor()   const
{
    return this->centerCircleColor;
}
QColor Dashboard::getTextColor()           const
{
    return this->textColor;
}

bool Dashboard::getShowOverlay()           const
{
    return this->showOverlay;
}
QColor Dashboard::getOverlayColor()        const
{
    return this->overlayColor;
}

Dashboard::PieStyle Dashboard::getPieStyle()          const
{
    return this->pieStyle;
}
Dashboard::PointerStyle Dashboard::getPointerStyle()  const
{
    return this->pointerStyle;
}

QSize Dashboard::sizeHint()                const
{
    return QSize(100, 100);
}
QSize Dashboard::minimumSizeHint()         const
{
    return QSize(10, 10);
}
//设置范围值
void Dashboard::setRange(double minValue, double maxValue)
{
    this->minValue=minValue;
    this->maxValue=maxValue;
}

void Dashboard::setRange(int minValue, int maxValue)
{
    this->minValue=minValue;
    this->maxValue=maxValue;
}

//设置最大最小值
void Dashboard::setMinValue(double minValue)
{
    this->minValue=minValue;
}

void Dashboard::setMaxValue(double maxValue)
{
    this->maxValue=maxValue;
}

//设置目标值
void Dashboard::setValue(double value=80)
{
     this->value=value;
     emit valueChanged(value);
}

void Dashboard::setValue(int value)
{
    this->value=value;
    emit valueChanged(value);
}

//设置精确度
void Dashboard::setPrecision(int precision)
{
    this->precision=precision;
}

//设置主刻度数量
void Dashboard::setScaleMajor(int scaleMajor)
{
     this->scaleMajor=scaleMajor;
}

//设置小刻度数量
void Dashboard::setScaleMinor(int scaleMinor)
{
    this->scaleMinor=scaleMinor;
}

//设置开始旋转角度
void Dashboard::setStartAngle(int startAngle)
{
    this->startAngle=startAngle;
}

//设置结束旋转角度
void Dashboard::setEndAngle(int endAngle)
{
    this->endAngle=endAngle;
}

//设置是否启用动画显示
void Dashboard::setAnimation(bool animation)
{
    this->animation=animation;
}

//设置动画显示的步长
void Dashboard::setAnimationStep(double animationStep)
{
    this->animationStep=animationStep;
}

//设置外圆背景颜色
void Dashboard::setOuterCircleColor(const QColor &outerCircleColor)
{
    this->outerCircleColor=outerCircleColor;
}

//设置内圆背景颜色
void Dashboard::setInnerCircleColor(const QColor &innerCircleColor)
{
    this->innerCircleColor=innerCircleColor;
}

//设置饼圆三种颜色
void Dashboard::setPieColorStart(const QColor &pieColorStart)
{
    this->pieColorStart=pieColorStart;
}

void Dashboard::setPieColorMid(const QColor &pieColorMid)
{
    this->pieColorMid=pieColorMid;
}
void Dashboard::setPieColorEnd(const QColor &pieColorEnd)
{
    this->pieColorEnd=pieColorEnd;
}

//设置覆盖圆背景颜色
void Dashboard::setCoverCircleColor(const QColor &coverCircleColor)
{
    this->coverCircleColor=coverCircleColor;
}
//设置刻度尺颜色
void Dashboard::setScaleColor(const QColor &scaleColor)
{
    this->scaleColor=scaleColor;
}
//设置指针颜色
void Dashboard::setPointerColor(const QColor &pointerColor)
{
    this->pointerColor=pointerColor;
}
//设置中心圆颜色
void Dashboard::setCenterCircleColor(const QColor &centerCircleColor)
{
    this->centerCircleColor=centerCircleColor;
}
//设置文本颜色
void Dashboard::setTextColor(const QColor &textColor)
{
    this->textColor=textColor;
}

//设置是否显示遮罩层
void Dashboard::setShowOverlay(bool showOverlay)
{
    this->showOverlay=showOverlay;
}
//设置遮罩层颜色
void Dashboard::setOverlayColor(const QColor &overlayColor)
{
    this->overlayColor=overlayColor;
}

//设置饼图样式
void Dashboard::setPieStyle(const PieStyle &pieStyle)
{
    this->pieStyle=pieStyle;
}
//设置指针样式
void Dashboard::setPointerStyle(const PointerStyle &pointerStyle)
{
    this->pointerStyle=pointerStyle;
}
void Dashboard::updateValue()
{

}
