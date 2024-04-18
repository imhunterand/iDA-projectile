/**
*
*/

#pragma once

#include <random>
#include <map>

#include <Eigen/Dense>

class SimProjectile {

public:

  SimProjectile() {};
  SimProjectile(int id, double t0,
      const Eigen::Vector3d& p0, const Eigen::Vector3d& v0, const Eigen::Vector3d& a0);

  /**
  * Is this projectile expired?
  */
  bool isExpired();

  // ID number
  int id;

  // Launch time
  double t0;

  // Initial state
  Eigen::Vector3d p0;
  Eigen::Vector3d v0;
  Eigen::Vector3d a0;

  // Current measured position
  Eigen::Vector3d p;
};

/**
* Generates projectiles at randomized times, with randomized initial
* positions and velocities. Throwing times are based on a Poisson
* distribution, and the initial position/velocity is normally
* distributed around a mean value.
*
* This class is for simulation *only*. When observing physical projectiles,
* it is not used.
*/
class ProjectileGenerator {

public:

  /**
  * Create a generator that on average launches one projectile every
  * t_avg seconds, with a velocity magnitude v_avg and angle from
  * horizontal theta_avg. Projectiles are launched towards the robot
  * from a randomized point in the +X direction.
  */
  ProjectileGenerator(double t_avg, double v_avg, double theta_avg);

  /**
  * Return a Projectile with launch time and initial position/velocity.
  */
  SimProjectile getNextProjectile();

  /**
  * Generate a new position observation for the given projectile, based
  * on its initial conditions and the current time. Add gaussian noise
  * to simulate measurement error.
  */
  Eigen::Vector3d observePosition(const SimProjectile& proj);

  void update();

  /**
  * Return a map of the currently seen projectiles.
  */
  const std::map<int, SimProjectile>& getProjectiles();

private:

  // Number of projectiles generated
  int count;

  // Statistical parameters
  double t_avg;
  double v_avg;
  double theta_avg;

  // Statistical generators
  std::default_random_engine generator;
  std::exponential_distribution<double> exp_dist;
  std::normal_distribution<double> normal_dist;

  // List of active projectiles
  std::map<int, SimProjectile> projectiles;

  // Next projectile to be generated
  SimProjectile pendingProjectile;
  bool pendingProjectileExists;
};
