#pragma once

#include "Defines.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <vector>
#include <istream>
#include <ostream>

namespace Imzadi
{
	class IMZADI_API RealToRealFunction
	{
	public:
		virtual double Evaluate(double t) const = 0;
	};

	class IMZADI_API RealToVector3Function
	{
	public:
		virtual Vector3 Evaluate(double t) const = 0;
	};

	class IMZADI_API Vector3ToVector3Function
	{
	public:
		virtual Vector3 Evaluate(const Vector3& v) const = 0;
	};

	/**
	 * These are quadratic polynomials with real coeficients and this class revolves
	 * around the idea of working with them and solving quadratic equations.
	 */
	class IMZADI_API Quadratic : public RealToRealFunction
	{
	public:
		Quadratic();
		Quadratic(const Quadratic& quadratic);
		Quadratic(double A, double B, double C);
		virtual ~Quadratic();

		/**
		 * For for NaN or Inf in the coeficients of this polynomial.
		 */
		bool IsValid() const;

		/**
		 * Evaluate this quadratic polynomial at x.
		 *
		 * @param[in] x This supplies x for the function f(x) = Ax^2 + Bx + C.
		 * @return The value of Ax^2 + Bx + C is returned.
		 */
		virtual double Evaluate(double x) const override;

		/**
		 * Solve the equation 0 = Ax^2 + Bx + C.  Of course, there can
		 * be only zero, one or two real roots.
		 *
		 * @param[out] realRoots This array is populated with the real roots, if any, of this quadratic polynomial.
		 */
		void Solve(std::vector<double>& realRoots) const;

		/**
		 * Calculate and return the descriminant of this quadratic polynomial.
		 *
		 * @return The value B^2 - 4AC is returned.
		 */
		double Descriminant() const;

		/**
		 * Find the quadratic that fits the given points using a Vandermonde matrix.
		 */
		bool FitToPoints(const Vector2& pointA, const Vector2& pointB, const Vector2& pointC);

		/**
		 * Write this quadratic to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this quadratic from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	public:
		double A, B, C;		///< These are the real coeficients of this quadratic polynomial.
	};

	/**
	 * These are cubic polynomials with real coeficients.
	 */
	class IMZADI_API Cubic : public RealToRealFunction
	{
	public:
		Cubic();
		virtual ~Cubic();

		/**
		 * Evaluate this cubic polynomial as x.
		 */
		virtual double Evaluate(double x) const override;

		/**
		 * Find the cubic that fits the given points using a Vandermonde matrix.
		 */
		bool FitToPoints(const Vector2& pointA, const Vector2& pointB, const Vector2& pointC, const Vector2& pointD);

		/**
		 * Write this cubic to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this cubic from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	public:
		double A, B, C, D;
	};

	/**
	 * This is a curve through space defined parametrically in terms of quadratic polynomials.
	 */
	class IMZADI_API QuadraticSpaceCurve : public RealToVector3Function
	{
	public:
		QuadraticSpaceCurve();
		virtual ~QuadraticSpaceCurve();

		virtual Vector3 Evaluate(double t) const override;

		bool FitToPoints(const Vector3& pointA, const Vector3& pointB, const Vector3& pointC, double tA, double tB, double tC);

		void Dump(std::ostream& stream) const;

		void Restore(std::istream& stream);

	public:
		Quadratic quadraticX;
		Quadratic quadraticY;
		Quadratic quadraticZ;
	};

	/**
	 * This is a curve through space defined parametrically in terms of cubic polynomials.
	 */
	class IMZADI_API CubicSpaceCurve : public RealToVector3Function
	{
	public:
		CubicSpaceCurve();
		virtual ~CubicSpaceCurve();

		virtual Vector3 Evaluate(double t) const override;

		bool FitToPoints(const Vector3& pointA, const Vector3& pointB, const Vector3& pointC, const Vector3& pointD, double tA, double tB, double tC, double tD);

		void Dump(std::ostream& stream) const;

		void Restore(std::istream& stream);

	public:
		Cubic cubicX;
		Cubic cubicY;
		Cubic cubicZ;
	};
}