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

#ifndef GRID_ELEMENT_HANDLER_H_
#define GRID_ELEMENT_HANDLER_H_

#include <igatools/base/config.h>
#include <igatools/base/cache_status.h>
#include <igatools/base/new_flags_handler.h>
#include <igatools/base/quadrature.h>
#include <igatools/utils/tensor_product_array.h>
#include <igatools/geometry/cartesian_grid.h>

IGA_NAMESPACE_OPEN

/**
 * Grid element value manager
 *
 * computational optimization cache, storing the interval length
 * in each direction.
 *
 */
template <int dim_>
class GridElementHandler
{
private:
    using GridType = CartesianGrid<dim_>;

protected:
    using ElementIterator = typename GridType::ElementIterator;
    using ElementAccessor = typename GridType::ElementAccessor;
    static const std::array<Size, UnitElement<dim_>::faces_per_element> faces;

protected:
    void init_element_cache(ElementAccessor &elem);
    void fill_element_cache(ElementAccessor &elem);
    // void fill_face_cache(ElementAccessor &elem, const int face);
public:
    static const int dim = dim_;
    static const NewValueFlags valid_flags = NewValueFlags::measure |
                                   NewValueFlags::w_measure |
                                   NewValueFlags::point|
                                   NewValueFlags::length;

    //Allocates and fill the (global) cache
    GridElementHandler(std::shared_ptr<const GridType> grid,
                         const NewValueFlags flag,
                         const Quadrature<dim> &quad);

    void reset(const NewValueFlags flag, const Quadrature<dim> &quad);
    /**
     * Allocates the space in ElementIterator element_cache
     * necessary for the given quadrature and flag combination.
     * It also fills the invariant (not changing) members of
     * the cache.
     */
    void init_element_cache(ElementIterator &elem);

    /**
     * Fills the ElementIterator element_cache
     * element dependent part
     */
    void fill_element_cache(ElementIterator &elem);

    /**
     * Fills the ElementIterator face_cache
     * element dependent part
     */
    void fill_face_cache(ElementIterator &elem, const int face);

    void print_info(LogStream &out) const;

protected:
    template <int k>
    void fill_element_cache_(ElementAccessor &elem, const int j);

private:
    std::shared_ptr<const GridType> grid_;
    std::tuple<GridFlags, GridFlags> flags_;
    Quadrature<dim> quad_;

protected:
    TensorProductArray<dim> lengths_;
};

IGA_NAMESPACE_CLOSE

#endif /* GRID_UNIFORM_QUAD_CACHE_H_ */