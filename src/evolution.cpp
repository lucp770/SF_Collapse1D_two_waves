/* .-----------------------------------------------------------------------.
 * | SFcollapse1D                                                          |
 * | Gravitational collapse of scalar fields in spherical symmetry         |
 * |                                                                       |
 * | Copyright (c) 2020, Leonardo Werneck                                  |
 * |                                                                       |
 * | This program is free software: you can redistribute it and/or modify  |
 * | it under the terms of the GNU General Public License as published by  |
 * | the Free Software Foundation, either version 3 of the License, or     |
 * | (at your option) any later version.                                   |
 * |                                                                       |
 * | This program is distributed in the hope that it will be useful,       |
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of        |
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
 * | GNU General Public License for more details.                          |
 * |                                                                       |
 * | You should have received a copy of the GNU General Public License     |
 * | along with this program.  If not, see <https://www.gnu.org/licenses/>.|
 * .-----------------------------------------------------------------------.
 */

/* Basic includes */
#include <iostream>
#include <fstream>
#include <cmath>
#include "macros.hpp"
#include "grid.hpp"
#include "gridfunction.hpp"
#include "evolution.hpp"
#include "utilities.hpp"

using namespace std;

/* Function to set the initial condition for all gridfunctions: phi, Phi, Pi, a, and alpha */
void evolution::initial_condition( grid::parameters grid, gridfunction &phi, gridfunction &Phi, gridfunction &Pi, gridfunction &a, gridfunction &alpha, real &phi0_2, real &position_2 ) {


  DECLARE_GRID_PARAMETERS;

  LOOP(0,Nx0Total) {

#if( INITIAL_CONDITION == GAUSSIAN_SHELL )

    /* .----------------------------------------------.
     * | phi(r,0) = phi0 * exp( -(r-r0)^2 / delta^2 ) |
     * .----------------------------------------------.
     */

    /* Set some useful auxiliary variables */
    const real r = r_ito_x0[j];
    const real factor = (r-R0)/SQR(DELTA);
    const real expfactor = (r-R0)*factor;
    const real exp_rmr0_over_deltasqrd = exp(-expfactor);

    // configure the second wave
    const real wave2_factor = (r-position_2)/SQR(DELTA2);
    const real wave2_expfactor = (r-position_2)*wave2_factor;
    const real wave2_exp_rmr0_over_deltasqrd = exp(-wave2_expfactor);

    /* Set the initial condition for phi */
    phi.level_nm1[j] = phi0*exp_rmr0_over_deltasqrd + phi0_2*wave2_exp_rmr0_over_deltasqrd;

#elif( INITIAL_CONDITION == GAUSSIAN_SHELL_V2 )

    /* .----------------------------------------------------.
     * | phi(r,0) = phi0 * r^3 * exp( -(r-r0)^2 / delta^2 ) |
     * .----------------------------------------------------.
     */

    /* Set some useful auxiliary variables */
    const real r                       = r_ito_x0[j];
    const real delta_sqrd              = SQR(DELTA);
    const real factor                  = (r-R0)/delta_sqrd;
    const real expfactor               = (r-R0)*factor;
    const real exp_rmr0_over_deltasqrd = exp(-expfactor);

    /* Set the initial condition for phi */
    phi.level_nm1[j] = phi0*r*r*r*exp_rmr0_over_deltasqrd;

#elif( INITIAL_CONDITION == TANH_SHELL )

    /* .---------------------------------------------------------------------.
     * | phi(r,0) = phi0 * ( tanh( (r-r01)/delta ) - tanh( (r-r02)/delta ) ) |
     * .---------------------------------------------------------------------.
     */

    /* Set some useful auxiliary variables */
    const real r                     = r_ito_x0[j];
    const real rmr01_over_delta      = (r - R0_1)/DELTA;
    const real rmr02_over_delta      = (r - R0_2)/DELTA;
    const real tanh_rmr01_over_delta = tanh(rmr01_over_delta);
    const real tanh_rmr02_over_delta = tanh(rmr02_over_delta);

    /* Set the initial condition for phi */
    phi.level_nm1[j] = 0.5 * phi0 * ( tanh_rmr01_over_delta - tanh_rmr02_over_delta );

#elif( INITIAL_CONDITION == TANH_SHELL_V2 )

    /* .------------------------------------------------------.
     * | phi(r,0) = phi0 * ( 1 - tanh( (r-r0)^2 / delta^2 ) ) |
     * .------------------------------------------------------.
     */

    /* Set some useful auxiliary variables */
    const real r                        = r_ito_x0[j];
    const real delta_sqrd               = SQR(DELTA);
    const real factor                   = (r-R0)/delta_sqrd;
    const real tanhfactor               = (r-R0)*factor;
    const real tanh_rmr0_over_deltasqrd = tanh(tanhfactor);

    /* Set the initial condition for phi */
    phi.level_nm1[j] = phi0 * ( 1.0 - tanh_rmr0_over_deltasqrd );

#else
      cerr << "(SFcollapse1D ERROR) Unknown initial condition" << endl;
      exit(1);
#endif

    /* Set the initial condition for Pi */
    Pi.level_nm1[j]  = 0.0;

    if( j>0 ) {

#if( INITIAL_CONDITION == GAUSSIAN_SHELL )
      /* Set the initial condition for Phi */
      // TODO: Mudar a condição para contemplar a segunda onda.
      Phi.level_nm1[j] = -2.0*factor*phi0*exp_rmr0_over_deltasqrd -2.0*wave2_factor*phi0_2*wave2_exp_rmr0_over_deltasqrd;
#elif( INITIAL_CONDITION == GAUSSIAN_SHELL_V2 )
      /* Set the initial condition for Phi */
      Phi.level_nm1[j] = 2.0*phi0*r*exp_rmr0_over_deltasqrd*( delta_sqrd - r*(r-R0) )/delta_sqrd;
#elif( INITIAL_CONDITION == TANH_SHELL )
      /* Set the initial condition for Phi */
      const real cosh_rmr01_over_delta = cosh(rmr01_over_delta);
      const real cosh_rmr02_over_delta = cosh(rmr02_over_delta);
      const real sech_rmr01_over_delta = 1.0/cosh_rmr01_over_delta;
      const real sech_rmr02_over_delta = 1.0/cosh_rmr02_over_delta;
      Phi.level_nm1[j] = 0.5 * phi0 * ( SQR(sech_rmr01_over_delta) - SQR(sech_rmr02_over_delta) ) / DELTA;
#elif( INITIAL_CONDITION == TANH_SHELL_V2 )
      /* Set the initial condition for Phi */
      const real cosh_rmr0_over_deltasqrd = cosh(tanhfactor);
      const real sech_rmr0_over_deltasqrd = 1.0/cosh_rmr0_over_deltasqrd;
      Phi.level_nm1[j] = -2.0*factor*phi0*SQR(sech_rmr0_over_deltasqrd);
#else
      cerr << "(SFcollapse1D ERROR) Unknown initial condition" << endl;
      exit(1);
#endif
      
      /* Compute a */
      a.level_nm1[j] = evolution::pointwise_solution_of_the_Hamiltonian_constraint( j, grid, Phi.level_nm1, Pi.level_nm1, a.level_nm1 );

      /* Compute alpha */
      alpha.level_nm1[j] = evolution::pointwise_solution_of_the_polar_slicing_condition( j, grid, a.level_nm1, alpha.level_nm1 );

    }
    else {
      /* If at inner boundary, impose Phi = 0 */
      Phi.level_nm1[j] = 0.0;
      /* If at inner boundary, impose a = 1 */
      a.level_nm1[j] = 1.0;
      /* If at inner boundary, impose alpha = 1 */
      alpha.level_nm1[j] = 1.0;
    }
  }

  /* Now rescale alpha */
  #if (LAPSE_RESCALING == 1 )
    evolution::rescaling_of_the_lapse(grid,a.level_nm1,alpha.level_nm1);
  #endif
}

/* Function to step phi, Phi, and Pi forward in time */
void evolution::time_step_scalarfield_gridfunctions( const int n, const grid::parameters grid,
						     const realvec phi_n, const realvec Phi_n   , const realvec Pi_n   , const realvec a_n    , const realvec alpha_n  ,
						     const realvec Phi_nm1 , const realvec Pi_nm1 , const realvec a_nm1  , const realvec alpha_nm1, 
						           realvec &Phi_np1,       realvec &Pi_np1,       realvec &phi_np1 ) {

  DECLARE_GRID_PARAMETERS;

  const real Phi_Pi_dt_coeff = ( n>=2 ? 2.0 * dt : ( n==1 ? dt : 0.5 * dt ) );
  const real phi_dt_coeff    = ( n>=1 ? dt :  0.5 * dt );

#pragma omp parallel for
  LOOP(1,Nx0Total-1) {
    /* Compute useful auxiliary variables to all RHSs */
    const real alpha_over_a_jm1      = alpha_n[j-1]/a_n[j-1];
    const real alpha_over_a_j_nm1    = alpha_nm1[j]/a_nm1[j];
    const real alpha_over_a_j_n      = alpha_n[j]  /a_n[j];
    const real alpha_over_a_jp1      = alpha_n[j+1]/a_n[j+1];
    const real alpha_Pi_over_a_jm1   = alpha_over_a_jm1 * Pi_n[j-1];
    const real alpha_Pi_over_a_j_nm1 = alpha_over_a_j_nm1 * Pi_nm1[j];
    const real alpha_Pi_over_a_j_n   = alpha_over_a_j_n * Pi_n[j];
    const real alpha_Pi_over_a_jp1   = alpha_over_a_jp1 * Pi_n[j+1];

    /* Compute the RHS of phi */
    if( j==1 ) {
      const real tmp0    = 1.5 * alpha_n[0]   * Pi_n[0]   / a_n[0];
      const real tmp1    = 0.5 * alpha_nm1[0] * Pi_nm1[0] / a_nm1[0];
      const real rhs_phi = tmp0 - tmp1;
      phi_np1[0]         = phi_n[0] + phi_dt_coeff * rhs_phi;
    }
    const real rhs_phi = 1.5 * alpha_Pi_over_a_j_n - 0.5 * alpha_Pi_over_a_j_nm1;
    
    phi_np1[j] = phi_n[j] + phi_dt_coeff * rhs_phi;

#if( COORD_SYSTEM == SPHERICAL )

    /* Compute useful auxiliary variables to the RHSs of Phi and Pi */
    const real r_sqr_jm1               = SQR(x[0][j-1]);
    const real r_cbd_jm1               = r_sqr_jm1 * x[0][j-1];
    const real r_sqr_jp1               = SQR(x[0][j+1]);
    const real r_cbd_jp1               = r_sqr_jp1 * x[0][j+1];
    const real alpha_Phi_r2_over_a_jm1 = r_sqr_jm1 * alpha_over_a_jm1 * Phi_n[j-1];
    const real alpha_Phi_r2_over_a_jp1 = r_sqr_jp1 * alpha_over_a_jp1 * Phi_n[j+1];

    /* Compute the RHSs of Phi and Pi */
    const real rhs_Phi = 0.5*inv_dx0 * ( alpha_Pi_over_a_jp1 - alpha_Pi_over_a_jm1 );
    const real rhs_Pi  = 3.0 * ( alpha_Phi_r2_over_a_jp1 - alpha_Phi_r2_over_a_jm1 )/( r_cbd_jp1 - r_cbd_jm1 );

#elif( COORD_SYSTEM == SINH_SPHERICAL )

    /* Compute useful auxiliary variables to the RHSs of Phi and Pi */
    const real sinh_x0_inv_W_jm1          = sinh( x[0][j-1] * inv_sinhW );
    const real sinh_x0_inv_W_j            = sinh( x[0][j]   * inv_sinhW );
    const real sinh_x0_inv_W_jp1          = sinh( x[0][j+1] * inv_sinhW );
    const real cosh_x0_inv_W_j            = cosh( x[0][j]   * inv_sinhW );
    const real alpha_Phi_shx02_over_a_jm1 = SQR(sinh_x0_inv_W_jm1) * alpha_over_a_jm1 * Phi_n[j-1];
    const real alpha_Phi_shx02_over_a_jp1 = SQR(sinh_x0_inv_W_jp1) * alpha_over_a_jp1 * Phi_n[j+1];
    const real coefficient = 0.5 * inv_dx0 * ( sinhW * sinh_inv_W / sinhA ) / cosh_x0_inv_W_j;

    /* Compute the RHSs of Phi and Pi */
    const real rhs_Phi = coefficient * ( alpha_Pi_over_a_jp1 - alpha_Pi_over_a_jm1 );
    const real rhs_Pi  = coefficient / SQR(sinh_x0_inv_W_j) * ( alpha_Phi_shx02_over_a_jp1 - alpha_Phi_shx02_over_a_jm1 );

#else
    utilities::SFcollapse1D_error(COORD_SYSTEM_ERROR);
#endif

    Phi_np1[j] = Phi_nm1[j] + Phi_Pi_dt_coeff * rhs_Phi;
    Pi_np1[j]  = Pi_nm1[j]  + Phi_Pi_dt_coeff * rhs_Pi;

  }

}

/* Function to apply outgoing radiation boundary conditions to phi, Phi, and Pi */
void evolution::apply_outgoing_radiation_bdry_cond( const int n, grid::parameters grid,
						    const realvec phi_nm1, const realvec Pi_nm1,
						    const realvec phi_n  , const realvec Phi_n, const realvec a_n, const realvec alpha_n,
						    realvec &phi_np1, realvec &Phi_np1, realvec &Pi_np1 ) {

  DECLARE_GRID_PARAMETERS;

  /* Declare useful auxiliary variables */
  const int J     = Nx0Total - 1;
  const real tmp0 = - phi_n[J] / r_ito_x0[J];
  const real tmp1 = 0.5 * inv_dx0;
  const real tmp2 = - tmp1 * ( 3.0*phi_n[J] - 4.0*phi_n[J-1] + phi_n[J-2] );
  
#if( COORD_SYSTEM == SPHERICAL )

  /* RHS of phi in Spherical coordinates */
  const real rhs_phi = tmp0 + tmp2;
  
#elif( COORD_SYSTEM == SINH_SPHERICAL )

  /* RHS of phi in SinhSpherical coordinates */
  const real tmp3 = ( sinhW / A_over_sinh_inv_W ) / cosh( x[0][J] * inv_sinhW );
  const real rhs_phi = tmp0 + tmp3 * tmp2;
  
#else
  utilities::SFcollapse1D_error(COORD_SYSTEM_ERROR);
#endif

  /* Compute phi at the outer boundary */
  const real phi_Pi_coeff = ( n>=2 ? 2.0 * dt : ( n==1 ? dt : 0.5 * dt ) );
  phi_np1[J] = phi_nm1[J] + phi_Pi_coeff * rhs_phi;

#if( COORD_SYSTEM == SPHERICAL )

  /* RHS of Phi in Spherical coordinates */
  const real rhs_Phi = tmp1 * ( 3.0*phi_np1[J] - 4.0*phi_np1[J-1] + phi_np1[J-2] );
  
#elif( COORD_SYSTEM == SINH_SPHERICAL )

  /* RHS of Phi in SinhSpherical coordinates */
  const real rhs_Phi = tmp1 * tmp3 * ( 3.0*phi_np1[J] - 4.0*phi_np1[J-1] + phi_np1[J-2] );
  
#else
  utilities::SFcollapse1D_error(COORD_SYSTEM_ERROR);
#endif

  /* Compute Phi at the outer boundary */
  Phi_np1[J] = rhs_Phi;

#if( COORD_SYSTEM == SPHERICAL )

  /* RHS of Pi in Spherical coordinates */
  const real r_sqd_Jm2 = SQR(x[0][J-2]);
  const real r_sqd_Jm1 = SQR(x[0][J-1]);
  const real r_sqd_J   = SQR(x[0][J]);
  const real r_cbd_Jm2 = r_sqd_Jm2 * x[0][J-2];
  const real r_cbd_J   = r_sqd_J   * x[0][J];
  const real coeff     = 3.0 / ( r_cbd_J - r_cbd_Jm2 );
  const real term1     = 3.0 * r_sqd_J   * ( alpha_n[J]   / a_n[J]   ) * Phi_n[J];
  const real term2     = 4.0 * r_sqd_Jm1 * ( alpha_n[J-1] / a_n[J-1] ) * Phi_n[J-1];
  const real term3     =       r_sqd_Jm2 * ( alpha_n[J-2] / a_n[J-2] ) * Phi_n[J-2];
  const real rhs_Pi    = coeff * ( term1 - term2 + term3 );
  
#elif( COORD_SYSTEM == SINH_SPHERICAL )
  
  const real sinh_x0_inv_W_Jm2          = sinh( x[0][J-2] * inv_sinhW );
  const real sinh_x0_inv_W_Jm1          = sinh( x[0][J-1] * inv_sinhW );
  const real sinh_x0_inv_W_J            = sinh( x[0][J]   * inv_sinhW );
  const real cosh_x0_inv_W_J            = cosh( x[0][J]   * inv_sinhW );
  const real alpha_over_a_Jm2           = alpha_n[J-2]/a_n[J-2];
  const real alpha_over_a_Jm1           = alpha_n[J-1]/a_n[J-1];
  const real alpha_over_a_J             = alpha_n[J]  /a_n[J];
  const real alpha_Phi_shx02_over_a_Jm2 = SQR(sinh_x0_inv_W_Jm2) * alpha_over_a_Jm2 * Phi_n[J-2];
  const real alpha_Phi_shx02_over_a_Jm1 = SQR(sinh_x0_inv_W_Jm1) * alpha_over_a_Jm1 * Phi_n[J-1];
  const real alpha_Phi_shx02_over_a_J   = SQR(sinh_x0_inv_W_J  ) * alpha_over_a_J   * Phi_n[J];
  const real coeff_Pi                   = tmp1 * ( sinhW * sinh_inv_W / sinhA ) / ( SQR(sinh_x0_inv_W_J) * cosh_x0_inv_W_J ) ;
  const real rhs_Pi                     = coeff_Pi * ( 3.0 * alpha_Phi_shx02_over_a_J - 4.0 * alpha_Phi_shx02_over_a_Jm1 + alpha_Phi_shx02_over_a_Jm2 );
  
#else
  utilities::SFcollapse1D_error(COORD_SYSTEM_ERROR);
#endif

  /* Compute Pi at the outer boundary */
  Pi_np1[J] = Pi_nm1[J] + phi_Pi_coeff * rhs_Pi;
    
}

/* Function to solve the Hamiltonian constraint */
real evolution::pointwise_solution_of_the_Hamiltonian_constraint( const int j, grid::parameters grid, const realvec Phi, const realvec Pi, const realvec a ) {

  DECLARE_GRID_PARAMETERS;

  /* Set auxiliary variables */
  const real A         = log(a[j-1]);               // A^{n+1}_{j}
  const real avgPhi    = 0.5*( Phi[j] + Phi[j-1] ); // 0.5*( Phi^{n+1}_{j+1} + Phi^{n+1}_{j} )
  const real avgPi     = 0.5*( Pi[j]  + Pi[j-1]  ); // 0.5*(  Pi^{n+1}_{j+1} +  Pi^{n+1}_{j} )
  const real PhiSqr    = SQR(avgPhi);
  const real PiSqr     = SQR(avgPi);
  const real midx0     = 0.5 * ( x[0][j] + x[0][j-1] );


#if( COORD_SYSTEM == SPHERICAL )
  const real PhiPiTerm = 2.0 * EPSILON * M_PI * midx0 * ( PhiSqr + PiSqr );//lado direito da equação D.4 sem constante cosmologica
  //midx0 é o ponto intermediario
  //
  const real half_invr = 0.5 / midx0;//1/(2*midx0)
  const real cosmological_term = 1 - COSMOLOGICAL_CONSTANT *  SQR(midx0);

#elif( COORD_SYSTEM == SINH_SPHERICAL )
  const real PhiPiTerm = 2.0 * EPSILON * M_PI * (SQR(sinhA) * inv_sinhW) * sinh(midx0*inv_sinhW) * cosh(midx0*inv_sinhW) / SQR(sinh_inv_W) * ( PhiSqr + PiSqr );
  const real half_invr = 0.5/( sinhW * tanh(midx0*inv_sinhW) );
  const real r_sinh = A_over_sinh_inv_W * sinh(inv_sinhW * midx0);
  const real cosmological_term = 1 - COSMOLOGICAL_CONSTANT *  SQR(r_sinh);
#else
  utilities::SFcollapse1D_error(COORD_SYSTEM_ERROR);
#endif

  /* Set Newton's guess */
  real A_old = log(a[j-1]);

  /* Set variable for output */
  real A_new = A_old;

  /* Set a counter */
  real iter = 0;
  
  /* Perform Newton's method */
  do{

    /* Update A_old */
    A_old = A_new;
    
    /* Compute f and df */
    const real tmp0 = half_invr * exp(A_old+A)*cosmological_term;

    const real f  = inv_dx0 * (A_old - A) + tmp0 - half_invr - PhiPiTerm;//equacao D4 completa
    const real df = inv_dx0 + tmp0;//minha incognita do método não é r ou é? essa eq faz sentindo derivando-se em A
    //pouco importa R, não é objetivo do metodo encontra-lo, mas sim Aj+1, essa á a incognita.

    /* Update A_new */
    A_new = A_old - f/df;

    /* Increment the iterator */
    iter++;

  } while( ( fabs(A_new - A_old) > NEWTON_TOL ) && ( iter <= NEWTON_MAX_ITER ) );

  /* Check for convergence */
  if( iter > NEWTON_MAX_ITER ) cerr << "\n(Newton's method WARNING) Newton's method did not converge to a root! j = " << j << " | iter = " << iter << endl;

  /* Return the value of a */
  return( exp(A_new) );
  
}

/* Function to solve the polar slicing condition */
real evolution::pointwise_solution_of_the_polar_slicing_condition( const int j, grid::parameters grid, const realvec a, const realvec alpha ) {

  DECLARE_GRID_PARAMETERS;

  /* Step 3.b: Compute auxiliary quantities */
  const real b = a[j] + a[j-1];
  const real c = a[j] - a[j-1];

#if( COORD_SYSTEM == SPHERICAL )
  const real midway_r = 0.5 * ( x[0][j] + x[0][j-1] );
  const real cosmological_term = 1 - COSMOLOGICAL_CONSTANT * SQR (midway_r);

#elif( COORD_SYSTEM == SINH_SPHERICAL )
  const real midway_r = sinhW * tanh( inv_sinhW * 0.5 * (x[0][j] + x[0][j-1]) );
  const real r_sinh = A_over_sinh_inv_W * sinh(inv_sinhW * (0.5 * (x[0][j] + x[0][j-1])));
  const real cosmological_term = 1 - COSMOLOGICAL_CONSTANT *  SQR(r_sinh);
#else
  utilities::SFcollapse1D_error(COORD_SYSTEM_ERROR);
#endif

  const real d = ( 1.0 - 0.25 * SQR(b) * cosmological_term )/( 2.0 * midway_r ) - inv_dx0 * c / b;

  /* Step 3.c: Compute alpha */
  return( alpha[j-1]*( 1.0 - d*dx0 )/( 1.0 + d*dx0 ) );
  
}

/* Function to perform the rescaling of the lapse function */
void evolution::rescaling_of_the_lapse( grid::parameters grid, const realvec a, realvec &alpha ) {

  DECLARE_GRID_PARAMETERS;



  ofstream outpuFile;//create an instance of a file
  
  outpuFile.open("rescaling_values.dat",ios_base::app);
  outpuFile.precision(15);


  /* Set the initial value of kappa */
  real kappa = a[0]/alpha[0];
  real initial_kappa = a[0]/alpha[0];

  /* -------------------A TESTAR ------------------*/
  #if (INVERT_RESCALING == 1)

    #if(EPSILON != (-1))

      cout << "Invalid configuration. Rescaling can only be inverted for phantom fields  !! " <<endl;
      cout << "Please check the macros" << endl;
      exit(0);

    #else

      cout<<"  inverted rescaling  "<<endl;
      //normalize the rescaling inverselly
      /* Loop over the grid, updating kappa if needed */
      LOOP(1,Nx0Total) {
        real kappa_new = a[j]/alpha[j];
        if( kappa_new > kappa )
          kappa = kappa_new;
      }
    #endif

  #else
    /* Loop over the grid, updating kappa if needed */
    LOOP(1,Nx0Total) {
      real kappa_new = a[j]/alpha[j];
      if( kappa_new < kappa )
        kappa = kappa_new;
    }
  #endif


  // print to file after rescaling

  outpuFile << scientific << initial_kappa << " " << kappa << endl;
  outpuFile.close();


  /* Rescale the lapse */
  LOOP(0,Nx0Total) alpha[j] *= kappa;

  

}
