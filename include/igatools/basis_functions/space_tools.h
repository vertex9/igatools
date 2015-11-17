//-+--------------------------------------------------------------------
// Igatools a general purpose Isogeometric analysis library.
// Copyright (C) 2012-2015  by the igatools authors (see authors.txt).
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

#ifndef SPACE_TOOLS_H_
#define SPACE_TOOLS_H_

#include <igatools/functions/ig_function.h>
#include <igatools/geometry/grid_tools.h>
#include <igatools/functions/sub_function.h>

#include <igatools/basis_functions/physical_space_element.h>
#include <igatools/basis_functions/phys_space_element_handler.h>

#include <igatools/linear_algebra/epetra_solver.h>

#include<set>

IGA_NAMESPACE_OPEN
namespace space_tools
{

#if 0
/**
 * Returns the (L2)-Projection of the function @p function
 * onto the space generated by the @p basis.
 * The integrals in the computations are done using the quadrature @p Q.
 */
template<class Basis, LAPack la_pack = LAPack::trilinos_epetra>
std::shared_ptr<IgFunction<Basis::dim,Basis::codim,Basis::range,Basis::rank> >
projection_l2(const Function<Basis::dim,Basis::codim,Basis::range,Basis::rank> &function,
              const std::shared_ptr<Basis> &basis,
              const std::shared_ptr<const Quadrature<Basis::dim>> &quad,
              const std::string &dofs_property = DofProperties::active)
{
  using ProjFunc = IgFunction<Basis::dim,Basis::codim,Basis::range,Basis::rank>;

  Epetra_SerialComm comm;

//    auto map = EpetraTools::create_map(*space, dofs_property, comm);
  const auto graph = EpetraTools::create_graph(*basis,dofs_property,*basis,dofs_property,comm);


  auto matrix = EpetraTools::create_matrix(*graph);
  auto rhs = EpetraTools::create_vector(matrix->RangeMap());
  auto sol = EpetraTools::create_vector(matrix->DomainMap());

  const auto space_grid = basis->get_grid();
  const auto func_grid = function.get_domain()->get_grid_function()->get_grid();


  Assert(space_grid->same_knots_or_refinement_of(*func_grid),
         ExcMessage("The space grid is not a refinement of the function grid."));

  const int dim = Basis::dim;


  using SpFlags = space_element::Flags;
  auto sp_flag = SpFlags::value |
                 SpFlags::w_measure;
  auto space_elem_handler = basis->create_cache_handler();
  space_elem_handler->template set_flags<dim>(sp_flag);

  auto f_elem = function.begin();
  auto elem = basis->begin();
  auto end  = basis->end();

  space_elem_handler->init_element_cache(*elem,quad);

  const int n_qp = quad->get_num_points();

  using space_element::_Value;

  if (space_grid == func_grid)
  {
    auto func_elem_handler = function.create_cache_handler();

    func_elem_handler->template set_flags<dim>(function_element::Flags::value);

    func_elem_handler->init_cache(*f_elem,quad);

    for (; elem != end; ++elem, ++f_elem)
    {
      const int n_basis = elem->get_num_basis(dofs_property);
      DenseVector loc_rhs(n_basis);
      DenseMatrix loc_mat(n_basis, n_basis);

      func_elem_handler->template fill_cache<dim>(*f_elem,0);
      space_elem_handler->fill_element_cache(*elem);

      loc_mat = 0.;
      loc_rhs = 0.;

      auto f_at_qp = f_elem->template get_values<function_element::_Value,dim>(0);
      auto phi = elem->get_element_values(dofs_property);

      // computing the upper triangular part of the local matrix
      auto w_meas = elem->template get_w_measures<dim>(0);
      for (int i = 0; i < n_basis; ++i)
      {
        const auto phi_i = phi.get_function_view(i);
        for (int j = i; j < n_basis; ++j)
        {
          const auto phi_j = phi.get_function_view(j);
          for (int q = 0; q < n_qp; ++q)
            loc_mat(i,j) += scalar_product(phi_i[q], phi_j[q]) * w_meas[q];
        }

        for (int q = 0; q < n_qp; q++)
          loc_rhs(i) += scalar_product(f_at_qp[q], phi_i[q]) * w_meas[q];
      }

      // filling symmetric ;lower part of local matrix
      for (int i = 0; i < n_basis; ++i)
        for (int j = 0; j < i; ++j)
          loc_mat(i, j) = loc_mat(j, i);

      const auto elem_dofs = elem->get_local_to_global(dofs_property);
      matrix->add_block(elem_dofs,elem_dofs,loc_mat);
      rhs->add_block(elem_dofs,loc_rhs);
    }
    matrix->FillComplete();
  }
  else
  {
//    AssertThrow(false,ExcNotImplemented());

    auto map_elems_id_fine_coarse =
      grid_tools::build_map_elements_id_between_grids(*space_grid,*func_grid);

    for (const auto &elems_id_pair : map_elems_id_fine_coarse)
    {
      elem->move_to(elems_id_pair.first);
      f_elem->move_to(elems_id_pair.second);

      const int n_basis = elem->get_num_basis(dofs_property);
      DenseVector loc_rhs(n_basis);
      DenseMatrix loc_mat(n_basis, n_basis);

      space_elem_handler->fill_element_cache(*elem);

      loc_mat = 0.;
      loc_rhs = 0.;


      //---------------------------------------------------------------------------
      // the function is supposed to be defined on the same grid of the space or coarser
      const auto &elem_grid_accessor = elem->get_grid_element();
      auto quad_in_func_elem = std::make_shared<Quadrature<dim>>(*quad);
      quad_in_func_elem->dilate_translate(
        elem_grid_accessor.
        template get_side_lengths<dim>(0),
        elem_grid_accessor.vertex(0));

      const auto &func_grid_elem =
        f_elem->get_domain_element().get_grid_function_element().get_grid_element();
      auto one_div_f_elem_size = func_grid_elem.template get_side_lengths<dim>(0);
      for (int dir : UnitElement<dim>::active_directions)
        one_div_f_elem_size[dir] = 1.0/one_div_f_elem_size[dir];

      auto f_elem_vertex = -func_grid_elem.vertex(0);
      quad_in_func_elem->translate(f_elem_vertex);
      quad_in_func_elem->dilate(one_div_f_elem_size);


      auto f_at_qp =
        f_elem->template evaluate_at_points<function_element::_Value>(quad_in_func_elem);
      //---------------------------------------------------------------------------


      auto phi = elem->get_element_values(dofs_property);

      // computing the upper triangular part of the local matrix
      auto w_meas = elem->template get_w_measures<dim>(0);
      for (int i = 0; i < n_basis; ++i)
      {
        const auto phi_i = phi.get_function_view(i);
        for (int j = i; j < n_basis; ++j)
        {
          const auto phi_j = phi.get_function_view(j);
          for (int q = 0; q < n_qp; ++q)
            loc_mat(i,j) += scalar_product(phi_i[q], phi_j[q]) * w_meas[q];
        }

        for (int q = 0; q < n_qp; q++)
          loc_rhs(i) += scalar_product(f_at_qp[q], phi_i[q]) * w_meas[q];
      }

      // filling symmetric ;lower part of local matrix
      for (int i = 0; i < n_basis; ++i)
        for (int j = 0; j < i; ++j)
          loc_mat(i, j) = loc_mat(j, i);

      const auto elem_dofs = elem->get_local_to_global(dofs_property);
      matrix->add_block(elem_dofs,elem_dofs,loc_mat);
      rhs->add_block(elem_dofs,loc_rhs);
    }
    matrix->FillComplete();
  }

  auto solver = EpetraTools::create_solver(*matrix, *sol, *rhs);
  auto result = solver->solve();
  AssertThrow(result == Belos::ReturnType::Converged,
              ExcMessage("No convergence."));

  return std::dynamic_pointer_cast<IgFunction<Basis::dim,Basis::codim,Basis::range,Basis::rank>>(
           ProjFunc::create(std::const_pointer_cast<Basis>(basis),
                            *sol, dofs_property));
}
#endif

/**
 * Returns the coefficients of the (L2)-Projection of the Function @p function
 * onto the space generated by the @p basis.
 * The integrals in the computations are done using the Quadrature @p quad.
 */
template<int dim,int codim,int range,int rank>
IgCoefficients
projection_l2_function(const Function<dim,codim,range,rank> &function,
                       const PhysicalSpaceBasis<dim,range,rank,codim> &basis,
                       const std::shared_ptr<const Quadrature<dim>> &quad,
                       const std::string &dofs_property = DofProperties::active)
{
  Epetra_SerialComm comm;

//    auto map = EpetraTools::create_map(*space, dofs_property, comm);
  const auto graph = EpetraTools::create_graph(basis,dofs_property,basis,dofs_property,comm);


  auto matrix = EpetraTools::create_matrix(*graph);
  auto rhs = EpetraTools::create_vector(matrix->RangeMap());
  auto sol = EpetraTools::create_vector(matrix->DomainMap());

  const auto space_grid = basis.get_grid();
  const auto func_grid = function.get_domain()->get_grid_function()->get_grid();


  Assert(space_grid->same_knots_or_refinement_of(*func_grid),
         ExcMessage("The space grid is not a refinement of the function grid."));

  using SpFlags = space_element::Flags;
  auto sp_flag = SpFlags::value |
                 SpFlags::w_measure;
  auto space_elem_handler = basis.create_cache_handler();
  space_elem_handler->set_element_flags(sp_flag);

  auto f_elem = function.begin();
  auto elem = basis.begin();
  auto end  = basis.end();

  space_elem_handler->init_element_cache(*elem,quad);


  using space_element::_Value;

  if (space_grid == func_grid)
  {
    auto func_elem_handler = function.create_cache_handler();

    func_elem_handler->set_element_flags(function_element::Flags::value);

    func_elem_handler->init_cache(*f_elem,quad);

    for (; elem != end; ++elem, ++f_elem)
    {
      func_elem_handler->template fill_cache<dim>(*f_elem,0);
      space_elem_handler->fill_element_cache(*elem);

      auto f_at_qp = f_elem->template get_values<function_element::_Value,dim>(0);

      const auto loc_mat = elem->integrate_u_v(dofs_property);
      const auto loc_rhs = elem->integrate_u_func(f_at_qp,dofs_property);

      const auto elem_dofs = elem->get_local_to_global(dofs_property);
      matrix->add_block(elem_dofs,elem_dofs,loc_mat);
      rhs->add_block(elem_dofs,loc_rhs);
    }
    matrix->FillComplete();
  }
  else
  {
//    AssertThrow(false,ExcNotImplemented());

    auto map_elems_id_fine_coarse =
      grid_tools::build_map_elements_id_between_grids(*space_grid,*func_grid);

    for (const auto &elems_id_pair : map_elems_id_fine_coarse)
    {
      elem->move_to(elems_id_pair.first);
      f_elem->move_to(elems_id_pair.second);

      space_elem_handler->fill_element_cache(*elem);


      //---------------------------------------------------------------------------
      // the function is supposed to be defined on the same grid of the space or coarser
      const auto &elem_grid_accessor = elem->get_grid_element();
      auto quad_in_func_elem = std::make_shared<Quadrature<dim>>(*quad);
      quad_in_func_elem->dilate_translate(
        elem_grid_accessor.
        template get_side_lengths<dim>(0),
        elem_grid_accessor.vertex(0));

      const auto &func_grid_elem =
        f_elem->get_domain_element().get_grid_function_element().get_grid_element();
      auto one_div_f_elem_size = func_grid_elem.template get_side_lengths<dim>(0);
      for (int dir : UnitElement<dim>::active_directions)
        one_div_f_elem_size[dir] = 1.0/one_div_f_elem_size[dir];

      auto f_elem_vertex = -func_grid_elem.vertex(0);
      quad_in_func_elem->translate(f_elem_vertex);
      quad_in_func_elem->dilate(one_div_f_elem_size);


      auto f_at_qp =
        f_elem->template evaluate_at_points<function_element::_Value>(quad_in_func_elem);
      //---------------------------------------------------------------------------


      const auto loc_mat = elem->integrate_u_v(dofs_property);
      const auto loc_rhs = elem->integrate_u_func(f_at_qp,dofs_property);

      const auto elem_dofs = elem->get_local_to_global(dofs_property);
      matrix->add_block(elem_dofs,elem_dofs,loc_mat);
      rhs->add_block(elem_dofs,loc_rhs);
    }
    matrix->FillComplete();
  }

  auto solver = EpetraTools::create_solver(*matrix, *sol, *rhs);
  auto result = solver->solve();
  AssertThrow(result == Belos::ReturnType::Converged,
              ExcMessage("No convergence."));

  IgCoefficients ig_coeffs;

  const auto &dof_distribution = *(basis.get_spline_space()->get_dof_distribution());
  const auto &active_dofs = dof_distribution.get_global_dofs(dofs_property);

  const auto &epetra_map = sol->Map();

  for (const auto glob_dof : active_dofs)
  {
    auto loc_id = epetra_map.LID(glob_dof);
    Assert(loc_id >= 0,
           ExcMessage("Global dof " + std::to_string(glob_dof) + " not present in the input EpetraTools::Vector."));
    ig_coeffs[glob_dof] = (*sol)[loc_id];
  }

  return ig_coeffs;
}

#if 0
template<int dim,int range, LAPack la_pack = LAPack::trilinos_epetra>
IgCoefficients
projection_l2_ig_grid_function(
  const IgGridFunction<dim,range> &ig_grid_function,
  const ReferenceSpaceBasis<dim,range,1> &ref_basis,
  const std::shared_ptr<const Quadrature<dim>> &quad,
  const std::string &dofs_property = DofProperties::active)
{
  Epetra_SerialComm comm;

  const auto graph =
    EpetraTools::create_graph(ref_basis,dofs_property,ref_basis,dofs_property,comm);


  auto matrix = EpetraTools::create_matrix(*graph);
  auto rhs = EpetraTools::create_vector(matrix->RangeMap());
  auto sol = EpetraTools::create_vector(matrix->DomainMap());

  const auto space_grid = ref_basis.get_grid();
  const auto func_grid = ig_grid_function.get_grid();


  Assert(space_grid->same_knots_or_refinement_of(*func_grid),
         ExcMessage("The space grid is not a refinement of the function grid."));


  using SpFlags = space_element::Flags;
  auto sp_flag = SpFlags::value |
                 SpFlags::w_measure;
  auto space_elem_handler = ref_basis.create_cache_handler();
  space_elem_handler->template set_flags<dim>(sp_flag);

  auto f_elem = ig_grid_function.cbegin();
  auto elem = ref_basis.cbegin();
  auto end  = ref_basis.cend();

  space_elem_handler->init_element_cache(*elem,quad);

  const int n_qp = quad->get_num_points();

  using space_element::_Value;

  using D0 = grid_function_element::_D<0>;
  if (space_grid == func_grid)
  {
    auto func_elem_handler = ig_grid_function.create_cache_handler();
    func_elem_handler->template set_flags<dim>(grid_function_element::Flags::D0);

    func_elem_handler->init_cache(*f_elem,quad);

    for (; elem != end; ++elem, ++f_elem)
    {
      const int n_basis = elem->get_num_basis(dofs_property);
      DenseVector loc_rhs(n_basis);
      DenseMatrix loc_mat(n_basis, n_basis);

      func_elem_handler->template fill_cache<dim>(*f_elem,0);
      space_elem_handler->fill_element_cache(*elem);

      loc_mat = 0.;
      loc_rhs = 0.;

      auto f_at_qp = f_elem->template get_values_from_cache<D0,dim>(0);
      auto phi = elem->get_element_values(dofs_property);

      // computing the upper triangular part of the local matrix
      auto w_meas = elem->template get_w_measures<dim>(0);
      for (int i = 0; i < n_basis; ++i)
      {
        const auto phi_i = phi.get_function_view(i);
        for (int j = i; j < n_basis; ++j)
        {
          const auto phi_j = phi.get_function_view(j);
          for (int q = 0; q < n_qp; ++q)
            loc_mat(i,j) += scalar_product(phi_i[q], phi_j[q]) * w_meas[q];
        }

        for (int q = 0; q < n_qp; q++)
          loc_rhs(i) += scalar_product(f_at_qp[q], phi_i[q]) * w_meas[q];
      }

      // filling symmetric ;lower part of local matrix
      for (int i = 0; i < n_basis; ++i)
        for (int j = 0; j < i; ++j)
          loc_mat(i, j) = loc_mat(j, i);

      const auto elem_dofs = elem->get_local_to_global(dofs_property);
      matrix->add_block(elem_dofs,elem_dofs,loc_mat);
      rhs->add_block(elem_dofs,loc_rhs);
    }
    matrix->FillComplete();
  }
  else
  {
    auto map_elems_id_fine_coarse =
      grid_tools::build_map_elements_id_between_grids(*space_grid,*func_grid);

    for (const auto &elems_id_pair : map_elems_id_fine_coarse)
    {
      elem->move_to(elems_id_pair.first);
      f_elem->move_to(elems_id_pair.second);

      const int n_basis = elem->get_num_basis(dofs_property);
      DenseVector loc_rhs(n_basis);
      DenseMatrix loc_mat(n_basis, n_basis);

      space_elem_handler->fill_element_cache(*elem);

      loc_mat = 0.;
      loc_rhs = 0.;


      //---------------------------------------------------------------------------
      // the function is supposed to be defined on a coarser grid of the space
      const auto &elem_grid_accessor = elem->get_grid_element();
      auto quad_in_func_elem = std::make_shared<Quadrature<dim>>(*quad);
      quad_in_func_elem->dilate_translate(
        elem_grid_accessor.
        template get_side_lengths<dim>(0),
        elem_grid_accessor.vertex(0));

      const auto &func_grid_elem = f_elem->get_grid_element();
      auto one_div_f_elem_size = func_grid_elem.template get_side_lengths<dim>(0);
      for (int dir : UnitElement<dim>::active_directions)
        one_div_f_elem_size[dir] = 1.0/one_div_f_elem_size[dir];

      auto f_elem_vertex = -func_grid_elem.vertex(0);
      quad_in_func_elem->translate(f_elem_vertex);
      quad_in_func_elem->dilate(one_div_f_elem_size);


      auto f_at_qp =
        f_elem->template evaluate_at_points<D0>(quad_in_func_elem);
      //---------------------------------------------------------------------------


      auto phi = elem->get_element_values(dofs_property);

      // computing the upper triangular part of the local matrix
      auto w_meas = elem->template get_w_measures<dim>(0);
      for (int i = 0; i < n_basis; ++i)
      {
        const auto phi_i = phi.get_function_view(i);
        for (int j = i; j < n_basis; ++j)
        {
          const auto phi_j = phi.get_function_view(j);
          for (int q = 0; q < n_qp; ++q)
            loc_mat(i,j) += scalar_product(phi_i[q], phi_j[q]) * w_meas[q];
        }

        for (int q = 0; q < n_qp; q++)
          loc_rhs(i) += scalar_product(f_at_qp[q], phi_i[q]) * w_meas[q];
      }

      // filling symmetric ;lower part of local matrix
      for (int i = 0; i < n_basis; ++i)
        for (int j = 0; j < i; ++j)
          loc_mat(i, j) = loc_mat(j, i);

      const auto elem_dofs = elem->get_local_to_global(dofs_property);
      matrix->add_block(elem_dofs,elem_dofs,loc_mat);
      rhs->add_block(elem_dofs,loc_rhs);
    }
    matrix->FillComplete();
  }

  auto solver = EpetraTools::create_solver(*matrix, *sol, *rhs);
  auto result = solver->solve();
  AssertThrow(result == Belos::ReturnType::Converged,
              ExcMessage("No convergence."));

  IgCoefficients ig_coeffs;

  const auto &dof_distribution = *(ref_basis.get_spline_space()->get_dof_distribution());
  const auto &active_dofs = dof_distribution.get_global_dofs(dofs_property);

  const auto &epetra_map = sol->Map();

  for (const auto glob_dof : active_dofs)
  {
    auto loc_id = epetra_map.LID(glob_dof);
    Assert(loc_id >= 0,
           ExcMessage("Global dof " + std::to_string(glob_dof) + " not present in the input EpetraTools::Vector."));
    ig_coeffs[glob_dof] = (*sol)[loc_id];
  }

  return ig_coeffs;
}
#endif



/**
 * Returns the coefficients of the (L2)-Projection of the GridFunction @p grid_function
 * onto the space generated by the @p basis.
 * The integrals in the computations are done using the Quadrature @p quad.
 */
template<int dim,int range>
IgCoefficients
projection_l2_grid_function(
  const GridFunction<dim,range> &grid_function,
  const ReferenceSpaceBasis<dim,range,1> &ref_basis,
  const std::shared_ptr<const Quadrature<dim>> &quad,
  const std::string &dofs_property = DofProperties::active)
{
  Assert(quad != nullptr,ExcNullPtr());

  Epetra_SerialComm comm;

  const auto graph =
    EpetraTools::create_graph(ref_basis,dofs_property,ref_basis,dofs_property,comm);


  auto matrix = EpetraTools::create_matrix(*graph);
  auto rhs = EpetraTools::create_vector(matrix->RangeMap());
  auto sol = EpetraTools::create_vector(matrix->DomainMap());

  const auto space_grid = ref_basis.get_grid();
  const auto func_grid = grid_function.get_grid();


  Assert(space_grid->same_knots_or_refinement_of(*func_grid),
         ExcMessage("The space grid is not a refinement of the function grid."));


  using SpFlags = space_element::Flags;
  auto sp_flag = SpFlags::value |
                 SpFlags::w_measure;
  auto space_elem_handler = ref_basis.create_cache_handler();
  space_elem_handler->set_element_flags(sp_flag);

  auto f_elem = grid_function.cbegin();
  auto elem = ref_basis.cbegin();
  auto end  = ref_basis.cend();

  space_elem_handler->init_element_cache(*elem,quad);

  using D0 = grid_function_element::_D<0>;
  if (space_grid == func_grid)
  {
    auto func_elem_handler = grid_function.create_cache_handler();
    func_elem_handler->set_element_flags(grid_function_element::Flags::D0);

    func_elem_handler->init_cache(*f_elem,quad);

    for (; elem != end; ++elem, ++f_elem)
    {
      func_elem_handler->template fill_cache<dim>(*f_elem,0);
      space_elem_handler->fill_element_cache(*elem);

      auto f_at_qp = f_elem->template get_values_from_cache<D0,dim>(0);

      const auto loc_mat = elem->integrate_u_v(dofs_property);
      const auto loc_rhs = elem->integrate_u_func(f_at_qp,dofs_property);

      const auto elem_dofs = elem->get_local_to_global(dofs_property);
      matrix->add_block(elem_dofs,elem_dofs,loc_mat);
      rhs->add_block(elem_dofs,loc_rhs);
    }
    matrix->FillComplete();
  }
  else
  {
    auto map_elems_id_fine_coarse =
      grid_tools::build_map_elements_id_between_grids(*space_grid,*func_grid);

    for (const auto &elems_id_pair : map_elems_id_fine_coarse)
    {
      elem->move_to(elems_id_pair.first);
      f_elem->move_to(elems_id_pair.second);

      space_elem_handler->fill_element_cache(*elem);


      //---------------------------------------------------------------------------
      // the function is supposed to be defined on a coarser grid of the space
      const auto &elem_grid_accessor = elem->get_grid_element();
      auto quad_in_func_elem = std::make_shared<Quadrature<dim>>(*quad);
      quad_in_func_elem->dilate_translate(
        elem_grid_accessor.
        template get_side_lengths<dim>(0),
        elem_grid_accessor.vertex(0));

      const auto &func_grid_elem = f_elem->get_grid_element();
      auto one_div_f_elem_size = func_grid_elem.template get_side_lengths<dim>(0);
      for (int dir : UnitElement<dim>::active_directions)
        one_div_f_elem_size[dir] = 1.0/one_div_f_elem_size[dir];

      auto f_elem_vertex = -func_grid_elem.vertex(0);
      quad_in_func_elem->translate(f_elem_vertex);
      quad_in_func_elem->dilate(one_div_f_elem_size);


      auto f_at_qp =
        f_elem->template evaluate_at_points<D0>(quad_in_func_elem);
      //---------------------------------------------------------------------------


      const auto loc_mat = elem->integrate_u_v(dofs_property);
      const auto loc_rhs = elem->integrate_u_func(f_at_qp,dofs_property);

      const auto elem_dofs = elem->get_local_to_global(dofs_property);
      matrix->add_block(elem_dofs,elem_dofs,loc_mat);
      rhs->add_block(elem_dofs,loc_rhs);
    }
    matrix->FillComplete();
  }

  auto solver = EpetraTools::create_solver(*matrix, *sol, *rhs);
  auto result = solver->solve();
  AssertThrow(result == Belos::ReturnType::Converged,
              ExcMessage("No convergence."));

  IgCoefficients ig_coeffs;

  const auto &dof_distribution = *(ref_basis.get_spline_space()->get_dof_distribution());
  const auto &active_dofs = dof_distribution.get_global_dofs(dofs_property);

  const auto &epetra_map = sol->Map();

  for (const auto glob_dof : active_dofs)
  {
    auto loc_id = epetra_map.LID(glob_dof);
    Assert(loc_id >= 0,
           ExcMessage("Global dof " + std::to_string(glob_dof) + " not present in the input EpetraTools::Vector."));
    ig_coeffs[glob_dof] = (*sol)[loc_id];
  }

  return ig_coeffs;
}


/**
 * Projects (using the L2 scalar product) a function to the whole or part
 * of the boundary of the domain.
 * The piece of the domain is indicated by the boundary ids and the
 * projection is computed using the provided quadrature rule.
 *
 * The projected function is returned in boundary_values, a map containing all
 * indices of degrees of freedom at the boundary and the computed coefficient value
 * for this degree of freedom.
 *
 */
template<int dim,int codim, int range, int rank>
void
project_boundary_values(
  std::map<int, std::shared_ptr<const Function<dim-1,codim+1,range,rank>>> &bndry_funcs,
  const PhysicalSpaceBasis<dim,range,rank,codim> &basis,
  const std::shared_ptr<const Quadrature<(dim > 1)?dim-1:0>> &quad,
  std::map<Index, Real>  &boundary_values)
{
  static_assert(dim > 1,"The dimension must be > 1");


  const int sdim = dim - 1;

  using Basis = PhysicalSpaceBasis<dim,range,rank,codim>;
  using InterSpaceMap = typename Basis::template InterSpaceMap<sdim>;

  using InterGridMap = typename Grid<dim>::template SubGridMap<sdim>;


  const auto grid = basis.get_grid();

  for (const auto &bndry : bndry_funcs)
  {
    InterGridMap elem_map;

    const int s_id = bndry.first;

    Assert(bndry.second != nullptr,ExcNullPtr());
    const auto &bndry_func = *bndry.second;

    const std::shared_ptr<const Grid<sdim>> sub_grid = grid->template get_sub_grid<sdim>(s_id,elem_map);

    InterSpaceMap  dof_map;
    const auto sub_basis = basis.template get_sub_space<sdim>(s_id, dof_map,sub_grid,elem_map);


    const auto coeffs = projection_l2_function(
                          bndry_func,*sub_basis,quad,DofProperties::active);

    const int face_n_dofs = dof_map.size();
    for (Index i = 0; i< face_n_dofs; ++i)
      boundary_values[dof_map[i]] = coeffs[i];
  }
}


#if 0
template<int dim,int range>
void
project_boundary_values(
//    std::map<int, std::shared_ptr<const GridFunction<dim-1,codim+1,range,rank>>> &bndry_funcs,
  const GridFunction<dim,range> &grid_func,
  const ReferenceSpaceBasis<dim,range> &basis,
  const std::shared_ptr<const Quadrature<(dim > 1)?dim-1:1>> &quad,
  const std::set<boundary_id>  &boundary_ids,
  std::map<Index, Real>  &boundary_values)
{
  static_assert(dim > 1,"The dimension must be > 1");

//  const auto &ig_grid_func =
//    dynamic_cast<const IgGridFunction<dim,range> &>(grid_func);


  const int sdim = dim - 1;

  using Basis = ReferenceSpaceBasis<dim,range>;
  using InterSpaceMap = typename Basis::template InterSpaceMap<sdim>;

  using InterGridMap = typename Grid<dim>::template SubGridMap<sdim>;


  const auto grid = basis.get_grid();

  std::set<int> sub_elems;
  auto bdry_begin = boundary_ids.begin();
  auto bdry_end   = boundary_ids.end();
  for (auto &s_id : UnitElement<dim>::template elems_ids<sdim>())
  {
    const auto bdry_id = grid->get_boundary_id(s_id);
    if (find(bdry_begin, bdry_end, bdry_id) != bdry_end)
      sub_elems.insert(s_id);
  }

  for (const Index &s_id : sub_elems)
  {
    InterGridMap elem_map;

    const std::shared_ptr<const Grid<sdim>> sub_grid = grid->template get_sub_grid<sdim>(s_id,elem_map);

    InterSpaceMap  dof_map;
    const auto sub_basis = basis.template get_ref_sub_space<sdim>(s_id, dof_map,sub_grid);

//    const auto sub_func = ig_grid_func.get_sub_function(s_id,sub_grid);
    const auto sub_func = grid_func.get_sub_function(s_id,sub_grid);

    const auto coeffs = projection_l2_grid_function(
                          *sub_func,*sub_basis,quad,DofProperties::active);

    const int face_n_dofs = dof_map.size();
    for (Index i = 0; i< face_n_dofs; ++i)
      boundary_values[dof_map[i]] = coeffs[i];
  }
}
#endif

/**
 * Returns the list of global ids of the non zero basis functions
 * on the faces with the given boundary ids.
 */
template<class Basis>
std::set<Index>
get_boundary_dofs(std::shared_ptr<const Basis> basis,
                  const std::set<boundary_id>  &boundary_ids)
{
  const int dim   = Basis::dim;
  std::set<Index> dofs;
  const int sub_dim = dim - 1;

  auto grid = basis->get_grid();

  std::set<int> sub_elems;
  auto bdry_begin = boundary_ids.begin();
  auto bdry_end   = boundary_ids.end();
  for (auto &s_id : UnitElement<Basis::dim>::template elems_ids<sub_dim>())
  {
    const auto bdry_id = grid->get_boundary_id(s_id);
    if (find(bdry_begin, bdry_end, bdry_id) != bdry_end)
      sub_elems.insert(s_id);
  }

  const auto &dof_distribution =
    *basis->get_spline_space()->get_dof_distribution();
  Topology<sub_dim> sub_elem_topology;
  for (const Index &s_id : sub_elems)
  {
    auto s_dofs = dof_distribution.get_boundary_dofs(s_id,sub_elem_topology);
    dofs.insert(s_dofs.begin(), s_dofs.end());
  }

  return dofs;
}



/**
 * Numerically computes the local element contribution
 * to the integral  \f$\int_\Omega D^kf\f$.
 * This contributions are written to the vector
 * @p element_error and the integral value is returned.
 *
 * @note It is generally not used directly, but usually called from other
 * functions
 */
template<int order, int dim, int codim = 0, int range = 1, int rank = 1>
Conditional<order==0,
            typename Function<dim, codim, range, rank>::Value,
            typename Function<dim, codim, range, rank>::template Derivative<order>>
integrate(Function<dim, codim, range, rank> &f,
          const std::shared_ptr<const Quadrature<dim>> &quad,
          SafeSTLMap<
          TensorIndex<dim>,
          Conditional<order==0,
          typename Function<dim, codim, range, rank>::Value,
          typename Function<dim, codim, range, rank>::template Derivative<order>>
          > &element_error)
{
  Assert(quad != nullptr,ExcNullPtr());

  using Value = Conditional<order==0,
        typename Function<dim, codim, range, rank>::Value,
        typename Function<dim, codim, range, rank>::template Derivative<order>>;

  using _Val =
    Conditional<order==0,
    typename function_element::_Value,
    Conditional<order==1,
    typename function_element::_Gradient,
    typename function_element::_D2
    >
    >;

  auto f_handler = f.create_cache_handler();

  const auto flag = function_element::Flags::value |
                    function_element::Flags::w_measure;
  f_handler->set_element_flags(flag);

  auto elem_f = f.begin();
  auto end = f.end();

  const int n_points = quad->get_num_points();
  f_handler->init_cache(elem_f, quad);
  Value val;
  for (; elem_f != end; ++elem_f)
  {
    f_handler->fill_element_cache(elem_f);

    auto f_val = elem_f->template get_values<_Val,dim>(0);
    auto w_meas = elem_f->get_domain_element().get_element_w_measures();

    val = 0.0;
    for (int pt = 0; pt < n_points; ++pt)
      val += f_val[pt] * w_meas[pt];

    element_error[ elem_f->get_index() ] = val;
  }

  val = 0.0;
  for (const auto &el_val : element_error)
    val += el_val.second;
  return val;
}

/**
 * Numerically computes the local element contribution
 * to the integral  \f$\int_\Omega |D^p(f-g)|^p\f$.
 * This contributions are added to the vector
 * @p element_error.
 *
 * @note It is generally not used directly, but usually called from other
 * functions
 */
template<int order, int dim, int codim = 0, int range = 1, int rank = 1>
void norm_difference(Function<dim, codim, range, rank> &f,
                     Function<dim, codim, range, rank> &g,
                     const Quadrature<dim> &quad,
                     const Real p,
                     SafeSTLVector<Real> &element_error)
{
  const bool is_inf = p==std::numeric_limits<Real>::infinity()? true : false;
  auto flag = domain_element::Flags::w_measure;
//  auto flag = ValueFlags::point | ValueFlags::w_measure | order_to_flag[order];

  using _Val =
    Conditional<order==0,
    typename function_element::_Value,
    Conditional<order==1,
    typename function_element::_Gradient,
    typename function_element::_D2
    >
    >;

  f.reset(flag, quad);
  g.reset(flag, quad);
  const int n_points = quad.get_num_points();

  auto elem_f = f.begin();
  auto elem_g = g.begin();
  auto end = f.end();

  const auto topology = Topology<dim>();

  f.init_cache(elem_f, topology);
  g.init_cache(elem_g, topology);

  for (; elem_f != end; ++elem_f, ++elem_g)
  {
    f.fill_cache(elem_f, topology, 0);
    g.fill_cache(elem_g, topology, 0);

    const int elem_id = elem_f->get_flat_index();

    auto f_val = elem_f->template get_values<_Val,dim>(0);
    auto g_val = elem_g->template get_values<_Val,dim>(0);
    auto w_meas = elem_f->template get_w_measures<dim>(0);

    Real elem_diff_pow_p = 0.0;
    Real val;
    for (int iPt = 0; iPt < n_points; ++iPt)
    {
      const auto err = f_val[iPt] - g_val[iPt];
      val = err.norm_square();
      if (is_inf)
        elem_diff_pow_p = std::max(elem_diff_pow_p, fabs(sqrt(val)));
      else
        elem_diff_pow_p += std::pow(val,p/2.) * w_meas[iPt];
    }
    element_error[ elem_id ] += elem_diff_pow_p;
  }
}



template<int dim, int codim = 0, int range = 1, int rank = 1>
Real l2_norm_difference(Function<dim, codim, range, rank> &f,
                        Function<dim, codim, range, rank> &g,
                        const Quadrature<dim> &quad,
                        SafeSTLVector<Real> &elem_error)
{
  const Real p=2.;
  const Real one_p = 1./p;
  const int order=0;

  space_tools::norm_difference<order,dim, codim, range, rank>(f, g, quad, p, elem_error);

  Real err = 0;
  for (Real &loc_err : elem_error)
  {
    err += loc_err;
    loc_err = std::pow(loc_err,one_p);

  }

  return std::pow(err,one_p);
}



template<int dim, int codim = 0, int range = 1, int rank = 1>
Real h1_norm_difference(Function<dim, codim, range, rank> &f,
                        Function<dim, codim, range, rank> &g,
                        const Quadrature<dim> &quad,
                        SafeSTLVector<Real> &elem_error)
{
  const Real p=2.;
  const Real one_p = 1./p;

  space_tools::norm_difference<0,dim, codim, range, rank>(f, g, quad, p, elem_error);
  space_tools::norm_difference<1,dim, codim, range, rank>(f, g, quad, p, elem_error);

  Real err = 0;
  for (Real &loc_err : elem_error)
  {
    err += loc_err;
    loc_err = std::pow(loc_err,one_p);
  }

  return std::pow(err,one_p);
}



template<int dim, int codim = 0, int range = 1, int rank = 1>
Real inf_norm_difference(Function<dim, codim, range, rank> &f,
                         Function<dim, codim, range, rank> &g,
                         const Quadrature<dim> &quad,
                         SafeSTLVector<Real> &elem_error)
{
  const Real p=std::numeric_limits<Real>::infinity();
  space_tools::norm_difference<0, dim, codim, range, rank>(f, g, quad, p, elem_error);
  Real err = 0;
  for (const Real &loc_err : elem_error)
    err = std::max(err,loc_err);

  return err;
}

};

IGA_NAMESPACE_CLOSE

#endif
