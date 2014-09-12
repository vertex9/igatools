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

#ifndef CARTESIAN_GRID_ELEMENT_ACCESSORS_H_
#define CARTESIAN_GRID_ELEMENT_ACCESSORS_H_

#include <igatools/base/config.h>
#include <igatools/base/cache_status.h>
#include <igatools/base/value_flags_handler.h>
#include <igatools/base/quadrature.h>
#include <igatools/geometry/topology.h>
#include <igatools/geometry/cartesian_grid_element.h>
#include <igatools/geometry/grid_uniform_quad_cache.h>
#include <igatools/geometry/grid_forward_iterator.h>
#include <igatools/utils/value_vector.h>

IGA_NAMESPACE_OPEN

/**
 * @brief Element accessor for the CartesianGrid.
 *
 * The element can be queried for informations
 * that can be generated on-the-fly
 * (i.e. without the use of a cache) and for informations
 * that are obtained through a cache mechanism
 *
 *
 * See module (and the submodules) on \ref accessors_iterators for a general overview.
 * @ingroup accessors
 *
 * @author S.Pauletti, 2012, 2013, 2014
 * @author M.Martinelli, 2013, 2014
 */
template <int dim_>
class CartesianGridElementAccessor :
    public CartesianGridElement<dim_>
{
private:
    using parent_t = CartesianGridElement<dim_>;

public:
    using typename parent_t::ContainerType;
    using parent_t::dim;

    /** Fill flags supported by this iterator */
    static const ValueFlags admisible_flag =
        ValueFlags::point|
        ValueFlags::measure |
        ValueFlags::w_measure |
        ValueFlags::face_point |
        ValueFlags::face_measure |
        ValueFlags::face_w_measure |
        ValueFlags::face_normal;

    /** Number of faces of the element. */
    static const Size n_faces = UnitElement<dim_>::faces_per_element;

public:
    /** @name Constructors */
    ///@{
    /**
     * Default constructor.
     */
    CartesianGridElementAccessor() = default;

    /**
     * Construct an accessor pointing to the element with
     * flat index @p elem_index of the CartesianGrid @p grid.
     */
    CartesianGridElementAccessor(const std::shared_ptr<ContainerType> grid,
                                 const Index elem_index);

    CartesianGridElementAccessor(const std::shared_ptr<ContainerType> grid,
                                 const TensorIndex<dim> elem_index);

    /**
     * Copy constructor.
     * @note For the constructed object it
     * creates a new element cache, but it shares
     * the one dimensional cache with the copied element.
     */
    CartesianGridElementAccessor(const CartesianGridElementAccessor<dim_> &elem)
        = default;

    /**
     * Move constructor.
     */
    CartesianGridElementAccessor(CartesianGridElementAccessor<dim_> &&elem)
        = default;

    /**
     * Destructor.
     */
    ~CartesianGridElementAccessor() = default;
    ///@}

    /** @name Assignment operators */
    ///@{
    /**
     * Copy assignment operator.
     * Creates a new element cache, but it shares
     * the one dimensional length cache with the copied element.
     */
    CartesianGridElementAccessor<dim_>
    &operator=(const CartesianGridElementAccessor<dim_> &elem)
    {
        parent_t::operator=(elem);
        return *this;
    }

    /**
     * Move assignment operator.
     */
    CartesianGridElementAccessor<dim_>
    &operator=(CartesianGridElementAccessor<dim_> &&elem) = default;
    ///@}

    /** @name Functions for the cache initialization and filling. */
    ///@{
    /**
     * Initializes the internal cache for the efficient
     * computation of the values requested in
     * the @p fill_flag at the given quadrature points.
     *
     * For the face values, it allows the reuse of the element
     * cache, i.e. it is like using a projected quadrature on
     * the faces.
     */
    //  void init_cache(const ValueFlags flag,
    //                  const Quadrature<dim_> &quad);

    /**
     * Initializes the internal cache for the efficient
     * computation of the values requested in
     * the @p fill_flag when no quadrature point is necessary
     */
//   void init_cache(const ValueFlags flag);

    /**
     * To use a different quadrature on the face instead of
     * the projected element quadrature
     */
//    void init_face_cache(const Index face_id,
//                         const ValueFlags flag,
//                         const Quadrature<dim_-1> &quad);

    /**
     * Fills the element values cache according to the evaluation points
     * and fill flags specifies in init_values.
     */
    //  void fill_cache(const TopologyId<dim_> &topology_id = ElemTopology<dim_>());

    /**
     * Fills the i-th face values cache according to the evaluation points
     * and fill flags specified in init_values.
     */
//    void fill_face_cache(const Index face_id);
    ///@}



    ///@name Query information that requires the use of the cache
    ///@{

    /**
     * Returns the lengths of the coordinate sides of the cartesian element.
     * For example in 2 dimensions
     * \code{.cpp}
       auto length = elem.coordinate_lenths();
       // length[0] is the length of the x-side of the element and
       // length[1] the length of the y-side of the element.
       \endcode
     */
    //  std::array<Real,dim_> get_coordinate_lengths() const;


    /**
     * Returns measure of the element or of the element-face in the
     * CartesianGrid.
     * @note The topology for which the measure is computed is specified by
     * the input argument @p topology_id.
     */
    Real get_measure(const TopologyId<dim_> &topology_id
                     = ElemTopology<dim_>()) const;


    /**
     * Returns measure of j-th face.
     */
    Real get_face_measure(const int j) const;



    /**
     * Returns the element measure multiplied by the weights of the quadrature
     * scheme used to initialize the accessor's cache.
     */
    ValueVector<Real> const &get_w_measures(const TopologyId<dim_> &topology_id
                                            = ElemTopology<dim_>()) const;

    /**
     * Returns the element-face measure multiplied by the weights of the
     * quadrature scheme
     * used to initialize the accessor's cache.
     * The face is specified by the input argument @p face_id
     */
    ValueVector<Real> const &get_face_w_measures(const Index face_id) const;


    /**
     * Return a const reference to the one-dimensional container with the
     * values of the map at the evaluation points.
     */
    ValueVector<Points<dim>> const get_points(const TopologyId<dim_> &topology_id
                                              = ElemTopology<dim_>()) const;

    /**
     * Return a const reference to the one-dimensional container with the
     * values of the map at the evaluation points on the face specified
     * by @p face_id.
     */
    ValueVector<Points<dim>> const get_face_points(const Index face_id) const;

    ///@}



    /**
     * Prints internal information about the CartesianGridElementAccessor.
     * Its main use is for testing and debugging.
     */
    void print_info(LogStream &out, const VerbosityLevel verbosity =
                        VerbosityLevel::normal) const;

    void print_cache_info(LogStream &out) const;

private:

//    /**
//     * @brief Global CartesianGrid cache, storing the interval length
//     * in each direction.
//     *
//     * For now only a uniform quad is taken care of.
//     */
//    class LengthCache : public CacheStatus
//    {
//    public:
//        /**
//         * Allocates space for the cache
//         */
//        void resize(const CartesianGrid<dim_> &grid);
//
//        /** pointer to the current entry of of length,
//         *  it could be used for optimization of uniform grid
//         */
//        CartesianProductArray<Real *, dim_> length_;
//
//        /** stores the interval length */
//        CartesianProductArray<Real , dim_> length_data_;
//
//    };


    /**
     * @brief Base class for cache of CartesianGridElementAccessor.
     */
    class ValuesCache : public CacheStatus
    {
    public:
        /**
         * Allocate space for the values at quadrature points
         */
        void resize(const GridElemValueFlagsHandler &flags_handler,
                    const Quadrature<dim_> &quad);

        /**
         * Fill the cache member.
         * @note The @p measure is an input argument because of the different
         * function calls
         * between element-measure and face-measure.
         */
        void fill(const Real measure);

        void print_info(LogStream &out) const
        {
            out.begin_item("Fill flags:");
            flags_handler_.print_info(out);
            out.end_item();

            out << "Measure: " << measure_ << std::endl;

            out.begin_item("Weigthed measure:");
            w_measure_.print_info(out);
            out.end_item();

            out.begin_item("Unit weights:");
            unit_weights_.print_info(out);
            out.end_item();

            out.begin_item("Unit points:");
            unit_points_.print_info(out);
            out.end_item();
        }


//TODO(pauletti, Sep 6, 2014): should the next be private?

        GridElemValueFlagsHandler flags_handler_;

        ///@name The "cache" properly speaking
        ///@{
        Real measure_;

        /** Element measure multiplied by the quadrature weights. */
        ValueVector<Real> w_measure_;

        TensorProductArray<dim_> unit_points_;

        ValueVector<Real> unit_weights_;
        ///@}

    };

    /**
     * @brief Cache for the element values at quadrature points
     */
    class ElementValuesCache : public ValuesCache
    {
    public:
        /**
         * Allocate space for the values at quadrature points
         */
        void resize(const GridElemValueFlagsHandler &flags_handler,
                    const Quadrature<dim_> &quad);
    };

    /**
     * @brief Cache for the face values at quadrature points
     */
    class FaceValuesCache : public ValuesCache
    {
    public:
        void resize(const GridFaceValueFlagsHandler &flags_handler,
                    const Quadrature<dim_> &quad,
                    const Index face_id);

        void resize(const GridFaceValueFlagsHandler &flags_handler,
                    const Quadrature<dim_-1> &quad,
                    const Index face_id);
    };

    /**
     * @todo Document this function
     */
    const ValuesCache &get_values_cache(const TopologyId<dim_> &topology_id = ElemTopology<dim_>())
    const;

    /**
     * @todo Document this function
     */
    ValuesCache &get_values_cache(const TopologyId<dim_> &topology_id = ElemTopology<dim_>());



    /** Element values cache */
    ElementValuesCache elem_values_;

    /** Face values cache */
    std::array<FaceValuesCache, n_faces> face_values_;

private:
    template <typename Accessor> friend class GridForwardIterator;
    friend class GridUniformQuadCache<dim>;

protected:
    /**
     * ExceptionUnsupported Value Flag.
     */
    DeclException2(ExcFillFlagNotSupported, ValueFlags, ValueFlags,
                   << "The passed ValueFlag " << arg2
                   << " contains a non admissible flag " << (arg1 ^arg2));

    DeclException1(ExcCacheInUse, int,
                   << "The global cache is being used by " << arg1
                   << " iterator. Changing its value not allowed.");
};

IGA_NAMESPACE_CLOSE

#endif /* CARTESIAN_GRID_ELEMENT_ACCESSORS_H_ */
