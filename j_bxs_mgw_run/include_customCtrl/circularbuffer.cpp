#include "circularbuffer.h"
#include <math.h>

CircularBuffer::CircularBuffer( double interval, size_t numPoints ):
    d_startIndex( 0 ),
    d_offset( 0.0 )
{
    fill(interval, numPoints );
}

void CircularBuffer::fill(double interval, size_t numPoints )
{
    if ( interval <= 0.0 || numPoints < 2 )
        return;
    d_values.resize( numPoints );
    d_values.fill( 0.0 );
    d_step = interval / ( numPoints - 2 );
    d_interval = interval;
}
void CircularBuffer::addValue(double newValue)
{
//    iTemp ++;
//    if(iTemp >= size())
//        iTemp = 0;
//    d_values[iTemp] = (double)((double)iTemp / (double)size()) * 100;
////    d_values[iTemp] = newValue;
//    qDebug()<<iTemp<<size()<<(double)((double)iTemp / (double)size());
    for(int i = 0;i < size() - 1;i ++)
    {
        d_values[i] = d_values[i + 1];
    }
    d_values[size() - 1] = newValue;
}

size_t CircularBuffer::size() const
{
    return d_values.size();
}

QPointF CircularBuffer::sample( size_t i ) const
{
    const int size = d_values.size();
    int index = d_startIndex + i;
    if ( index >= size )
        index -= size;
    const double x = i * d_step - d_offset - d_interval;
    const double y = d_values.data()[index];
    return QPointF( x, y );
}


QRectF CircularBuffer::boundingRect() const
{
    return QRectF( -100.0, -d_interval, 200.0, d_interval );
}
