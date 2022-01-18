// this is for emacs file handling -*- mode: c++; indent-tabs-mode: nil -*-

// -- BEGIN LICENSE BLOCK ----------------------------------------------
// -- END LICENSE BLOCK ------------------------------------------------

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Klaus Fischnaller <fischnal@fzi.de>
 * \date    2014-10-30
 *
 */
//----------------------------------------------------------------------

#ifndef _WSB_CALLBACK_
#define _WSB_CALLBACK_

#include <string>


namespace schunk_svh_library {
namespace websocket {

/*!
 * \brief The wsb_callback class
 *
 * Classes inheriting this class can receive messages
 * sent to the \class ipc_server
 */
class WsbCallback
{
public:
  virtual ~WsbCallback() {}
  virtual void onWSBClientMessage(std::string msg) = 0;
};

} // namespace websocket
} // namespace schunk_svh_library

#endif
