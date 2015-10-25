/*
 * Kaiyu Zhao
 * Jan 2013
 *
 */

#ifndef XMDVTOOLTYPES_H
#define XMDVTOOLTYPES_H

//I feel like it's better not to typedef vector of xmdv objects here;
//some of of them referring to many other headers and may cause
//dependency problems; although I didn't tried

#include <string>
#include <vector>
#include <iterator>
#include <QRect>
#include "datatype/Vec2.h"

typedef std::vector<int> IntVector;
typedef IntVector::iterator IntIterator;

typedef std::vector<double> DoubleVector;
typedef DoubleVector::iterator DoubleIterator;

typedef std::vector<bool> BoolVector;
typedef BoolVector::iterator BoolIterator;

typedef std::vector<std::string> StringVector;
typedef StringVector::iterator StringIterator;

typedef std::vector<Vec2> Vec2Vector;
typedef Vec2Vector::iterator Vec2Iterator;

typedef std::vector<QRect> QRectVector;
typedef QRectVector::iterator QRectIterator;

#endif // XMDVTOOLTYPES_H
