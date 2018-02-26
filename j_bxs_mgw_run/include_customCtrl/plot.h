#ifndef _PLOT_H_
#define _PLOT_H_ 1

#include <qwt_plot.h>
#include <qwt_system_clock.h>
#include "settings.h"
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
//class QwtPlotGrid;
//class QwtPlotCurve;
class LegendItem;
//class QwtLegend;
class Plot: public QwtPlot
{
    Q_OBJECT
    struct curveStruct
    {
        QString curveName;
        QwtPlotCurve* plotCurve;
        double curValue;
    };

public:
    Plot( double minValue,double maxValue,QList<QString> paraNameList,QWidget* = NULL ,double interval = 10.0);
    virtual ~Plot();
    Settings getSettings();
    void upDateValue(double newValue, QString paraName);
public Q_SLOTS:
    void setSettings( const Settings & );
protected:
    virtual void timerEvent( QTimerEvent *e );
private:
    void alignScales();
    QwtPlotGrid *d_grid;

    QwtSystemClock d_clock;
    double d_interval;
    int d_timerId;
    Settings d_settings;
    double m_maxValue;
    double m_minValue;
    QList<curveStruct*> m_curveList;
//    QVector<double> m_curValue;
//    double m_curValue;
    QwtLegend *d_externalLegend;
    LegendItem *d_legendItem;
};

#endif
