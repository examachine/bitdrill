#ifndef Getopt_Interface
#define Getopt_Interface

#include "generic/General.hxx"

namespace Getopt {

  class Option
  {
  public:
    Option(string _long, string _short) 
      : long_name (_long), short_name(_short)  {
    }
    virtual ~Option() {
    }
    // get parameter of option
    inline virtual string get_parameter() {
      return string();
    };
    virtual bool has_param() = 0;
    virtual void parse(const char* arg) = 0;
    string long_name, short_name;
  };
  
  class BoolOption : public Option
  {
  public:
    BoolOption(string _long, string _short) 
      : Option(_long, _short) {
      val = false;
    }
    bool get_value() const {
      return val;
    }
    virtual bool has_param() { return true; }
    virtual void parse(const char* arg) {
      istringstream is(arg);
      is >> val; 
    }
  protected:
    bool val;
  };

  class SetBoolOption : public BoolOption
  {
  public:
    SetBoolOption(string _long, string _short) 
      : BoolOption(_long, _short) {}
    virtual bool has_param() { return false; }
    virtual void parse(char* arg) {
      val = true;
    }
  };

  class IntOption : public Option
  {
  public:
    IntOption(string _long, string _short, int def=0) 
      : Option(_long, _short) {
      val = def;
    }
    int get_value() const {
      return val;
    }
    virtual bool has_param() { return true; }
    virtual void parse(const char* arg) {
      istringstream is(arg);
      is >> val;
    }
  private:
    int val;
  };

  class StringOption : public Option
  {
  public:
    StringOption(string _long, string _short, const char* def = "") 
      : Option(_long, _short) {
      val = def;
    }
    const string& get_value() const {
      return val;
    }
    virtual bool has_param() { return true; }
    virtual void parse(const char* arg) {
      val = arg;
    }
  private:
    string val;
  };
  class Options
  {
  public:
    void add(Option* opt) {
      opt_list.push_back(opt);
    }
    list<Option*> opt_list;
  };

  vector<string> get_options(int argc, char *argv[], Options& options);
};


#endif
