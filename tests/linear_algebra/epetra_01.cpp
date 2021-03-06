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
//TODO: This test should be splitted in: SafeSTLVector test, matrix test and solver test

/*
 * Test for developing epetra minimal and efficient linear algebra interaction
 *  author: pauletti
 *  date: 2015-03-30
 *
 */

#include "../tests.h"

#include <igatools/geometry/grid.h>
#include <igatools/basis_functions/bspline.h>
#include <igatools/linear_algebra/epetra.h>

#include <igatools/basis_functions/basis_tools.h>

using basis_tools::get_boundary_dofs;

template<int dim>
void matrix_map(const int deg, const int n_knots)
{
  OUTSTART
  auto grid = Grid<dim>::create(n_knots);
  auto space = SplineSpace<dim>::create(deg, grid);
  auto basis = BSpline<dim>::create(space);

  Epetra_SerialComm comm;


  auto map = EpetraTools::create_map(*basis, "active", comm);
  map->Print(out.get_file_stream());

  auto graph = EpetraTools::create_graph(*basis, "active", *basis, "active",comm);
  graph->Print(out.get_file_stream());

  auto matrix = EpetraTools::create_matrix(*graph);
  auto vec = EpetraTools::create_vector(*map);

  matrix->Print(out.get_file_stream());
  vec->Print(out.get_file_stream());

  OUTEND
}

#if 0

template<int dim>
void matrix_map1(const int deg, const int n_knots)
{
  OUTSTART
  auto grid = Grid<dim>::create(n_knots);
  auto r_basis = BSpline<dim>::create(deg, grid);
  auto c_basis = BSpline<dim>::create(deg+1, grid);
  MatrixGraph<LAPack::trilinos_epetra> graph(r_basis, "active", c_basis, "active");
  graph.print_info(out);

  OUTEND
}



struct DofProp
{
  static const std::string interior;
  static const std::string dirichlet;
  static const std::string neumman;
};

const std::string DofProp::interior = "interior";
const std::string DofProp::dirichlet  = "dirichlet";
const std::string DofProp::neumman  = "neumman";



enum  bc : boundary_id
{
  dir=0, neu
};

template<int dim, int range = 1>
void matrix_map2(const int deg, const int n_knots)
{
  OUTSTART
  using Basis = BSpline<dim>;
  auto grid = Grid<dim>::create(n_knots);

  grid->set_boundary_id(0, 1);

  auto basis = BSpline<dim>::create(deg, grid);

  std::set<boundary_id>  dir_ids = {0};
  auto dir_dofs = get_boundary_dofs<Basis>(basis, dir_ids);

  auto int_dofs = basis->get_interior_dofs();

  std::set<boundary_id>  neu_ids = {1};
  auto neu_dofs = get_boundary_dofs<Basis>(basis, neu_ids);
  SafeSTLVector<Index> common(dim*range);
  auto end1 =
    std::set_intersection(neu_dofs.begin(), neu_dofs.end(),
                          dir_dofs.begin(), dir_dofs.end(), common.begin());
  common.resize(end1-common.begin());
  for (auto &id : common)
    neu_dofs.erase(id);

  auto dof_dist = basis->get_ptr_dof_distribution();
  dof_dist->add_dofs_property(DofProp::interior);
  dof_dist->add_dofs_property(DofProp::dirichlet);
  dof_dist->add_dofs_property(DofProp::neumman);


  dof_dist->set_dof_property_status(DofProp::interior, int_dofs, true);
  dof_dist->set_dof_property_status(DofProp::dirichlet, dir_dofs, true);
  dof_dist->set_dof_property_status(DofProp::neumman, neu_dofs, true);

  MatrixGraph<LAPack::trilinos_epetra> graph(basis, DofProp::interior,
                                             basis, DofProp::neumman);

  graph.print_info(out);

  //auto matrix = graph.create_matrix();

  OUTEND
}

#endif

int main()
{
  //matrix_map<1>(1,3);
  matrix_map<2>(1,3);
  //matrix_map1<1>(1,3);
  //matrix_map1<2>(1,3);
  //matrix_map2<1>(1,3);
  //matrix_map2<2>(1,3);

  return 0;

}
