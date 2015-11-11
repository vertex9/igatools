#-+--------------------------------------------------------------------
# Igatools a general purpose Isogeometric analysis library.
# Copyright (C) 2012-2015  by the igatools authors (see authors.txt).
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

# QA (pauletti, Mar 19, 2014):
from init_instantiation_data import *

include_files = ['basis_functions/bspline.h',
                 'basis_functions/nurbs.h',
                 'geometry/grid_element.h',
#                 'geometry/mapping_element.h',
                 'geometry/push_forward.h',
                 'basis_functions/bspline_element.h',
                 'basis_functions/physical_space_element.h']

data = Instantiation(include_files)
(f, inst) = (data.file_output, data.inst)


sub_dim_members = \
 ['std::shared_ptr<const typename class::SubSpace<k> > ' +
   'class::get_sub_space(const int s_id, InterSpaceMap<k> &dof_map, ' +
   'const std::shared_ptr<const Grid<k>> &sub_grid, ' + 
   'SubGridMap<k> &elem_map, void *) const;']


spaces = ['PhysicalSpaceBasis<0,0,1,0>']
templated_funcs = []

for sp in inst.SubPhysSpaces:
    x = sp.spec
#    f.write( 'template class %s;\n' %sp.name)
    spaces.append(sp.name)
    for fun in sub_dim_members:
        for k in range(0,max(x.dim-1,0)+1):
            if ((x.dim != 0) and (k>=0)):
                s = fun.replace('class', sp.name).replace('k', '%d' % (k));
                templated_funcs.append(s)
#        f.write('template ' + s + '\n')


for sp in inst.PhysSpaces:
    x = sp.spec
#    f.write( 'template class %s;\n' %sp.name)
    spaces.append(sp.name)
    for fun in sub_dim_members:
#         for k in inst.sub_dims(x.dim):
        for k in range(0,max(x.dim-1,0)+1):
            if ((x.dim != 0) and (k>=0)):
                s = fun.replace('class', sp.name).replace('k', '%d' % (k));
                templated_funcs.append(s)
#            f.write('template ' + s + '\n')



for space in unique(spaces):
    f.write( 'template class %s;\n' %space)

for func in unique(templated_funcs):
    f.write('template ' + func + '\n')

