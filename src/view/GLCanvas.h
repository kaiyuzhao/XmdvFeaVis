/*
 * GLCanvas.h
 *
 *  Created on: Apr 1, 2008
 *      Ported by Zaixian Xie
 *
 *  Updated on Jan 2013
 *      Kaiyu Zhao
 */

/*
 * Class GLCanvas is a helper class to provide functions
 * to help developers draw different visual elements
 * on QGLWidget
 */
#ifndef GLCANVAS_H_
#define GLCANVAS_H_

#include "datatype/XmdvType.h"
#include "main/XmdvTool.h"
#include <QGLWidget>
#include <QFont>
#include <QPainter>

class GLCanvas
{
public:
	GLCanvas(QGLWidget *glWidget);
	virtual ~GLCanvas();

public:
	QGLWidget *glWidget;
    //QPainter *painter;
	RGBt fg_color;
	RGBt bg_color;
	bool translucent;
    bool flatcolor;
    bool xxor;
	double alpha;
    Vec2 zoom;

	bool stencilmode;

public:

	bool startPan;
	int lnPos;
	int byps;

public:
    void setGLWidget(QGLWidget *glWidget);
    QGLWidget *getGLWidget();

    void makeCurrent();
    void flush();
    void beginDraw();
    void endDraw();
    void setForeground( unsigned long color, double alpha=1.0 );
    void setForeground( const RGBt &color, double alpha=1.0 );
    void setBackground( unsigned long color );
    void drawLine( double x1, double y1, double x2, double y2);
	void drawLine( std::vector<Vec2> buffer, int pos);
    void drawLines( const std::vector<Vec2> &pts);
    //draw drawCompleteCircle actually draw a complete circle,
    //use this long name to differentiate it from drawCircle
    void drawCompleteCircle(float x, float y, float r, int segments);
	//the circle is acturally a disk, inner radius: radius-offset, out radius: radius+offset
	void drawCircle( double x1, double y1, double radius, double offset);
	void drawArc( double centerx, double centery, double radius, double start_angle, double sweep_angle);
	void drawPartialDisk( double x1, double y1, double innerRadius,
						  double outerRadius, double startAngle, double sweepAngle);
    void drawRectangle( double x, double y, double width, double ht );
    void drawRectangle2(double x1, double x2, double y1, double y2 );
    void fillRectangle( double x, double y, double width, double ht);
	void drawPoint( double x, double y);
    void drawPoint( double x, double y, double size);
    void drawPoints( const std::vector<Vec2> &pts, double size);
    void drawPolygon( const std::vector<Vec2> &pts);
    void fillPolygon( const std::vector<Vec2> &pts);
    void fillBand( const std::vector<Vec2> &pts, double ratio);
    void drawTransparentBand( const std::vector<Vec2> &pts, double opacity_bottom, double opacity_top, RGBt color_bottom, RGBt color_top);
    void drawTransparentBand2( const std::vector<Vec2> &pts, double opacity_bottom, double opacity_top, double quartile);
//jing
	void drawTransparentSquare2( const std::vector<Vec2> &pts, double opacity_bottom, double opacity_top, double frac);
	void drawTransparentSquare( const std::vector<Vec2> &pts, double opacity_bottom, double opacity_top, RGBt color_bottom, RGBt color_top);
//jing
    void drawString( int x, int y, char* text, QFont *font = &g_globals.textFont);
    void drawOrientedString(int x, int y, QString text, QFont *font = &g_globals.textFont);

    void drawConfinedString( double x, double y, char *text, double width, QFont *font = &g_globals.textFont);
    void drawChar( int x, int y, char ch);
    Vec2 mapScreenToData( const Vec2 &sc );
    Vec2 mapDataToScreen( const Vec2 &p );
//Kaiyu
    //draw each dimension as a profile/star glyph represent the dimension property (i.e. histogram)
    //after we draw each dimension on the screen, dimensions with similar properties are displayed near each other

    //passing the position of one dimension on the screen (pos), the position (pos) is determined by a MDS transformation of the
    //feature pairwise similarities. The profile/star glyph is a representation of the feature distribution
    //the heights represent the height of each bar in the histogram, it can be transformed to a star glyph very easily
    //void drawGlyph( const Vec2 pos, const std::vector<float> &heights, double size );
    //void drawVString(int x, int y, QString text, QFont *font = &g_globals.textFont);
    //QImage image;
};

#endif /*GLCANVAS_H_*/
