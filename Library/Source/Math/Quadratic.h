#pragma once

#include "Defines.h"
#include <vector>

namespace Collision
{
	/**
	 * These are quadratic polynomials with real coeficients and this class revolves
	 * around the idea of working with them and solving quadratic equations.
	 */
	class COLLISION_LIB_API Quadratic
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
		double Evaluate(double x) const;

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

	public:
		double A, B, C;		//< These are the real coeficients of this quadratic polynomial.
	};
}