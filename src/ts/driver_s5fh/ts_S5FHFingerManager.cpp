// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer
 * \date    2014-02-10
 *
 */
//----------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <driver_s5fh/S5FHFingerManager.h>

using namespace driver_s5fh;

BOOST_AUTO_TEST_SUITE(ts_S5FHFingerManager)

BOOST_AUTO_TEST_CASE(ConfigParameters)
{
  icl_core::logging::initialize();

  S5FHFingerManager finger_mananger;

}

BOOST_AUTO_TEST_SUITE_END()
