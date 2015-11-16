// headers for spline spaces
#include <igatools/basis_functions/bspline.h>
#include <igatools/basis_functions/space_tools.h>
#include <igatools/basis_functions/bspline_element.h>
#include <igatools/basis_functions/bspline_element_handler.h>
// headers for quadrature
#include <igatools/base/quadrature_lib.h>
// headers for linear algebra objects
#include <igatools/linear_algebra/dense_matrix.h>
#include <igatools/linear_algebra/dense_vector.h>
// headers for system solving
//#include <igatools/linear_algebra/epetra_solver.h>
#include <igatools/linear_algebra/dof_tools.h>
// headers for function representations
#include <igatools/functions/function_lib.h>
#include <igatools/geometry/grid_function_lib.h>
// headers for output
#include <igatools/io/writer.h>
#include <igatools/base/logstream.h>
// finally! my stuff!
//#include "my_formula_grid_function.h"
#include "custom_grid_function.h"

// headers for Trilinos stuff
#include <Teuchos_GlobalMPISession.hpp>
#include <Tpetra_DefaultPlatform.hpp>
#include <Tpetra_Version.hpp>

using namespace iga;
using namespace std;
using namespace EpetraTools;
LogStream out;

#include "poisson_problem.h"

// ----------------------------------------------------------------------------
//   MY CUSTOM FUNCTION
// ----------------------------------------------------------------------------
#define PI 3.141592653589793
template<int dim> // exact solution
Values<dim,1,1> exact_solution(Points<dim> pts) {
  Values<dim,1,1> x = {1.0};
  for (int idim=0; idim<dim; idim++) {
    x *= sin( 4.0 * pts[idim] * PI);
  }
  return x;
}

template<int dim> // source term
Values<dim,1,1> source_term(Points<dim> pts) {
  Values<dim,1,1> x = {16.0 * dim * PI * PI};
  for (int idim=0; idim<dim; idim++) {
    x *= sin(4.0 * pts[idim] * PI);
  }
  return x;
}

// ----------------------------------------------------------------------------
//   MAIN
// ----------------------------------------------------------------------------
int main() {

  // problem input
  const int dim  = 2;
  const int nel  = 16;
  const int deg  = 2;
  // problem output
  int it;
  double cond, cond2;
  using CustomFunct = grid_functions::CustomGridFunction<dim,1>;
  
  Real err1, err2, eoc, h1, h2;
  for (int ideg=2; ideg<=2; ideg++) { printf("\n");
  for (int iel=4; iel<=nel; iel+=4) {
    auto problem = PoissonProblem<dim>::create(iel,ideg);
    auto f = CustomFunct::const_create(problem->grid,&source_term);
    problem->assemble(f);
    problem->custom_solve(it,cond,cond2);
    auto u = CustomFunct::const_create(problem->grid,&exact_solution);
    err2 = problem->l2_error(u);
    h2   = (double)1.0/iel;
    printf("deg = %d, nel = %2d:\t l2_err = %le",ideg,iel,err2);
    if (iel==4)
      printf("  eoc = -.-");
    else {
      eoc = (log(err2)-log(err1))/(log(h2)-log(h1));
      printf("  eoc = %1.5f",eoc);
    }
    //printf("\t it = %2d\tcond = %3.2f\tcond2 = %3.2f\n",it,cond,cond2);
    printf("\t it = %2d\tcond = %3.2f\n",it,cond);
    err1 = err2;
    h1 = h2;
  }
  }

  /*auto problem = PoissonProblem<dim>::create(nel,deg);
  auto f = CustomFunct::const_create(problem->grid,&source_term);
  problem->assemble(f);
  problem->how_are_you_doin();
  problem->custom_solve(it,cond);
  auto u = CustomFunct::const_create(problem->grid,&exact_solution);
  cout << "  --------------------------" << endl;
  cout << "        L2-error: " << problem->l2_error(u) << endl;
  cout << "      iterations: " << it << endl;
  cout << "condition number: " << cond << endl;
  cout << endl;

  auto solution = IgGridFunction<dim,1>::const_create(problem->basis,*problem->sol);
  const int npt = 10;
  Writer<dim> output(problem->grid,npt);
  output.template add_field(*solution,"solution");
  output.template add_field(*u,"exact solution");
  output.template add_field(*f,"source term");
  output.save("plot");*/

  return 0;
}
