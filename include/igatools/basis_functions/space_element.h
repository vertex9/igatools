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


#ifndef SPACE_ELEMENT_H_
#define SPACE_ELEMENT_H_

#include <igatools/base/config.h>
#include <igatools/basis_functions/values_cache.h>

#include <igatools/base/quadrature.h>

#include <igatools/utils/static_multi_array.h>
#include <igatools/utils/cartesian_product_indexer.h>

#include <igatools/basis_functions/spline_space.h>

#include <igatools/basis_functions/space.h>
#include <igatools/basis_functions/space_element_base.h>



IGA_NAMESPACE_OPEN



/**
 *
 *
 * @ingroup serializable
 */
template<int dim_,int codim_,int range_,int rank_>
class SpaceElement : public SpaceElementBase<dim_>
{
protected:
    using base_t =  SpaceElementBase<dim_>;


private:
    using self_t = SpaceElement<dim_,codim_,range_,rank_>;

public:

    using Func = Function<dim_,codim_,range_,rank_>;

    using RefPoint = typename Func::RefPoint;
    using Point = typename Func::Point;
    using Value = typename Func::Value;
    template <int order>
    using Derivative = typename Func::template Derivative<order>;
    using Div = typename Func::Div;

    static const int dim = dim_;
    static const int space_dim = Func::space_dim;

    using ContainerType = Space<dim_,codim_,range_,rank_>;

    /**
     * For each component gives a product array of the dimension
     */
    template<class T>
    using ComponentContainer = typename SplineSpace<dim_,range_,rank_>::template ComponentContainer<T>;
    using TensorSizeTable = typename SplineSpace<dim_,range_,rank_>::TensorSizeTable;
    ///@}


    /** @name Constructors */
    ///@{
protected:
    /**
     * Default constructor. It does nothing but it is needed for the
     * <a href="http://www.boost.org/doc/libs/release/libs/serialization/">boost::serialization</a>
     * mechanism.
     */
    SpaceElement() = default;

public:

    /**
     * Constructs an accessor to element number <tt>elem_index</tt> of a
     * function space.
     */
    SpaceElement(const std::shared_ptr<const Space<dim_,codim_,range_,rank_>> space,
                 const Index elem_index);

    /**
     * Copy constructor.
     * It can be used with different copy policies (i.e. deep copy or shallow copy).
     * The default behaviour (i.e. using the proper interface of a classic copy constructor)
     * uses the deep copy.
     */
    SpaceElement(const self_t &elem,
                 const CopyPolicy &copy_policy = CopyPolicy::deep);

    /**
     * Move constructor.
     */
    SpaceElement(self_t &&elem) = default;

    /**
     * Destructor.
     */
    virtual ~SpaceElement() = default;
    ///@}

    /** @name Assignment operators */
    ///@{
    /**
     * Copy assignment operator. Performs a <b>shallow copy</b> of the input @p element.
     *
     * @note Internally it uses the function shallow_copy_from().
     */
    self_t &operator=(const self_t &element);

    /**
     * Move assignment operator.
     */
    self_t &operator=(self_t &&elem) = default;
    ///@}

#if 0
    /**
     * @name Functions for performing different kind of copy.
     */
    ///@{
    /**
     * Performs a deep copy of the input @p element,
     * i.e. a new local cache is built using the copy constructor on the local cache of @p element.
     *
     * @note In DEBUG mode, an assertion will be raised if the input local cache is not allocated.
     */
    virtual void deep_copy_from(const self_t &element);

    /**
     * Performs a shallow copy of the input @p element. The current object will contain a pointer to the
     * local cache used by the input @p element.
     */
    virtual void shallow_copy_from(const self_t &element);
    ///@}
#endif

    /**
     * Creates a new object performing a deep copy of the current object using
     * the
     * copy constructor of the derived class.
     *
     * @warning This function should be not called directly
     * (if called it raises an assertion in Debug mode),
     * instead it should be called its specialization on a derived class.
     * It would be better to define this function
     * <em>pure virtual</em> but this will not allow to dereference an iterator
     * containing a pointer to an object of kind SpaceElement.
     */
    virtual std::shared_ptr<self_t> clone() const;


    template <class ValueType, int sub_elem_dim = dim_>
    auto
    get_basis(const int sub_elem_id, const std::string &dofs_property = DofProperties::active) const
    {
        const auto &values_all_elem_dofs = this->get_data_from_sub_elem_cache<ValueType,sub_elem_dim>(sub_elem_id);

        //--------------------------------------------------------------------------------------
        // filtering the values that correspond to the dofs with the given property --- begin
        SafeSTLVector<Index> dofs_global;
        SafeSTLVector<Index> dofs_local_to_patch;
        SafeSTLVector<Index> dofs_local_to_elem;

        this->space_->get_element_dofs(
            this->get_flat_index(),
            dofs_global,
            dofs_local_to_patch,
            dofs_local_to_elem,
            dofs_property);

        const auto n_filtered_dofs = dofs_local_to_elem.size();
        const auto n_pts = values_all_elem_dofs.get_num_points();

        using VType = typename std::remove_reference<decltype(values_all_elem_dofs)>::type;
        VType values_filtered_elem_dofs(n_filtered_dofs,n_pts);

        int fn = 0;
        for (const auto loc_dof : dofs_local_to_elem)
        {
            const auto values_all_elem_dofs_fn = values_all_elem_dofs.get_function_view(loc_dof);

            const auto values_filtered_elem_dofs_fn = values_filtered_elem_dofs.get_function_view(fn);

            std::copy(values_all_elem_dofs_fn.begin(),
                      values_all_elem_dofs_fn.end(),
                      values_filtered_elem_dofs_fn.begin());

            ++fn;
        }
        // filtering the values that correspond to the dofs with the given property --- end
        //--------------------------------------------------------------------------------------

        return values_filtered_elem_dofs;
    }



    template <class ValueType>
    auto
    get_basis_element(const std::string &dofs_property = DofProperties::active) const
    {
        return this->template get_basis<ValueType,dim_>(0,dofs_property);
    }

    template <class ValueType, int sub_elem_dim = dim_>
    auto
    linear_combination(const SafeSTLVector<Real> &loc_coefs,
                       const int sub_elem_id,
                       const std::string &dofs_property) const
    {
        const auto &basis_values =
            this->template get_basis<ValueType, sub_elem_dim>(sub_elem_id,dofs_property);
        return basis_values.evaluate_linear_combination(loc_coefs) ;
    }

    /**
     *  Maximum number of non zero basis functions, over the current element.
     *  @note The "true" number of basis functions may differ from this value because
     *  some basis functions may be defined to be "inactive".
     *
     * @warning This function should be not called directly
     * (if called it raises an assertion in Debug mode),
     * instead it should be called its specialization on a derived class.
     * It would be better to define this function
     * <em>pure virtual</em> but this will not allow to dereference an iterator
     * containing a pointer to an object of kind SpaceElement.
     *
     */
    virtual Size get_max_num_basis() const;


    /**
     * @name Functions for the basis evaluations without the use of the cache.
     */
    ///@{
    /**
     * Returns a ValueTable with the quantity specified by the template parameter @p ValueType,
     * computed for all local basis function,
     * at each point (in the unit domain) specified by the input argument <tt>points</tt>.
     * @note This function does not use the cache and therefore can be called any time without
     * needing to pre-call init_cache()/fill_cache().
     * @warning The evaluation <tt>points</tt> must belong to the unit hypercube
     * \f$ [0,1]^{\text{dim}} \f$ otherwise, in Debug mode, an assertion will be raised.
     */
    template <class ValueType>
    auto
    evaluate_basis_at_points(
        const Quadrature<dim_> &points,
        const std::string &dofs_property)
    {
        auto elem_handler = this->space_->get_elem_handler();
        elem_handler->reset_selected_elements(ValueType::flag,points,SafeSTLVector<int>(1,this->get_flat_index()));
        elem_handler->template init_cache<dim_>(*this);
        elem_handler->template fill_cache<dim_>(*this,0);

        return this->template get_basis<ValueType,dim_>(0,dofs_property);
    }

    ///@}




    /**
     * Returns the <tt>k</tt> dimensional j-th sub-element measure
     * multiplied by the weights of the quadrature.
     */
    template <int k>
    ValueVector<Real> get_w_measures(const int j) const;



    virtual void print_info(LogStream &out) const;

    void print_cache_info(LogStream &out) const;

    using CType = boost::fusion::map<
                  boost::fusion::pair<     _Value,DataWithFlagStatus<ValueTable<Value>>>,
                  boost::fusion::pair<  _Gradient,DataWithFlagStatus<ValueTable<Derivative<1>>>>,
                  boost::fusion::pair<   _Hessian,DataWithFlagStatus<ValueTable<Derivative<2>>>>,
                  boost::fusion::pair<_Divergence,DataWithFlagStatus<ValueTable<Div>>>
                  >;

    using Cache = BasisValuesCache<dim_,CType>;

protected:



    /** The local (element and face) cache. */
    std::shared_ptr<AllSubElementsCache<Cache>> all_sub_elems_cache_;

public:
    // TODO (pauletti, Mar 17, 2015): this cannot be public, if needed it means wrong desing
    std::shared_ptr<AllSubElementsCache<Cache> > &
    get_all_sub_elems_cache()
    {
        return this->all_sub_elems_cache_;
    }

private:
    template <class ValueType, int topology_dim>
    const auto &
    get_data_from_sub_elem_cache(const int topology_id) const
    {
        Assert(all_sub_elems_cache_ != nullptr, ExcNullPtr());
        const auto &cache = all_sub_elems_cache_->template get_sub_elem_cache<topology_dim>(topology_id);
        return cache.template get_data<ValueType>();
    }


protected:
    /**
     * Performs a copy of the input @p element.
     * The type of copy (deep or shallow) is specified by the input parameter @p copy_policy.
     */
    void copy_from(const self_t &element,
                   const CopyPolicy &copy_policy);


private:

    std::shared_ptr<const Space<dim_,codim_,range_,rank_>> space_;

#ifdef SERIALIZATION
    /**
     * @name Functions needed for boost::serialization
     * @see <a href="http://www.boost.org/doc/libs/release/libs/serialization/">boost::serialization</a>
     */
    ///@{
    friend class boost::serialization::access;

    template<class Archive>
    void
    serialize(Archive &ar, const unsigned int version);
    ///@}
#endif // SERIALIZATION
};


IGA_NAMESPACE_CLOSE



#endif // #ifndef SPACE_ELEMENT_H_

