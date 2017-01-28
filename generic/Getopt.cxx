#include "Getopt.hxx"

namespace Getopt
{

  vector<string> get_options(int argc, char *argv[],
				     Options& options)
  {
    vector<string> remaining_args;
    list<char*> args;
    copy(argv+1, argv+argc, back_inserter(args));
    while (!args.empty()) {
      string arg(args.front());
      args.pop_front();
      bool is_param = false;
      for (list<Option*>::iterator it=options.opt_list.begin(); 
	   it!=options.opt_list.end(); it++) {
	Option* opt = *it;
	if ( (arg=="--" + opt->long_name) |
	     (arg=='-' + opt->short_name) ) {
          assert(!args.empty());
          if (opt->has_param()) {
            char* arg = args.front();
            opt->parse(arg);
            args.pop_front();
          }
          else {
            opt->parse("");
          }
          is_param = true;
          break;
	}
      }
      if (!is_param)
	remaining_args.push_back(arg);
    }
  
    return remaining_args;
  }

};

