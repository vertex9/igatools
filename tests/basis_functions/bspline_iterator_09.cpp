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
 *  Test for the BSpline element iterator derivatives values
 *  author: pauletti
 *  date: Aug 21, 2014
 *
 */

#include "../tests.h"

#include <igatools/base/quadrature_lib.h>
#include <igatools/basis_functions/bspline_element.h>
#include <igatools/basis_functions/bspline_handler.h>

const SafeSTLArray<basis_element::Flags, 3> der_flag =
{
  basis_element::Flags::value,
  basis_element::Flags::gradient,
  basis_element::Flags::hessian
};

template <int der, int dim, int range=1, int rank=1>
void elem_derivatives(const int n_knots = 5, const int deg=1)
{
  OUTSTART

  auto grid  = Grid<dim>::const_create(n_knots);

  auto space = SplineSpace<dim,range,rank>::const_create(deg, grid);

  using Basis = BSpline<dim, range, rank>;
  auto basis = Basis::const_create(space);


  auto flag = der_flag[der];

  auto quad = QGauss<dim>::create(2);


  auto elem = basis->begin();
  auto end = basis->end();

  auto elem_handler = basis->create_cache_handler();

  elem_handler->template set_flags<dim>(flag);
  elem_handler->init_element_cache(elem,quad);

  using Elem = BSplineElement<dim,range,rank>;
  using _Value = typename Elem::_Value;
  using _Gradient = typename Elem::_Gradient;
  using _Hessian = typename Elem::_Hessian;

  for (; elem != end; ++elem)
  {
    elem_handler->fill_element_cache(elem);

    out << "Element ID : " << elem->get_index() << std::endl;

    if (der == 0)
    {
      out.begin_item("Basis function values:");
      elem->template get_basis_data<_Value, dim>(0,DofProperties::active).print_info(out);
      out.end_item();
    }
    else if (der == 1)
    {
      out.begin_item("Basis function gradients:");
      elem->template get_basis_data<_Gradient, dim>(0,DofProperties::active).print_info(out);
      out.end_item();
    }
    else if (der == 2)
    {
      out.begin_item("Basis function hessians:");
      elem->template get_basis_data<_Hessian, dim>(0,DofProperties::active).print_info(out);
      out.end_item();
    }
    else
      AssertThrow(false,ExcMessage("Invalid derivative order."));
  }

  OUTEND
}


int main()
{
  const int values = 0;
  const int grad   = 1;
  const int hess   = 2;

  elem_derivatives<values, 1>();
  elem_derivatives<values, 2>();
  elem_derivatives<values,1,2>();
  elem_derivatives<values,1,3>();


  elem_derivatives<grad, 1>(3);
  elem_derivatives<grad, 2>();
  elem_derivatives<grad,1,2>(2);
  elem_derivatives<grad,1,3>();

  elem_derivatives<hess, 1>(3);
  elem_derivatives<hess, 2>();
  elem_derivatives<hess,1,2>(2);
  elem_derivatives<hess,1,3>();


  return  0;
}
