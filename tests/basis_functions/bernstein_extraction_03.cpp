//-+--------------------------------------------------------------------
// Igatools a general purpose Isogeometric analysis library.
// Copyright (C) 2012-2016  by the igatools authors (see authors.txt).
//
// This file is part of the igatools library.
//
// The igatools library is free software: you can use it, redistribute
// it and/or modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//-+--------------------------------------------------------------------
/*
 *  Test for bernstein extraction class
 *  author: pauletti
 *  date:
 *
 */

#include "bernstein_extraction_tests_common.h"
#include <igatools/basis_functions/bernstein_extraction.h>

using std::shared_ptr;

int main()
{
  out.depth_console(10);

  const int dim = 1;
  using SplineSpace = SplineSpace<dim>;
  using DegreeTable = typename SplineSpace::DegreeTable;
  using MultiplicityTable = typename SplineSpace::MultiplicityTable;
  using Periodicity = typename SplineSpace::Periodicity;
  using PeriodicityTable = typename SplineSpace::PeriodicityTable;
  using EndBehaviourTable = typename SplineSpace::EndBehaviourTable;
  using BoundaryKnotsTable = typename SplineSpace::BoundaryKnotsTable;

  {
    DegreeTable deg {{2}};

    auto grid = Grid<dim>::const_create(4);

    auto int_mult = MultiplicityTable({ {{1,3}} });
    auto periodicity = PeriodicityTable(Periodicity(false));
    auto space = SplineSpace::const_create(deg, grid, int_mult,periodicity);

    SafeSTLArray<SafeSTLVector<Real>,2> bn_x {{-0.5, 0, 0}, {1.1, 1.2, 1.3}};
    BoundaryKnotsTable bdry_knots { {bn_x} };

    typename SplineSpace::EndBehaviour endb(BasisEndBehaviour::end_knots);
    EndBehaviourTable endb_t { {endb} };
    auto rep_knots = compute_knots_with_repetitions(*space,bdry_knots,endb_t);
    auto acum_mult = space->accumulated_interior_multiplicities();


    BernsteinExtraction<dim> operators(*grid, rep_knots, acum_mult, deg);
    operators.print_info(out);
  }

  {
    DegreeTable deg {{3}};

    SafeSTLArray<SafeSTLVector<Real>,dim> knots({{0,1,2,3,4}});
    auto grid = Grid<dim>::const_create(knots);
    auto int_mult = SplineSpace::get_multiplicity_from_regularity(InteriorReg::maximum,
                    deg, grid->get_num_intervals());

    auto periodicity = PeriodicityTable(Periodicity(false));

    auto space = SplineSpace::const_create(deg, grid, int_mult,periodicity);

    BoundaryKnotsTable bdry_knots;

    typename SplineSpace::EndBehaviour endb(BasisEndBehaviour::interpolatory);
    EndBehaviourTable endb_t { {endb} };
    auto rep_knots = compute_knots_with_repetitions(*space,bdry_knots,endb_t);
    auto acum_mult = space->accumulated_interior_multiplicities();


    BernsteinExtraction<dim> operators(*grid, rep_knots, acum_mult, deg);
    operators.print_info(out);
  }

  return 0;
}
