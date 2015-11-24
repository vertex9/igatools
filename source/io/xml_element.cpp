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

#include <igatools/io/xml_element.h>

#ifdef XML_IO

#include <igatools/utils/safe_stl_vector.h>

#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMText.hpp>
//#include <xercesc/dom/DOMAttr.hpp>

using std::string;
using std::shared_ptr;
using std::unique_ptr;
using xercesc::XMLString;
using xercesc::DOMElement;
using xercesc::DOMNode;
using xercesc::DOMNodeList;
using xercesc::DOMText;

IGA_NAMESPACE_OPEN


XMLElement::
XMLElement(const DOMElemPtr_ dom_elem)
    :
    root_elem_ (dom_elem)
{
    Assert (root_elem_ != nullptr, ExcNullPtr());
}



auto
XMLElement::
create(const DOMElemPtr_ dom_elem) ->
SelfPtr_
{
    return SelfPtr_ (new XMLElement (dom_elem));
}



auto
XMLElement::
get_children_elements() const ->
SafeSTLVector<SelfPtr_>
{
    DOMNodeList *elems = root_elem_->getChildNodes();

    const Size n_children = elems->getLength();

    SafeSTLVector<SelfPtr_> children;
    for (int i = 0; i < n_children; ++i)
    {
        DOMNode *n = elems->item(i);

        if (n->getNodeType() && // true is not NULL
            n->getNodeType() == DOMNode::ELEMENT_NODE)  // is element
        {
            const auto elem_ptr = dynamic_cast<DOMElemPtr_>(n);
            Assert (elem_ptr != nullptr, ExcNullPtr());
            children.push_back(Self_::create(elem_ptr));
        }
    }

    return children;
}



auto
XMLElement::
get_children_elements(const string &name) const ->
SafeSTLVector<SelfPtr_>
{
    SafeSTLVector<SelfPtr_> children;

    DOMNodeList *elems = root_elem_->getChildNodes();
    const Size n_children = elems->getLength();

    for (int i = 0; i < n_children; ++i)
    {
        DOMNode *n = elems->item(i);
        if (n->getNodeType() && // true is not NULL
            XMLString::transcode(n->getNodeName()) == name &&
            n->getNodeType() == DOMNode::ELEMENT_NODE)  // is element
        {
            const auto elem_ptr = dynamic_cast<DOMElemPtr_>(n);
            Assert (elem_ptr != nullptr, ExcNullPtr());
            children.push_back(Self_::create(elem_ptr));
        }
    }

    return children;
}



string
XMLElement::
get_name() const
{
  return XMLString::transcode(root_elem_->getNodeName());
}



bool
XMLElement::
has_attribute(const string &name) const
{
  return root_elem_->hasAttribute(XMLString::transcode(name.c_str()));
}



bool
XMLElement::
has_element(const string &name) const
{
  for (const auto &el : this->get_children_elements())
  {
    const auto el_name = el->get_name();
    if (el_name == name)
      return true;
  }
  return false;
}



DOMText *
XMLElement::
get_single_text_element() const
{
    DOMNodeList *elems = root_elem_->getChildNodes();
    const Size n_children = elems->getLength();

    DOMText *element;

    for (int i = 0; i < n_children; ++i)
    {
        DOMNode *n = elems->item(i);

        if (n->getNodeType() && // true is not NULL
            n->getNodeType() == DOMNode::TEXT_NODE)  // is element
        {
            element = dynamic_cast<DOMText *>(n);
            break;
        }
    }

  // if there is more than, or less than, one element, an error is thrown.
#ifndef NDEBUG
    Size n_elems = 0;
    for (int i = 0; i < n_children; ++i)
    {
        DOMNode *n = elems->item(i);

        if (n->getNodeType() && // true is not NULL
            n->getNodeType() == DOMNode::TEXT_NODE) // is element
            ++n_elems;
    }
    Assert(n_elems == 1, ExcDimensionMismatch(n_elems, 1));
#endif

    return element;
}



template <class T>
T
XMLElement::
get_value () const
{
  try
  {
    const auto text_elem = this->get_single_text_element();
    return XMLString::transcode(text_elem->getWholeText());
  }
  catch (...)
  {
    AssertThrow(false, ExcMessage("Error parsing text element."));
    return "Error"; // For avoiding the warning.
  }
}



template <>
Index
XMLElement::
get_attribute<Index> (const string &name) const
{
  Assert(this->has_attribute(name), ExcMessage("Attribute not present."));
  const string str = XMLString::transcode(
          root_elem_->getAttribute(XMLString::transcode(name.c_str())));

  try
  {
    return std::stoul(str);
  }
  catch (...)
  {
    AssertThrow(false, ExcMessage("Value has not type Index."));
    return 0;
  }
}



template <>
Real
XMLElement::
get_attribute<Real> (const string &name) const
{
  Assert(this->has_attribute(name), ExcMessage("Attribute not present."));
  const string str = XMLString::transcode(
          root_elem_->getAttribute(XMLString::transcode(name.c_str())));
  try
  {
    return std::stod(str);
  }
  catch (...)
  {
    AssertThrow(false, ExcMessage("Value has not type Real."));
    return 0.0;
  }
}



template <>
string
XMLElement::
get_attribute<string> (const string &name) const
{
  Assert(this->has_attribute(name), ExcMessage("Attribute not present."));
  return XMLString::transcode(
          root_elem_->getAttribute(XMLString::transcode(name.c_str())));
}



template <>
bool
XMLElement::
get_attribute<bool> (const string &name) const
{
  Assert(this->has_attribute(name), ExcMessage("Attribute not present."));
  string str = XMLString::transcode(
          root_elem_->getAttribute(XMLString::transcode(name.c_str())));

  try
  {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::istringstream is(str);
    bool b;
    is >> std::boolalpha >> b;
    return b;
  }
  catch (...)
  {
    AssertThrow(false, ExcMessage("Value has not type boolean."));
    return false;
  }
}



template <class T>
SafeSTLVector<T>
XMLElement::
get_values_vector() const
{
  SafeSTLVector<T> data;
  const auto text_elem = this->get_single_text_element();

  const string str = XMLString::transcode(text_elem->getWholeText());

  try
  {
    T v;
    std::stringstream line_stream(str);
    while (line_stream >> v)
      data.push_back(v);

  }
  catch (...)
  {
    AssertThrow(false, ExcMessage("Impossible to parse vector."));
  }

  return data;
}



auto
XMLElement::
get_single_element() -> SelfPtr_
{
  DOMNodeList *children = root_elem_->getChildNodes();
  const Size n_children = children->getLength();

  SelfPtr_ element;

  for (int i = 0; i < n_children; ++i)
  {
    DOMNode *n = children->item(i);

    if (n->getNodeType() && // true is not NULL
        n->getNodeType() == DOMNode::ELEMENT_NODE)  // is element
    {
      element = Self_::create(dynamic_cast<DOMElemPtr_>(n));
      break;
    }
  }

  // if there is more than one element, an error is thrown.
#ifndef NDEBUG
  Size n_children_elems = 0;
  for (int i = 0; i < n_children; ++i)
  {
    auto *n = children->item(i);

    if (n->getNodeType() && // true is not NULL
        n->getNodeType() == DOMNode::ELEMENT_NODE)  // is element
      ++n_children_elems;
  }
  Assert(n_children_elems == 1, ExcDimensionMismatch(n_children_elems, 1));
  Assert(element != nullptr, ExcNullPtr());
#endif

  return element;
}



auto
XMLElement::
get_single_element(const string &name) -> SelfPtr_
{
  // Getting all the elements with the given name.
  auto *children = root_elem_->getChildNodes();
  const Size n_children = children->getLength();

  Size n_matching_childs = 0;
  SelfPtr_ element;
  for (int c = 0; c < n_children; ++c)
  {
      auto *elem = dynamic_cast<DOMElemPtr_>(children->item(c));
      if (elem != nullptr && XMLString::transcode(elem->getNodeName()) == name)
      {
          element = Self_::create(elem);
          ++n_matching_childs;
      }
  }

  Assert(n_matching_childs == 1, ExcDimensionMismatch(n_matching_childs, 1));
  Assert(element != nullptr, ExcNullPtr());

  return element;
}


template SafeSTLVector<Real> XMLElement::get_values_vector<Real>() const;
template SafeSTLVector<Index> XMLElement::get_values_vector<Index>() const;

IGA_NAMESPACE_CLOSE

#endif // XML_IO
