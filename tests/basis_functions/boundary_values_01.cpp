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
 *  Test for the boundary l2 projection function.
 *  On a PhysicalBasis
 *
 *  author: pauletti
 *  date: 2014-11-14
 *
 */

#include "../tests.h"
#include <igatools/basis_functions/basis_tools.h>
#include <igatools/linear_algebra/dof_tools.h>

#include "common_functions.h"

#include <igatools/base/quadrature_lib.h>
#include <igatools/basis_functions/bspline.h>

#include <igatools/functions/grid_function_lib.h>



template<int dim , int codim, int range ,int rank>
void do_test(const int p, const int num_knots = 10)
{
  auto grid = Grid<dim>::create(num_knots);
  auto space = SplineSpace<dim,range,rank>::create(p,grid);
  auto ref_basis = BSpline<dim,range,rank>::create(space) ;
  auto map = grid_functions::IdentityGridFunction<dim>::create(grid);
  auto domain = Domain<dim,codim>::create(map);
  auto basis = PhysicalBasis<dim,range,rank,codim>::create(ref_basis, domain);


  const int sdim = dim-1;
  const int s_id = 0;

  using SubGridElemMap = typename Grid<dim>::template SubGridMap<sdim>;
  SubGridElemMap sub_grid_elem_map;
  const std::shared_ptr<const Grid<sdim>> sub_grid = grid->template get_sub_grid<sdim>(s_id,sub_grid_elem_map);


  auto bndry_domain = domain->get_sub_domain(s_id,sub_grid_elem_map,sub_grid);
  out.begin_item("Boundary Domain");
  bndry_domain->print_info(out);
  out.end_item();

  using BndFunc = Function<dim-1,1,range,1>;
  SafeSTLMap<int,std::shared_ptr<const BndFunc>> boundary_functions;
  boundary_functions[s_id] = TestBoundaryFunction<dim-1,range>::const_create(bndry_domain);


  const int n_qpoints = 4;
  auto quad = QGauss<dim-1>::create(n_qpoints);


  std::map<Index,Real> boundary_values;
  basis_tools::project_boundary_values<dim,codim,range,rank>(
    boundary_functions, *basis, quad, boundary_values);

  out << "basis index \t value" << endl;
  for (auto entry : boundary_values)
    out << entry.first << "\t" << entry.second << endl;
}



int main()
{


  do_test<2, 0, 1, 1>(3);

  //do_test<3,3,1,1>(3);

  //do_test<2,3,1,0>(3);

  return 0;
}

