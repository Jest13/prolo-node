// Copyright (c) 2019-2024, The Prolo Project
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

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <boost/program_options.hpp>
#include "misc_log_ex.h"
#include "common/util.h"
#include "common/command_line.h"
#include "common/dns_utils.h"
#include "version.h"

#undef PROLO_DEFAULT_LOG_CATEGORY
#define PROLO_DEFAULT_LOG_CATEGORY "debugtools.dnschecks"

namespace po = boost::program_options;

enum lookup_t { LOOKUP_A, LOOKUP_TXT };

static std::vector<std::string> lookup(lookup_t type, const char *hostname)
{
  bool dnssec_available = false, dnssec_valid = false;
  std::vector<std::string> res;
  switch (type)
  {
    case LOOKUP_A: res = tools::DNSResolver::instance().get_ipv4(hostname, dnssec_available, dnssec_valid); break;
    case LOOKUP_TXT: res = tools::DNSResolver::instance().get_txt_record(hostname, dnssec_available, dnssec_valid); break;
    default: MERROR("Invalid lookup type: " << (int)type); return {};
  }
  if (!dnssec_available)
  {
    MWARNING("No DNSSEC for " << hostname);
    return {};
  }
  if (!dnssec_valid)
  {
    MWARNING("Invalid DNSSEC check for " << hostname);
    return {};
  }
  MINFO(res.size() << " valid signed result(s) for " << hostname);
  return res;
}

static void lookup(lookup_t type, const std::vector<std::string> hostnames)
{
  std::vector<std::vector<std::string>> results;
  for (const std::string &hostname: hostnames)
  {
    auto res = lookup(type, hostname.c_str());
    if (!res.empty())
    {
      std::sort(res.begin(), res.end());
      results.push_back(res);
    }
  }
  std::map<std::vector<std::string>, size_t> counter;
  for (const auto &e: results)
    counter[e]++;
  size_t count = 0;
  for (const auto &e: counter)
    count = std::max(count, e.second);
  if (results.size() > 1)
  {
    if (count < results.size())
      MERROR("Only " << count << "/" << results.size() << " records match");
    else
      MINFO(count << "/" << results.size() << " records match");
  }
}

int main(int argc, char* argv[])
{
  TRY_ENTRY();

  tools::on_startup();

  po::options_description desc_cmd_only("Command line options");
  po::options_description desc_cmd_sett("Command line options and settings options");

  command_line::add_arg(desc_cmd_only, command_line::arg_help);

  po::options_description desc_options("Allowed options");
  desc_options.add(desc_cmd_only).add(desc_cmd_sett);

  po::variables_map vm;
  bool r = command_line::handle_error_helper(desc_options, [&]()
  {
    po::store(po::parse_command_line(argc, argv, desc_options), vm);
    po::notify(vm);
    return true;
  });
  if (! r)
    return 1;

  if (command_line::get_arg(vm, command_line::arg_help))
  {
    std::cout << "Prolo '" << PROLO_RELEASE_NAME << "' (v" << PROLO_VERSION_FULL << ")" << ENDL << ENDL;
    std::cout << desc_options << std::endl;
    return 1;
  }

  mlog_configure("", true);
  mlog_set_categories("+" PROLO_DEFAULT_LOG_CATEGORY ":INFO");

  lookup(LOOKUP_A, {"seeds.proloseeds.se", "seeds.proloseeds.ae.org", "seeds.proloseeds.ch", "seeds.proloseeds.li"});

  lookup(LOOKUP_TXT, {"updates.prolopulse.org", "updates.prolopulse.net", "updates.prolopulse.co", "updates.prolopulse.se", "updates.prolopulse.fr", "updates.prolopulse.de", "updates.prolopulse.no", "updates.prolopulse.ch"});

  lookup(LOOKUP_TXT, {"checkpoints.prolopulse.org", "checkpoints.prolopulse.net", "checkpoints.prolopulse.co", "checkpoints.prolopulse.se"});

  // those are in the code, but don't seem to actually exist
#if 0
  lookup(LOOKUP_TXT, {"testpoints.prolopulse.org", "testpoints.prolopulse.net", "testpoints.prolopulse.co", "testpoints.prolopulse.se");

  lookup(LOOKUP_TXT, {"stagenetpoints.prolopulse.org", "stagenetpoints.prolopulse.net", "stagenetpoints.prolopulse.co", "stagenetpoints.prolopulse.se"});
#endif

  lookup(LOOKUP_TXT, {"segheights.prolopulse.org", "segheights.prolopulse.net", "segheights.prolopulse.co", "segheights.prolopulse.se"});

  return 0;
  CATCH_ENTRY_L0("main", 1);
}
