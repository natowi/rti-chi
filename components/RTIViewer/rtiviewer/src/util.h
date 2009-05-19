/****************************************************************************
* RTIViewer                                                         o o     *
* Single and Multi-View Reflectance Transformation Image Viewer   o     o   *
*                                                                _   O  _   *
* Copyright(C) 2008                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
****************************************************************************/


#ifndef UTIL_H
#define UTIL_H

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#include <vcg/space/point3.h>
#include <vcg/math/base.h>

#include <QString> 

/*!
  Number of mip-mapping level used.
*/
#define MIP_MAPPING_LEVELS 4


//! Special Rendering mode.
/*!
  Enumeration of special rendering mode supported by the browser.
*/
enum BrowserMode
{
	DEFAULT_MODE, /*!< Rendering di default. */
	NORMALS_MODE, /*!< Normals map. */
	SMOOTH_MODE, /*!< Map to show smoothed normals or smoothed luminance. */
	CONTRAST_MODE, /*!< Map to show contrast signal for normals or luminance. */
	ENHANCED_MODE, /*!< Map to show enhanced normals or enhanced luminance. */
	LUM_UNSHARP_MODE, /*!< Luminance map used in unsharp masking method. */
	LUM_MODE, /*!< Luminance map from coefficients. */
	RGB_MODE, /*!< Map of rgb components. */
	LUMR_MODE, /*!< Map of red components. */
	LUMG_MODE, /*!< Map of green components. */
	LUMB_MODE, /*!< Map of blue components. */
	A0_MODE, /*!< Map of a0 coefficient. */
	A1_MODE, /*!< Map of a1 coefficient. */
	A2_MODE, /*!< Map of a2 coefficient. */
	A3_MODE, /*!< Map of a3 coefficient. */
	A4_MODE, /*!< Map of a4 coefficient. */
	A5_MODE, /*!< Map of a5 coefficient. */
	LIGHT_VECTOR, /*!< Light vector direction used in the multi-light method. */
	LIGHT_VECTOR2, /*!< Light vector direction used in the multi-light method. */
};

//! Rendering info struct
/*!
  The struct contains the info needed for the redenring of RTI image.
*/
struct RenderingInfo
{
	int offx; /*!< Top-left x-coordinate of the current sub-image displayed in the browser */
	int offy; /*!< Top-left y-coordinate of the current sub-image displayed in the browser */
	int height; /*!< Height of the current sub-image displayed in the browser */
	int width; /*!< Width of the current sub-image displayed in the browser */
	int level; /*!< Level of mip-mapping to use for the output texture. */
	int mode; /*!< Special rendering mode applied by the browser. */
	const vcg::Point3f& light; /*!< Light vector. */
	int ordlen; /*< Number of coefficients per channel. */
};


static const double zerotol = 1.0e-5;

/*!
  Type of callback to update progress window.
*/
typedef bool CallBackPos(int, QString);



/*!
	  Reads from the file a line ended by char '\n'
	  \param file file pointer.
	  \param eof flag for the end of file.
	  \return returns the readed line.
*/
static QString getLine(FILE* file, bool* eof)
{
	char c;
	QString str = "";
	*eof = false;
	while(!feof(file) && fread(&c, sizeof(char), 1, file)!=0 && c!='\n')
		str.append(c);
	if (feof(file))
		*eof = true;
	return str;
}



/*!
  Evaluates the biquadratic polynomial:

  a[0]*lu*lu + a[1]*lv*lv + a[2]*lu*lv + a[3]*lu + a[4]*lv + a[5];

  \param a array of six coefficients.
  \param lu, lv projections of light vector on uv-plane.
*/
static double evalPoly(const int* a, float lu, float lv)
{
	return a[0]*lu*lu + a[1]*lv*lv + a[2]*lu*lv + a[3]*lu + a[4]*lv + a[5]; 
}


/*!
  Converts a double value as unsigned char value.
  \param value value to convert.
  \return returns the converted value as unsigned char. 
*/
static unsigned char tobyte(double value)
{
	unsigned char v;

	if (value < 0.0)
		v = 0;
	else if (value > 255.0)
		v = 255;
	else
		v = static_cast<unsigned char>(value);

	return v;
}


/*!
  Converts the float value as a color component in the interval [0, 255].
  \param normal value to convert.
  \return returns the equivalent color component.
*/
static unsigned char toColor(float normal)
{
	float f = ((normal + 1.0f) / 2.0f) * 255.0f;
	return tobyte(f);
}


/*!
  Check if the value \a x is near to zero (in the interval [-(1e-9), (1e-9)]).
  \param x value to check.
  \return returns true if the value is contained by the interval, returns false otherwise.
*/
static bool isZero(double x)
{
	double limit = 1e-9;
	return x > -limit && x < limit;
}


/*!
  Calculates the cube root.
  \param x input value.
  \return the cube root of \a x.
*/
static double cubeRoot(double x)
{
	if (x > 0)
		return pow(x, 1.0 / 3.0);
	else if (x < 0)
		return -pow(-x, 1.0 / 3.0);
	return 0;
}


/*!
  Finds the solutions of a quadratic equation.
  \param c equation coefficients.
  \param s solutions.
  \param n index of the next free element in the solutions array.
  \return the number of solutions.
*/
static int solveQuadric(double c[3], double s[4], int n)
{
	double p, q, D;
	
	/* normal form: x^2 + px + q = 0 */

	p = c[1] / (2 * c[2]);
	q = c[0] / c[2];

	D = p * p - q;

	if (isZero(D))
	{
		s[0 + n] = -p;
		return 1;
	} 
	else if (D < 0)
	{
		return 0;
	} 
	else if (D > 0)
	{
		double sqrt_D = sqrt(D);
		s[0 + n] = sqrt_D - p;
		s[1 + n] = -sqrt_D - p;
		return 2;
	}
	return -1;
}


/*!
  Finds the solutions of a quadratic equation.
  \param c equation coefficients.
  \param s solutions.
  \return the number of solutions.
*/
static int solveQuadric(double c[3], double s[4]) {
	return solveQuadric(c, s, 0);
}



/*!
  Finds the solutions of a cubic equation.
  \param c equation coefficients.
  \param s solutions.
  \return the number of solutions.
*/
static int solveCubic(double c[4], double s[4]) 
{
	int i, num;
	double sub;
	double A, B, C;
	double sq_A, p, q;
	double cb_p, D;

	/* normal form: x^3 + Ax^2 + Bx + C = 0 */
	A = c[2] / c[3];
	B = c[1] / c[3];
	C = c[0] / c[3];

	/* substitute x = y - A/3 to eliminate quadric term: x^3 +px + q = 0 */
	sq_A = A * A;
	p = 1.0 / 3 * (-1.0 / 3 * sq_A + B);
	q = 1.0 / 2 * (2.0 / 27 * A * sq_A - 1.0 / 3 * A * B + C);

	/* use Cardano's formula */
	cb_p = p * p * p;
	D = q * q + cb_p;

	if (isZero(D))
	{
		if (isZero(q)) /* one triple solution */
		{
			s[0] = 0;
			num = 1;
		}
		else /* one single and one double solution */
		{
			double u = cubeRoot(-q);
			s[0] = 2 * u;
			s[1] = -u;
			num = 2;
		}
	} 
	else if (D < 0) /* Casus irreducibilis: three real solutions */
	{
		double phi = 1.0 / 3 * acos(-q / sqrt(-cb_p));
		double t = 2 * sqrt(-p);

		s[0] = t * cos(phi);
		s[1] = -t * cos(phi + M_PI / 3);
		s[2] = -t * cos(phi - M_PI / 3);
		num = 3;
	} 
	else /* one real solution */
	{
		double sqrt_D = sqrt(D);
		double u = cubeRoot(sqrt_D - q);
		double v = -cubeRoot(sqrt_D + q);

		s[0] = u + v;
		num = 1;
	}

	/* resubstitute */
	sub = 1.0 / 3 * A;
	for (i = 0; i < num; ++i)
		s[i] -= sub;
	return num;
}


/*!
  Finds the solutions of a fourth degree equation.
  \param c equation coefficients.
  \param s solutions.
  \return the number of solutions.
*/
static int solveQuartic(double c[5], double s[4])
{
	double coeffs[4];
	double z, u, v, sub;
	double A, B, C, D;
	double sq_A, p, q, r;
	int i, num;

	/* normal form: x^4 + Ax^3 + Bx^2 + Cx + D = 0 */
	A = c[3] / c[4];
	B = c[2] / c[4];
	C = c[1] / c[4];
	D = c[0] / c[4];

	/* substitute x = y - A/4 to eliminate cubic term: x^4 + px^2 + qx + r = 0 */
	sq_A = A * A;
	p = -3.0 / 8 * sq_A + B;
	q = 1.0 / 8 * sq_A * A - 1.0 / 2 * A * B + C;
	r = -3.0 / 256 * sq_A * sq_A + 1.0 / 16 * sq_A * B - 1.0 / 4 * A * C + D;

	if (isZero(r)) {
		/* no absolute term: y(y^3 + py + q) = 0 */
		coeffs[0] = q;
		coeffs[1] = p;
		coeffs[2] = 0;
		coeffs[3] = 1;

		num = solveCubic(coeffs, s);

		s[num++] = 0;
	}
	else
	{
		/* solve the resolvent cubic ... */
		coeffs[0] = 1.0 / 2 * r * p - 1.0 / 8 * q * q;
		coeffs[1] = -r;
		coeffs[2] = -1.0 / 2 * p;
		coeffs[3] = 1;
		solveCubic(coeffs, s);

		/* ... and take the one real solution ... */
		z = s[0];

		/* ... to build two quadric equations */
		u = z * z - r;
		v = 2 * z - p;
		if (isZero(u))
			u = 0;
		else if (u > 0)
			u = sqrt(u);
		else
			return 0;
		if (isZero(v))
			v = 0;
		else if (v > 0)
			v = sqrt(v);
		else
			return 0;

		coeffs[0] = z - u;
		coeffs[1] = q < 0 ? -v : v;
		coeffs[2] = 1;
		num = solveQuadric(coeffs, s);
		coeffs[0] = z + u;
		coeffs[1] = q < 0 ? v : -v;
		coeffs[2] = 1;
		num += solveQuadric(coeffs, s, num);
	}

	/* resubstitute */
	sub = 1.0 / 4 * A;
	for (i = 0; i < num; ++i)
		s[i] -= sub;
	return num;
}


static int computeMaximumOnCircle(double* a, double &lx, double &ly)
{
	double db0, db1, db2, db3, db4;
	double zeros[4];
	double u, v, maxval, maxu = -1, maxv = -1, inc, arg, polyval;
	int index, nroots;

	index = -1;
	nroots = -1;

	db0 = a[2] - a[3];
	db1 = 4 * a[1] - 2 * a[4] - 4 * a[0];
	db2 = -6 * a[2];
	db3 = -4 * a[1] - 2 * a[4] + 4 * a[0];
	db4 = a[2] + a[3];

	/* polynomial is constant on circle, pick (0,1) as a solution */
	if (vcg::math::Abs(db0) < zerotol && vcg::math::Abs(db1) < zerotol && 
			vcg::math::Abs(db2) < zerotol && vcg::math::Abs(db3) < zerotol)
	{
		lx = 0.0;
		ly = 1.0;
		return 1;
	}

	if (db0 != 0)
	{
		double c[5] = { db4, db3, db2, db1, db0 };
		nroots = solveQuartic(c, zeros);
	} 
	else if (db1 != 0)
	{
		double c[4] = { db4, db3, db2, db1 };
		nroots = solveCubic(c, zeros);
	}
	else 
	{
		double c[3] = { db4, db3, db2 };
		nroots = solveQuadric(c, zeros);
	}
			

	if (nroots <= 0)
		return -1;
	
	switch (nroots) {
		case 1:
			index = 0;
			break;
		default:
			double* vals = new double[nroots];
			index = 0;
			for (int i = 0; i < nroots; i++) 
			{
				u = 2 * zeros[i] / (1 + zeros[i] * zeros[i]);
				v = (1 - zeros[i] * zeros[i]) / (1 + zeros[i] * zeros[i]);
				vals[i] = a[0] * u * u + a[1] * v * v + a[2] * u * v + a[3] * u + a[4] * v + a[5];
				if (vals[i] > vals[index])
					index = i;
			}
			delete[] vals;
	}

	/*
	 * I noticed that the fact that the pont (0,-1) on the circle can only
	 * be attained in the limit causes it to be missed in case it really is
	 * the maximum. Hence it is necessary to investigate a neighboring
	 * region to find the potential maximum there, we look at the segment
	 * from 260 degress to 280 degrees (270 degrees being the limit point).
	 */

	lx = 2 * zeros[index] / (1 + zeros[index] * zeros[index]);
	ly = (1 - zeros[index] * zeros[index])/ (1 + zeros[index] * zeros[index]);

	/*
	 * test the correctness of solution:
	 */

	maxval = -1000;

	for (int k = 0; k <= 20; k++)
	{
		inc = (1 / 9.0) / 20 * k;
		arg = M_PI * (26.0 / 18.0 + inc);
		u = cos(arg);
		v = sin(arg);
		polyval = a[0] * u * u + a[1] * v * v + a[2] * u * v + a[3] * u	+ a[4] * v + a[5];
		if (maxval < polyval) {
			maxval = polyval;
			maxu = u;
			maxv = v;
		}
	}

	u = 2 * zeros[index] / (1 + zeros[index] * zeros[index]);
	v = (1 - zeros[index] * zeros[index]) / (1 + zeros[index] * zeros[index]);
	double val1 = a[0] * u * u + a[1] * v * v + a[2] * u * v + a[3] * u + a[4] * v + a[5];
	if (maxval > val1) {
		lx = maxu;
		ly = maxv;
	}
	return 1;
}

/*!
  Returns the first nine Hemispherical Harmonics computed in the theta and phi angles.
*/
static void getHSH(double theta, double phi, double* hweights)
{
	hweights[0] = 1/sqrt(2*M_PI);
	hweights[1] = sqrt(6/M_PI)      *  (cos(phi)*sqrt(cos(theta)-cos(theta)*cos(theta)));
	hweights[2] = sqrt(3/(2*M_PI))  *  (-1. + 2.*cos(theta));
	hweights[3] = sqrt(6/M_PI)      *  (sqrt(cos(theta) - cos(theta)*cos(theta))*sin(phi));
	hweights[4] = sqrt(30/M_PI)     *  (cos(2.*phi)*(-cos(theta) + cos(theta)*cos(theta)));
	hweights[5] = sqrt(30/M_PI)     *  (cos(phi)*(-1. + 2.*cos(theta))*sqrt(cos(theta) - cos(theta)*cos(theta)));
	hweights[6] = sqrt(5/(2*M_PI))  *  (1 - 6.*cos(theta) + 6.*cos(theta)*cos(theta));
	hweights[7] = sqrt(30/M_PI)     *  ((-1 + 2.*cos(theta))*sqrt(cos(theta) - cos(theta)*cos(theta))*sin(phi));
	hweights[8] = sqrt(30/M_PI)     *  ((-cos(theta) + cos(theta)*cos(theta))*sin(2.*phi));
}

#endif  /* UTIL_H */
