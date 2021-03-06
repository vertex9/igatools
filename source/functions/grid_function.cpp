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

#include <igatools/functions/grid_function.h>
#include <igatools/functions/grid_function_element.h>
#include <igatools/utils/unique_id_generator.h>

using std::shared_ptr;

IGA_NAMESPACE_OPEN

template<int dim_, int range_>
GridFunction<dim_, range_>::
GridFunction()
  :
  object_id_(UniqueIdGenerator::get_unique_id())
{}


template<int dim_, int range_>
GridFunction<dim_, range_>::
GridFunction(const SharedPtrConstnessHandler<GridType> &grid)
  :
  grid_(grid),
  object_id_(UniqueIdGenerator::get_unique_id())
{
//  Assert(grid_ != nullptr, ExcNullPtr());
}



template<int dim_, int range_>
auto
GridFunction<dim_, range_>::
get_grid() const -> std::shared_ptr<const GridType>
{
  return grid_.get_ptr_const_data();
}



template<int dim_, int range_>
auto
GridFunction<dim_, range_>::
get_grid() -> std::shared_ptr<GridType>
{
  return grid_.get_ptr_data();
}



template<int dim_, int range_>
const std::string &
GridFunction<dim_, range_>::
get_name() const
{
  return name_;
}



template<int dim_, int range_>
void
GridFunction<dim_, range_>::
set_name(const std::string &name)
{
  name_ = name;
}



template<int dim_, int range_>
Index
GridFunction<dim_, range_>::
get_object_id() const
{
  return object_id_;
}



template<int dim_, int range_>
auto
GridFunction<dim_, range_>::
create_element_begin(const PropId &prop) const
-> std::unique_ptr<ElementAccessor>
{
  using Elem = ElementAccessor;

//  const auto elem_it = grid_->get_elements_with_property(prop).cbegin();
  return std::unique_ptr<Elem>(new
  Elem(this->shared_from_this(),grid_->create_element_begin(prop)));
}



template<int dim_, int range_>
auto
GridFunction<dim_, range_>::
create_element_end(const PropId &prop) const
-> std::unique_ptr<ElementAccessor>
{
  using Elem = ElementAccessor;

//  const auto elem_it = grid_->get_elements_with_property(prop).cend();
  return std::unique_ptr<Elem>(new
  Elem(this->shared_from_this(),grid_->create_element_end(prop)));
}



template<int dim_, int range_>
auto
GridFunction<dim_, range_>::
begin(const PropId &prop) const -> ElementIterator
{
  return this->cbegin(prop);
}



template<int dim_, int range_>
auto
GridFunction<dim_, range_>::
end(const PropId &prop) const -> ElementIterator
{
  return this->cend(prop);
}



template<int dim_, int range_>
auto
GridFunction<dim_, range_>::
cbegin(const PropId &prop) const -> ElementIterator
{
  return ElementIterator(
           this->create_element_begin(prop));
}



template<int dim_, int range_>
auto
GridFunction<dim_, range_>::
cend(const PropId &prop) const -> ElementIterator
{
  return ElementIterator(
           this->create_element_end(prop));
}


#ifdef IGATOOLS_WITH_MESH_REFINEMENT
template<int dim_, int range_>
boost::signals2::connection
GridFunction<dim_, range_>::
connect_insert_knots(const typename Grid<dim_>::SignalInsertKnotsSlot &subscriber)
{
  return grid_.get_ptr_data()->connect_insert_knots(subscriber);
}



template<int dim_, int range_>
void
GridFunction<dim_,range_>::
create_connection_for_insert_knots(const std::shared_ptr<self_t> &grid_function)
{
  Assert(grid_function != nullptr, ExcNullPtr());
  Assert(&(*grid_function) == &(*this), ExcMessage("Different objects."));

  auto func_to_connect =
    std::bind(&self_t::rebuild_after_insert_knots,
              grid_function.get(),
              std::placeholders::_1,
              std::placeholders::_2);

  using SlotType = typename Grid<dim_>::SignalInsertKnotsSlot;
  this->connect_insert_knots(SlotType(func_to_connect).track_foreign(grid_function));
}

template<int dim_, int range_>
auto
GridFunction<dim_,range_>::
get_grid_function_previous_refinement() const -> std::shared_ptr<const self_t>
{
  return grid_function_previous_refinement_;
}

#endif // IGATOOLS_WITH_MESH_REFINEMENT

template<int dim_, int range_>
auto
GridFunction<dim_,range_>::
evaluate_preimage(const ValueVector<Value> &phys_points) const
-> ValueVector<GridPoint>
{
  ValueVector<GridPoint> param_point;
  AssertThrow(false,ExcMessage("This function must be implemented in a derived class."));

  return param_point;
}

#ifdef IGATOOLS_WITH_SERIALIZATION
template<int dim_, int range_>
template<class Archive>
void
GridFunction<dim_,range_>::
serialize(Archive &ar)
{
  ar &make_nvp("grid_",grid_);

  ar &make_nvp("name_",name_);


#ifdef IGATOOLS_WITH_MESH_REFINEMENT
  auto tmp = std::const_pointer_cast<self_t>(grid_function_previous_refinement_);
  ar &make_nvp("grid_function_previous_refinement_",tmp);
  grid_function_previous_refinement_ = tmp;
#endif
}
#endif // IGATOOLS_WITH_SERIALIZATION

IGA_NAMESPACE_CLOSE

#include <igatools/functions/grid_function.inst>

