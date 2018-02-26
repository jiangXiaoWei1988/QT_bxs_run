#include <qglobal.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#define QWT_NO_OPENGL
#ifndef QWT_NO_OPENGL
#include <qevent.h>
#include <qwt_plot_glcanvas.h>
#endif
#include "plot.h"
#include "circularbuffer.h"
#include "settings.h"
#include <qwt_plot_legenditem.h>
class LegendItem: public QwtPlotLegendItem
{
public:
    LegendItem()
    {
        setRenderHint( QwtPlotItem::RenderAntialiased );

        QColor color( Qt::white );

        setTextPen( color );
#if 1
        setBorderPen( color );

        QColor c( Qt::gray );
        c.setAlpha( 200 );

        setBackgroundBrush( c );
#endif
    }
};
#ifndef QWT_NO_OPENGL
class GLCanvas: public QwtPlotGLCanvas
{
public:
    GLCanvas( QwtPlot *parent = NULL ):
        QwtPlotGLCanvas( parent )
    {
        setContentsMargins( 1, 1, 1, 1 );
    }
protected:
    virtual void paintEvent( QPaintEvent *event )
    {
        QPainter painter( this );
        painter.setClipRegion( event->region() );
        QwtPlot *plot = qobject_cast< QwtPlot *>( parent() );
        if ( plot )
            plot->drawCanvas( &painter );
        painter.setPen( palette().foreground().color() );
#if QT_VERSION >= 0x050000
        painter.drawRect( rect().adjusted( 1, 1, 0, 0 ) );
#else
        painter.drawRect( rect().adjusted( 0, 0, -1, -1 ) );
#endif
    }
};
#endif

Plot::Plot(double minValue, double maxValue, QList<QString> paraNameList, QWidget *parent , double interval ):
    QwtPlot( parent ),
    d_externalLegend( NULL ),
    d_legendItem( NULL ),
//    d_interval( 10.0 ), // seconds
    d_timerId( -1 )
{
    d_interval = interval;
    m_maxValue = maxValue;
    m_minValue = minValue;
    // Assign a title
//    setTitle( "curve1" );

    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setFrameStyle( QFrame::Box | QFrame::Plain );
    canvas->setLineWidth( 1 );
    canvas->setPalette( Qt::white );
    setCanvas( canvas );
    alignScales();
    // Insert grid
    d_grid = new QwtPlotGrid();
    d_grid->attach( this );
    // Insert curve    
    for(int i = 0;i < paraNameList.count();i ++)
    {
        QwtPlotCurve *d_curve = new QwtPlotCurve( "Data Moving Right" );
        d_curve->setPen( Qt::black );
        d_curve->setData( new CircularBuffer(d_interval ,d_interval * 1000) );
        d_curve->attach( this );
        curveStruct *newCurveStruct = new curveStruct();
        newCurveStruct->plotCurve = d_curve;
        newCurveStruct->curveName = paraNameList.at(i);
        newCurveStruct->curValue = 0.0;
        m_curveList.append(newCurveStruct);
    }
    // Axis
    setAxisTitle( QwtPlot::xBottom, "Seconds" );
    setAxisScale( QwtPlot::xBottom, -d_interval,0.0 );
    setAxisTitle( QwtPlot::yLeft, "Values" );
    setAxisScale( QwtPlot::yLeft, m_minValue, m_maxValue );
    d_clock.start();
    setSettings( d_settings );
    //    m_curValue.resize(paraNameList.count());
}

Plot::~Plot()
{
    delete d_externalLegend;
}


//
//  Set a plain canvas frame and align the scales to it
//
void Plot::alignScales()
{
    // The code below shows how to align the scales to
    // the canvas frame, but is also a good example demonstrating
    // why the spreaded API needs polishing.
    for ( int i = 0; i < QwtPlot::axisCnt; i++ )
    {
        QwtScaleWidget *scaleWidget = axisWidget( i );
        if ( scaleWidget )
            scaleWidget->setMargin( 0 );

        QwtScaleDraw *scaleDraw = axisScaleDraw( i );
        if ( scaleDraw )
            scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
    }
    plotLayout()->setAlignCanvasToScales( true );
}

Settings Plot::getSettings()
{
    return d_settings;
}

void Plot::upDateValue(double newValue,QString paraName)
{
    for(int i = 0;i < m_curveList.count();i ++)
    {
        if(paraName == m_curveList.at(i)->curveName)
        {
            m_curveList.at(i)->curValue = newValue;
        }
    }
}

void Plot::setSettings( const Settings &s )
{
    if ( d_timerId >= 0 )
        killTimer( d_timerId );
    d_timerId = startTimer( s.updateInterval );    
    d_grid->setPen( s.grid.pen );
    d_grid->setVisible( s.grid.pen.style() != Qt::NoPen );
    for(int i = 0;i < m_curveList.count();i ++)
    {
        QwtPlotCurve *d_curve = m_curveList[i]->plotCurve;
        CircularBuffer *buffer = static_cast<CircularBuffer *>( d_curve->data() );
        if ( s.definedCurve.numPoints != buffer->size()/* ||
                s.curve.functionType != d_settings.curve.functionType*/ )
        {
            buffer->fill(d_interval, d_interval * s.definedCurve.numPoints );
        }
//        d_curve->title() = m_curveList[i]->curveName;
        d_curve->setTitle(m_curveList[i]->curveName);
        d_curve->setPen( s.definedCurve.pen );
        if(i < s.colorList.count())
        {
            QPen newPen = s.definedCurve.pen;
            newPen.setColor(s.colorList.at(i));
            d_curve->setPen(newPen);
        }
        d_curve->setBrush( s.definedCurve.brush );
        d_curve->setPaintAttribute( QwtPlotCurve::ClipPolygons,
            s.definedCurve.paintAttributes & QwtPlotCurve::ClipPolygons );
        d_curve->setPaintAttribute( QwtPlotCurve::FilterPoints,
            s.definedCurve.paintAttributes & QwtPlotCurve::FilterPoints );
        d_curve->setRenderHint( QwtPlotItem::RenderAntialiased,
            s.definedCurve.renderHint & QwtPlotItem::RenderAntialiased );
    }

    //铭牌
    if ( legend() )
    {
        // remove legend controlled by the plot
        insertLegend( NULL );
    }

    //外部铭牌，这次用不到
    if(s.legend.isEnabled)
    {
        if ( d_externalLegend == NULL )
        {
            d_externalLegend = new QwtLegend(this);
            d_externalLegend->setWindowTitle("Legend");
    //        connect(
    //            this,
    //            SIGNAL( legendDataChanged( const QVariant &,
    //                const QList<QwtLegendData> & ) ),
    //            d_externalLegend,
    //            SLOT( updateLegend( const QVariant &,
    //                const QList<QwtLegendData> & ) ) );
            d_externalLegend->show();
            insertLegend( new QwtLegend(),
                QwtPlot::LegendPosition( QwtPlot::BottomLegend ) );
            // populate the new legend
            updateLegend();
        }
    }

    if(s.legendItem.isEnabled)
    {
        if ( d_legendItem == NULL )
        {
            d_legendItem = new LegendItem();
            d_legendItem->attach( this );
        }

        d_legendItem->setMaxColumns( /*settings.legendItem.numColumns */1);
        d_legendItem->setAlignment( Qt::AlignRight | Qt::AlignTop);
        d_legendItem->setBackgroundMode(
            QwtPlotLegendItem::BackgroundMode( QwtPlotLegendItem::ItemBackground ) ); //QwtPlotLegendItem::LegendBackground

        d_legendItem->setBorderRadius( 4 );
        d_legendItem->setMargin( 0 );
        d_legendItem->setSpacing( 4 );
        d_legendItem->setItemMargin( 2 );
    //    if ( settings.legendItem.backgroundMode ==
    //        QwtPlotLegendItem::ItemBackground )
    //    {
    //        d_legendItem->setBorderRadius( 4 );
    //        d_legendItem->setMargin( 0 );
    //        d_legendItem->setSpacing( 4 );
    //        d_legendItem->setItemMargin( 2 );
    //    }
    //    else
    //    {
    //        d_legendItem->setBorderRadius( 8 );
    //        d_legendItem->setMargin( 4 );
    //        d_legendItem->setSpacing( 2 );
    //        d_legendItem->setItemMargin( 0 );
    //    }

        QFont font = d_legendItem->font();
        font.setPointSize( /*settings.legendItem.size*/12 );
        d_legendItem->setFont( font );
    }

#ifndef QWT_NO_OPENGL
    if ( s.canvas.openGL )
    {
        QwtPlotGLCanvas *plotCanvas = qobject_cast<QwtPlotGLCanvas *>( canvas() );
        if ( plotCanvas == NULL )
        {
            plotCanvas = new GLCanvas();
            plotCanvas->setPalette( QColor( "khaki" ) );
            setCanvas( plotCanvas );
        }
    }
    else
#endif
    {
        QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas *>( canvas() );
        if ( plotCanvas == NULL )
        {
            plotCanvas = new QwtPlotCanvas();
            plotCanvas->setFrameStyle( QFrame::Box | QFrame::Plain );
            plotCanvas->setLineWidth( 1 );
            plotCanvas->setPalette( Qt::white );
            setCanvas( plotCanvas );
        }
        plotCanvas->setAttribute( Qt::WA_PaintOnScreen, s.canvas.paintOnScreen );
        plotCanvas->setPaintAttribute(
            QwtPlotCanvas::BackingStore, s.canvas.useBackingStore );
        plotCanvas->setPaintAttribute(
            QwtPlotCanvas::ImmediatePaint, s.canvas.immediatePaint );

    }
    QwtPainter::setPolylineSplitting( s.definedCurve.lineSplitting );
    d_settings = s;
}
int iTemp = 0;
void Plot::timerEvent( QTimerEvent * )
{
     for(int i = 0;i < m_curveList.count();i ++)
     {
         QwtPlotCurve *d_curve = m_curveList[i]->plotCurve;
         CircularBuffer *buffer = static_cast<CircularBuffer *>( d_curve->data() );
         buffer->addValue(m_curveList[i]->curValue);
     }

//    iTemp++;
//    if(iTemp > 100)
//        iTemp = 0;

//    if(iTemp > 20 && iTemp < 50)
//    {
//        d_settings.curve.pen.setColor( QColor("white") );
//        d_curve->setPen( d_settings.curve.pen );
//    }
//    else {
//        d_settings.curve.pen.setColor( QColor("red") );
//        d_curve->setPen( d_settings.curve.pen );
//    }
    QMetaObject::invokeMethod( canvas(), "replot", Qt::DirectConnection );
}
