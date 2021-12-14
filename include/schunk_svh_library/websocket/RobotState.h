// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Georg Heppner <heppner@fzi.de>
 * \date    2014-5-20
 *
 */
//----------------------------------------------------------------------
#ifndef ROBOTSTATE_H
#define ROBOTSTATE_H

#include <vector>
#include <map>
#include <stdlib.h>
#include <string>
#include <set>
#include <schunk_svh_library/ImportExport.h>

namespace schunk_svh_library {
namespace websocket {

/*!
 * \brief The RobotState class is a helper class to hold the relevant information about the current robot state
 * A robo has many axes, a movement state, fault states for every joint. In Short this class was meant to be used for the
 * SCHUNK LWA4-P Together with canopen. However it might also be used with other arms
 */
class DRIVER_SVH_IMPORT_EXPORT RobotState
{
public:




  /*!
   * \brief The ErrorState enum describes the possible error states a joint may indicate
   */
  enum ErrorState
  {
    eERR_NO_ERROR = 0,
    eERR_WARNING = 1,
    eERR_FAULT = 2,
    eERR_DIMENSION
  };

  struct RobotValues
  {
    double position;
    double velocity;
    ErrorState error;
    ErrorState homed;
    ErrorState enabled;
  };


  typedef std::map<size_t, RobotValues> jnt_value_map;

  /*!
   * \brief RobotState Object to hold the current state of a robotic arm (i.e LWA4P)
   * \param num_axes Number of axes to send out
   * \param ticks_per_second Number of simulation stepds per second if the simulation should be used. Default = 100. Not needed if you use real data
   */
  RobotState();

  /*!
   * \brief simulateTick Simulates one step of a dummy simulation to generate random output values
   */
  virtual void simulateTick();

  //! Shuffles the Error values to contain random values from the pool of possible errors (used for simulation)
  virtual void shuffleErrors();

  /*!
   * \brief getStateJSON Returns the current robot state as JSON encoded String
   * \return current robot state as JSON encoded String
   */
  virtual std::string getStateJSON() = 0;

  /*!
   * \brief getHintsJSON Returns the current set of hints that are active as a JSON encoded String
   * \return current set of hints as a JSON encoded String
   */
  std::string getHintsJSON();

  /*!
   * \brief setHint sets a hint to be active. It will be sent with the next robot status send
   * \param hint number representing the type of Hint
   */
  void setHint(const int &hint);

  /*!
   * \brief clearHint removes a hint from the set of active hints (if present)
   * \param hint number representing the type of hint
   */
  void clearHint(const int &hint);

  /*!
   * \brief clearAllHints removes all hints, i.e. removes all errors that
   */
  void clearAllHints();

  /*!
   * \brief getActiveHints returns the set of currently active hints
   * \return set of currently active hints
   */
  std::set<int> getActiveHints();

  /*!
   * \brief setJointPositions Sets the current joint positionts. Only num_axes elements will be used. Unset element will retain their previous value
   * \param joint_positions new joint positions in your format, this will be multiplied by the toRadMultiplier
   */
  void setJointPositions(const std::vector<double> &joint_positions);

  /*!
   * \brief setJointPosition sets the current koint position of a particular joint.
   * \param joint_position Current Position in your format, this will be multiplied by the toRadMultiplier
   * \param joint index of the joint (starts with zero, obviously)
   */
  void setJointPosition(const double &joint_position,size_t joint);

  /*!
   * \brief setJointPositions Sets the current joint positionts. Only num_axes elements will be used. Unset element will retain their previous value
   * \param joint_positions new joint positions in your format, this will be multiplied by the toRadMultiplier
   */
  void setJointPositions(const std::vector<int> &joint_positions);

  /*!
   * \brief setJointPosition sets the current koint position of a particular joint.
   * \param joint_position Current Position in your format, this will be multiplied by the toRadMultiplier
   * \param joint index of the joint (starts with zero, obviously)
   */
  void setJointPosition(const int &joint_position,size_t joint);

  /*!
   * \brief setJointVelocties Sets the current joint velocities. Only num_axes elements will be used. Unset element will retain their previous value
   * \param joint_velocities new joint velocities in your format, this will be multiplied by the toRadMultiplier
   */
  void setJointVelocites(const std::vector<double> &joint_velocities);

  /*!
   * \brief setJointVelocty sets the current joint velocity of a single joint
   * \param joint_velocit< new joint velocities inyour format, this will be multiplied by the toRadMultiplier
   */
  void setJointVelocity(const double &joint_velocity,const size_t &joint);

  /*!
   * \brief setJointErrors Sets the current joint error state. Only num_axes elements will be used. Unset element will retain their previous value
   * \param joint_errors new joint error state
   */
  void setJointErrors(const std::vector<ErrorState> &joint_errors);

  /*!
   * \brief setJointError sets the current joint error state of a particular joint
   * \param joint_error new error state to set
   * \param joint the joint to set the error for
   */
  void setJointError(const ErrorState &joint_error,const size_t &joint);

  /*!
   * \brief setJointErrors Sets the current joint errors in a simplified way (only fault or no fault) if no further information is available
   * \param faults true if joint had a fault, false otherwise
   */
  void setJointErrors(const std::vector<bool> &faults);

  /*!
   * \brief setJointError Simplified version of error setting for a particular joint if only fault or no fault is supported
   * \param fault true if joint had a fault, false otherwise
   * \param joint the joint to set the error for
   */
  void setJointError(const bool &fault, const size_t &joint);


  void setJointsEnabled(const std::vector<bool> &joints_enabled);

  void setJointEnabled(const bool &enabled,const size_t &joint);

  void setJointsHomed(const std::vector<bool> &joints_homed);

  void setJointHomed(const bool &homed,const size_t &joint);



  virtual void setMovementState(const int &movement_state) = 0;


  //virtual void setState(const std::vector<double> &joint_positions,const std::vector<double> &joint_velocities,const std::vector<bool> &faults,const MovementState &movement_state);


  void setTps(unsigned int tps)
  {
    m_tps = tps;
  }

  size_t getNumAxes() const;

  double getInputToRadFactor() const
  {
    return m_input_to_rad_factor;
  }

  void setInputToRadFactor(double input_to_rad_factor)
  {
    m_input_to_rad_factor = input_to_rad_factor;
  }

protected:
  //! Map of all joints with their status values
  jnt_value_map m_joints;

//  //! Position of the joints(rad)
//  jnt_value_map m_joint_positions;

//  //! Speed of the joints (rad/sec)
//  jnt_value_map m_joint_velocities;

//  //! Error indication for individual joint
//  std::map<size_t, ErrorState> m_joint_errors;

//  //! Indication if a joint is homed
//  jnt_error_map m_joints_homed;

//  //! Indication if a joint is enabled
//  jnt_error_map m_joints_enabled;

  //! Ticks per Second can be used to simulate data in the robot
  unsigned int  m_tps;
  //! Current tick counter for simulation purposes
  unsigned int  m_ticks;

  //! in case the input positions are not Rad we use this factor
  double m_input_to_rad_factor;

  //! Hints are identifiers for a diagnostics gui indicating what is wrong with a certain setup, these hints need to be represented in the config of the delivered diagnostics page
  std::set<int> m_hints;

};


}}

#endif // ROBOTSTATE_H
