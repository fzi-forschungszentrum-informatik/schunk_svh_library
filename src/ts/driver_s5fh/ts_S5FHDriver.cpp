// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Georg Heppner
 * \date    2014-02-03
 *
 */
//----------------------------------------------------------------------
#include <icl_comm/ByteOrderConversion.h>
#include <boost/test/unit_test.hpp>

#include <driver_s5fh/S5FHPositionSettings.h>
#include <driver_s5fh/S5FHCurrentSettings.h>

using icl_comm::ArrayBuilder;
using namespace driver_s5fh;

BOOST_AUTO_TEST_SUITE(ts_S5FHDriver)

S5FHPositionSettings test_pos_settings =  {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1};
S5FHCurrentSettings test_cur_settings =  {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.1};

BOOST_AUTO_TEST_CASE(ConvertSettingsTest)
{
  // See if the conversion of position settings worked
  ArrayBuilder<> ab(40);
  ArrayBuilder<> ab2(40);
  ab << test_pos_settings;
  ab2 << test_cur_settings;
  std::cout << "S5FH Driver array builder test:" << std::endl;
  std::cout << "  Position Settings raw data: " << ab << std::endl;
  std::cout << "------------------------------" << std::endl;
  std::cout << "  Current Settings raw data: " << ab2 << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
