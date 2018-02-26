#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#include <qwt_series_data.h>
#include <qvector.h>
//我的缓冲区与时间无关，每次来一个点就存储一个
class CircularBuffer: public QwtSeriesData<QPointF>
{
public:
    CircularBuffer(double interval, size_t numPoints);
    void fill(double interval, size_t numPoints );  //当绘图总点数有所改变时需要调用它
    void addValue(double newValue);
//    void setReferenceTime( double );
//    double referenceTime() const;
    virtual size_t size() const;
    virtual QPointF sample( size_t i ) const;
    virtual QRectF boundingRect() const;
//    void setFunction( double( *y )( double ) );
private:
//    double d_referenceTime;
    double d_interval;
    QVector<double> d_values;
    double d_step;
    int d_startIndex;
    double d_offset;
};

#endif
