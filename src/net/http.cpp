// Copyright (c) 2020-2024, The Prolo Project

//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "http.h"

#include "parse.h"
#include "socks_connect.h"

namespace net
{
namespace http
{

bool client::set_proxy(const std::string &address)
{
  if (address.empty())
  {
    set_connector(epee::net_utils::direct_connect{});
  }
  else
  {
    const auto endpoint = get_tcp_endpoint(address);
    if (!endpoint)
    {
      auto always_fail = net::socks::connector{boost::asio::ip::tcp::endpoint()};
      set_connector(always_fail);
    }
    else
    {
      set_connector(net::socks::connector{*endpoint});
    }
  }

  disconnect();

  return true;
}

std::unique_ptr<epee::net_utils::http::abstract_http_client> client_factory::create()
{
  return std::unique_ptr<epee::net_utils::http::abstract_http_client>(new client());
}

} // namespace http
} // namespace net
