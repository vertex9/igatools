//-+--------------------------------------------------------------------
// Igatools a general purpose Isogeometric analysis library.
// Copyright (C) 2012-2014  by the igatools authors (see authors.txt).
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
//TODO: This test should be splitted in: vector test, matrix test and solver test

/*
 * Test for Epetra Matrix definition
 *  this test i going to be an adaptation of:
 *  http://trilinos.sandia.gov/packages/docs/dev/packages/didasko/doc/html/epetra_ex23.html
 *  author: nico
 *  date: 2013-01-10
 *
 */
// TODO (pauletti, Jun 3, 2014): the comment is not consistent with the test
#include "../tests.h"

#include <igatools/geometry/cartesian_grid.h>

#include <igatools/basis_functions/bspline_space.h>
#include <igatools/basis_functions/bspline_element_accessor.h>
#include <igatools/geometry/unit_element.h>

#include <igatools/linear_algebra/dof_tools.h>
#include <igatools/linear_algebra/linear_solver.h>

int main()
{

    const int dim_domain = 2;
    const int dim_range  = 1;
    const int rank=1;
    const int p = 2;

    out << "Domain dim: " << dim_domain;
    out << " Range dim: " << dim_range;
    out << " Degree: " << p <<endl;


    int n_knots = 3;
    CartesianProductArray<Real , dim_domain> coord ;
    for (int i = 0; i < dim_domain; ++i)
    {
        vector<Real> tmp_coord;
        for (int j = 0; j < n_knots; ++j)
            tmp_coord.push_back(j);
        coord.copy_data_direction(i,tmp_coord);
    }



    auto knots = CartesianGrid<dim_domain>::create(coord);

    auto bspline_space = BSplineSpace< dim_domain, dim_range, rank>::create(p, knots) ;

#if defined(USE_TRILINOS)
    const auto la_pack = LAPack::trilinos;
#elif defined(USE_PETSC)
    const auto la_pack = LAPack::petsc;
#endif
    using VectorType = Vector<la_pack>;
    using MatrixType = Matrix<la_pack>;
    using LinSolverType = LinearSolver<la_pack>;

    MatrixType matrix(
        dof_tools::get_sparsity_pattern(*bspline_space->get_dofs_manager()));


    const Index num_rows = matrix.get_num_rows() ;

    for (Index i = 0; i < num_rows ; i++)
        matrix.add_entry(i,i,2.0);

    matrix.fill_complete();

    matrix.print(out);
    out << std::endl;

    VectorType b(bspline_space->get_num_basis());
    for (Index i = 0; i<b.size() ; i++)
        b.add_entry(i,i*1.0);

    b.print(out);
    out << endl;

    VectorType x(bspline_space->get_num_basis());

    LinSolverType solver(LinSolverType::SolverType::GMRES);
    solver.solve(matrix,b,x);

    x.print(out);
    out << endl;

    out << "Achieved Tolerance = " << solver.get_achieved_tolerance() << endl;
    out << "Num. Iterations    = " << solver.get_num_iterations() << endl;


    return 0;

}
