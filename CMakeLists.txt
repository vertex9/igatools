#-+--------------------------------------------------------------------
# Igatools a general purpose Isogeometric analysis library.
# Copyright (C) 2012-2016  by the igatools authors (see authors.txt).
#
# This file is part of the igatools library.
#
# The igatools library is free software: you can use it, redistribute
# it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#-+--------------------------------------------------------------------

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Main cmake file for the igatools library
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#TODO: Define the configure time variable for dimension compilation
#TODO: upadate to cmake >= 3

cmake_minimum_required(VERSION 3.1)

project(iga CXX C)

list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Include all local macros
file(GLOB_RECURSE macro_list ${PROJECT_SOURCE_DIR}/cmake/*.cmake)
foreach(item ${macro_list})
  include (${item})
endforeach(item)
#
#-------------------------------------------------------------------------------

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Make sure the build directory is not the source or
# the install directory
#
macro_ensure_out_of_source_build()
macro_ensure_out_of_build_install()
#
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# If the prefix is not provided by the user but the
# enviroment variable IGATOOLS_PREFIX exists then we use it
# instead of the default prefix
if((DEFINED ENV{IGATOOLS_PREFIX}) AND (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT))
set(CMAKE_INSTALL_PREFIX 
   $ENV{IGATOOLS_PREFIX} CACHE PATH "Installation prefix" FORCE)
endif()
#
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Set the igatools version 
#
set(IGATOOLS_MAJOR_VERSION 1)
set(IGATOOLS_MINOR_VERSION 99)
set(IGATOOLS_PATCH_VERSION 0)
set(IGATOOLS_VERSION
  ${IGATOOLS_MAJOR_VERSION}.${IGATOOLS_MINOR_VERSION}.${IGATOOLS_PATCH_VERSION})
#
#-------------------------------------------------------------------------------



#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Find Required programs
#
find_sed()
find_python()
find_doxygen()
#
#-------------------------------------------------------------------------------



#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Optional features for building  igatools
#

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(USE_DEPRECATED "Use the part of the igatools code marked as deprecated" OFF)

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(IGATOOLS_USES_TRILINOS "Use the linear algebra packages from Trilinos" ON)
if (IGATOOLS_USES_TRILINOS)
    message("-- TRILINOS support is enabled.")
else(IGATOOLS_USES_TRILINOS)
    message("-- TRILINOS support is not enabled.")    
endif(IGATOOLS_USES_TRILINOS)
#-------------------------------------------------------------------------------




#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(IGATOOLS_WITH_NURBS "Enable NURBS support" ON)
if (IGATOOLS_WITH_NURBS)
    message("-- NURBS support is enabled.")
else(IGATOOLS_WITH_NURBS)
    message("-- NURBS support is not enabled.")    
endif(IGATOOLS_WITH_NURBS)
#-------------------------------------------------------------------------------



#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(REAL_IS_LONG_DOUBLE "Enable quadruple precision" OFF)
if (REAL_IS_LONG_DOUBLE)
    message("-- Quadruple precision support (EXPERIMENTAL) is enabled.")
else(REAL_IS_LONG_DOUBLE)
    message("-- Quadruple precision support (EXPERIMENTAL) is not enabled.")    
endif(REAL_IS_LONG_DOUBLE)
#-------------------------------------------------------------------------------



#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(MPATCH "Enable multipatch features of the library (experimental)" OFF)
if (MPATCH)
    message("-- Multipatch features (EXPERIMENTAL) are enabled.")
else(MPATCH)
    message("-- Multipatch features (EXPERIMENTAL) are not enabled.")    
endif(MPATCH)
#-------------------------------------------------------------------------------



#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(HIERARCHICAL "Enable Hierarchical splines support (experimental)" OFF)
if (HIERARCHICAL)
    message("-- Hierarchical splines support (EXPERIMENTAL) is enabled.")
else(HIERARCHICAL)
    message("-- Hierarchical splines support (EXPERIMENTAL) is not enabled.")    
endif(HIERARCHICAL)
#-------------------------------------------------------------------------------



#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(IGATOOLS_WITH_MESH_REFINEMENT "Enable mesh refinement support" ON)
if (IGATOOLS_WITH_MESH_REFINEMENT)
    if (NOT IGATOOLS_USES_TRILINOS)
        message (FATAL_ERROR "IGATOOLS_WITH_MESH_REFINEMENT variable set to ON is not "
        "compatible with IGATOOLS_USES_TRILINOS set to OFF. Trilinos support must be "
        "activated.")
    endif ()

    message("-- Mesh refinement support (EXPERIMENTAL) is enabled.")
else(IGATOOLS_WITH_MESH_REFINEMENT)
    message("-- Mesh refinement support (EXPERIMENTAL) is not enabled.")    
endif(IGATOOLS_WITH_MESH_REFINEMENT)
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(IGATOOLS_WITH_SERIALIZATION "Enable serialization support" OFF)
if (IGATOOLS_WITH_SERIALIZATION)
    message("-- Serialization support (EXPERIMENTAL) with CEREAL library is enabled.")    
else(IGATOOLS_WITH_SERIALIZATION)
    message("-- Serialization support (EXPERIMENTAL) is not enabled.")    
endif(IGATOOLS_WITH_SERIALIZATION)
#-------------------------------------------------------------------------------



#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(IGATOOLS_WITH_PARAVIEW_PLUGIN "Enable the building of the ParaView plugin" OFF)
if (IGATOOLS_WITH_PARAVIEW_PLUGIN)
    if (NOT IGATOOLS_WITH_SERIALIZATION)
        message (FATAL_ERROR "IGATOOLS_WITH_PARAVIEW_PLUGIN variable set to ON is not "
        "compatible with IGATOOLS_WITH_SERIALIZATION set to OFF. Serialization must be "
        "activated.")
    endif ()
    message("-- Paraview plugin building (EXPERIMENTAL) is enabled.")
else(IGATOOLS_WITH_PARAVIEW_PLUGIN)
    message("-- Paraview plugin building (EXPERIMENTAL) is not enabled.")
endif(IGATOOLS_WITH_PARAVIEW_PLUGIN)
#-------------------------------------------------------------------------------



#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(IGATOOLS_WITH_XML_IO "Enable XML I/O support" OFF)
if (IGATOOLS_WITH_XML_IO)
		find_xercesc()
    if (XERCESC_FOUND)
      message("-- XML I/O support (EXPERIMENTAL) is enabled.")
    else(XERCESC_FOUND)
      message(FATAL_ERROR "XML I/O support (EXPERIMENTAL) is enabled but Xerces-C is NOT FOUND.")
    endif(XERCESC_FOUND)
else(IGATOOLS_WITH_XML_IO)
    message("-- XML I/O support (EXPERIMENTAL) is not enabled.")    
endif(IGATOOLS_WITH_XML_IO)
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(USE_VTK "Enable VTK support" OFF)
if (USE_VTK)
		find_package(VTK REQUIRED)
    if (VTK_FOUND)
      include(${VTK_USE_FILE})
      message("-- VTK support (EXPERIMENTAL) is enabled.")
    else(VTK_FOUND)
      message(FATAL_ERROR "VTK support (EXPERIMENTAL) is enabled but the VTK library is NOT FOUND.")
    endif(VTK_FOUND)
else(USE_VTK)
    message("-- VTK support (EXPERIMENTAL) is not enabled.")    
endif(USE_VTK)
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
option(USE_CGAL "Enable CGAL support" OFF)
if (USE_CGAL)
		find_cgal()
    if (CGAL_FOUND)
#      include(${CGAL_USE_FILE})
      message("-- CGAL support (EXPERIMENTAL) is enabled.")
    else(CGAL_FOUND)
      message(FATAL_ERROR "CGAL support (EXPERIMENTAL) is enabled but the CGAL library is NOT FOUND.")
    endif(CGAL_FOUND)
else(USE_CGAL)
    message("-- CGAL support (EXPERIMENTAL) is not enabled.")    
endif(USE_CGAL)
#-------------------------------------------------------------------------------


option(use_other_templates "Enable a mechanism to use uninstantiated templates" ON)
option(submit_test "Allow test submition" OFF)

#
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Find Required libraries
#
find_boost()
#
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Find optional libraries

if (IGATOOLS_USES_TRILINOS)
  find_trilinos()
endif()

#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Compiler and compile flags

if(CMAKE_BUILD_TYPE MATCHES Release)
    message("-- IGATOOLS build type: RELEASE")
else ()
    message("-- IGATOOLS build type: DEBUG")
endif()

init_cxx_flags()
#
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#  bases dimensions and ranks to compile the library for
#
set(dim   CACHE STRINGS "list of domain dimensions")
set(codim CACHE STRINGS "list of domain codimension")
set(range CACHE STRINGS "list of reference basis range dim")
set(rank  CACHE STRINGS "list of reference basis rank")
set(trans_type  CACHE STRINGS "list of transformation type")
set(max_der_order 2 CACHE STRINGS "maximum order of derivatives")
set(num_sub_elem 1 CACHE STRINGS "number of sub element to consider 1 for faces")
#
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Configuration of the library 
#
set(iga_lib_name igatools)

# igatools source files
#set (source_dirs base utils geometry basis_functions linear_algebra io contrib functions)
set (source_dirs base utils geometry basis_functions linear_algebra functions io operators)
#set (source_dirs base utils geometry linear_algebra functions)
foreach(dir ${source_dirs})
  file(GLOB source ${PROJECT_SOURCE_DIR}/source/${dir}/*.cpp)
  list(APPEND source_files ${source})
endforeach()

# igatools include directories
if (IGATOOLS_WITH_SERIALIZATION)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                       ${CMAKE_CURRENT_SOURCE_DIR}/include/igatools/contrib/cereal/include 
                       ${CMAKE_CURRENT_BINARY_DIR}/include
                       ${CMAKE_CURRENT_BINARY_DIR}/include/igatools/contrib/cereal/include )
else (IGATOOLS_WITH_SERIALIZATION)                    
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include 
                       ${CMAKE_CURRENT_BINARY_DIR}/include)
endif(IGATOOLS_WITH_SERIALIZATION)

# the igatools library target
add_library(${iga_lib_name} SHARED  ${source_files})
target_link_libraries(${iga_lib_name} 
  ${Trilinos_TPL_LIBRARIES} 
  ${Trilinos_LIBRARIES} 
  ${Petsc_LIBRARIES} 
  ${XERCESC_LIBRARIES}
  ${Boost_LIBRARIES}
  ${VTK_LIBRARIES}
  ${CGAL_LIBRARIES})
set_property(TARGET ${iga_lib_name} PROPERTY VERSION ${IGATOOLS_VERSION})


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Generate configure dependent source files (in the build directory)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/${iga_lib_name}/base/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/${iga_lib_name}/base/config.h")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/${iga_lib_name}Config.cmake.in 
  ${CMAKE_CURRENT_BINARY_DIR}/${iga_lib_name}Config.cmake @ONLY)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/${iga_lib_name}ConfigVersion.cmake.in 
  ${CMAKE_CURRENT_BINARY_DIR}/${iga_lib_name}ConfigVersion.cmake @ONLY)


# Create the instantiation files
message("Configuring instantiations")
create_basis_table()
generate_instantiations()
generate_serial()
#
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Create install target
create_install_target(${use_other_templates})
#
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Create target to generate documentation
create_doc_target()
#
#-------------------------------------------------------------------------------

## #+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
## # ParaView plugin section
## if (IGATOOLS_WITH_PARAVIEW_PLUGIN)
##   add_subdirectory(${PROJECT_SOURCE_DIR}/addons/paraview_plugin)
## endif()
## #
## #-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Tutorial example section
add_subdirectory(${PROJECT_SOURCE_DIR}/tutorial)
#
#-------------------------------------------------------------------------------


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# Unit Test configuration
add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
#
#-------------------------------------------------------------------------------


print_final_message()
