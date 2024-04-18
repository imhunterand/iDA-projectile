/**
* IronDomeApp.hpp
* ---------------
* Main class for the Iron Dome project. Holds all data structures,
* the main state machine, and computations.
*/

#pragma once

#include <mutex>
#include <Eigen/Dense>
#include "redox.hpp"

#include <scl/DataTypes.hpp>
#include <scl/data_structs/SGcModel.hpp>
#include <scl/dynamics/scl/CDynamicsScl.hpp>
#include <scl/dynamics/tao/CDynamicsTao.hpp>
#include <scl/parser/sclparser/CParserScl.hpp>
#include <scl/graphics/chai/CGraphicsChai.hpp>
#include <scl/graphics/chai/ChaiGlutHandlers.hpp>
#include <scl/util/DatabaseUtils.hpp>

#include <GL/freeglut.h>

#include "projectile/projectile.hpp"

class IronDomeApp {

public:
  IronDomeApp();

  /**
  * Loop to continuously update controls.
  * Call from a separate thread.
  */
  void controlsLoop();

  /**
  * Loop to continuously update graphics.
  * Call from a separate thread.
  */
  void graphicsLoop();

  /**
  * Loop to continuously receive projectile position measurements
  * and update trajectory estimates of them.
  * Call from a separate thread.
  */
  void visionLoop();

  /**
  * Loop to continuously send desired robot joint positions
  * and get actual robot joint positions back
  * Call from a separate thread.
  */
  void robotLoop();

  /**
  * Loop to continuously get user input.
  * Call from a separate thread.
  */
  void shellLoop();

  /**
  * Command the robot to a desired state.
  */
  void setDesiredPosition(const Eigen::Vector3d& pos);
  void setDesiredPosition(double x, double y, double z);
  void setDesiredOrientation(const Eigen::Matrix3d& R);
  void setDesiredOrientation(const Eigen::Quaterniond& quat);
  void setDesiredOrientation(double x, double y, double z);
  void setDesiredJointPosition(const Eigen::VectorXd& q_new);

  /**
  * Relative movements of desired state.
  */
  void translate(double x, double y, double z);
  void rotate(double x, double y, double z);

  /**
  * Gains.
  */
  void setControlGains(double kp_p, double kv_p, double kp_r, double kv_r);
  void setJointFrictionDamping(double kv_friction);

  void printState();

  bool isPaused();

private:

  /**
  * Update the member variables to reflect the state of the robot.
  */
  void updateState();

  /**
  * Compute torque based on 6DOF task space PD control from the
  * position and velocity error vectors.
  */
  void fullTaskSpaceControl();

  /**
  * Same as fullTaskSpaceControl, but the position and orientation error
  * vectors dx and dphi are clamped at a max magnitude. This is analogous
  * to commanding the robot to move in small increments.
  */
  void incrementalTaskSpaceControl();

  /**
  * Calculate the position and orientation error vectors in task space, then
  * use the Jacobian transpose to convert them to joint space error vectors. Then,
  * apply PD control in joint space.
  */
  void resolvedMotionRateControl();

  /**
  * Simple PD control in joint space.
  */
  void jointSpaceControl();

  void applyTorqueLimits();
  void applyJointFriction();
  void applyGravityCompensation();

  /**
  * State machine that sets the desired position and orientation.
  */
  void stateMachine();

  /**
  * Command task-space position and orientation to the physical robot.
  */
  void sendToRobot();

  void commandTorque(Eigen::VectorXd torque);

  void integrate();

  bool testJointLimit(int joint_num);

  /**
  * Apply torques in task space to keep the joints away from their
  * limits.
  */
  void applyJointLimitPotential();

  redox::Redox rdx;                // Communication via Redis
  redox::Redox rdx_robot;          // Communication via Redis
  redox::Redox rdx_vision;         // Communication via Redis

  scl::SRobotParsed rds;     // Robot data structure
  scl::SGraphicsParsed rgr;  // Robot graphics data structure
  scl::SGcModel rgcm;        // Robot data structure with dynamic quantities
  scl::SRobotIO rio;         // I/O data structure
  scl::CDynamicsScl dyn_scl; // Robot kinematics and dynamics computation object
  scl::CDynamicsTao dyn_tao; // Robot physics integrator
  scl::CParserScl parser;    // Parser from file

  scl::CGraphicsChai rchai;  // Chai interface for rendering graphics
  scl::SGraphicsChai* graphics;
  chai3d::cWorld* chai_world;

  std::mutex data_lock; // Mutex that assures thread safety to data resources

  double t; // Run-time of program
  double t_sim; // Simulated time
  double dt_real, dt_sim; // Actual and simulated time between frames

  long iter; // Number of frames
  bool finished; // Flag to shut down

  scl::SRigidBodyDyn* ee; // End effector link
  const Eigen::Vector3d op_pos; // Position of operational poin w.r.t. end-effector

  int dof; // Degrees of freedom of our robot

  double kp_p, kv_p, kp_r, kv_r; // Control gains

  Eigen::MatrixXd J; // Jacobian
  Eigen::VectorXd q, dq, ddq; // Generalized position/velocity/acceleration

  Eigen::Vector3d x_c, x_d, dx; // Position, current/desired/difference
  Eigen::Vector3d v; // Linear velocity

  Eigen::Matrix3d R_c, R_d; // End-effector orientations, current/desired
  Eigen::Vector3d dphi; // Difference in end-effector orientations
  Eigen::Vector3d omega; // Angular velocity

  Eigen::Vector3d F_p, F_r;          // Task space forces, pos/rot
  Eigen::Vector6d F;
  Eigen::MatrixXd lambda, lambda_inv; // Generalized mass matrix and inverse
  Eigen::VectorXd tau_jlim; // Restoring torque for joint limit avoidance
  Eigen::VectorXd q_sat; // Joint limit saturation
  Eigen::VectorXd q_d, q_diff; // Desired position in joint-space control mode

  Eigen::VectorXd g_q; // Generalized gravity force
  Eigen::VectorXd tau; // Commanded generalized force

  Eigen::VectorXd kp_q, kv_q; // Gains in joint space control

  Eigen::VectorXd q_sensor; // Joint position read from actual robot

  Eigen::Vector3d x_inc; // Incremental position towards goal

  Eigen::VectorXd ready_pos_joint; // Ready position, in joint space

  // Class for managing the current state of projectiles
  ProjectileManager projectile_manager;

  // State of the robot
  int state;

  // Projectile we are currently chasing
  Projectile* target;

  // Whether the projectile interception is paused
  bool paused;

  // Whether we are simulating or controlling the real robot
  bool simulation;

  // Whether we are controlling in joint space or task space
  bool joint_space;
};
