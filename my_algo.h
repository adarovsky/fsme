/*
FSME - A State Machine Editor
Copyright (c) 2003-2005 Alexander Darovsky

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include <functional>
#include <numeric>
#include <cmath>
#include <QPointF>
#include <QRectF>

#ifndef __MY__ALGO__
#define __MY__ALGO__

template<class Const, class T>
struct const_t : public std::unary_function<T, Const> {
    Const data;
    const_t( Const c ) : data(c) {}
    Const operator()( const T& ) const { return data; }
};

template <class T>
T next(T x) { return ++x; }

template <class T>
T prior(T x) { return --x; }

inline double cross_prod( QPointF p1, QPointF p2, QPointF p3)
{
    //    return p1.x()*p2.y() - p1.x()*p3.y() - p2.x()*p1.y() + p2.x()*p3.y() + p3.x()*p1.y() - p3.x()*p2.y();
    QPointF pa = p2 - p1;
    QPointF pb = p3 - p1;
    return pa.x()*pb.y() - pb.x()*pa.y();
}

inline double cross_prod(double x1,double y1, // root
                         double x2,double y2, // first vertex
                         double x3,double y3) // second vertex
{
    double xa = x2 - x1;
    double ya = y2 - y1;
    double xb = x3 - x1;
    double yb = y3 - y1;
    return xa*yb - xb*ya;
}

inline bool inside_triangle( QPoint p, QPoint a, QPoint b, QPoint c )
{
    double cp = cross_prod( a, b, c );
    return ( cp * cross_prod( p, b, c ) >= 0) &&
        ( cp * cross_prod( p, c, a ) >= 0 ) &&
        ( cp * cross_prod( p, a, b ) >= 0 );
}

/// A squared int value
template <class X> inline X sqr( X i ) { return i*i; }

/// Distance between two points
inline double dist( QPointF p1, QPointF p2 )
{
    return sqrt( sqr(p1.x()-p2.x()) + sqr(p1.y()-p2.y()) );
}

/** \brief Distance between line and a point
 *
 * \param p1 first point of a line
 * \param p2 second point
 * \param p  the standalone point
 */
inline double dist( QPointF p1, QPointF p2, QPointF p )
{
    double d  = dist( p1, p2 );
    Q_ASSERT( d > 0 );
    double nx = p1.x()+(p2.x()-p1.x())/d;
    double ny = p1.y()+(p2.y()-p1.y())/d;

    return fabs(cross_prod( p1.x(), p1.y(), nx, ny, p.x(), p.y() ));
}

inline bool is_close( QPointF p1, QPointF p2, QPointF p, double width = 3 )
{
    return dist( p1, p2, p ) <= width;
}

inline QVector<QPointF> make_loop(const QRectF& bounds)
{
    double angle = 0;
    QPointF pos = bounds.center();
    int radius = qMax(bounds.width(), bounds.height());
    QVector<QPointF> cp(2);
    cp[0].setX( int(pos.x() + radius * cos(angle)) );
    cp[0].setY( int(pos.y() + radius * sin(angle)) );
    angle += M_PI / 4;
    cp[1].setX( int(pos.x() + radius * cos(angle)) );
    cp[1].setY( int(pos.y() + radius * sin(angle)) );
    angle += M_PI / 4;

    return cp;
}

#endif

// Local Variables:
// mode: c++
// End:
