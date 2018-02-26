#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <qpen.h>
#include <qbrush.h>

class Settings
{
public:
//    enum FunctionType
//    {
//        NoFunction = -1,
//        Wave,
//        Noise
//    };

//    enum UpdateType
//    {
//        RepaintCanvas,
//        Replot
//    };

    Settings()
    {
        grid.pen = Qt::NoPen;
        grid.pen.setCosmetic( true );
        definedCurve.brush = Qt::NoBrush;
        definedCurve.numPoints = 50;
//        curve.functionType = Wave;
        definedCurve.paintAttributes = 0;
        definedCurve.renderHint = 0;
        definedCurve.lineSplitting = true;
        definedCurve.pen.setColor( QColor("red") );
        canvas.useBackingStore = false;
        canvas.paintOnScreen = false;
        canvas.immediatePaint = true;
//#ifndef QWT_NO_OPENGL
//        canvas.openGL = false;
//#endif
//        updateType = RepaintCanvas;
//        updateInterval = 20;
        updateInterval = 20;
        colorList.append(QColor("red"));
        colorList.append(QColor("yellow"));
        colorList.append(QColor("blue"));
        colorList.append(QColor("cyan"));
        colorList.append(QColor("magenta"));
        colorList.append(QColor("green"));
        colorList.append(QColor("darkBlue"));
        colorList.append(QColor("gray"));
        legend.isEnabled = false;
        legendItem.isEnabled = true;
    }

    struct
    {
        bool isEnabled;
        int position;
    } legend; //外部铭牌

    struct
    {
        bool isEnabled;
        int numColumns;
        int alignment;
        int backgroundMode;
        int size;

    } legendItem; //内部铭牌

    QList<QColor> colorList;
    struct gridSettings
    {
        QPen pen;
    } grid;

    struct curveSettings
    {
        QPen pen;
        QBrush brush;
        uint numPoints;
//        FunctionType functionType;
        int paintAttributes;
        int renderHint;
        bool lineSplitting;
    } definedCurve;

    struct canvasSettings
    {
        bool useBackingStore;
        bool paintOnScreen;
        bool immediatePaint;
#ifndef QWT_NO_OPENGL
        bool openGL;
#endif
    } canvas;

//    UpdateType updateType;
    int updateInterval;

};

#endif
